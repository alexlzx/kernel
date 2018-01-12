/* kbd.c */

#include <kbd.h>
// #include <scancodesToAscii.h>
#include <i386.h>
#include <stdarg.h>
#include <xeroskernel.h>
#include <xeroslib.h>
#include <tty.h>
#include "./scancodesToAscii.c"


unsigned char kdb_buff[4];
int echo = 0; // echo is on / off
int major;
int minor;
int EOF_flag;
static unsigned char VALUE;
unsigned int ascii_value;
int kbuff_head = -2;
int kbuff_tail = -1;
struct pcb* argP = NULL;
char* user_buffer = NULL;
int user_buff_len = 0;
int trans_count = 0;
unsigned char EOF_MASK = 0x04;
void (*call_back)(void*,int,int) = NULL;

int kbd_open(pcb* p,  int fd){
    if (fd == 0) {
        echo = 0;   // turn off echo
    } else if (fd ==1) {
        echo = 1;  // turn on echo
    }

    major = p->fd_table[fd].major;
    minor = p->fd_table[fd].minor;

    int i;
    for(i=0; i < (sizeof(kdb_buff)/ sizeof(kdb_buff[0])); i++){
        kdb_buff[i] = 0;
    }

	// enabling interrupts for keyboard controller
	// IRQ for keyboard controller is 1
    enable_irq(1, 0);
    return 0;
}

int kbd_close(pcb* p,  int fd){
	// disabling interrupts for keyboard controller
	// IRQ for keyboard controller is 1
	enable_irq(1,1);
    reset_global();
	return 0; 
}

int kbd_write(void* buff, int bufflen){
	return -1;
}


/* 
 Input:		buff - user provided buffer location to write to
 			bufflen - user provided length to read
 			cb_func - call back function to unblock process if read finish
 			currP - parameter to call back function
 			count - number of chars read successfully so far
 Output:	0 - if EOF key was pressed
 			user_buff_len - if read is finished
 			BLOCK_PROC - if read did not finish yet
 			-1 - other errors 	
*/

int kbd_read(int fd, void* buff, int bufflen){
    trans_count = 0;

    if(user_buffer == NULL){
        user_buffer = buff;
        user_buff_len = bufflen;
    }
	
    if(EOF_flag){
        return 0;
    }

    int left_to_read = user_buff_len - trans_count;
    int i;
    for (i = 0; i < left_to_read; i++){
        if(kbuff_head != -2 && kbuff_tail != -1){
            user_buffer[trans_count] = kdb_buff[kbuff_head];
            trans_count++;
            kbuff_head ++;
            kbuff_head = (kbuff_head)%KKB_BUFFER_SIZE;
        }

        if(kbuff_head == kbuff_tail){
            kbuff_head = -2;
            kbuff_tail = -1;
        }
    }

    if(trans_count == user_buff_len){
        trans_count = 0;
        return user_buff_len;
    }
    else if (trans_count < user_buff_len){
        return 0;
    }

    return -1;
}

int kbd_ioctl(unsigned long command,  unsigned int args){
	va_list arg = (va_list) args;
    unsigned long * outc;

	switch(command){
		case SET_EOF :
			outc = (unsigned long*) va_arg(arg, unsigned int);
            char temp = *outc;
            *outc = EOF_MASK;
            EOF_MASK = temp;
            return 0;
		case ECHO_OFF :
			minor = KB_0;
			break;
		case ECHO_ON :
			minor = KB_1;
			break;
		default :
			return -1;
	}	
}


void kbd_int_handler(){

    /* read from keyboard input port*/
    unsigned char in_value = read_char();
    /* if no EOF key detected*/
    if(!EOF_flag){

        /* if it's not key up*/
        if(in_value != NOCHAR && in_value != 0){

            /*if is echo version, print*/
            if(minor == KB_1){
                kprintf("%c",in_value);
            }

            /* if there is process wating*/
            if(user_buffer != NULL){            
                /* write character to user buffer*/
                user_buffer[trans_count] = (char)in_value;
                /* increment counter*/
                trans_count++;
                
                /*check if read is complete*/
                if(trans_count == user_buff_len){
                    /* reset counter and user buffer*/
                    trans_count = 0;
                    user_buffer = NULL;
                    /*unblock process*/             
                    call_back(argP,user_buff_len, trans_count);

                }
                /* if ENTER detected */
                if(in_value == ENTER){
                    /* saves the current count number*/
                    int tmp = trans_count;
                    /* reset counter and user buffer */
                    trans_count = 0;
                    user_buffer = NULL;
                    /* unblock process */
                    call_back(argP,tmp, trans_count);
                }       
            }
            else{
            /*block queue is empty,save to kernel buffer*/
                save_kchar(in_value);
            }
        }
    }
    else{/* EOF key is detected, return with 0*/
        if(call_back != NULL){
            call_back(argP,0, trans_count);         
        }
    }
}


extern unsigned char read_char(void){
    __asm __volatile( " \
        pusha  \n\
        in  $0x60, %%al  \n\
        movb %%al, VALUE  \n\
        popa  \n\
            "
        :
        : 
        : "%eax"
        );  
    ascii_value = kbtoa(VALUE); 
    if(ascii_value == EOT || ascii_value == EOF_MASK){
        EOF_flag = TRUE;

    }

    return ascii_value;
}


void save_kchar(unsigned char value){

    // if kernel buff is empty
    if(kbuff_head == -2 && kbuff_tail == -1){
        kbuff_head = 0;
        kbuff_tail = 0;
        kkb_buffer[kbuff_tail]=value;
        kbuff_tail++;
    }
    // if kernel buffer is not full yet,add value
    else if(kbuff_head != kbuff_tail){
        kkb_buffer[kbuff_tail] = value;
        kbuff_tail++;
        kbuff_tail = (kbuff_tail) % KKB_BUFFER_SIZE;
    }

}

void save_user_buff(void){
    /*get how much more the process want to read*/
    int left_to_read = user_buff_len - trans_count;
    int i;
    /* for each desire byte to read*/
    for (i = 0; i < left_to_read; i++){
        /* if kernel buffer is not empty*/
        if(kbuff_head != -2 && kbuff_tail != -1){
            /* copy the value from kernel buffer to user buffer*/
            user_buffer[trans_count] = kkb_buffer[kbuff_head];
            /* increment user buffer counter*/
            trans_count++;
            /* increment kernel head index */
            kbuff_head ++;
            kbuff_head = (kbuff_head)%KKB_BUFFER_SIZE;
        }
        /* if kernel buffer is empty , reset head and tail index*/
        if(kbuff_head == kbuff_tail){
            kbuff_head = -2;
            kbuff_tail = -1;
        }
    }
}


void reset_global (void){
    minor = -1;
    major = -1;
    EOF_flag = FALSE;
    kbuff_head = -2;
    kbuff_tail = -1;

    call_back = NULL;
    argP = NULL;

    user_buffer = NULL;
    user_buff_len = 0;
    trans_count = 0;
}