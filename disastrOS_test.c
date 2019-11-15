/*#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"
#include "disastrOS_semaphore.h"

#define CHILDREN 10
#define NRES 3

int Global_Resources[NRES];
int Semaphores_fd[NRES];

void sem_errors(int err_code) {

    if (err_code >= 0) {
        printf("\e[32mSuccess\e[0m\n");
        return;
    }

    printf("\e[34m");
    switch (err_code) {
        case DSOS_EEXIST:
             printf("Semaphore already exists\n\n");
             break;
        case DSOS_EMFILE:
             printf("Max num. semaphores exceeded\n\n");
             break;
        case DSOS_EINVAL:
             printf("Invalid semaphore id\n\n");
             break;
        case DSOS_ERESOURCEOPEN:
             printf("Couldn't open semaphore\n\n");
             break;
        default:
             printf("Unknown Error: %d\n\n", err_code);
             break;
    }
    printf("\e[0m");
}

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

int pid_map(int pid){
    // We subtract two because the first
    // two pids in this example are "init" and "sleeper"
    //so we start from third one 
    //0 if we have pid multiple NRES
    return (pid -2) % NRES;
}

void print_GR() {
    printf("Global Resources: ");
    for (int i = 0; i < NRES; i++)
        printf("%d ", Global_Resources[i]); 
    printf("\n");
}

//managing child with semaphores
void main_child(){
    int pid = disastrOS_getpid();
    int map = pid_map(pid);

    //As Id of semaphore we have process Map 
    // 0 as flag
    // 0 as value

    int fd = disastrOS_semOpen(map, 0, 0);

    int r;
    printf("\e[36m[%d]\e[0m WAIT ON SEMAPHORE %d \n", pid, map);
    r = disastrOS_semWait(fd);
    sem_errors(r);
    

    //printing global resources
    for (int i = 0; i < 10; ++i) {
        //process id, sem id <- map 
        printf("\e[36m[%d]\e[0m ADD TO GLOBAL_RESOURCES[%d]\n", pid, map);
        Global_Resources[map] ++;
        print_GR();
        disastrOS_sleep(3);
    }

    printf("\e[36m[%d]\e[0m POST ON SEMAPHORE %d \n", pid, map);
    r = disastrOS_semPost(fd);
    sem_errors(r);

    printf("\e[36m[%d]\e[0m CLOSING SEMAPHORE %d \n", pid, map);
    r = disastrOS_semClose(fd);
    sem_errors(r);

    disastrOS_exit(0);
}


void initFunction(void* args) {
    disastrOS_printStatus();
    printf("hello, I am init and I just started\n");
    disastrOS_spawn(sleeperFunction, 0);

    int ret, fd, fd1;
    printf("SEMAPHORES:\n");
    //ID: 0
    //FLAG:0
    //VALUE: 1
    printf("Open uninitialized Sem. 0 without CREATE, with value 1:\n");
    ret = disastrOS_semOpen(0, 0, 1);
    sem_errors(ret);

    printf("Open Sem. 0 with value 1 (EXCL):\n");
    ret = disastrOS_semOpen(0, DSOS_CREATE | DSOS_EXCL, 1);
    fd = ret;
    sem_errors(ret);
    disastrOS_printStatus();

    printf("Reopen Sem. 0 with value 1 (Not EXCL):\n");
    ret = disastrOS_semOpen(0, DSOS_CREATE, 2);
    fd1 = ret;
    sem_errors(ret);
    disastrOS_printStatus();

    printf("Close Sem Ptr created with previous syscall:\n");
    ret = disastrOS_semClose(fd1);
    sem_errors(ret);
    disastrOS_printStatus();

    printf("Reopen Sem. 0 with value 1 (EXCL):\n");
    ret = disastrOS_semOpen(0, DSOS_CREATE | DSOS_EXCL, 1);
    sem_errors(ret);

    printf("Post on uninitialized semaphore:\n");
    ret = disastrOS_semPost(32);
    sem_errors(ret);

    printf("Wait on uninitialized semaphore:\n");
    ret = disastrOS_semWait(32);
    sem_errors(ret);

    printf("Close Sem. 0:\n");
    ret = disastrOS_semClose(fd);
    sem_errors(ret);
    disastrOS_printStatus();

    printf("Reclose Sem. 0:\n");
    ret = disastrOS_semClose(fd);
    sem_errors(ret);

    printf("Open sem. -5\n");
    ret = disastrOS_semOpen(-5, DSOS_CREATE | DSOS_EXCL, 1);
    fd = ret;
    sem_errors(ret);
    disastrOS_printStatus();

    printf("Close sem. -5\n");
    ret = disastrOS_semClose(fd);
    sem_errors(ret);
    disastrOS_printStatus();

    //printf("Open sem. 2000\n");
    //r = disastrOS_semOpen(2000, DSOS_CREATE | DSOS_EXCL, 1);
    //fd = r;
    //sem_errors(r);
    //disastrOS_printStatus();

    //printf("Close sem. 2000\n");
    //r = disastrOS_semClose(fd);
    //sem_errors(r);
    //disastrOS_printStatus();


    int fds[MAX_NUM_SEMAPHORES+1];
    printf("Open %d semaphores (1 over limit)\n", MAX_NUM_SEMAPHORES+1);
    for (int i = 0; i < MAX_NUM_SEMAPHORES+1; ++i) {
        ret = disastrOS_semOpen(i, DSOS_CREATE | DSOS_EXCL, 1);
        fds[i] = ret;
        sem_errors(ret);
    }

    printf("Close %d semaphores (even the extra one)\n", MAX_NUM_SEMAPHORES+1);
    for (int i = 0; i < MAX_NUM_SEMAPHORES+1; ++i) {
        ret = disastrOS_semClose(fds[i]);
        sem_errors(ret);
    }

    printf("________________________________\n");


    printf("\e[36m[init]\e[0m Initialize sems with ids from 0 to %d, all at value 1\n", NRES-1);
    for (int i = 0; i < NRES; ++i) {
        ret = disastrOS_semOpen(i, DSOS_CREATE | DSOS_EXCL, 1);
        sem_errors(ret);
        Semaphores_fd[i] = ret;
    }

    disastrOS_printStatus();

    printf("\e[36m[init]\e[0m Creating %d children\n", CHILDREN);
    for (int i = 0; i < CHILDREN; ++i)
        disastrOS_spawn(main_child, 0);

    disastrOS_printStatus();

    int retval;
    int pid;
    printf("\e[36m[init]\e[0m Wait for all children\n");
    for (int i = 0; i < CHILDREN; ++i) {
        pid = disastrOS_wait(0, &retval);
        printf("\e[36m[init]\e[0m child %d returned with code: %d\n", pid, retval);
        disastrOS_printStatus();
    }

    disastrOS_printStatus();

    printf("\e[36m[init]\e[0m Close all sems.\n");
    for (int i = 0; i < NRES; ++i) {
        ret = disastrOS_semClose(Semaphores_fd[i]);
        sem_errors(ret);
    }

    disastrOS_printStatus();

    printf("shutdown!");
    disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;

  if (argc>1) {
    logfilename=argv[1];
  }

  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);

  return 0;
}*/


