#include <stdio.h>
#include <proc.h>
#include <pthread.h>
#include <signal.h>

extern void xmain_1();
extern void xmain_2();
extern void xmain_3();

struct xentry xtab[10]; 
int currxid = 0; 

void error_print(){
   printf("\nWrong input! Program terminates\n");
   printf("Input Example:\n");
   printf("./a.out 1 --- This input will run part 1\n");
   printf("./a.out 2 --- This input will run part 2 (send and receive)\n");
   printf("./a.out 3 --- This input will run part 2 (broadcast and receive)\n\n");
}

void catcher(){

   sigset_t sigset;//clean
   sigemptyset(&sigset);
   sigaddset(&sigset, SIGALRM);
   sigprocmask(SIG_UNBLOCK, &sigset, NULL);
   if(xtab[currxid].xstate != XPEND) 
      xtab[currxid].xstate = XREADY;
   resched();
}

void main(int argc, char *argv[])
{
   if(argc < 2){
      error_print();
      return;
   }

   if(atoi(argv[1]) != 1 && atoi(argv[1]) != 2 && atoi(argv[1]) != 3){
      error_print();
      return;
   }

   register struct xentry *xptr;
   struct xentry m;
   int i;
   int xidxmain;

   for(i=0 ; i < NPROC; i++){
      xptr = &xtab[i];
      xptr->xid = i;
      xptr->xlimit =  (WORD) malloc(STKSIZE);
      xptr->xbase = xptr->xlimit + STKSIZE - sizeof(WORD);
      xptr->xstate = XFREE;
   }

   signal(SIGALRM, catcher);
   ualarm(10000, 0);
   /* the first thread runs user's xmain with id 0*/
   if(atoi(argv[1]) == 1)
      xidxmain = xthread_create(xmain_1, 2, argc, argv);
   else if(atoi(argv[1]) == 2)
      xidxmain = xthread_create(xmain_2, 2, argc, argv);
   else
      xidxmain = xthread_create(xmain_3, 2, argc, argv);
   xtab[xidxmain].xstate = XRUN; 
   ctxsw(m.xregs, xtab[xidxmain].xregs);
   /* never be here */
}


