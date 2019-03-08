#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dlfcn.h>

int rand() {
    return 42; //the most random number in the universe
}

// case 2 : heap
size_t allocated = 0;

typedef void *(*orig_malloc)(size_t size);

void *malloc(size_t size) {
    pid_t pid = getpid();
    if (size > 4000000 || allocated > 4000000) {
        fprintf(stderr, "pid attempting to occupy more than 4 MB of heap memory : %d\n", pid);
        kill(pid, SIGKILL);
    }
    //fprintf(stderr, "pid running malloc() to allocate %lu bytes of heap space, with a total of %lu bytes used,is : %d\n", size, allocated, pid);
    allocated += size;
    orig_malloc orig_m;
    orig_m = (orig_malloc) dlsym(RTLD_NEXT,"malloc");
    return orig_m(size);
}

typedef void *(*orig_calloc)(size_t nitems, size_t size);

void *calloc(size_t nitems, size_t size) {
    pid_t pid = getpid();
    if (nitems * size > 4000000 || allocated > 4000000) {
        fprintf(stderr, "child attempting to occupy more than 4 MB of heap memory : %d\n", pid);
        kill(pid, SIGKILL);
    }
    //fprintf(stderr, "pid running calloc() to allocate %lu bytes of heap space, with a total of %lu bytes used,is : %d\n", nitems * size, allocated, pid);
    allocated += (nitems * size);
    orig_calloc orig_c;
    orig_c = (orig_calloc) dlsym(RTLD_NEXT,"calloc");
    return orig_c(nitems, size);
}

// case 3 : fork
pid_t fork(void) {
    pid_t pid = getpid();
    fprintf(stderr, "pid attempting to fork : %d\n", pid);
    kill(pid, SIGKILL);
}


// case 4 : thread creation
typedef int (*orig_pthread_create)(pthread_t *thread, 
        const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, 
        void *(*start_routine) (void *), void *arg) {
    pid_t pid = getpid();
    fprintf(stderr, "pid attempting to create a thread : %d\n", pid);
    kill(pid, SIGKILL);

}

// case 5 : open
FILE *fopen(const char *file, const char *mode) {
    pid_t pid = getpid();
    fprintf(stderr, "pid attempting to open '%s' : %d\n", file, pid);
    kill(pid, SIGKILL);
}

typedef int (*orig_open_f_type)(const char *pathname, int flags);

int open(const char *pathname, int flags, ...) {
    pid_t pid = getpid();
    fprintf(stderr, "pid attempting to open '%s' : %d\n", pathname, pid);
    kill(pid, SIGKILL);
}