/*#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include "disastrOS_semaphore.h"
#include "disastrOS.h"

#define CHILDREN 10
#define NRES 3

int Global_Resources[NRES];
int Semaphores_fd[NRES];
//handling errors' codes 
void sem_errors(int err_code) {
    //if error code not negative, OK!
    if (err_code >= 0) {
        printf("\e[32mSuccess\e[0m\n");
        return;
    }

    printf("\e[34m");
    switch (err_code) {
        case DSOS_EEXIST:
             printf("Semaphore already exists\n\n");
             break;
        case DSOS_EMFILE:
             printf("Max num. semaphores exceeded\n\n");
             break;
        case DSOS_EINVAL:
             printf("Invalid semaphore id\n\n");
             break;
        case DSOS_ERESOURCEOPEN:
             printf("Couldn't open semaphore\n\n");
             break;
        default:
             printf("Unknown Error: %d\n\n", err_code);
             break;
    }
    printf("\e[0m");
}

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

int get_Map(int pid){
    return (pid-2)%NRES;
}

void print_GR() {
    printf("Global Resources: ");
    for (int i = 0; i < NRES; i++)
        printf("%d ", Global_Resources[i]); 
    printf("\n");
}


void childFunction(void* args){
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int pid = disastrOS_getpid();
  int map = get_Map(pid);
  
  int fd = disastrOS_semOpen(map, 0, 0);
  
  int ret;
  printf("\e[36m[%d]\e[0m WAIT ON SEMAPHORE %d \n", pid, map);
  ret = disastrOS_semWait(fd);
  sem_errors(ret);
  
  for (int i = 0; i < 10; ++i) {
        //process id, sem id <- map 
        printf("\e[36m[%d]\e[0m ADD TO GLOBAL_RESOURCES[%d]\n", pid, map);
        Global_Resources[map] ++;
        print_GR();
        disastrOS_sleep(3);
    }
    
  printf("\e[36m[%d]\e[0m POST ON SEMAPHORE %d \n", pid, map);
    ret = disastrOS_semPost(fd);
    sem_errors(ret);
    
    printf("\e[36m[%d]\e[0m CLOSING SEMAPHORE %d \n", pid, map);
    ret = disastrOS_semClose(fd);
    sem_errors(ret);

    disastrOS_exit(0);
}
  
 
void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);
    
  int ret,fd,fd1;
    
  printf("SEMAPHORES:\n");
  //ID: 0
  //FLAG:0
  //VALUE: 1
  printf("Open uninitialized Sem. 0 without CREATE, with value 1:\n");
  ret = disastrOS_semOpen(0, 0, 1);
  sem_errors(ret);

  printf("Open Sem. 0 with value 1 (EXCL):\n");
  ret = disastrOS_semOpen(0, DSOS_CREATE | DSOS_EXCL, 1);
  fd = ret;
  sem_errors(ret);
  disastrOS_printStatus();

  printf("Reopen Sem. 0 with value 1 (Not EXCL):\n");
  ret = disastrOS_semOpen(0, DSOS_CREATE, 2);
  fd1 = ret;
  sem_errors(ret);
  disastrOS_printStatus();

  printf("Close Sem Ptr created with previous syscall:\n");
  ret = disastrOS_semClose(fd1);
  sem_errors(ret);
  disastrOS_printStatus();

  printf("Reopen Sem. 0 with value 1 (EXCL):\n");
  ret = disastrOS_semOpen(0, DSOS_CREATE | DSOS_EXCL, 1);
  sem_errors(ret);

  printf("Post on uninitialized semaphore:\n");
  ret = disastrOS_semPost(32);
  sem_errors(ret);

  printf("Wait on uninitialized semaphore:\n");
  ret = disastrOS_semWait(32);
  sem_errors(ret);

  printf("Close Sem. 0:\n");
  ret = disastrOS_semClose(fd);
  sem_errors(ret);
  disastrOS_printStatus();

  printf("Reclose Sem. 0:\n");
  ret = disastrOS_semClose(fd);
  sem_errors(ret);

  printf("Open sem. -5\n");
  ret = disastrOS_semOpen(-5, DSOS_CREATE | DSOS_EXCL, 1);
  fd = ret;
  sem_errors(ret);
  disastrOS_printStatus();

  printf("Close sem. -5\n");
  ret = disastrOS_semClose(fd);
  sem_errors(ret);
  disastrOS_printStatus();
    
  int fds[MAX_NUM_SEMAPHORES+1];
    printf("Open %d semaphores\n", MAX_NUM_SEMAPHORES+1);
    for (int i = 0; i < MAX_NUM_SEMAPHORES+1; ++i) {
        ret = disastrOS_semOpen(i, DSOS_CREATE | DSOS_EXCL, 1);
        fds[i] = ret;
        sem_errors(ret);
    }
  
  printf("Close %d semaphores\n", MAX_NUM_SEMAPHORES+1);
    for (int i = 0; i < MAX_NUM_SEMAPHORES+1; ++i) {
        ret = disastrOS_semClose(fds[i]);
        sem_errors(ret);
    }
  printf("\e[36m[init]\e[0m Initialize sems with ids from 0 to %d, all at value 1\n", NRES-1);
    for (int i = 0; i < NRES; ++i) {
        ret = disastrOS_semOpen(i, DSOS_CREATE | DSOS_EXCL, 1);
        sem_errors(ret);
        Semaphores_fd[i] = ret;
    }

    disastrOS_printStatus();

    printf("\e[36m[init]\e[0m Creating %d children\n", CHILDREN);
    for (int i = 0; i < CHILDREN; ++i)
        disastrOS_spawn(childFunction, 0);

    disastrOS_printStatus();

    int retval;
    int pid;
    printf("\e[36m[init]\e[0m Wait for all children\n");
    for (int i = 0; i < CHILDREN; ++i) {
        pid = disastrOS_wait(0, &retval);
        printf("\e[36m[init]\e[0m child %d returned with code: %d\n", pid, retval);
        disastrOS_printStatus();
    }

    disastrOS_printStatus();

    printf("\e[36m[init]\e[0m Close all sems.\n");
    for (int i = 0; i < NRES; ++i) {
        ret = disastrOS_semClose(Semaphores_fd[i]);
        sem_errors(ret);
    }

  disastrOS_printStatus();
  printf("shutdown!");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}*/

