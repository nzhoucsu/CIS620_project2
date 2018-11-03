/* resched.c  -  resched */
#include <stdio.h>
#include <proc.h>

/*------------------------------------------------------------------------
 * resched  --  find a live thread to run
 *
 *------------------------------------------------------------------------
 */
void resched()
{
    register struct  xentry  *cptr;  /* pointer to old thread entry */
    register struct  xentry  *xptr;  /* pointer to new thread entry */
    int i,next;

    cptr = &xtab[currxid];

    next = currxid ;
    for(i=0; i<NPROC; i++) { 
        if( (++next) >= NPROC)
             next = 0; 
        if(xtab[next].xstate == XREADY) {
            xtab[next].xstate = XRUN;
            xptr = &xtab[next];
            currxid = next;            
            ualarm(10000,0);//before move to another thread, should set timer for the next thread.
            ctxsw(cptr->xregs,xptr->xregs);
            return;
        }
    }

    for(i=0; i<NPROC; i++) { 
        if(xtab[i].xstate == XPEND)
            return;
    }
    
    printf("XT: no threads to run!\n");
    exit(0);
}


