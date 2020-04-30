#include <xinu.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


#ifdef FS
#include <fs.h>

static struct fsystem fsd;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

extern int dev0;

char block_cache[512];

#define SB_BLK 0
#define BM_BLK 1
#define RT_BLK 2

#define NUM_FD 16
struct filetable oft[NUM_FD]; // open file table
int next_open_fd = 0;


#define INODES_PER_BLOCK (fsd.blocksz / sizeof(struct inode))
#define NUM_INODE_BLOCKS (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock);

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock) {
  int diskblock;

  if (fileblock >= INODEBLOCKS - 2) {
    printf("No indirect block support\n");
    return SYSERR;
  }

  diskblock = oft[fd].in.blocks[fileblock]; //get the logical block address

  return diskblock;
}

/* read in an inode and fill in the pointer */
int fs_get_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;
  int inode_off;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_get_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  inode_off = inn * sizeof(struct inode);

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  printf("inn*sizeof(struct inode): %d\n", inode_off);
  */

  bs_bread(dev0, bl, 0, &block_cache[0], fsd.blocksz);
  memcpy(in, &block_cache[inode_off], sizeof(struct inode));

  return OK;

}

/* write inode indicated by pointer to device */
int fs_put_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_put_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  */

  bs_bread(dev0, bl, 0, block_cache, fsd.blocksz);
  memcpy(&block_cache[(inn*sizeof(struct inode))], in, sizeof(struct inode));
  bs_bwrite(dev0, bl, 0, block_cache, fsd.blocksz);

  return OK;
}
     
/* create file system on device; write file system block and block bitmask to
 * device */
int fs_mkfs(int dev, int num_inodes) {
  int i;
  
  if (dev == 0) {
    fsd.nblocks = dev0_numblocks;
    fsd.blocksz = dev0_blocksize;
  }
  else {
    printf("Unsupported device\n");
    return SYSERR;
  }

  if (num_inodes < 1) {
    fsd.ninodes = DEFAULT_NUM_INODES;
  }
  else {
    fsd.ninodes = num_inodes;
  }

  i = fsd.nblocks;
  while ( (i % 8) != 0) {i++;}
  fsd.freemaskbytes = i / 8; 
  
  if ((fsd.freemask = getmem(fsd.freemaskbytes)) == (void *)SYSERR) {
    printf("fs_mkfs memget failed.\n");
    return SYSERR;
  }
  
  /* zero the free mask */
  for(i=0;i<fsd.freemaskbytes;i++) {
    fsd.freemask[i] = '\0';
  }
  
  fsd.inodes_used = 0;
  
  /* write the fsystem block to SB_BLK, mark block used */
  fs_setmaskbit(SB_BLK);
  bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(struct fsystem));
  
  /* write the free block bitmask in BM_BLK, mark block used */
  fs_setmaskbit(BM_BLK);
  bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

  return 1;
}

/* print information related to inodes*/
void fs_print_fsd(void) {

  printf("fsd.ninodes: %d\n", fsd.ninodes);
  printf("sizeof(struct inode): %d\n", sizeof(struct inode));
  printf("INODES_PER_BLOCK: %d\n", INODES_PER_BLOCK);
  printf("NUM_INODE_BLOCKS: %d\n", NUM_INODE_BLOCKS);
}

/* specify the block number to be set in the mask */
int fs_setmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  fsd.freemask[mbyte] |= (0x80 >> mbit);
  return OK;
}

/* specify the block number to be read in the mask */
int fs_getmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  return( ( (fsd.freemask[mbyte] << mbit) & 0x80 ) >> 7);
  return OK;

}

/* specify the block number to be unset in the mask */
int fs_clearmaskbit(int b) {
  int mbyte, mbit, invb;
  mbyte = b / 8;
  mbit = b % 8;

  invb = ~(0x80 >> mbit);
  invb &= 0xFF;

  fsd.freemask[mbyte] &= invb;
  return OK;
}

/* This is maybe a little overcomplicated since the lowest-numbered
   block is indicated in the high-order bit.  Shift the byte by j
   positions to make the match in bit7 (the 8th bit) and then shift
   that value 7 times to the low-order bit to print.  Yes, it could be
   the other way...  */
void fs_printfreemask(void) { // print block bitmask
  int i,j;

  for (i=0; i < fsd.freemaskbytes; i++) {
    for (j=0; j < 8; j++) {
      printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
    }
    if ( (i % 8) == 7) {
      printf("\n");
    }
  }
  printf("\n");
}


int fs_open(char *filename, int flags) {
	struct inode node;
	// check for flags and check if it is already open
	for(int i = 0; i < fsd.root_dir.numentries; i++){
		if(strcmp(filename, fsd.root_dir.entry[i].name) == 0){
			if(flags == O_RDONLY || flags == O_WRONLY || flags == O_RDWR){
				if(fs_get_inode_by_num(dev0, i, &node) == SYSERR){
					kprintf("Error while getting inode by num\n");
					return SYSERR;
				}
				if(oft[i].state == FSTATE_OPEN){
					kprintf("this file is already open\n");
					return SYSERR;
				}
				oft[i].state = FSTATE_OPEN;
				oft[i].fileptr = 0;
				oft[i].de = &fsd.root_dir.entry[i];
				oft[i].in = node;
				oft[i].flag = flags;
				return i;
			}
			else{
				kprintf("Invalid Flag\n");
				return SYSERR;
			}
		}
	}
	kprintf("file doedn't exist\n");
	return SYSERR;
}