#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"
#include "disastrOS_semaphore.h"

#define CHILDREN 10
#define NRES 3

int Global_Resources[NRES];
int Semaphores_fd[NRES];

void sem_errors(int err_code) {

    if (err_code >= 0) {
        printf("\e[32mSuccess\e[0m\n");
        return;
    }

    printf("\e[34m");
    switch (err_code) {
        case DSOS_EEXIST:
             printf("Semaphore already exists\n\n");
             break;
        case DSOS_EMFILE:
             printf("Max num. semaphores exceeded\n\n");
             break;
        case DSOS_EINVAL:
             printf("Invalid semaphore id\n\n");
             break;
        case DSOS_ERESOURCEOPEN:
             printf("Couldn't open semaphore\n\n");
             break;
        default:
             printf("Unknown Error: %d\n\n", err_code);
             break;
    }
    printf("\e[0m");
}

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

int pid_map(int pid){
    // We subtract two because the first
    // two pids in this example are "init" and "sleeper"
    //so we start from third one 
    //0 if we have pid multiple NRES
    return (pid -2) % NRES;
}

void print_GR() {
    printf("Global Resources: ");
    for (int i = 0; i < NRES; i++)
        printf("%d ", Global_Resources[i]); 
    printf("\n");
}

//managing child with semaphores
void main_child(){
    int pid = disastrOS_getpid();
    int map = pid_map(pid);

    //As Id of semaphore we have process Map 
    // 0 as flag
    // 0 as value

    int fd = disastrOS_semOpen(map, 0, 0);

    int r;
    printf("\e[36m[%d]\e[0m WAIT ON SEMAPHORE %d \n", pid, map);
    r = disastrOS_semWait(fd);
    sem_errors(r);
    

    //printing global resources
    for (int i = 0; i < 10; ++i) {
        //process id, sem id <- map 
        printf("\e[36m[%d]\e[0m ADD TO GLOBAL_RESOURCES[%d]\n", pid, map);
        Global_Resources[map] ++;
        print_GR();
        disastrOS_sleep(3);
    }

    printf("\e[36m[%d]\e[0m POST ON SEMAPHORE %d \n", pid, map);
    r = disastrOS_semPost(fd);
    sem_errors(r);

    printf("\e[36m[%d]\e[0m CLOSING SEMAPHORE %d \n", pid, map);
    r = disastrOS_semClose(fd);
    sem_errors(r);

    disastrOS_exit(0);
}


