#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semPost(){
	int fd = running -> syscall_args[0]; 

	SemDescriptor *sem_desc = SemDescriptorList_byFd(&running -> sem_descriptors, fd);
    
    //validation of sem_desc
	if (!sem_desc) {
        running -> syscall_retvalue = DSOS_EINVAL; 
        return;
    }

   
     
    // process blocked in a sem_wait call will be woken
    if (sem_desc -> semaphore -> count == 0 && sem_desc -> semaphore -> waiting_descriptors.size) {
        SemDescriptorPtr *sem_desc_ptr = (SemDescriptorPtr*) List_detach(&sem_desc -> semaphore -> waiting_descriptors, sem_desc -> semaphore -> waiting_descriptors.first);
        
        //taking PCB of process that was in waiting list
        PCB *resume = sem_desc_ptr -> descriptor -> pcb;
        resume = (PCB*) List_detach(&waiting_list, (ListItem*) resume);
        resume -> status = Ready;
        //So the process is now ready 
        List_insert(&ready_list, ready_list.last, (ListItem*) resume);

        SemDescriptorPtr_free(sem_desc_ptr);
        running -> syscall_retvalue =  0;
        return;
    }
  
}
