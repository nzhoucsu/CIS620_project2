#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <proc.h>

extern pthread_mutex_t m;

// Define xthread_init_mbox()
int xthread_init_mbox(xthread_mbox_t *mptr){
   mptr = (xthread_mbox_t *)malloc(sizeof(xthread_mbox_t));
   if(mptr == NULL)
      return -1;
   mptr->msg = -1; // msg <0 means no msg in mailbox.
   mptr->pthd_list_head = NULL;
   mptr->pthd_list_end  = NULL;
   return 0;
}


// Define xthread_send()
int xthread_send(xthread_mbox_t *mptr, int msg){
   PTHD_SLOT *p_slot;
   pthread_mutex_lock(&m);
   /* Mailbox is full, return. */
   if(mptr->msg > 0){ // By default, msg is a positive integer.
      pthread_mutex_unlock(&m);
      return -1;
   }
   /* Mailbox is not full. */
   // Update mailbox.
   mptr->msg = msg;
   /* Check if threads are pending. */
   p_slot = mptr->pthd_list_head;
   while(p_slot){
      if(p_slot->msg == -1){ // This thread is still starving
                             // and in pending status.
         // Assign mailbox msg to this thread.
         p_slot->msg = mptr->msg;
         // Update mailbox. msg in mailbox is cleared up.
         mptr->msg = -1; 
         // This pending thread is reset to ready status.
         xtab[p_slot->thdid].xstate = XREADY;
         break;
      }
      else{ // This thread has received msg.
            // It is waiting to reschedule to run.
         p_slot = p_slot->next;
      }
   }
   // Success, return.
   pthread_mutex_unlock(&m);
   return 0;
}


// Define xthread_recv()
void xthread_recv(xthread_mbox_t *mptr, int *msgptr){
   PTHD_SLOT *p_slot;
   PTHD_SLOT *pre_p_slot;
   pthread_mutex_lock(&m);
   /* Check if mailbox is full. */
   if(mptr->msg > 0){ // Mailbox is full.
      *msgptr = mptr->msg;
      // Update mailbox;
      mptr->msg = -1;
      pthread_mutex_unlock(&m);
   
      return;
   }
   else{ // Mailbox is not full.
      // Create a pending slot for pending queue.
      p_slot = (PTHD_SLOT *)malloc(sizeof(PTHD_SLOT *));
      p_slot->thdid = currxid;
 
      p_slot->msg   = -1;
      p_slot->next  = NULL;
      // Update pending queue.
      if(mptr->pthd_list_head == NULL){ // Pending queue is empty.
         mptr->pthd_list_head = p_slot;
         mptr->pthd_list_end  = mptr->pthd_list_head;
      }
      else{ // Pending queue is not empty.
            mptr->pthd_list_end->next = p_slot;
            mptr->pthd_list_end = mptr->pthd_list_end->next;
      }
      // Update status of this thread.
      xtab[currxid].xstate = XPEND;
      resched(); 
      pthread_mutex_unlock(&m);
    
      while(1){
         if(xtab[currxid].xstate == XRUN){
            // Acquire msg corresponding to this thread from ready queue.
       
            pthread_mutex_lock(&m);
            p_slot = mptr->pthd_list_head;
            while(p_slot){
               if(p_slot->thdid == currxid){ 
                  // Acquire msg corresponding to this thread.
                  if(p_slot->msg <= 0)
                     printf("Wrong msg in ready thread!\n");
                  *msgptr = p_slot->msg;
                 
                  // Update pending queue.
                  if(p_slot == mptr->pthd_list_head){
                     mptr->pthd_list_head = mptr->pthd_list_head->next;
                     if(mptr->pthd_list_head == NULL)
                        mptr->pthd_list_end = mptr->pthd_list_head;
                  }
                  else if(p_slot == mptr->pthd_list_end){
                     mptr->pthd_list_end = pre_p_slot;
                  }
                  else{
                     pre_p_slot->next = p_slot->next;
                  }
                  free(p_slot);
                  pthread_mutex_unlock(&m);
                  return;
               }
               pre_p_slot = p_slot;
               p_slot = p_slot->next;
            }
            if(pre_p_slot->thdid != currxid)
               printf("Error: current thread is not in pending queue\n");
            pthread_mutex_unlock(&m);
         }
      }
   }
}


// Define xthread_broadcast()
int xthread_broadcast(xthread_mbox_t *mptr, int msg){
   PTHD_SLOT *p_slot, *pp_slot;
   pthread_mutex_lock(&m);
   /* Mailbox is full, return. */
   if(mptr->msg > 0){ 
      // Error, return.
      pthread_mutex_unlock(&m);
      return -1;
   }
   /* Mailbox is not full. Check if threads are pending. */
   if(mptr->pthd_list_head != NULL){ // One or more threads are in pending queue.
      // Send msg to all pending threads and set all pending threads into ready status.
      p_slot = mptr->pthd_list_head;
      while(p_slot){
         if(p_slot->msg < 0){
            p_slot->msg = msg;
            xtab[p_slot->thdid].xstate = XREADY;
         }
         pp_slot = p_slot->next;
         p_slot = pp_slot;         
      }
   }
   else{ // No thread is in pending queue. 
      // Update mailbox.
      mptr->msg = msg;
   }
   // Success, return.
   pthread_mutex_unlock(&m);
   return msg;
}
