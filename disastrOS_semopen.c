#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semOpen(){
	int id = running -> syscall_args[0];
    int flag = running -> syscall_args[1];
    int value = running -> syscall_args[2];
    
    
    //taking semaphore
    Semaphore *sem = SemaphoreList_byId(&semaphores_list, id);

    if (flag & DSOS_CREATE) {
        if (sem) {
            if (flag & DSOS_EXCL) {
                running -> syscall_retvalue = DSOS_EEXIST; 
                return;
                //semaphore is here
            }
        } else {
            if (semaphores_list.size < MAX_NUM_SEMAPHORES) {
                sem = Semaphore_alloc(id, value);
                //allocating and inserting semaphore in semaphore_list
                List_insert(&semaphores_list, semaphores_list.last, (ListItem*) sem);
            } else {
                running -> syscall_retvalue = DSOS_EMFILE;
                return;
            }
        }

    }

    //I have to check if I've actually a semaphore

    if (!sem) {
        running ->  syscall_retvalue = DSOS_ERESOURCEOPEN;
        return;
    }

    if (running -> sem_descriptors.size == MAX_NUM_SEMDESCRIPTORS_PER_PROCESS) {
    	//checking for descriptors: if we are full: EMFILE
    	//(too many opened)
        running -> syscall_retvalue = DSOS_EMFILE;
        return;
    }

    //Now I can create descriptor

    SemDescriptor *sem_desc = SemDescriptor_alloc(running -> last_sem_fd, sem, running);
    if (!sem_desc) {
         running->syscall_retvalue=DSOS_ERESOURCENOFD;
         return;
    }
    

    //update last_sem_fd
    running -> last_sem_fd++;
    
    SemDescriptorPtr *sem_desc_ptr = SemDescriptorPtr_alloc(sem_desc);
    
    List_insert(&running -> sem_descriptors, running -> sem_descriptors.last, (ListItem*) sem_desc);

    sem_desc-> ptr = sem_desc_ptr;
    List_insert(&sem -> descriptors, sem -> descriptors.last, (ListItem*) sem_desc_ptr);

    running -> syscall_retvalue = sem_desc -> fd;



  
}
