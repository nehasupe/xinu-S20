#include <xinu.h>
#include <stream_proc.h>
#include <future.h>
#include<future_test.h>
//#include "tscdf_input1.h"
////#include "input_1000000.h"
#include "tscdf.h"
uint pcport;
int work_queue_depth;
int num_streams;
int time_window;
int output_time;

uint stream_producer_future(future_t** fut) {
	int i = 0;
	int stream_id;
	for(i=0; i<n_input;i++){
		struct data_element *d;
		d = (struct data_element*)getmem(sizeof(de));
		char* a = (char *)stream_input[i++];
		stream_id = atoi(a);
		while (*a++ != '\t');
		d -> time = atoi(a);
		while (*a++ != '\t');
		d -> value = atoi(a);
		future_set(fut[stream_id],(char *)&d);
		kprintf("%d\ %d\ %d\n",stream_id, d -> value, d -> time);

		//kprintf("value: %d\n", d -> value);
		//kprintf("time: %d\n", d -> time);
	}
}
uint stream_consumer_future(int32 id, future_t* fut) {
	int32 c;
	int32 t;
	int counter = 0;
	int i, status;
	struct tscdf* tc = tscdf_init(time_window);
	kprintf("stream_consumer_future id:%d (pid:%d)\n", id, getpid());
	while(1){
		struct data_element *d;
		d = (struct data_element*)getmem(sizeof(de));
		//de *d = (de *)getmem(sizeof(de));
		//future_get(fut,d);
	

		future_get(fut,(char*)&d);

		//kprintf("%d\ %d\ %d\n",id, d -> value, d -> time);
		//kprintf("time: %d\n", d -> time);
		c = d -> value;
		t = d -> time;
		if(t==0 && c==0){
			//kprintf("stream_consumer exiting\n");
			ptsend(pcport, getpid());
			return;
		}
		counter++;
		tscdf_update(tc, t, c);
		if(counter==output_time){
			counter = 0;
			char output[100];
			int32* qarray = tscdf_quartiles(tc);
	
			if(qarray == NULL) {
		        	kprintf("tscdf_quartiles returned NULL\n");
			        continue;
			}

	         sprintf(output, "s%d: %d %d %d %d %d", id, qarray[0], qarray[1], qarray[2], qarray[3], qarray[4]);
		 kprintf("%s\n", output);
			freemem((char *)qarray, (6*sizeof(int32)));
		}
		
	
	}
	ptsend(pcport, getpid());
	return OK;
}

int stream_proc_futures(int nargs, char* args[]) {
	// Parse arguments
	char usage[] = "Usage: -s num_streams -w work_queue_depth -t time_window -o output_time\n";
	num_streams = -1;
	work_queue_depth = -1;
	time_window = -1;
	output_time = -1;
	/* Parse arguments out of flags */
	/* if not even # args, print error and exit */
	if (!(nargs % 2)) {
		printf("%s", usage);
		return(-1);
	}
	else {
		int i = nargs - 1;
		while (i > 0) {
			char* ch = args[i-1];
			char c = *(++ch);
			switch(c) {
				case 's':
					num_streams = atoi(args[i]);
	 				break;
				case 'w':
					work_queue_depth = atoi(args[i]);
	 				break;
				case 't':
					time_window = atoi(args[i]);
					break;
	   			case 'o':
					output_time = atoi(args[i]);
					break;
				default:
					printf("%s", usage);
					return(-1);
			}
			i -= 2;
		}
	}
	
	ulong secs, msecs, time;
	secs = clktime;
	msecs = clkticks;
	int i;
	if((pcport = ptcreate(num_streams)) == SYSERR) {
		printf("ptcreate failed\n");
		return(-1);
	}
	// Create array to hold `n_streams` number of futures
	//future_t *farray[num_streams];
	future_t** farray = (future_t **)getmem(sizeof(future_t *)* num_streams);
	// Create consumer processes and allocate futures
	for(i=0;i<num_streams;i++){
		if((farray[i]=future_alloc(FUTURE_QUEUE,sizeof(de), work_queue_depth))==(future_t*)SYSERR){
			printf("Error creating future!");
			return -1;
	}
	}

	//resume( create(stream_producer_future, 1024, 20, "stream_producer_future", 1, farray));
	for(i = 0; i < num_streams; i++){
		resume (create(stream_consumer_future, 1024, 20, "stream_consumer_future", 2, i, farray[i]));
	}
	//resume( create(stream_producer_future, 1024, 20, "stream_producer_future", 1, farray));

	// Use `i` as the stream id.
	// Future mode = FUTURE_QUEUE
	// Size of element = sizeof(struct data_element)
	// Number of elements = work_queue_depth
	//for (i = 0; i < num_streams; i++) {
	 
	//}
	// Parse input header file data and set future values
	//kprintf("n input %d\n", n_input);
	for(i = 0; i< n_input; i++){
		char* a = (char *)stream_input[i];
		int st = atoi(a);
		while (*a++ != '\t');
		int ts = atoi(a);
		while (*a++ != '\t');
		int v = atoi(a);
		de *d = (de *)getmem(sizeof(de));
		d->time = ts;
		d->value = v;
		//kprintf("%d\ %d\ %d\n",st, d -> value, d -> time);

		future_set(farray[st],(char*)&d);
		
	}

	// Wait for all consumers to exit
	for(i=0; i < num_streams; i++) {
		uint32 pm;
		pm = ptrecv(pcport);
		kprintf("process %d exited\n", pm);
	}
	
	// free futures array
	//for(i =0; i < num_streams; i++){
	//	future_free(farray[i]);
	//}

	ptdelete(pcport, 0);
	time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
	printf("time in ms: %u\n", time);
		
       	return 0;
}
