
#define STRUCT_SIZE 24
#define MULTIPLIER 10
#define ALIGN_SIZE 8

#define ALIGN(size) (size+ALIGN_SIZE)&~(ALIGN_SIZE-1)





typedef struct chunkstatus{
	unsigned int size;
	int available;
	struct chunkstatus *next;
	struct chunkstatus *prev;
	char end[1];
}chunkstatus;

//chunkstatus *splitchunk(chunkstatus *, unsigned int size);
