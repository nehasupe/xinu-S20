#include <xinu.h>
#include <future.h>

//future_alloc
////Parameters:
////future_mode_t mode - The mode this future will operate in
////uint size - The size of the data element in bytes
////uint nelems - The number of elements
////Returns: future_t - SYSERR or a pointer to a valid future

future_t* future_alloc(future_mode_t mode, uint size, uint nelems){
	future_t* newfut = (future_t*) getmem(sizeof(future_t));
	newfut -> state = FUTURE_EMPTY;
	newfut -> size = size;
	newfut -> mode = mode;
	newfut -> data = (char*) getmem(sizeof(char)*size*nelems);
	newfut -> max_elems = nelems;
	newfut -> count = 0;
	newfut -> head = 0;
	newfut -> tail = 0;
	newfut -> pid = NULL;
	newfut -> set_queue = NULL;
	newfut -> get_queue = NULL;
	//char *proc_name = proctab[getpid()].prname;
	return newfut;
	
}

syscall future_free(future_t* f){
		
	intmask mask;
	mask = disable();
	if( freemem(f -> data,f -> size )== SYSERR){
		restore(mask);
		return SYSERR;
	}
	restore(mask);
	return OK; 
		
}

syscall future_get(future_t* f, char* out){
		
	intmask mask;
	mask = disable();
	if(f == (future_t*)NULL){
		restore(mask);
		return SYSERR;
	}
	if(f -> mode == FUTURE_QUEUE){
		if(f -> state == FUTURE_EMPTY){
			f -> state = FUTURE_WAITING;
			fnode_t* node = (fnode_t *) getmem(sizeof(fnode_t));
			node -> pid = getpid();
			node -> next = NULL;
			f -> get_queue = node;
			f -> state = FUTURE_WAITING;
			suspend(getpid());
			char* headelemptr = f->data + (f->head * f->size);
			memcpy(out, headelemptr, f -> size);
			f -> head = (f -> head + 1) % (f -> max_elems);
			int counter = f -> count;
			f -> count = counter - 1;
			restore(mask);
			return OK;
		}
		if(f -> state == FUTURE_READY){
			//f -> state = FUTURE_WAITING;
			//fnode_t *node = (fnode_t *) getmem(sizeof(fnode_t));
			//node -> pid = getpid();

			if(f -> count == 0){
				 fnode_t *node = (fnode_t *) getmem(sizeof(fnode_t));
				                         node -> pid = getpid();


				fnode_t *ptr;
			node -> next = NULL;
			ptr = f -> get_queue;
			while(ptr -> next != NULL)
				ptr = ptr -> next;
			ptr -> next = node;
			suspend(getpid());
			}
			char* headelemptr = f->data + (f->head * f->size);
			memcpy(out, headelemptr, f -> size);
			f -> head = (f -> head + 1) % (f -> max_elems);
			int counter = f -> count;
			f -> count = counter - 1;
			fnode_t *ptr;
			ptr = f -> set_queue;
			int c = 0;
			while(ptr != NULL){
				ptr = ptr -> next;
				c = c + 1;
			}
			if(c > 0){
				fnode_t *tmp;
				tmp = f -> set_queue;
				f-> set_queue = f->set_queue -> next;
				resume(tmp -> pid);
				freemem(tmp, sizeof(fnode_t*));
			}
			restore(mask);
			return OK;
		}
		if(f -> state == FUTURE_WAITING){
			 fnode_t *node = (fnode_t *) getmem(sizeof(fnode_t));
			                         node -> pid = getpid();


			fnode_t *ptr;
			node -> next = NULL;
			ptr = f -> get_queue;
			while(ptr -> next != NULL)
				ptr = ptr -> next;
			ptr -> next = node;
			suspend(getpid());
			char* headelemptr = f->data + (f->head * f->size);
			memcpy(out, headelemptr, f -> size);
			f -> head = (f -> head + 1) % (f -> max_elems);
			int counter = f -> count;
			f -> count = counter - 1;
			restore(mask);
			return OK;
			
	
		}
	}
	/*
	//WORKING CODE
	if(f -> mode == FUTURE_QUEUE){
		if(f->state == FUTURE_EMPTY){
			fnode_t* node = (fnode_t *) getmem(sizeof(fnode_t));
			node -> pid = getpid();
			node -> next = NULL;
			f -> get_queue = node;
			f -> state = FUTURE_WAITING;
			suspend(getpid());

		}
		else if(f -> state == FUTURE_WAITING){
			f -> state = FUTURE_WAITING;
			fnode_t *node = (fnode_t *) getmem(sizeof(fnode_t));
			node -> pid = getpid();
		        //if(f -> set_queue == NULL){
			//	node -> next = NULL;
			//	f -> get_queue = node;
			//}
			//else{
				fnode_t *ptr;
				node -> next = NULL;
				ptr = f -> get_queue;
				while(ptr -> next != NULL)
					ptr = ptr -> next;
				ptr -> next = node;
			//}
			suspend(getpid());

		}
			char* headelemptr = f->data + (f->head * f->size);
			memcpy(out, headelemptr, f -> size);
			f -> head = (f -> head + 1) % (f -> max_elems);
			int counter = f -> count;
			f -> count = counter - 1;
			// did not set state to empty
			// if get queue is not empty state = waiting
			fnode_t *ptr;
			ptr = f -> set_queue;
			int c = 0;
			while(ptr != NULL){
				ptr = ptr -> next;
				c = c + 1;
			}
			if(c > 0){
				fnode_t *tmp;
				tmp = f -> set_queue;
				f-> set_queue = f->set_queue -> next;
				resume(tmp -> pid);
				freemem(tmp, sizeof(fnode_t*));
			}
			
		
	}*/	
		/*if(f -> state == FUTURE_EMPTY){
		if(f -> mode == FUTURE_SHARED || f -> mode == FUTURE_QUEUE){
			if(f -> get_queue != NULL && f -> mode == FUTURE_SHARED){
				restore(mask);
				return SYSERR;
			}
				fnode_t* node = (fnode_t *) getmem(sizeof(fnode_t));
				node -> pid = getpid();
				node -> next = NULL;
				f -> get_queue = node;
				f -> state = FUTURE_WAITING;
				suspend(getpid());
		}
		else if(f -> mode == FUTURE_EXCLUSIVE){
		       	f -> state = FUTURE_WAITING;
			f -> pid = getpid();
			suspend(f -> pid);
		}
	}
	else if(f -> state == FUTURE_WAITING){
		if(f -> mode == FUTURE_EXCLUSIVE){
			restore(mask);
			return SYSERR;
		}
		else if(f -> mode == FUTURE_SHARED|| f -> mode == FUTURE_QUEUE){
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
		}
		else if(f -> mode == FUTURE_SHARED){
			memcpy(out, f -> data, f -> size);
		}
		else if(f -> mode == FUTURE_QUEUE){
			char* headelemptr = f->data + (f->head * f->size);
			memcpy(out, headelemptr, f -> size);
			f -> head = (f -> head + 1) % (f -> max_elems);
			int counter = f -> count;
			f -> count = counter - 1;
			// did not set state to empty
			// if get queue is not empty state = waiting
			fnode_t *ptr;
			ptr = f -> set_queue;
			int c = 0;
			while(ptr != NULL)
			{
				ptr = ptr -> next;
				c = c + 1;
			}
			//kprintf("c %d\n", c);
			if(c > 0){
				//kprintf("popc %d", c);
				//pop the set_queue and resume the process
				fnode_t *tmp;
				tmp = f -> set_queue;
				f-> set_queue = f->set_queue -> next;
				resume(tmp -> pid);
				freemem(tmp, sizeof(fnode_t*));
			}
		}
			
	}*/
	restore(mask);
	return OK;
}