void initFunction(void* args) {
    disastrOS_printStatus();
    printf("hello, I am init and I just started\n");
    disastrOS_spawn(sleeperFunction, 0);

    int ret, fd, fd1;
    printf("SEMAPHORES:\n");
    //ID: 0
    //FLAG:0
    //VALUE: 1
    printf("Open uninitialized Sem. 0 without CREATE, with value 1:\n");
    ret = disastrOS_semOpen(0, 0, 1);
    sem_errors(ret);

    printf("Open Sem. 0 with value 1 (EXCL):\n");
    ret = disastrOS_semOpen(0, DSOS_CREATE | DSOS_EXCL, 1);
    fd = ret;
    sem_errors(ret);
    disastrOS_printStatus();

    printf("Reopen Sem. 0 with value 1 (Not EXCL):\n");
    ret = disastrOS_semOpen(0, DSOS_CREATE, 2);
    fd1 = ret;
    sem_errors(ret);
    disastrOS_printStatus();

    printf("Close Sem Ptr created with previous syscall:\n");
    ret = disastrOS_semClose(fd1);
    sem_errors(ret);
    disastrOS_printStatus();

    printf("Reopen Sem. 0 with value 1 (EXCL):\n");
    ret = disastrOS_semOpen(0, DSOS_CREATE | DSOS_EXCL, 1);
    sem_errors(ret);

    printf("Post on uninitialized semaphore:\n");
    ret = disastrOS_semPost(32);
    sem_errors(ret);

    printf("Wait on uninitialized semaphore:\n");
    ret = disastrOS_semWait(32);
    sem_errors(ret);

    printf("Close Sem. 0:\n");
    ret = disastrOS_semClose(fd);
    sem_errors(ret);
    disastrOS_printStatus();

    printf("Reclose Sem. 0:\n");
    ret = disastrOS_semClose(fd);
    sem_errors(ret);

    printf("Open sem. -5\n");
    ret = disastrOS_semOpen(-5, DSOS_CREATE | DSOS_EXCL, 1);
    fd = ret;
    sem_errors(ret);
    disastrOS_printStatus();

    printf("Close sem. -5\n");
    ret = disastrOS_semClose(fd);
    sem_errors(ret);
    disastrOS_printStatus();

    //printf("Open sem. 2000\n");
    //r = disastrOS_semOpen(2000, DSOS_CREATE | DSOS_EXCL, 1);
    //fd = r;
    //sem_errors(r);
    //disastrOS_printStatus();

    //printf("Close sem. 2000\n");
    //r = disastrOS_semClose(fd);
    //sem_errors(r);
    //disastrOS_printStatus();


    int fds[MAX_NUM_SEMAPHORES+1];
    printf("Open %d semaphores (1 over limit)\n", MAX_NUM_SEMAPHORES+1);
    for (int i = 0; i < MAX_NUM_SEMAPHORES+1; ++i) {
        ret = disastrOS_semOpen(i, DSOS_CREATE | DSOS_EXCL, 1);
        fds[i] = ret;
        sem_errors(ret);
    }

    printf("Close %d semaphores (even the extra one)\n", MAX_NUM_SEMAPHORES+1);
    for (int i = 0; i < MAX_NUM_SEMAPHORES+1; ++i) {
        ret = disastrOS_semClose(fds[i]);
        sem_errors(ret);
    }

    printf("________________________________\n");


    printf("\e[36m[init]\e[0m Initialize sems with ids from 0 to %d, all at value 1\n", NRES-1);
    for (int i = 0; i < NRES; ++i) {
        ret = disastrOS_semOpen(i, DSOS_CREATE | DSOS_EXCL, 1);
        sem_errors(ret);
        Semaphores_fd[i] = ret;
    }

    disastrOS_printStatus();

    printf("\e[36m[init]\e[0m Creating %d children\n", CHILDREN);
    for (int i = 0; i < CHILDREN; ++i)
        disastrOS_spawn(main_child, 0);

    disastrOS_printStatus();

    int retval;
    int pid;
    printf("\e[36m[init]\e[0m Wait for all children\n");
    for (int i = 0; i < CHILDREN; ++i) {
        pid = disastrOS_wait(0, &retval);
        printf("\e[36m[init]\e[0m child %d returned with code: %d\n", pid, retval);
        disastrOS_printStatus();
    }

    disastrOS_printStatus();

    printf("\e[36m[init]\e[0m Close all sems.\n");
    for (int i = 0; i < NRES; ++i) {
        ret = disastrOS_semClose(Semaphores_fd[i]);
        sem_errors(ret);
    }

    disastrOS_printStatus();

    printf("SHUTDOWN!\n");
    disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;

  if (argc>1) {
    logfilename=argv[1];
  }

  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);

  return 0;
}