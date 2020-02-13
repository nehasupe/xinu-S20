#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>

void consumer_bb(int count){
	int32 i;
	int32 c;
	
	char *proc_name = proctab[getpid()].prname;
	for(i=0; i<count; i++){
		wait(csem);
		wait(mutex);
		
		c =arr_q[head];
		printf("name : %s, read: %d\n", proc_name, c);
		head=(head+1)%5;
		
		signal(mutex);
		signal(psem);
	}
}
void consumer(int count) {
	int32 i;
	int32 c; //to store value of n to be consumed
	for( i=0 ; i<count ; i++ ){
		c=n;
		printf("consumed: %d\n", c);
	}
}

