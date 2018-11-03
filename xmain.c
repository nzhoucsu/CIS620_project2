#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <proc.h>

#define SEND_NUM   1 //200
#define STOP_FLAG -100

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
// Define a mailbox variable
xthread_mbox_t mb; 
//
int xidfoo, xidbar;
int x=0;
//
int xidsnd1, xidsnd2,xidbrdc;
int xidrcv1, xidrcv2, xidrcv3, xidrcv4, xidrcv5, xidrcv6;
int usec;

// func 
int foo(int f);
int bar(int p, int q);
void snd(int aaa);
void rcv(int aaa);
void A();
void B();
void broadcast(xthread_mbox_t *mptr);
int xthread_init_mbox(xthread_mbox_t *mptr);
int xthread_send(xthread_mbox_t *mptr, int msg);
void xthread_recv(xthread_mbox_t *mptr, int *msgptr);
void xthread_recv1(xthread_mbox_t *mptr, int *msgptr);
int xthread_broadcast(xthread_mbox_t *mptr, int msg);


xmain_1(int argc, char* argv[])
{
   xidfoo = xthread_create(foo, 1, 7);
   xidbar = xthread_create(bar, 2, 32, 12);
}

xmain_2(int argc, char* argv[])
{
   // Initialize mailbox.
   if(xthread_init_mbox(&mb) == -1){
      printf("Creation of mailbox pointer mb fails!\n");
       printf("Program terminates!\n");
   } 

  
   usec = ualarm(0,0);// stop the timer interrupt during xthread_create
    xidrcv1 = xthread_create(A, 1, 1);
    xidsnd1  = xthread_create(B, 1, 0);
   ualarm(usec,0); //continue timer interrupt with remaining time

   /*another test case with one sender and two receivers  */
  // usec = ualarm(0,0);
  //  xidrcv3 = xthread_create(rcv, 1, 1);
  //  xidsnd2 = xthread_create(snd, 1, 0);
  //  xidrcv2 = xthread_create(rcv, 1, 2);
 //  ualarm(usec,0); 

}


xmain_3(int argc, char* argv[])
{
   // Initialize mailbox.
   if(xthread_init_mbox(&mb) == -1){
      printf("Creation of mailbox pointer mptr fails!\n");
      printf("Program terminates!\n");
     // return;
   }
   // Start six receiving threads.
   xidrcv1 = xthread_create(rcv, 1, 1);
   xidrcv2 = xthread_create(rcv, 1, 2);
   xidrcv3 = xthread_create(rcv, 1, 3);
   xidrcv4 = xthread_create(rcv, 1, 4);
   xidrcv5 = xthread_create(rcv, 1, 5);
   xidrcv6 = xthread_create(rcv, 1, 6);

   // Start a broadcasting thread.
   xidbrdc = xthread_create(broadcast, 1, &mb);
}

int foo(int f)
{
   int i,g,a;
   for(i=0;i<100;i++){
      printf("This is foo %d, %d\n", f, x++);
      for(g = 0; g < 5000000; g++){
         a = 1+1;              
      }
   } 
}


int bar(int p, int q)
{
   int j,h,b;
   for(j=0;j<100;j++){
      printf("This is bar %d, %d\n", p-q, x++);
      for(h = 0; h < 5000000; h++){
         b = 1+1;              
      }
   }
}


void A(int aaa){
   printf("A begings !\n");
   int msg;
   xthread_recv(&mb, &msg);
   printf("A got : %d\n", msg);
   printf("A ends\n");
      
}

void B(int aaa){
   printf("B begings !\n");
   xthread_send(&mb, 777);
   printf("B ends\n");
      
}


void snd(int aaa){   
   time_t t;
   int i, j, k, msg;
   int fal_count = 0;
   int suc_count = 0;

   // Set time variable for rand integer generation.
   srand((unsigned) time(&t));
   // Send msg.
   for(i=0; i<SEND_NUM; i++){
      // Create a msg.
      msg = rand()+1;
      // Send msg.
      if(xthread_send(&mb, msg) == -1){ // Sending fails.
         printf("!!! Sending #%d msg %d fails.\n", i, msg);
         fal_count ++;
      }
      else{// Sending successes.
         printf("+++ Sending #%d msg %d successes.\n", i, msg);
         suc_count ++;
      }
      // Wait for a while.
      for(j = 0; j < 5000000; j++){
         k = 1+1;              
      }
   }
   // Send STOP flag.
   i = -1;
   while (i==-1){
      i = xthread_send(&mb, STOP_FLAG);
      for(j = 0; j < 5000000; j++){
         k = 1+1;              
      }
   }
   // Return.
   printf("\nSending finishes. total: %d     success: %d     failure: %d\n", SEND_NUM, suc_count, fal_count);
   return;
}


void rcv(int aaa){
   int msg;
   int rcv_count = 0;

   while(1){
      // Wait and receive.
      xthread_recv(&mb, &msg);
      printf("--- rcv #%d receives msg %d\n", aaa, msg);
      rcv_count ++;
      if(msg == STOP_FLAG) // Receive STOP flag.
         break;
   }
   // Return.
   printf("rcv #%d finishes. Total receive: %d message (including stop signal).\n", aaa, rcv_count);   
   return;
}



void broadcast(xthread_mbox_t *mptr){
   PTHD_SLOT *p_slot;
   int msg[5] = {1, 2, 3, 4, STOP_FLAG}; // Broadcast sends STOP flag.
   int i, j, k;
   int brdc_rtn;

   for(i=0;i<5;i++){
      for(j = 0; j < 5000000*3; j++)
               k = 1+1;              
      p_slot = mptr->pthd_list_head;
      k = 0;
      while(p_slot){
         k++;
         p_slot = p_slot->next;
      }
      printf("\nThere are %d threads pending in queue\n", k);
      while(1){
         brdc_rtn = xthread_broadcast(mptr, msg[i]);
         if(brdc_rtn != -1){ // Broadcast successes.
            if(brdc_rtn == -100)
               printf("broadcast terminal signal: %d\n", msg[i]);
            else
               printf("broadcast: %d\n", msg[i]);
            break;
         }
         else{// Broadcast fails.
            for(j = 0; j < 5000000; j++){
                  k = 1+1;              
            }
         }            
      }
   }      
}

