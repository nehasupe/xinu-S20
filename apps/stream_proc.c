#include <xinu.>

void stream_consumer(int32 id, struct stream *str);

int stream_proc(int nargs, char* args[]) {
	// Parse arguments
	char usage[] = "Usage: -s num_streams -w work_queue_depth -t time_window -o output_time\n";

	/* Parse arguments out of flags */
	/* if not even # args, print error and exit */
	if (!(nargs % 2)) {
		printf("%s", usage);
		return(-1);
	}
	else {
		i = nargs - 1;
		while (i > 0) {
			ch = args[i-1];
			c = *(++ch);
			     
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
	  //   // Create streams
	  //
	  //     // Create consumer processes and initialize streams
	//Single producer reading input streams
	a = (char *)stream_input[i];
	st = atoi(a);
	while (*a++ != '\t');
	ts = atoi(a);
	while (*a++ != '\t');
	v = atoi(a);
	
	  //       // Use `i` as the stream id.
        for (i = 0; i < num_streams; i++) {
	  //
	}
	  //
	  //             // Parse input header file data and populate work queue
  	ulong secs, msecs, time;
    	secs = clktime;
      	msecs = clkticks;

  	if((pcport = ptcreate(num_streams)) == SYSERR) {
		printf("ptcreate failed\n");
		return(-1);
		}

  	// Your code for Phase 1 goes here
	for(i=0; i < num_streams; i++) {
		uint32 pm;
		pm = ptrecv(pcport);
   		printf("process %d exited\n", pm);
                }
   
        ptdelete(pcport, 0);
  
        // Your code for Phase 1 goes here
	//
	time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
	printf("time in ms: %u\n", time);
	return 0;
}
