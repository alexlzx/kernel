/*kbd.h for keybarods */

#include <xeroskernel.h>

#ifndef KBD_H
#define KBD_H

#define ENTER 0x0A
#define EOT 0x04
#define BLOCK_PROC -3

int kbd_write(void* buff, int bufflen);
int kbd_read(int fd, void* buff, int bufflen);
int kbd_open(pcb* p, int fd);
int kbd_close(pcb* p, int fd);
int kbd_ioctl(unsigned long command,  unsigned int args);
void kbd_int_handler(void);
void reset_global (void);
unsigned char read_char(void);
void save_kchar(unsigned char value);
void save_user_buff(void);

#endif