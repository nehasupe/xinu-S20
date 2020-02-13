#include <xinu.h>
#include <stdio.h>
#include <prodcons_bb.h>
#include <stdlib.h>

int arr_q[5];
sid32 psem, csem, mutex;
int head;
int tail;
void prodcons_bb(int nargs, char* args[]){
	if (nargs==1 && strncmp(args[0], "prodcons_bb", 11) == 0){
	fprintf(stderr, "Usage: %s\n\n", args[0]);
	                        fprintf(stderr, "run prodcons_bb [# of producer processes] [# of consumer processes] [# of iterations the producer runs] [# of iterations the consumer runs]\n");
				                return SYSERR;

	}	
	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		    fprintf(stderr, "Usage: %s\n\n", args[0]);
		        fprintf(stderr, "run prodcons_bb [# of producer processes] [# of consumer processes] [# of iterations the producer runs] [# of iterations the consumer runs]\n");
		return SYSERR;
	}
  	if (nargs > 5) {
		      fprintf(stderr, "too many arguments\n");
		          fprintf(stderr, "Try 'run %s --help' for more information\n", args[0]);
		return SYSERR;
	}	
	if (nargs < 5){
		fprintf(stderr, "too few arguments\n");
		fprintf(stderr, "Try 'run %s --help' for more information\n", args[0]);
		return SYSERR;
		}
	if (nargs == 5){
		if((atoi(args[1])*atoi(args[3]))!=atoi(args[2])*atoi(args[4])){
			fprintf(stderr, "Make sure that the number of total producer iterations and the number of total consumer iterations are equal.\n");
			fprintf(stderr, "For ex:\nUsing the command $ run prodcons_bb 2 4 10 5:\n2 producers, each running 10 iterations -> 20 total producer/write iterations\n4 consumers, each running 5 iterations -> 20 total consumer/read iterations\n");
		 return SYSERR;
		}	
	}
	mutex=semcreate(1);
	psem=semcreate(5);
	csem=semcreate(0);
	
	tail = 0;
	head = 0;
	
	int32 i;
	int32 j;
	int32 k;
	int32 l;
	char str2[3];
	//for(i=0;i<5;i++){
	//	arr_q[i]=-1;
	//}
	for(k=0;k<atoi(args[1]);k++){
		char str1[]="producer_";
		//Ref: https://beginnersbook.com/2015/02/c-program-to-concatenate-two-strings-without-using-strcat/
		//for concatinating 2 strings
		sprintf(str2, "%d", k);
		for(i=0; str1[i]!='\0'; ++i); 
		for(j=0; str2[j]!='\0'; ++j, ++i){
			str1[i]=str2[j];
		       }
		      // \0 represents end of string
		      str1[i]='\0';
		      resume(create(producer_bb, 1024, 20, str1, 1, atoi(args[3])));
	
	}
	int32 m;
	int32 a;
	char str3[3];
	for(l=0;l<atoi(args[2]); l++){
		char str4[]="consumer_";
		//Ref: https://beginnersbook.com/2015/02/c-program-to-concatenate-two-strings-without-using-strcat/
		//for concatinating 2 strings
		sprintf(str3,"%d", l);
		for(m=0; str4[m]!='\0'; ++m);
		for(a=0; str3[a]!='\0'; ++a, ++m){
		        str4[m]=str3[a];
		}
		// \0 represents end of string
		str4[m]='\0';	
		resume(create(consumer_bb, 1024, 20, str4, 1,atoi(args[4])));

  	}
	return(0);
}

shellcmd xsh_run(int nargs, char *args[])
{
	                /* nargs is number of arguments passed through commandline
			 * and args is the array of arrguments passed*/
	if (nargs == 2 && strncmp(args[1], "--help", 7)==0){
		printf("Usage:\n\n");
		printf("run [command]\n");
		printf("'run list' for list of commands\n");
	}
	
	if ((nargs == 2) && (strncmp(args[1], "list", 5) == 0)) {
	      	printf("hello\n");
	      	printf("prodcons\n");
		printf("prodcons_bb\n");
		return OK;
		}
	 if (nargs == 1){
	        printf("Usage:\n\n");
	        printf("run [command]\n");
		printf("--help\n");
	        printf("'run list' for list of commands\n");
	        }

    	/* This will go past "run" and pass the function/process name and its
	* arguments.
        * */
    	args++;
    	nargs--;
	if(strncmp(args[0], "prodcons_bb", 9) == 0) {
		                  /* create a process with the function as an entry point. */
		                 resume (create((void *)prodcons_bb, 4096, 20, "prodcons_bb", 2, nargs, args));
				                               }

	//if(strncmp(args[1], "prodcons", 8) == 0) {
	         /* create a process with the function as an entry point. */
	//	resume (create((void *)prodcons, 4096, 20, "prodcons", 2, nargs, args));
	//	}
	return (0);
}


