#include <xinu.h>
#include <prodcons.h>

void consumer(int count) {
	int32 i;
	int32 c; //to store value of n to be consumed
	for( i=0 ; i<count ; i++ ){
		c=n;
		printf("consumed: %d\n", c);
	}
}

