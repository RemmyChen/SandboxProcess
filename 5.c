// open a file
#include <stdio.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#define BUFLEN 1000
main() { 
    //open("/etc/passwd", O_RDONLY);
    FILE *fp = fopen("/etc/passwd", "r"); 
    sleep(1); 
    char buf[BUFLEN]; 
    while (! feof(fp)) { 
        fgets(buf, BUFLEN, fp); 
        printf("%s",buf); 
    } 
    fprintf(stderr, "oink!\n"); 
} 
