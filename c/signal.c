/* signal.c - support for signal handling
   This file is not used until Assignment 3
 */

#include <xeroskernel.h>
#include <xeroslib.h>

/* Your code goes here */



void sigtramp(void (*handler) (void *), void *cntx){
    handler(cntx);
    syssigreturn(cntx);  // get old stack pointer value from cntx
}


// return 0 on success
// return -1 if signal is invalid, or attempt to change signal 31 
// return -2 if newhandler is invalid
int signalhandler(pcb *p, int signalNumber, void (*newhandler)(void *), void (** oldHandler)(void *)) {
    if (signalNumber < 0 || signalNumber > 31){
        return -1;
    }

    if (newhandler == NULL) {
        return -2;
    }

    // remember a previously installed handler
    *oldHandler = p->signal_table[signalNumber];
    p->signal_table[signalNumber] = newhandler;

    return 0;
}

// TODO: --------
// return 0 if success
// return -1 if PID is invalid
// return -2 if signalNumber is invalid
// if targeted process is blocked, then targeted process is unblocked and return -99

int signal(int PID, int signalNumber){
    if (signalNumber < 0 || signalNumber > 31){
        return -2;
    }

    pcb *targeted_pcb = findPCB(PID);
    if(targeted_pcb == NULL){
        return -1;
    }

    return 0;
}

// ---------------------------------- new added by Alex ---------------------------------- 
// make the calling process to wait until PID terminate
// put current process in ready list
// return -1: no such process is waiting for the signal
// return 0: if the call terminates normally
int proc_to_wait(pcb *calling_pcb, int targeted_PID){
    // find the process from targeted_PID
    pcb *targeted_pcb = findPCB(targeted_PID);
    if(targeted_pcb == NULL){
        return -1;
    }

    if (targeted_pcb->state != STATE_READY){
        return -1;
    }

    removeFromReady(calling_pcb);
    ready(targeted_pcb);
    return 0;
    

}