int fs_close(int fd) {
  	if(fd > -1 && fd < NUM_FD){
		if(oft[fd].state== FSTATE_OPEN){
			oft[fd].state = FSTATE_CLOSED;
			oft[fd].fileptr = 0;
			return OK;
		}
		else{
			kprintf("File not open\n");
			return SYSERR;
		}
	}
	printf("Invalid fd\n");
	return SYSERR;
}

int fs_create(char *filename, int mode) {
	if(mode == O_CREAT){
		if(strlen(filename) > FILENAMELEN || strlen(filename) == 0){
			printf("Incorrect file name\n");
			return SYSERR;
		}
		
		//Check if the filename already does not exists
		for(int i = 0; i < fsd.root_dir.numentries; i++){
		       if(strcmp(filename, fsd.root_dir.entry[i].name) == 0){
				kprintf("This file already exists\n");
				return SYSERR;
		       }
		}
	       struct inode node;
	       if(fs_get_inode_by_num(dev0, fsd.inodes_used, &node)== SYSERR){
		       return SYSERR;
	       }
	       node.id = fsd.inodes_used;
	       node.type = INODE_TYPE_FILE;
	       node.nlink = 1;
	       node.device = dev0;
	       node.size = 0;
		if(fs_put_inode_by_num(dev0, fsd.inodes_used, &node) == SYSERR){
			return SYSERR;
		}
	       fsd.inodes_used = fsd.inodes_used + 1;
	       fsd.root_dir.entry[fsd.root_dir.numentries].inode_num = fsd.inodes_used;
	       strcpy(fsd.root_dir.entry[fsd.root_dir.numentries].name, filename);
	       fsd.root_dir.numentries = fsd.root_dir.numentries + 1;
	       return fs_open(filename, O_RDWR);
		}

	kprintf("Wrong mode\n");
	return SYSERR;
}

int fs_seek(int fd, int offset) {
	if(oft[fd].state == FSTATE_OPEN && fd > -1 && fd < NUM_FD){
		oft[fd].fileptr = oft[fd].fileptr + offset;
		return OK;
	}
	kprintf("File NOT OPEN\n");
	return SYSERR;
}

int fs_read(int fd, void *buf, int nbytes) {
	if( fd < NUM_FD || fd > -1){
		if(oft[fd].state != FSTATE_OPEN){
			kprintf("File not open\n");
			return SYSERR;
		}
		if(oft[fd].flag == O_RDONLY || oft[fd].flag == O_RDWR){
			int start = oft[fd].fileptr;
			int end = oft[fd].fileptr + nbytes;
			//if the actual file size is less than the 
			if (end > oft[fd].in.size){
				nbytes = oft[fd].in.size - oft[fd].fileptr;
				end = oft[fd].in.size;
			}
			int startblock = start / fsd.blocksz;
			int endblock = end / fsd.blocksz;
			
			if(end % fsd.block != 0){
				endblock = endblock + 1;
			}
			
  			int offset = start % fsd.blocksz;
 			int size = fsd.blocksz - offset;
  			char *buffer = (char *)buf;		
			for (i = startblock; i < endblock; i++) {
				if(bs_bread(dev0, oft[fd].in.blocks[i], offset, buffer, size) == SYSERR){
					kprintf("error in reading file\n");
					return SYSERR;
				}
				offset = 0;
				size = fsd.blocksz - offset;				
				buffer = buffer + size;				
			}
			oft[fd].fileptr = end;
			return nbytes;
		}
	}
	return SYSERR;
}

int fs_write(int fd, void *buf, int nbytes) {
	if (oft[fd].state != FSTATE_OPEN) {
    		kprintf("File not open\n");
    		return SYSERR;
	}
	if(oft[fd].flag == O_WRONLY || oft[fd].flag == O_RDWR){
		int start = oft[fd].fileptr;
		int end = oft[fd].fileptr + nbytes;
		int startblock = start / fsd.blocksz;
		int endblock = end / fsd.blocksz;
		if(end % fsd.block != 0){
			endblock = endblock + 1;
		}
		int currentblock = oft[fd].in.size / fsd.blocksz;
		
		int j = FIRST_INODE_BLOCK + NUM_INODE_BLOCKS;
		for(int i = currentblock; i < endblock; i++){
			while(fs_getmaskbit(j) != 0){
				j++;
			}
			fs_setmaskbit(j);
			oft[fd].in.blocks[i] = j;
		}
		//if the actual file size is less than the 
		if (end > oft[fd].in.size){
			oft[fd].in.size = end;
		}
		int offset = start % fsd.blocksz;
		int size = fsd.blocksz - offset;
		char *buffer = (char *)buf;		
		for (int i = startblock; i < endblock; i++) {
			if(bs_bwrite(dev0, oft[fd].in.blocks[i], offset, buffer, size) == SYSERR){
				kprintf("error in reading file\n");
				return SYSERR;
			}
			offset = 0;
			size = fsd.blocksz - offset;				
			buffer = buffer + size;				
		}
		oft[fd].fileptr = end;
		return nbytes;
		
	}
	return SYSERR;
}

int fs_link(char *src_filename, char* dst_filename) {
	
	return SYSERR;
}

int fs_unlink(char *filename) {
	return SYSERR;
}
#endif /* FS */
