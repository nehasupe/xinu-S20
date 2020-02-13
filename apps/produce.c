#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>

void producer_bb(int count){
	int32 i;
	int32 p;
	
	char *proc_name = proctab[getpid()].prname;
	for(i=0; i<count; i++){
		p=i;
		wait(psem);
		wait(mutex);
		
		arr_q[tail]=p;
		printf("name : %s, write : %d\n", proc_name, p);
		tail=(tail+1)%5;
						
		signal(mutex);
		signal(csem);
		
	}
}
void producer(int count) {
	int32 i;
	for( i=0 ; i<count ; i++ ){
		n = i;
		printf("produced: %d\n", n);
	}
}



