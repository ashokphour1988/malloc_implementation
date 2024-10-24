#include <stdio.h>
#include <unistd.h>

int main()
{
	int *bp0;
	printf("\n1\tsbrk(0)=%ld\n",sbrk(0)); // first printf also take sapce from heap so first time programm heam will rise due to printf
	printf("\n1\tsbrk(0)=%ld\n",sbrk(0)); // this will give 2nd programe break point after heam memory rise due to printf; 
	bp0 = sbrk(0);
	printf("\nsbrek(100)=%ld\n",sbrk(100));
	printf("\n2\tsbrk(0)=%ld\n",sbrk(0)); // this will return the programe break after the 100 bytes of memory rise by sbrk(100) 
	printf("\n3\tdiff=%ld\n",(int*)sbrk(0)-bp0);
	printf("diff = %d\n",0x225000-0x204000);
	return 0;
}
