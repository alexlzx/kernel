/* di_calls.c */

#include <xeroskernel.h>
#include <xeroslib.h>
#include <tty.h>

/* Your code goes here */


// --------------------------------- new added by Alex ---------------------------------

// return -1: if fails
// return fd (0 - 3): if success 
int di_open(pcb *p, int device_no){
    // check if device_no is valid
    if (device_no < 0 || device_no >= DEVICE_TABLE_SIZE){
        return -1;
    }

    // get the device major number
    int fd = devtab[device_no].dvnum;
    if (fd < 0 || fd >= FD_TABLE_SIZE){
        return -1;
    }



    // initialize the file descriptor table entry for device_no
    p->fd_table[fd].major = devtab[device_no].dvnum;
    p->fd_table[fd].minor = devtab[device_no].dvminor;   
    p->fd_table[fd].dev_block_table = &devtab[device_no];
    p->fd_table[fd].control_data = NULL; 
    p->fd_table[fd].status = TRUE; 
    
    struct devsw *devptr;
    devptr = p->fd_table[fd].dev_block_table; 
    devptr->dvopen(p, fd);

    //kprintf("I'm at di_open(): this is the fd: %d\n", fd);
    return fd;
}

// return 0 for success
// return -1 for failure
int di_close(pcb *p, int fd){
    if (fd < 0 || fd > 3){
        return -1;
    }

    if (p->fd_table[fd].status == TRUE){
        if (p->fd_table[fd].major == fd){
                p->fd_table[fd].status = FALSE;
                struct devsw *devptr;
                devptr = p->fd_table[fd].dev_block_table; 
                devptr->dvclose(p, fd);                
                return 0;
        }   
    }

    return -1; 

}

// return -1 if there's an error
// return # of bytes written (may be less tan bufflen)
int di_write(pcb *p, int fd, void* buff, int bufflen){
    //kprintf("I'm here at di_write ... \n");
    if (fd < 0 || fd >= FD_TABLE_SIZE){
        return -1;
    }

    // if the device is not open
    if (p->fd_table[fd].status == FALSE){
        return -1;
    }

    struct devsw *devptr;
    devptr = p->fd_table[fd].dev_block_table;
    return devptr->dvwrite(buff, bufflen);
}

// return -1 if there's an error
// return # of bytes read
// return 0: indicate end-of-file (EOF)
int di_read(pcb *p, int fd, void* buff, int bufflen){
    if (fd < 0 || fd >= DEVICE_TABLE_SIZE) {
        return -1;
    }

    int i, found = 0;
    for (i = 0; i < FD_TABLE_SIZE; i++) {
        if (p->fd_table[i].major == fd) {
            found = i;
        }
    }

    if (found == 0) {
        return -1;
    }
    else {
        struct devsw *devptr;
        devptr = p->fd_table[fd].dev_block_table;
        return devptr->dvread(fd, buff, bufflen);
    }
}


//e.g. ioctl(d, DEV_BUFFER_CONFIG, 65536, ASYNC); 
//e.g. ioctl(d, SET_VOLUME, 10); 
// return -1 if there's an error 
// otherwise return 0
int di_ioctl(pcb* p, int fd, unsigned long command,  unsigned int args){
    if (fd < 0 || fd > 3){
        return -1;
    }

    if (p->fd_table[fd].status == FALSE){
        return -1;
    }

    struct devsw *devptr;
    devptr = p->fd_table[fd].dev_block_table;

    return devptr->dvcntl(command, args);;
}