syscall future_set(future_t* f, char* in){
	intmask mask;
	mask = disable();
	if(f == (future_t*)NULL){
		restore(mask);
		return SYSERR;
	}
	if(f-> mode == FUTURE_QUEUE){
		if(f -> count == f -> max_elems-1){
			fnode_t *node = (fnode_t *) getmem(sizeof(fnode_t));
			node -> pid = getpid();
			if(f -> set_queue == NULL){
				node -> next = NULL;
				f -> set_queue = node;
			}
			else{
				fnode_t *ptr;
				node -> next = NULL;
				ptr = f -> set_queue;
				while(ptr -> next != NULL)
					ptr = ptr -> next;
				ptr -> next = node;
			}
			suspend(getpid());
		}
		if(f -> state == FUTURE_EMPTY){
			f -> count++;
			char* tailelemptr = f->data + (f->tail * f->size);
			memcpy(tailelemptr, in, f -> size);
			f -> tail = (f -> tail + 1) % (f -> max_elems);
			f -> state = FUTURE_READY;
			fnode_t *ptr;
			ptr = f -> get_queue;
			int c = 0;
			while(ptr != NULL){
				ptr = ptr -> next;
				c = c + 1;
			}
			if(c > 0){
				fnode_t *tmp;
				tmp = f -> get_queue;
				f -> get_queue = f->get_queue -> next;
				resume(tmp -> pid);
				freemem(tmp, sizeof(fnode_t*));
			}
			restore(mask);
			return OK;
		}
		if(f -> state == FUTURE_WAITING){

			f -> count++;
			char* tailelemptr = f->data + (f->tail * f->size);
			memcpy(tailelemptr, in, f -> size);
			f -> tail = (f -> tail + 1) % (f -> max_elems);
			f -> state = FUTURE_READY;
			//kprintf("add element to dQ\n");
			fnode_t *ptr;
			ptr = f -> get_queue;
			int c = 0;
			while(ptr != NULL){
				ptr = ptr -> next;
				c = c + 1;
			}
			if(c > 0){
				fnode_t *tmp;
				tmp = f -> get_queue;
				f -> get_queue = f->get_queue -> next;
				resume(tmp -> pid);
				freemem(tmp, sizeof(fnode_t*));
			}
			restore(mask);
			return OK;
		}
		if(f -> state == FUTURE_READY){
		//WORKING CODE
			/*if(f -> count == f -> max_elems-1){
				fnode_t *node = (fnode_t *) getmem(sizeof(fnode_t));
				node -> pid = getpid();
				if(f -> set_queue == NULL){
					node -> next = NULL;
					f -> set_queue = node;
				}
				else{
					fnode_t *ptr;
					node -> next = NULL;
					ptr = f -> set_queue;
					while(ptr -> next != NULL)
						ptr = ptr -> next;
					ptr -> next = node;
				}
				suspend(getpid());
                        }*/
			f -> count++;
			char* tailelemptr = f->data + (f->tail * f->size);
			memcpy(tailelemptr, in, f -> size);
			f -> tail = (f -> tail + 1) % (f -> max_elems);
			f -> state = FUTURE_READY;
			fnode_t *ptr;
			ptr = f -> get_queue;
			int c = 0;
			while(ptr != NULL){
				ptr = ptr -> next;
				c = c + 1;
			}
			if(c > 0){
				fnode_t *tmp;
				tmp = f -> get_queue;
				f -> get_queue = f->get_queue -> next;
				resume(tmp -> pid);
				freemem(tmp, sizeof(fnode_t*));
			}// WORKING CODE ENDS HEre
			restore(mask);
			return OK;
		}

	}
	/*
	if(f -> state == FUTURE_READY){
		if(f -> mode == FUTURE_EXCLUSIVE || f -> mode == FUTURE_SHARED){
			restore(mask);
			return SYSERR;
		}
		else if(f -> mode == FUTURE_QUEUE){
			if(f -> count == f -> max_elems-1){
				fnode_t *node = (fnode_t *) getmem(sizeof(fnode_t));
				node -> pid = getpid();
				if(f -> set_queue == NULL){
					node -> next = NULL;
					f -> set_queue = node;
				}
				else{
					fnode_t *ptr;
					node -> next = NULL;
					ptr = f -> set_queue;
					while(ptr -> next != NULL)
						ptr = ptr -> next;
					ptr -> next = node;
				}
				suspend(getpid());
	
			}	
			char* tailelemptr = f->data + (f->tail * f->size);
			memcpy(tailelemptr, in, f -> size);
			int counter = f -> count;
			f -> count = counter + 1;
			f -> tail = (f -> tail + 1) % (f -> max_elems);
			f  -> state = FUTURE_READY;
			fnode_t *ptr;
			ptr = f -> get_queue;
			int c = 0;
			while(ptr != NULL){
				ptr = ptr -> next;
				c = c + 1;
			}
			if(c > 0){

				fnode_t *tmp;
				tmp = f -> get_queue;
				f -> get_queue = f->get_queue -> next;
				resume(tmp -> pid);
				freemem(tmp, sizeof(fnode_t*));
			}

		}
	}
	if(f -> state == FUTURE_EMPTY || f -> state == FUTURE_WAITING){
		if(f -> state == FUTURE_WAITING){
			if(f -> mode == FUTURE_EXCLUSIVE){
				memcpy(f -> data, in, f -> size);
				f -> state = FUTURE_READY;
				resume(f -> pid);
			}
			else if(f -> mode == FUTURE_SHARED){
				memcpy(f -> data, in, f -> size);
				f -> state = FUTURE_READY;
				fnode_t *ptr;
				if(f -> get_queue!= NULL){
					ptr = f->get_queue;
					while(ptr != NULL){
						fnode_t *tmp;
						tmp = f -> get_queue;
						f-> get_queue = f->get_queue -> next;
						resume(tmp -> pid);
						freemem(tmp, sizeof(fnode_t*));
						ptr = f -> get_queue;
					}
				}
			}
			else if(f -> mode == FUTURE_QUEUE){
		
				if(f -> count == f -> max_elems-1){
					
					fnode_t *node = (fnode_t *) getmem(sizeof(fnode_t));
					node -> pid = getpid();

					if(f -> set_queue == NULL){
						 node -> next = NULL;
					 	f -> set_queue = node;	 
					}
					else{
						fnode_t *ptr;
						node -> next = NULL;
						ptr = f -> set_queue;
					while(ptr -> next != NULL)
						ptr = ptr -> next;
					ptr -> next = node;
					}
					suspend(getpid());
				}

				char* tailelemptr = f->data + (f->tail * f->size);
				memcpy(tailelemptr, in, f -> size);
				int counter = f -> count;
				f -> count = counter + 1;
				f -> tail = (f -> tail + 1) % (f -> max_elems);
				f -> state = FUTURE_READY;
				fnode_t *ptr;
				ptr = f -> get_queue;
				int c = 0;
				while(ptr != NULL){
					ptr = ptr -> next;
					c = c + 1;
				}
				if(c > 0){

					fnode_t *tmp;
					tmp = f -> get_queue;
					f -> get_queue = f->get_queue -> next;
					resume(tmp -> pid);
					freemem(tmp, sizeof(fnode_t*));
				}
			}
		}
		else if(f-> state == FUTURE_EMPTY){
			if(f -> mode == FUTURE_SHARED || f -> mode == FUTURE_EXCLUSIVE){
			memcpy(f -> data, in, f -> size);
			}
			if(f -> mode == FUTURE_QUEUE){
				f -> count = 1;
				char* tailelemptr = f->data + (f->tail * f->size);
				memcpy(tailelemptr, in, f -> size);
				f -> tail = (f -> tail + 1) % (f -> max_elems);
			}
			f -> state = FUTURE_READY;
		}
	}*/
	restore(mask);
	return (OK);
}


