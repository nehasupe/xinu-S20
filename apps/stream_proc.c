#include <xinu.h>
#include <stream_proc.h>
//#include "tscdf_input1.h"
//#include "input_1000000.h"
#include "tscdf.h"
uint pcport;
int work_queue_depth;
int num_streams;
int time_window;
int output_time;

void stream_producer(struct stream** stream_index){
	int i = 0;
	int stream_id;
	//printf("work queue depth: %d\n", work_queue_depth);
	while(1){
		struct data_element *d;
	        d = (struct data_element*)getmem(sizeof(de));
		char* a = (char *)stream_input[i++];
		stream_id = atoi(a);
		while (*a++ != '\t');
		d -> time = atoi(a);
		while (*a++ != '\t');
		d -> value = atoi(a);
		//kprintf("stream id: %d, time: %d, value: %d\n", stream_id, d -> time, d -> value);
		wait(stream_index[stream_id] -> spaces);
		wait(stream_index[stream_id] -> mutex);
		(stream_index[stream_id] -> queue)[stream_index[stream_id] -> tail] = *d;
		//kprintf("%d produced time: %d value: %d\n", stream_id, d->time, d->value);
		stream_index[stream_id] -> tail = (stream_index[stream_id] -> tail +1) % work_queue_depth;
		signal(stream_index[stream_id] -> mutex);
		signal(stream_index[stream_id] -> items);

	}

}
void stream_consumer(int32 id, struct stream *str){
	int32 c;
	int32 t;
	char output[100];
	int counter = 0;
	//printf("time window: %d\n", time_window);
	//printf("output time: %d\n", output_time);
	struct tscdf* tc = tscdf_init(time_window);
	kprintf("stream_consumer id:%d (pid:%d)\n", id, getpid()); 
	while(1){
		wait(str -> items);
		wait(str -> mutex);
		c = (str -> queue)[str -> head].value;
		t = (str -> queue)[str -> head].time;
		str -> head = (str -> head + 1) % work_queue_depth;
		//kprintf("%d consumed value: %d, time: %d",id, c, t);
		
		if(t==0 && c==0){
			kprintf("stream_consumer exiting\n");
			ptsend(pcport, getpid());
			return;
		}
		counter++;
		tscdf_update(tc, t, c);
		if(counter==output_time){
			counter = 0;
			int32* qarray = tscdf_quartiles(tc);

		if(qarray == NULL) {
		        kprintf("tscdf_quartiles returned NULL\n");
			continue;
	 	}
	 	sprintf(output, "s%d: %d %d %d %d %d", id, qarray[0], qarray[1], qarray[2], qarray[3], qarray[4]);
	 	kprintf("%s\n", output);	       
	 	freemem((char *)qarray, (6*sizeof(int32)));
		}
		//if(t==0 && c==0){
		//	kprintf("stream_consumer exiting\n");
		//	ptsend(pcport, getpid());
		//	return;
		//}
		signal(str -> mutex);
		signal(str -> spaces);

	}
}

int stream_proc(int nargs, char* args[]) {
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

  	if((pcport = ptcreate(num_streams)) == SYSERR) {
		printf("ptcreate failed\n");
		return(-1);
		}

	struct stream* stream_index[num_streams];
	int i;
	for (i = 0; i < num_streams; i++) {
		stream_index[i] = (struct stream*) getmem(sizeof(struct stream));
		stream_index[i] -> queue = (struct data_element*)getmem(work_queue_depth*sizeof(struct data_element));
		stream_index[i] -> head = 0;
		stream_index[i] -> tail = 0;
		stream_index[i] -> mutex = semcreate(1);
		stream_index[i] -> spaces = semcreate(work_queue_depth);
		stream_index[i] -> items = semcreate(0);
		
	}
	resume( create(stream_producer, 1024, 20, "stream_producer", 1, stream_index));
	for(i = 0; i < num_streams; i++){
		resume (create(stream_consumer, 1024, 20, "stream_consumer", 2, i, stream_index[i]));
	}

	for(i=0; i < num_streams; i++) {
		uint32 pm;
		pm = ptrecv(pcport);
   	kprintf("process %d exited\n", pm);
                }
   
        ptdelete(pcport, 0);
  
	time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
	printf("time in ms: %u\n", time);
	return 0;
}
