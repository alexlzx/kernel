/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */

#ifndef XEROSKERNEL_H
#define XEROSKERNEL_H

/* Symbolic constants used throughout Xinu */

typedef    char    Bool;        /* Boolean type                  */
typedef unsigned int size_t; /* Something that can hold the value of
                              * theoretical maximum number of bytes
                              * addressable in this architecture.
                              */
#define    FALSE   0       /* Boolean constants             */
#define    TRUE    1
#define    EMPTY   (-1)    /* an illegal gpq                */
#define    NULL    0       /* Null pointer for linked lists */
#define    NULLCH '\0'     /* The null character            */

#define CREATE_FAILURE -1  /* Process creation failed     */



/* Universal return constants */

#define    OK            1         /* system call ok               */
#define    SYSERR       -1         /* system call failed           */
#define    EOF          -2         /* End-of-file (usu. from read)    */
#define    TIMEOUT      -3         /* time out  (usu. recvtim)     */
#define    INTRMSG      -4         /* keyboard "intr" key pressed    */
/*  (usu. defined as ^B)        */
#define    BLOCKERR     -5         /* non-blocking op would block  */

/* Functions defined by startup code */


void           bzero(void *base, int cnt);
void           bcopy(const void *src, void *dest, unsigned int n);
void           disable(void);
unsigned short getCS(void);
unsigned char  inb(unsigned int);
void           init8259(void);
int            kprintf(char * fmt, ...);
void           lidt(void);
void           outb(unsigned int, unsigned char);


/* Some constants involved with process creation and managment */

/* Maximum number of processes */
#define MAX_PROC        64
/* Kernel trap number          */
#define KERNEL_INT      80
/* Interrupt number for the timer */
#define TIMER_INT      (TIMER_IRQ + 32)
/* Minimum size of a stack when a process is created */
#define PROC_STACK      (4096 * 4)

/* Number of milliseconds in a tick */
#define MILLISECONDS_TICK 10


/* Constants to track states that a process is in */
#define STATE_STOPPED   0
#define STATE_READY     1
#define STATE_SLEEP     22
#define STATE_RUNNING   23

/* System call identifiers */
#define SYS_STOP        10
#define SYS_YIELD       11
#define SYS_CREATE      22
#define SYS_TIMER       33
#define SYS_GETPID      144
#define SYS_PUTS        155
#define SYS_SLEEP       166
#define SYS_KILL        177
#define SYS_CPUTIMES    178

// ---------------------------- new added by Alex ----------------------------
#define SIGNAL_TABLE_SIZE  32
#define DEVICE_TABLE_SIZE  2 // only 2 devices
#define FD_TABLE_SIZE      4 // 0 - 3 inclusive
#define BUFFER_SIZE 32
#define KKB_BUFFER_SIZE 4
#define KEYBOARD_INT      (1 + 32)

// for syscall.c
#define SYS_SIGHANDLER  200
#define SYS_RETURN      210
#define SYS_WAIT        220   
#define SYS_OPEN        230
#define SYS_CLOSE       240
#define SYS_WRITE       250
#define SYS_READ        260
#define SYS_IOCTL       270


/* Structure to track the information associated with a single process */



// ---------------------------- new added by Alex ----------------------------
// file decriptor structure
struct struct_fd{
    int major;
    int minor;
    struct devsw *dev_block_table; // pointer to device block table 
    void *control_data;   	// control_data returned by device driver
    int status;				// device status
};
typedef struct struct_fd fd;



typedef struct struct_pcb pcb;
struct struct_pcb {
    void        *esp;    /* Pointer to top of saved stack           */
    pcb         *next;   /* Next process in the list, if applicable */
    pcb         *prev;   /* Previous proccess in list, if applicable*/
    int          state;  /* State the process is in, see above      */
    unsigned int pid;    /* The process's ID                        */
    int          ret;    /* Return value of system call             */
    /* if process interrupted because of system*/
    /* call                                    */
    long         args;
    unsigned int otherpid;
    void        *buffer;
    int          bufferlen;
    int          sleepdiff;
    long         cpuTime;  /* CPU time consumed                     */

    // ---------------------------- new added by Alex ----------------------------
    // for signal
    void (*signal_table[SIGNAL_TABLE_SIZE]) (void*);
    int sig_status[SIGNAL_TABLE_SIZE];

