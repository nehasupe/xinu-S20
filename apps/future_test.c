#include <xinu.h>
#include <future.h>

#include <future_test.h>
uint future_test(int nargs, char *args[]){
	

	if(nargs == 2 && strncmp(args[1], "-pc", 3) == 0){
	future_t* f_exclusive, * f_shared;
	f_exclusive = future_alloc(FUTURE_EXCLUSIVE, sizeof(int), 1);

	f_shared    = future_alloc(FUTURE_SHARED, sizeof(int), 1);
	// Test FUTURE_EXCLUSIVE
	resume( create(future_cons, 1024, 20, "fcons1", 1, f_exclusive) );
	resume( create(future_prod, 1024, 20, "fprod1", 2, f_exclusive, (char*) &one) );
	
	// Test FUTURE_SHARED
	resume( create(future_cons, 1024, 20, "fcons2", 1, f_shared) );
	resume( create(future_cons, 1024, 20, "fcons3", 1, f_shared) );
	resume( create(future_cons, 1024, 20, "fcons4", 1, f_shared) );
	resume( create(future_cons, 1024, 20, "fcons5", 1, f_shared) );
	resume( create(future_prod, 1024, 20, "fprod2", 2, f_shared, (char*) &two) );
	return OK;
	}
	else if(nargs == 3 && strncmp(args[1], "-f", 2) == 0){

	//fibonchchi snippets
	int fib = -1, i;

	fib = atoi(args[2]);

	if (fib > -1) {
		int final_fib;
		int future_flags = FUTURE_SHARED; // TODO - add appropriate future mode here
		// create the array of future pointers
		if ((fibfut = (future_t **)getmem(sizeof(future_t *) * (fib + 1))) == (future_t **) SYSERR) {
			printf("getmem failed\n");
			return(SYSERR);
			}
		// get futures for the future array
	        for (i=0; i <= fib; i++) {
			if((fibfut[i] = future_alloc(future_flags, sizeof(int), 1)) == (future_t *) SYSERR) {
				printf("future_alloc failed\n");
				return(SYSERR);
				}
			}
		// spawn fib threads and get final value
		// TODO - you need to add your code here
		for(i=0; i <= fib; i++){
			resume(create (ffib, 1024, 20, "ffib", 1, i));
		}
		future_get(fibfut[fib], (char*) &final_fib);
		for (i=0; i <= fib; i++) {
			future_free(fibfut[i]);
		}
		freemem((char *)fibfut, sizeof(future_t *) * (fib + 1));
		printf("Nth Fibonacci value for N=%d is %d\n", fib, final_fib);
		return(OK);
		}
	
	}
	else{
		printf("Wrong command\n");
		printf("-ps: prodcons\n");
		printf("-f [Integer]: Fibonachchi\n");
		return(OK);
	}
	return OK;

}
