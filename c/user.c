/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>
#include <tty.h>


int current_pid;
// 2.7.1 init program
static char *username = "cs415";
static char *password = "EveryonegetsanA";

void busy( void ) {
    int myPid;
    char buff[100];
    int i;
    int count = 0;
    
    myPid = sysgetpid();
    
    for (i = 0; i < 10; i++) {
        sprintf(buff, "My pid is %d\n", myPid);
        sysputs(buff);
        if (myPid == 2 && count == 1) syskill(3, 0);
        count++;
        sysyield();
    }
}

void producer( void ) {
    /****************************/
    
    int         i;
    char        buff[100];
    
    
    // Sping to get some cpu time
    for(i = 0; i < 100000; i++);
    
    syssleep(3000);
    for( i = 0; i < 20; i++ ) {
        
        sprintf(buff, "Producer %x and in hex %x %d\n", i+1, i, i+1);
        sysputs(buff);
        syssleep(1500);
        
    }
    for (i = 0; i < 15; i++) {
        sysputs("P");
        syssleep(1500);
    }
    sprintf(buff, "Producer finished\n");
    sysputs( buff );
    sysstop();
}

void consumer( void ) {
    /****************************/
    
    int         i;
    char        buff[100];
    
    for(i = 0; i < 50000; i++);
    syssleep(3000);
    for( i = 0; i < 10; i++ ) {
        sprintf(buff, "Consumer %d\n", i);
        sysputs( buff );
        syssleep(1500);
        sysyield();
    }
    
    for (i = 0; i < 40; i++) {
        sysputs("C");
        syssleep(700);
    }
    
    sprintf(buff, "Consumer finished\n");
    sysputs( buff );
    sysstop();
}

// --------------------------------- new added by Alex ---------------------------------
// Part 3 Testing
void signal_handler(void* c){
    char buffer[1024];
    sprintf(&buffer[0], "Handling the siganl......!\n");
    sysputs(buffer);
}
void syssighandler_test(void){
    unsigned long old_handler_byte = 64;
    void (**old_handler) (void*)  = (void (**) (void *))old_handler_byte;
    char buff[512];
    
    sysputs("syssighandler_test: the signal number is invalid .... \n");
    int ret = syssighandler(34, &signal_handler, old_handler);
    if (ret  == -1){
        sprintf(buff, "PASS: This is the invalid signal number %d\n",34);
    }
    else {
        sysputs("Unexpected Error!!!!!");
    }
    sysputs(buff);
}
// ---------------------------------
void syskill_test(void){
    char buff[512];
    sysputs("syskill_test: the current pid cannot kill idle pid 0 .... \n");
    int ret = syskill(0,6);
    if (ret == -1){
        sprintf(buff, "PASS: This is the invalid pid %d\n", 0);
    }
    else {
        sysputs("ERROR: This is nexpected!!!!!");
    }
    sysputs(buff);
}
// ---------------------------------
void process_A(void){
    sysputs("starting Process A ... \n");
    syssleep(10000);
    sysputs("terminating Process A ...\n");
}
void process_B(void){
    sysputs("starting Process B, going to wait until Process A terminates ... \n");
    int ret = syswait(current_pid);
    if (ret == -1){
        sysputs("PASS: Process A is not on the ready queue\n");
    }
    else {
        sysputs("ERROR: This is nexpected!!!!!");
    }
}
void syssigwait_test(void){
    int pid_a;
    pid_a = syscreate(&process_A, PROC_STACK);
    current_pid = pid_a;   // store the current pid in global variable
    syscreate(&process_B, PROC_STACK);
}
// ---------------------------------
void sysopen_test(void){
    char buff[512];
    sysputs("sysopen_test: the device number is invalid\n");
    int ret = sysopen(5);
    if (ret == -1){
        sprintf(buff, "PASS: this is the invalid device number %d\n", 5);
    }
    else{
        sysputs("ERROR: This is unexpected!!!!!");
    }
    sysputs(buff);
};
// ---------------------------------
void syswrite_test(void){
    int ret;
    char buff[512];
    sysputs("syswrite_test: the fd is invalid\n");
    ret = sysopen(KB_1);  // open device KB_1
    ret = syswrite(4, buff, 8);
    if (ret == -1){
        sprintf(buff, "PASS: this is invalid file descriptor %d\n", 4);
    }
    else {
        sysputs("ERROR: This is unexpected!!!!!");
    }
}
// ---------------------------------
void kill_handler(void){
    sysstop();
}
void foo_a(void){
    sysputs("A starts\n");  
    unsigned long old_handler_byte = 8;
    void (**old_handler) (void*)  = (void (**) (void *))old_handler_byte;
    int ret;
    ret = syssighandler(8, &signal_handler, old_handler);
    if (ret < 0){
        sysputs("sighandler 8 failed\n");
    }
    ret = syssighandler(9, &kill_handler, old_handler);
    if (ret < 0){
        sysputs("sighandler 9 failed\n");
    }  
    syssleep(6000);
    sysputs("ERROR: This is unexpected!!!!!\n");           
}
void foo_b(void){
    char buff[128];
    int result;
    sysputs("B starts\n");
    syssleep(2000);
    result = syskill(current_pid,8);
    sprintf(buff,"B: syskill result=%d\n",result);
    sysputs(buff);
}
void foo_c(void){
    char buff[128];
    int result;
    sysputs("C starts\n");
    syssleep(2000);  
    result = syskill(current_pid,9);
    sprintf(buff,"C: syskill result=%d\n",result);
    sysputs(buff);  
}
// ---------------------------------
void sysioctl_test(void){
    int ret, fd;
    sysputs("sysioctl_test: with invalid command\n");
    fd = sysopen(KB_1);
    ret = sysioctl(fd, 100, 200);
    if (ret == -1 ){
        sysputs("PASS: the commands are invalid\n");
    }
    else {
        sysputs("ERROR: This is unexpected!!!!!");
    }
}
// ---------------------------------
void sysread_test(void){
    int ret, fd; 
    char buff[512];

    sysputs("sysread_test(): more characters buffered in kernel than requests\n");
    char buffer[128];
    int bufflen = 5;

    fd = sysopen(KB_1);
    syssleep(10000);
    ret = sysread(fd, buff, bufflen);

    sysputs("\nReading 5 characters ... ");
    int i;
    for (i=0; i < bufflen; i++){
        sprintf(&buffer[i], "%c",buff[i]);
    }
    sysputs(buffer);
    sprintf(buffer,"This is sysread ret= %d", ret);
    sysputs(buffer);
}
// ---------------------------------
void other_test1(void){
    char buff[512];
    sysputs("other_test1: sysclose should close opened device\n");
    int fd = sysopen(KB_1);
    int ret = sysclose(fd);
    if (ret == 0){
        sysputs("PASS: successfulling close device \n");
    }
    else{
        sysputs("ERROR: This is unexpected!!!!!");
    }
    sysputs(buff);
}

