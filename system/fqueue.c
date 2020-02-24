#include <fqueue.h>

//fqueue_t* initialize(){
//	fqueue_t* q = (fqueue_t*)getmem(sizeof(fqueue_t));
//	q -> front = NULL;
//	q -> rear = NULL;
//	return q;
//}

//uint isfqempty(fqueue_t* q){
//	if(q -> rear != NULL){
//		return 0;
//	}
//	return 1;
//}

//void fqenqueue(fqueue_t* q, pid32 pid){
//	fnode_t* tmp = (fnode_t*)getmem(sizeof(fnode_t));
//	tmp -> pid = pid;
//	tmp -> next = NULL;
//	if(!isfqempty(q)){
//		q -> rear -> next = tmp;
//		q -> rear = tmp;
//	}
//	else{
//		q -> front = q -> rear = tmp;
//	}
	
//}

//pid32 fqdequeue(fqueue_t* q){
//	fnode_t* tmp;
//	pid32 pid = q -> front -> pid;
//     	tmp = q -> front;
//	q -> front = q -> front -> next;
//	freemem(tmp,(fnode_t*)sizeof(fnode_t));
//	return pid;
//}

//freequeu

