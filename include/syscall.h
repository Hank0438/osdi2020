#define __NR_syscalls	    8

#define SYS_CORE_TIMER_IRQ_ENABLE   0
#define SYS_GET_TASKID              1
#define SYS_EXIT                    2
#define SYS_FORK                    3    
#define SYS_UART_READ               4
#define SYS_UART_WRITE              5   
#define SYS_KILL                    6
#define SYS_EXEC                    7
#define SYS_PRINTF                  8

#ifndef __ASSEMBLER__
unsigned long call_sys_get_taskid();
unsigned long exec(void(*func)());
int fork();
unsigned long exit();
char call_sys_uart_read();
void call_sys_uart_write(char *s);
void call_sys_printf(char *s);
#endif