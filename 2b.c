// grab a HUGE heap record 
#include <stdio.h> 
#include <malloc.h> 

main() 
{ 
    //int *p=(int *)malloc(1200*1024*sizeof(int)); 
    int *p=(int *)calloc(1024*1024, sizeof(int));
    printf("oink!\n"); 
} 
