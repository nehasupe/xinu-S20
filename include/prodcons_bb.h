// declare globally shared array
extern int arr_q[5];
// declare globally shared semaphores
extern sid32 psem, csem;
extern sid32 mutex;
// // declare globally shared read and write indices
extern int head;
extern int tail;
// function prototypes
void consumer_bb(int count);
void producer_bb(int count);