    // for device
    fd fd_table[FD_TABLE_SIZE];
};


typedef struct struct_ps processStatuses;
struct struct_ps {
    int  entries;            // Last entry used in the table
    int  pid[MAX_PROC];      // The process ID
    int  status[MAX_PROC];   // The process status
    long  cpuTime[MAX_PROC]; // CPU time used in milliseconds
};


/* The actual space is set aside in create.c */
extern pcb     proctab[MAX_PROC];

#pragma pack(1)

/* What the set of pushed registers looks like on the stack */
typedef struct context_frame {
    unsigned long        edi;
    unsigned long        esi;
    unsigned long        ebp;
    unsigned long        esp;
    unsigned long        ebx;
    unsigned long        edx;
    unsigned long        ecx;
    unsigned long        eax;
    unsigned long        iret_eip;
    unsigned long        iret_cs;
    unsigned long        eflags;
    unsigned long        stackSlots[];
} context_frame;



// ---------------------------- new added by Alex ----------------------------
// Device structure
struct devsw {
    int dvnum;
    int (*dvopen)(void*, int);
    int (*dvclose)(void*, int);
    int (*dvread)(int, void*, int);
    int (*dvwrite)(void*, int);
    int (*dvcntl)(unsigned long, unsigned int);
    int dvminor;
};
// device table
struct devsw devtab[DEVICE_TABLE_SIZE];

unsigned int kkb_buffer[KKB_BUFFER_SIZE];


/* Memory mangement system functions, it is OK for user level   */
/* processes to call these.                                     */

int      kfree(void *ptr);
void     kmeminit( void );
void     *kmalloc( size_t );


/* A typedef for the signature of the function passed to syscreate */
typedef void    (*funcptr)(void);


/* Internal functions for the kernel, applications must never  */
/* call these.                                                 */
void     dispatch( void );
void     dispatchinit( void );
void     ready( pcb *p );
pcb      *next( void );
void     contextinit( void );
int      contextswitch( pcb *p );
int      create( funcptr fp, size_t stack );
void     set_evec(unsigned int xnum, unsigned long handler);
void     printCF (void * stack);  /* print the call frame */
int      syscall(int call, ...);  /* Used in the system call stub */
void     sleep(pcb *, unsigned int);
void     removeFromSleep(pcb * p);
void     tick( void );
int      getCPUtimes(pcb *p, processStatuses *ps);

// ---------------------------- new added by Alex ----------------------------
void     removeFromReady(pcb * p);
pcb      *findPCB( int pid );
void     signal_helper(pcb* p, int real_sig_no);
int      isSignal(pcb* p);

/* Function prototypes for system calls as called by the application */
int          syscreate( funcptr fp, size_t stack );
void         sysyield( void );
void         sysstop( void );
unsigned int sysgetpid( void );
unsigned int syssleep(unsigned int);
void         sysputs(char *str);
int          sysgetcputimes(processStatuses *ps);


// ---------------------------- new added by Alex ----------------------------
// signal.c
int          signalhandler(pcb *p, int signalNumber, void (*newhandler)(void *), void (** oldHandler)(void *));
int          signal(int PID, int signalNumber);
void         sigtramp(void (*handler) (void *), void *cntx);
int          proc_to_wait(pcb *calling_pcb, int targeted_PID);

// di_calls.c
int          di_open(pcb *p, int device_no);
int          di_close(pcb *p, int fd);
int          di_write(pcb *p, int fd, void* buff, int bufflen);
int          di_read(pcb *p, int fd, void* buff, int bufflen);
int          di_ioctl(pcb* p, int fd, unsigned long command, unsigned int args);

// syscall.c ---- for signal
int          syskill(int PID, int signalNumber);
void         syssigreturn(void *old_sp);
int          syswait(int PID);
int          syssighandler(int signal, void (*newhandler)(void *), void (** oldHandler)(void *));

// syscall.c ---- for device
int          sysopen(int device_no);
int          sysclose(int fd);
int          syswrite(int fd, void *buff, int bufflen);
int          sysread(int fd, void*buff, int bufflen);
int          sysioctl(int fd, unsigned long command, ...);






/* The initial process that the system creates and schedules */
void     root( void );


void           set_evec(unsigned int xnum, unsigned long handler);


/* Anything you add must be between the #define and this comment */
#endif

