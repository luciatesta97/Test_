#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){
  int fd = running -> syscall_args[0];
  SemDescriptor *sem_desc = SemDescriptorList_byFd(&running -> sem_descriptors, fd);

  //check if there's sem_desc
  if (!sem_desc) {
    running -> syscall_retvalue = DSOS_EINVAL; 
    return;
    }  

  //If the semaphore currently has the value zero, then the call blocks 

  if (sem_desc -> semaphore -> count == 0) {
        running -> syscall_retvalue = 0;
        //turn process status in waiting
        running -> status = Waiting;
        //insert the process in waiting list
        List_insert(&(waiting_list), waiting_list.last, (ListItem*) running);
        
        SemDescriptorPtr *sem_desc_ptr = SemDescriptorPtr_alloc(sem_desc);

        sem_desc_ptr = (SemDescriptorPtr*) List_insert(&(sem_desc -> semaphore -> waiting_descriptors), sem_desc -> semaphore -> waiting_descriptors.last, (ListItem*) sem_desc_ptr); 
        assert(sem_desc_ptr);

        
        //Take ready process to be sheduled 
		running = (PCB*) List_detach(&ready_list, ready_list.first);
        running -> status = Running;
        return;
    }
}
