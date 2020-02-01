#include <xinu.h>
#include <prodcons.h>

void producer(int count) {
	int32 i;
	for( i=0 ; i<count ; i++ ){
		n = i;
		printf("produced: %d\n", n);
	}
}



