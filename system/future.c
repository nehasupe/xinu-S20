#include <xinu.h>
#include <future.h>


/* future_get, future_set, future_free
 * go here
 * Does future_alloc goes somewhere too? */


//future_alloc
//Parameters:
//future_mode_t mode - The mode this future will operate in
//uint size - The size of the data element in bytes
//uint nelems - The number of elements
//Returns: future_t - SYSERR or a pointer to a valid future


//function: Allocates a new future (in the FUTURE_EMPTY state) with the given mode. We will use the getmem() call to allocate size amount of space for the new future's data. 
future_t* future_alloc(future_mode_t mode, uint size, uint nelems){

	//In C, z=(C*)malloc(sizeof(C));
	future_t* newfut = (future_t*) getmem(sizeof(future_t));
	newfut -> state = FUTURE_EMPTY;
	newfut -> size = size;
	newfut -> mode = mode;
	//In C, z->t=(int*)malloc(sizeof(int));
	newfut -> data = (char*) getmem(size);
	newfut -> pid = NULL;
	newfut -> set_queue = NULL;
	newfut -> get_queue = NULL;
	//printf("Neeeeeeeeeeeeew queueueueu");
	//printf("In alloc pid:%d\n", getpid());
	//char *proc_name = proctab[getpid()].prname;
	//printf("alloc name : %s\n", proc_name);
	
	//
	//Should return erroe in case of erroe
	//Returns: future_t - SYSERR or a pointer to a valid future
	return newfut;
}

//future_free
//
//Parameters:
//future_t* f - future to free
//Returns: syscall - SYSERR or OK

//functioin: Frees the allocated future. Use the freemem() system call to free the data space. 
syscall future_free(future_t* f){
	
	intmask mask;
	mask = disable();
	
	if( freemem(f -> data,f -> size )== SYSERR){
		restore(mask);
		return SYSERR;
	}
	//see if this fails and return SYSERR	
	restore(mask);
	return OK; 
	
}

//future_get
//
//Parameters:
//future_t* f - Future on which to wait for and obtain value
//char* out - Address into which the future's value will be copied.
//Returns: syscall - SYSERR or OK
//

//FUNCTION: Get the value of a future set by an operation and may change the state of future. 
syscall future_get(future_t* f, char* out){
	
	intmask mask;
        mask = disable();
	
	//Check if the future passed exists otherwise error
	if(f == (future_t*)NULL){
		restore(mask);
		return SYSERR;
	}
	if(f -> state == FUTURE_EMPTY){
		if(f -> mode == FUTURE_SHARED){
			//f -> get_queue = initialize();
			//fqenqueue(&f -> get_queue, getpid());
			if(f -> get_queue != NULL){
				restore(mask);
				return SYSERR;
			}
			fnode_t* node = (fnode_t *) getmem(sizeof(fnode_t));
			node -> pid = getpid();
			node -> next = NULL;
			f -> get_queue = node;
			f -> state = FUTURE_WAITING;
			//printf("suspended empty %d",getpid()); 
			suspend(getpid());
		}
		else if(f -> mode == FUTURE_EXCLUSIVE){
		       	f -> state = FUTURE_WAITING;
		
			//printf("State changed to waiting\n");
			f -> pid = getpid();
			//char *proc_name = proctab[getpid()].prname;
			//printf("future_get process name that is suspended: %s\n", proc_name);
		 	//printf("the thread suspended pid %d:\n", getpid());
			//fnode_t *node = (fnode_t *) getmem(sizeof(fnode_t));
			//fnode_t *ptr;
			//node -> pid = getpid();



			//traverse 
			suspend(f -> pid);
			//printf("the thread suspended pid %d:\n", getpid());
			//enqueue(f -> pid, f -> get_queue);
		}
	}
	else if(f -> state == FUTURE_WAITING){
		if(f -> mode == FUTURE_EXCLUSIVE){
			restore(mask);
			return SYSERR;
		}
		else if(f -> mode == FUTURE_SHARED){
			//fqenqueue(&f -> get_queue, getpid());
			//printf("waiting %d", getpid());
			
			
			fnode_t *node = (fnode_t *) getmem(sizeof(fnode_t));
			fnode_t *ptr;
			node -> pid = getpid();
			node -> next = NULL;
			ptr = f -> get_queue;
			while(ptr -> next != NULL)
				ptr = ptr -> next;
			ptr -> next = node;
			suspend(getpid());
		}
	}
	if(f -> state == FUTURE_READY){
		if(f -> mode == FUTURE_EXCLUSIVE){
			memcpy(out, f -> data, f -> size);
			f -> state = FUTURE_EMPTY;
			//resume(f -> pid);
		}
		else if(f -> mode == FUTURE_SHARED){
			memcpy(out, f -> data, f -> size);
			//SINCE THEY ARE IMMUTABLE DO THEY GO INTO WAITING STATE OR EMPYTY OR STAY IN READY?
			//f -> state = FUTURE_EMPTY;
			//if(!(f -> get_queue).isfqempty()){
			//	pid32 pid = fqdequeue(f -> get_queue);
			//	resume
			
		}

	}
	
	//else if(f -> state == FUTURE_WAITING){
		//add to the quue 
		//f -> pid = getpid();
		//suspend(f -> pid);
		//printf("suspend\n");
		//enqueue(f -> pid, f -> get_queue);
	//}
	//if( f -> state == FUTURE_READY){
	  //     if(f -> mode == FUTURE_EXCLUSIVE){
	//	printf("after producer is ready now going to resume the consumer\n");
		//resume(f -> pid);
	//	memcpy(out, f->data, f -> size);
	//	f -> state = FUTURE_EMPTY;
		//char *proc_name = proctab[getpid()].prname;
		//printf("future_get process name that is going to be resumed: %s\n", proc_name);
		//suspend(f -> pid);

          //      }
		//printf("resumed process pid %d\n", f -> pid);
	//}
	// future shared

	restore(mask);
	return OK;
}
//kiii
//future_set
//
//Parameters:
//future_t* f - Future in which to set the value
//char* in - Result of an operation to be set as data in the future
//Returns: syscall - SYSERR or OK
//
//function: //Set a value in a future and may change the state of future.
syscall future_set(future_t* f, char* in){
	
	intmask mask;
	mask = disable();
	
	if(f == (future_t*)NULL || f -> state == FUTURE_READY){
		restore(mask);
		return SYSERR;
	}
	if(f -> state == FUTURE_EMPTY || f -> state == FUTURE_WAITING){
		memcpy(f -> data, in, f -> size);
		if(f -> state == FUTURE_WAITING){
			f -> state = FUTURE_READY;
			if(f -> mode == FUTURE_EXCLUSIVE){
				resume(f -> pid);
			}
			else if(f -> mode == FUTURE_SHARED){
			fnode_t *ptr;
			if(f -> get_queue!= NULL){
				ptr = f->get_queue;
				while(ptr != NULL){
					//printf("deletding node %d",ptr ->pid);
					fnode_t *tmp;
					tmp = f -> get_queue;
					f-> get_queue = f->get_queue -> next;
					resume(tmp -> pid);
					freemem(tmp, sizeof(fnode_t*));
					ptr = f -> get_queue;
				}
			}
				
			
			}
		}
		else if(f-> state == FUTURE_EMPTY){
			f -> state = FUTURE_READY;
		}
	}
	restore(mask);
	return (OK);
}

