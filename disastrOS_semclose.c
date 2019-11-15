#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){
  int fd = running -> syscall_args[0];

  SemDescriptor *sem_desc = SemDescriptorList_byFd(&running -> sem_descriptors, fd);
  if (!sem_desc) {
      running -> syscall_retvalue = DSOS_EINVAL; 
      return;
  }

  sem_desc = (SemDescriptor*) List_detach(&running -> sem_descriptors, (ListItem*) sem_desc);
  assert(sem_desc);

  //I have now the descriptor!

  //from the descriptor I take the semaphore

  Semaphore *sem = sem_desc -> semaphore;
  SemDescriptorPtr *sem_desc_ptr = (SemDescriptorPtr*) List_detach(&sem -> descriptors, (ListItem*) (sem_desc -> ptr));
    
  assert(sem_desc_ptr);

  //we are closing semaphore, so free needed
  SemDescriptor_free(sem_desc);
  SemDescriptorPtr_free(sem_desc_ptr);

  //if semaphore was not used, I can use the List_detach to remove it by the system itself
  if (!sem -> descriptors.size) {
    sem = (Semaphore*) List_detach(&semaphores_list, (ListItem*) sem);
    assert(sem);
    Semaphore_free(sem);
   }

   running -> syscall_retvalue = 0;

}
