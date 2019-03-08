# SandboxProcess
Manage the behavior of processes whose source code and behavior one cannot control

## What the code does
watch reacts to several conditions in the child process with preventative actions, such that child proccesses only do things that are explicitly allowed according to a predetermined policy, and are killed if they do anything not allowed by the policy:
  - If the child occupies more than 4 MB of stack memory, it should be killed and this event should be reported. The program 1.c does this.
  - If the child occupies more than 4 MB of heap memory, it should be killed and this event should be reported. The program 2.c does this.
  - If the child forks, it should be killed and the event should be reported. The program 3.c does this.
  - If the child creates a thread, it should be killed and the event should be reported. The program 4.c does this.
  - If the child opens any file (other than the pre-opened files stdin, stdout, stderr), this should be prevented and the program should be terminated. The program 5.c does this.

shadow.so is a shadow library that contains wrappers around functions such as malloc() and calloc(), and is preloaded before other libraries (i.e. glibc) to overwrite the default behavior of these functions.

## To run
To compile watch : gcc -g -o watch watch.c -lpthread -lrt <br />
To compile shadow.so : gcc -shared -fPIC -g -o shadow.so shadow.c -ldl <br />
To execute : ./watch [compiled child program]