void other_test2(void){
    int ret, fd; 
    char buff[16];

    sysputs("other_test2(): reading end of file (EOF)\n");
    char buffer[64];
    int bufflen = 128;

    fd = sysopen(KB_1);
    syssleep(5000);
    ret = sysread(fd, buff, bufflen);

    int i;
    for (i=0; i < bufflen; i++){
        sprintf(&buffer[i], "%c",buff[i]);
    }

    if (ret == 0){
        sysputs("PASS: reading EOF \n");
    }
    else{
        sysputs("ERROR: This is unexpected!!!!!");
    }

    sysputs(buffer);
}


// shell program
void shell(void){
    sysputs("Starting shell program.....");
}

void     root( void ) {
    /****************************/
    
    char  buff[100];
    int pids[5];
    int proc_pid, con_pid;
    int i;

    sysputs("Root has been called\n");
    
    int valid = TRUE;
    for (;;){
        int fd;
        int ret;
        int user_bufflen = 7;
        char user_buffer[user_bufflen]; // "cs415"
        int password_bufflen = 17;
        char password_buffer[password_bufflen]; // "EveryonegetsanA"
        int shell_pid;

        do{

            // 1. prints banner
            kprintf("*********** Welcome to Xeros - an experimental OS ***********\n");
         
            // 2. opens the keyboard
            fd = sysopen(KB_0);
         
            // 3. prints username
            sysputs("Username: ");
         
            // 4. Reads the username
            ret = sysread(fd, user_buffer, user_bufflen);
         
            // 5. Turns keyboard echoing off
            ret = sysioctl(fd, ECHO_OFF);
         
            // 6. prints password
            sysputs("Password: \n");
         
            // 7. reads the password
            ret = sysread(fd, password_buffer, password_bufflen);
         
            // 8. closes the keyboard
            ret = sysclose(fd);
         
            // 9. verifies the username and password
            if (strncmp(user_buffer, username, 7) == 0 && strncmp(password_buffer, password, 17) == 0){
                sysputs("SUCCESS: username and password are valid\n");
                valid = FALSE;
            }
            // 10. if fails, goes back to step 1
            else {
                sysputs("ERROR: Bad username/password. Please try again!!!\n");
            }
        } while (valid == TRUE);

        // 11. create shell program
        shell_pid = syscreate(&shell, PROC_STACK);

        // 12. wait for shell program to exit
        ret = syswait(shell_pid);

        // 13. go back to step 1 ...
    }
    
    
    // /*------------------------ testing prioritization of signal ----------------------*/
    // kprintf("\n");
    // kprintf("Test (1): \n");
    // int pid_a;
    // pid_a = syscreate(&foo_a, PROC_STACK);
    // current_pid = pid_a;
    // syscreate(&foo_b, PROC_STACK);
    // syscreate(&foo_c, PROC_STACK);
    // 
    // /*------------------------ testing syssighandler ----------------------*/
    // kprintf("Test (2): \n");
    // syssighandler_test();
    // kprintf("n");
    // 
    // /*------------------------ testing syskill ----------------------*/
    // kprintf("Test (3): \n");
    // //syskill_test();
    // kprintf("\n");
    // 
    // /*------------------------ testing syssigwait ----------------------*/
    // kprintf("Test (4): \n");
    // syssigwait_test();
    // kprintf("\n");
    // 
    // /*------------------------ testing sysopen ----------------------*/
    // kprintf("Test (5): \n");
    // sysopen_test();
    // kprintf("\n");
    // 
    // /*------------------------ testing syswrite ----------------------*/
    // kprintf("Test (6): \n");
    // syswrite_test();
    // kprintf("\n");

    // /*------------------------ testing sysioctl ----------------------*/
    // kprintf("Test (7): \n");
    // sysioctl_test();
    // kprintf("\n");

    // /*------------------------ testing sysread ----------------------*/
    // kprintf("Test (8): \n");
    // sysread_test();
    // kprintf("\n");

    // /*------------------------ testing other1 ----------------------*/
    // kprintf("Test (9): \n");
    // other_test1();
    // kprintf("\n");

    // /*------------------------ testing other2 ----------------------*/
    // kprintf("Test (10): \n");
    // other_test2();
    // kprintf("\n");
    
    
    

    
    sprintf(buff, "Root finished\n");
    sysputs( buff );
    sysstop();
    
    for( ;; ) {
        sysyield();
    }
    
}

