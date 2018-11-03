/*    proc.h    */
#include <stdio.h>
#include <stdlib.h>

typedef int WORD;

#define PNREGS 5

#define NPROC 10

#define SP     0

/* state */
#define XFREE    0
#define XREADY   1
#define XRUN     2
#define XPEND    3

struct xentry  {
       int  xid;
       WORD xregs[PNREGS];    /* save SP */
       WORD xbase;
       WORD xlimit;
       int  xstate;
};

struct thd_struct{
   int thdid;
   int msg;
   struct thd_struct *next;
};
typedef struct thd_struct  PTHD_SLOT;
struct mailbox_struct{
   int msg;           // Record msg in mailbox.
   PTHD_SLOT *pthd_list_head;// Record head ptr of thread pending queue.
   PTHD_SLOT *pthd_list_end; // Record end ptr of thread pending queue
};
typedef struct mailbox_struct xthread_mbox_t;
// Define a mutex variable


#define STKSIZE 8192 

extern struct xentry xtab[];

extern int currxid;



