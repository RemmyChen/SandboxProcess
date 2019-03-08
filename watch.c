

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <signal.h> 
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>

/* 
 * to compile watch : gcc -g -o watch watch.c -lpthread -lrt
 * to compile shadow.so : gcc -shared -fPIC -g -o shadow.so shadow.c -ldl
 * to execute : ./watch [compiled child program]
 * 
 * ./watch a.out will watch whatever process that 
 * results from executing a.out. 
 * 
 * case 1 : limit stack size DONE 
 *
 * The child's stack size is limited to 4MB using setrlimit(), which
 * is a syscall that sets resource limits. If the limit is up, 
 * SIGSEGV kills the process, which results in an exit code of 139.
 * In this case, there doesn't seem to be a way to exit on 9.
 *
 * case 2 : limit heap size DONE
 *
 * The child's heap size is limited to 4MB by hooking malloc() and
 * calloc(). The shadow library shadow.so contains wrappers around
 * malloc() and calloc(), and the Linux shell environment LD_PRELOAD 
 * is passed to execve() to load the shadow library before any other 
 * library (including glibc). A global variable is used to check 
 * whether the 4MB will be exceeded if a memory allocation request
 * is granted.
 *
 * case 3 : prohibit forking : DONE
 * 
 * Forking by child is prohibited by hooking fork(). The shadow 
 * library shadow.so contains a wrapper around fork(), and the 
 * Linux shell environment LD_PRELOAD is passed to execve() to 
 * load the shadow library before any other library (including glibc).
 *
 * case 4 : prohibit thread creation DONE
 *
 * POSIX thread creation by child is prohibited by hooking pthread_create().
 * The shadow library shadow.so contains a wrapper around pthread_create(),
 * and the Linux shell environment LD_PRELOAD is passed to execve() to load
 * the shadow library before any other library (including glibc).
 *
 * case 5 : prohibit opening files DONE
 *
 * File opening by child is prevented by hooking fopen() and open(). The
 * shadow library shadow.so contains wrappers around fopen() and open(),
 * and the Linux shell environment LD_PRELOAD is passed to execve() to load
 * the shadow library before any other library (including glibc).
 *
 * case 6 : limit globals size DONE
 *
 * Memory allocation into initialized and uninitialized data segments by
 * global segments is limited to a total of 1MB by using a hacky way of
 * calling the size command on a given file argument and parsing out the 
 * output for the memory usage values corresponding to initialized and
 * uninitialized data segments. 
 * in the initialized
 *
 * */


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "%s usage: %s [compiled child program]\n", 
                argv[0], argv[0]);
        exit(1);
    }

    pid_t pid1, pid2; 
    int status; 
    struct rusage child_cpu;     
    struct rusage usage; 
    time_t curr_time;
    struct rlimit lim;
    char *const envs[] = {"LD_PRELOAD=./shadow.so", NULL};
    int fd[2]; 
    char buf[1024]; 
    int child_stdout_line_no = 0;
    pipe(fd);

    if ((pid1=fork())) {    // parent
        //fprintf(stderr, "parent %d : has child %d\n",getpid(),pid1); 
        FILE *read = fdopen(fd[0],"r"); 
        close(fd[1]); 

        /* wait for child process to be done */
        while (!(pid2=wait3(&status,WNOHANG,&usage))) { 
            sleep (1);  
        }

        /* get wallclock time of child process' death */
        time(&curr_time);

        /* get child process' CPU time */
        getrusage(RUSAGE_CHILDREN, &child_cpu);
        double child_cpu_stime = (double)child_cpu.ru_stime.tv_sec +
            ((double)child_cpu.ru_stime.tv_usec) / 1e6;
        double child_cpu_utime = (double)child_cpu.ru_utime.tv_sec +
            ((double)child_cpu.ru_utime.tv_usec) / 1e6;

        /* count # lines printed to stdout by child process */
        while (fgets(buf,1024,read) != NULL) {
            child_stdout_line_no++;
            //fprintf(stderr, "line %d : %s\n", child_stdout_line_no, buf);
        }
        fclose(read);

        /* report */
        fprintf(stderr, "\nchild process %d :\n"
                "\texit code :  %d\n"
                "\tCPU time : %e\n"
                "\tnumber of lines printed to stdout : %d\n"
                "\twallclock time of death : %s\n", 
                pid2, status, child_cpu_stime + child_cpu_utime, 
                child_stdout_line_no, ctime(&curr_time)); 

    } else {            // child

        /* case 1 : set stack limit to 4MB */
        lim.rlim_cur = 4000000;
        lim.rlim_max = 4000000;
        if (setrlimit(RLIMIT_STACK, &lim) != 0) {
            fprintf(stdout, "setrlimit() failed\n");
            return 1;
        }
        //system("bash -c 'ulimit -a'");


        /* case 6 : set globals limit to 1MB */
        char *str1;
        char *str2; 
        char *str3;
        str1 = "size ";
        str2 = argv[1];
        str3 = " | tail -n 1";
        char *str4 = (char *) malloc(1 + strlen(str1) + strlen(str2) + strlen(str3));
        strcpy(str4, str1);
        strcat(str4, str2);
        strcat(str4, str3);
        FILE *fd2 = popen(str4, "r");
        int d1, d2, d3, d4;
        char s1[100];
        fscanf(fd2, "%d %d %d %d %s", &d1, &d2, &d3, &d4, s1);
        fclose(fd2);
        free(str4);
        if (d2 + d3 > 1000000) {
            fprintf(stderr, "attempting to declare more than 1MB of global variables");
        }

        /* send stdout to pipe */
        close(1); /* close existing stdout, so that 1 is unused */ 
        dup(fd[1]); /* copy fd[1] into slot 1 */ 
        close(fd[1]); /* close copy */
        close(fd[0]); /* close other side */ 

        /* execute program */
        //fprintf(stdout, "child %d: calling %s...\n", getpid(), argv[1]); 
        execle(argv[1], argv[1], 0, envs);
        fprintf(stdout, "we should never get here!\n"); 
    } 
    return 0;
}



