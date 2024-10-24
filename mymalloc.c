#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "mymalloc.h"


#define MAX_SIZE 500

/*typedef struct chunkstatus{
	int size;
        int available;
        struct chunkstatus *next;
        struct chunkstatus *prev;
        char end[1];
}chunkstatus;*/

chunkstatus* head=NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

chunkstatus* lastvisited=NULL;
//chunkstatus* curptr=NULL;
void* brkpoint0=NULL;
void* brkpoint1=NULL;
pthread_mutex_t mem_lock;


int *a[MAX_SIZE];
int front=-1;
int rear = -1;

int is_empty()
{
	if((front == -1 && rear == -1))
		return 1;
	else
		return 0;
}

int is_full()
{
	//printf("front=%d\trear=%d\n",front,rear);
	if((rear == MAX_SIZE && front == 0 ) || (rear%MAX_SIZE == front))
		return 1;
	else
		return 0;
}

void enqueue(int *data)
{
	if(is_empty()){
		front++;
		rear++;
	}
	else if(is_full()){
		printf("Overflow Queue Full\n");
		//display();
		return;
	}
	else if((rear == MAX_SIZE) && ( rear%MAX_SIZE != front )){
		rear=0;
	}
	
	a[rear++] = data;
	//printf("front=%d\trear=%d\n",front,rear);
	
}

int* dequeue()
{
	int* data;
	if(is_empty()){
		printf("Underflow\n");
		return -1;
	}
	
	data = a[front];
	a[front] = 0;
	front++;
	//printf("front=%d\trear=%d\n",front,rear);
	if( front == rear){
		front=-1;
		rear=-1;
	}
	
	if(front == MAX_SIZE && (front%MAX_SIZE != rear))
		front=0;
		
	return data;
}



void splitchunk(chunkstatus* curptr, unsigned int size){
	//printf("_____________________splitchunk________________\n");
        chunkstatus *temp = (chunkstatus*)(curptr->end+size);

        temp->size = curptr->size-size-STRUCT_SIZE;
        temp->available=1;
        temp->next=curptr->next;
        temp->prev=curptr;
        if(temp->next != NULL)
                (temp->next)->prev = temp;

        curptr->size=size;
        curptr->next=temp;
        curptr->available=0;
}

void mergechunknext(chunkstatus *ptr){

	//printf("_____________________mergechunknext________________\n");
	chunkstatus *temp=ptr->next;
	//printf("ptr=%p\n",ptr);	
	//printf("ptr->next=%p\n",ptr->next);	
	if(temp!=NULL && temp->available == 1){
		ptr->size = temp->size+ptr->size+STRUCT_SIZE;
		printf("ptr->size=%d\n",ptr->size);
		ptr->available=1;
		ptr->next=temp->next;
		if(temp->next!=NULL)
			(temp->next)->prev=ptr;
	}
}

void mergechunkprev(chunkstatus *ptr){

	//printf("_____________________mergechunkprev________________\n");
        chunkstatus *temp=ptr->prev;

        if(temp!=NULL && temp->available == 1){
                temp->size = temp->size+ptr->size+STRUCT_SIZE;
                ptr->available=1;
                temp->next=ptr->next;
                if(ptr->next!=NULL)
                        (ptr->next)->prev=temp;
        }
}

chunkstatus* findChunk(chunkstatus* first, unsigned int size){
	chunkstatus *temp = first;

	//printf("_____________________findchunk________________\n");
	while(temp!=NULL){
		if(temp->size >= size && temp->available == 1){
			return temp;
		}
		lastvisited=temp;
		temp=temp->next;
	}
	return temp;
}

chunkstatus* increaseAllocation(chunkstatus *lastVisitedPtr, unsigned int usersize){
	
	//printf("_____________________increaseallocation________________usercase=%d\n",usersize);
	chunkstatus* temp=NULL;
	brkpoint1 = sbrk(0);
    		
	if(sbrk(MULTIPLIER*(usersize+STRUCT_SIZE)) == (void*)-1){
		pthread_mutex_unlock(&lock);
		return NULL;
	}
	temp=brkpoint1;
	temp->size=MULTIPLIER*(usersize+STRUCT_SIZE)-STRUCT_SIZE;
	printf("temp->size=%d\n",temp->size);
	temp->available=0;
	temp->next=NULL;

	lastvisited->next = temp;
	temp->prev=lastvisited;

	if(temp->size > usersize)
		splitchunk(temp ,usersize);

	//printf("increase allocation ##### temp=%p\n",temp);
	//printf("increase allocation ##### temp->end=%p\n",temp->end);
	brkpoint1=sbrk(0);
	return temp;
}

void printList(chunkstatus *headptr)
{
int i = 0;
  chunkstatus *p = headptr;

  while(p != NULL)
  {
    printf("[%d] p: %p\n", i, p);
    printf("[%d] p->size: %d\n", i, p->size);
    printf("[%d] p->available: %d\n", i, p->available);
    printf("[%d] p->prev: %p\n", i, p->prev);
    printf("[%d] p->next: %p\n", i, p->next);
    if(i==0)
    	printf("[%d] diff: %ld\n", i, p->next-p);
    printf("__________________________________________________\n");
    i++;
    p = p->next;
  }
}

void *mymalloc(unsigned int usersize){

	unsigned int size = ALIGN(usersize);
	unsigned int myneed = MULTIPLIER*(size+STRUCT_SIZE);
	
	chunkstatus *ptr;

	pthread_mutex_lock(&lock);
	
	//printf("\n################_____start____mymalloc____##########\n");
	if(head == NULL){
		brkpoint0 = sbrk(0);	
		if(sbrk(myneed) == (void*)-1){
			pthread_mutex_unlock(&lock);
			return NULL;
		}
		
		brkpoint1=sbrk(0);
		head=brkpoint0;
    		printf("mymalloc\tbrkpoint0=%p\tafter allocation\tdbrk(0)=%p\tdiff=%ld\n",brkpoint0,sbrk(0),(sbrk(0)-brkpoint0));		
		head->size = myneed-STRUCT_SIZE;
		head->available=0;
		head->next=NULL;
		head->prev=NULL;
		
		ptr = head;
    		//printf("mymalloc    ptr=%p\n",ptr);		
    		//printf("mymalloc    end=%p\n",ptr->end);		
    		//printf("mymalloc    diff=%d\n",(unsigned int)ptr->end - (unsigned int)ptr);		

		if(MULTIPLIER>1){
			splitchunk(ptr,size);
		}
		pthread_mutex_unlock(&lock);
		printf("mymalloc : ptr->end=%p\n",ptr->end);
		return (void*)ptr->end;
				
	}
	else{  
		ptr=head;
		ptr = findChunk(ptr, size);  //traverse list for available size 

		if(ptr != NULL){
			if(ptr->size > size)
				splitchunk(ptr, size);  //split node if find available with matching size
		}
		else{
			ptr = increaseAllocation(lastvisited, size); // extend the heap size if not find any matching size available node
			printf("ptr=%p\n",ptr);
			if(ptr==NULL){
				pthread_mutex_unlock(&lock);
				return NULL;
			}		
		}
	}
	pthread_mutex_unlock(&lock);
	printf("mymalloc returning ptr->end=%p\n",ptr->end);
	return ptr->end;

}

int myfree(void *ptr){

	pthread_mutex_lock(&lock);
	chunkstatus* freeptr = (chunkstatus*)(ptr-STRUCT_SIZE);
	//printf("_____________________myfree________________\n");
	printf("myfree : ptr=%p\n",ptr);
	//printf("freeptr=%p\n",freeptr);
	//printf("head=%p\n",head);
	//printf("brkpoint1=%p\n",brkpoint1);
	if(freeptr >= head && freeptr < brkpoint1){
		freeptr->available=1;
		mergechunknext(freeptr);
		mergechunkprev(freeptr);
		pthread_mutex_unlock(&lock);

		return 0;
	}
	else{
		pthread_mutex_unlock(&lock);
		return 0;
	}
}


void *memmory_allocationThread(void *args){

	int i=10;
	int count=0;
	printf("##### satrting memory_allocation_thread ###############\n");
	while(1){
		if(count == 10){
			printf("This thream completed service 10 times\n");
			pthread_exit(NULL);
		}
		sleep(0.5);
		pthread_mutex_lock(&mem_lock);
		count++;
			
		int *p = (int*)mymalloc(50);
		printf("sending %p address\n",p);
		enqueue(p);
		printList(head);
		
		i= i+100;
		pthread_mutex_unlock(&mem_lock);
	}
	return NULL;
}

void *memmory_freeThread(void *args){

	int count=0;
	sleep(2);
	printf("####### starting free thread ###############\n");
	while(1){
		if(count == 10){
			printf("This thream completed service 10 times\n");
			pthread_exit(NULL);
		}
		sleep(1);
		pthread_mutex_lock(&mem_lock);
		count++;
			
		int *p = dequeue();
		printf("received %p address\n",p);			
		myfree(p);	
		printList(head);

		pthread_mutex_unlock(&mem_lock);
	}
	return NULL;
}

int main()
{

	int num_threads = 50;
	pthread_t threads1[num_threads];
	pthread_t threads2[num_threads];
	int thread_ids[num_threads];

	printf("sizeof int* = %d\n",sizeof(int*));
    // Create threads
	for (int i = 0; i < num_threads; i++) {
		thread_ids[i] = i;
		if (pthread_create(&threads1[i], NULL, memmory_allocationThread, &thread_ids[i]) != 0) {
            		perror("Failed to create thread");
            		return 1;
        	}
    	}

    	for (int i = 0; i < num_threads; i++) {
        	thread_ids[i] = i;
        	if (pthread_create(&threads2[i], NULL, memmory_freeThread, &thread_ids[i]) != 0) {
            		perror("Failed to create thread");
            		return 1;
        	}
    	}

    // Join threads
	for (int i = 0; i < num_threads; i++) {
        	if (pthread_join(threads1[i], NULL) != 0) {
			perror("Failed to join thread");
			return 2;
        }
    }

	for (int i = 0; i < num_threads; i++) {
		if (pthread_join(threads2[i], NULL) != 0) {
 			perror("Failed to join thread");
			return 2;
        }
    }

	printList(head);
	printf("#############################################################################################\n");
	return 0;
}
