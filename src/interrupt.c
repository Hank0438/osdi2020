#include "../include/gpio.h"
#include "../include/uart.h"
#include "../include/peripheral.h"
#include "../include/task.h"

unsigned int CORE_TIMER_COUNT = 0;
unsigned int LOCAL_TIMER_COUNT = 0;

void debug(unsigned long fp){
    uart_puts("debugggggg!!\n");
    unsigned long elr, sp_el0, sp;
    asm volatile ("mrs %0, elr_el1" : "=r"(elr));
    asm volatile ("mrs %0, sp_el0" : "=r"(sp_el0));
    asm volatile ("mov %0, sp" : "=r"(sp));
    uart_puts("\n\t  ELR_EL1: 0x");
    uart_hex(elr);
    uart_puts("\n\t  SP_EL0: 0x");
    uart_hex(sp_el0);
    uart_puts("\n\t  SP: 0x");
    uart_hex(sp);
    uart_puts("\n\t  FP: 0x");
    uart_hex(fp);
    uart_puts("\n");
}

void disable_irq() 
{
    asm volatile("msr daifset, 0xf");
}

void enable_irq() 
{
    asm volatile("msr daifclr, 0xf");
}

void set_HCR_EL2_IMO()
{
    //asm volatile("mov x0, #(1 << 4)");
    //asm volatile("msr hcr_el2, x0");
    asm volatile("mrs x0, hcr_el2");
    asm volatile("orr x0, x0, #16");
    asm volatile("msr hcr_el2, x0");
}

void core_timer_counter()
{
    uart_puts("Core Timer interrupt received ");
    uart_hex(CORE_TIMER_COUNT++);
    uart_puts("\n");
    timer_tick();  // set reschedule_flag
}

#define CORE0_TIMER_IRQ_CTRL (unsigned int* )(0x40000040+0xffff000000000000)
#define EXPIRE_PERIOD 0xfffff

void core_timer_enable()
{
    unsigned int val = EXPIRE_PERIOD;
    asm volatile("msr cntp_tval_el0, %0" :: "r" (val));
    
    asm volatile("mov x0, 1");
	asm volatile("msr cntp_ctl_el0, x0");
    *CORE0_TIMER_IRQ_CTRL = 0x2;

}

void core_timer_enable_user()
{
    asm volatile("mov x0, #0\n" "svc #0\n");
}

void core_timer_handler()
{
    unsigned int val = EXPIRE_PERIOD;
    asm volatile("msr cntp_tval_el0, %0" :: "r" (val));
    core_timer_counter();
}


#define LOCAL_TIMER_CONTROL_REG (unsigned int* )0x40000034

void local_timer_init()
{
    unsigned int flag = 0x30000000; // enable timer and interrupt.
    unsigned int reload = 25000000;
    *LOCAL_TIMER_CONTROL_REG = (flag | reload);
}

void local_timer_counter()
{
    uart_puts("Local Timer interrupt received ");
    uart_hex(LOCAL_TIMER_COUNT++);
    uart_puts("\n");
}

#define LOCAL_TIMER_IRQ_CLR (unsigned int* )0x40000038
#define LOCAL_TIMER_RELOAD 0xc0000000//0xc0000000
void local_timer_handler()
{
    *LOCAL_TIMER_IRQ_CLR = LOCAL_TIMER_RELOAD; // clear interrupt and reload.
    local_timer_counter();
}

#define CORE0_INTERRUPT_SRC (unsigned int* )0x40000060
void interrupt_handler()
{
    // uart_puts("\r\n++++++++++  interrupt_handler begin  ++++++++++\n");
    struct task* current = get_current();
    current->state = IRQ_CONTEXT;
    unsigned int interrupt_src = *CORE0_INTERRUPT_SRC;
    char r;

    //if (arm & 0x80000) {
    if (interrupt_src & (1<<8)) {
        // uart interrupt
        if (*UART0_RIS & (1<<4)) {	// UARTRXINTR - uart_getc()
            while (RX_FIFO_FULL) { RX_BUF[0] = (char)(*UART0_DR); }
            *UART0_ICR = 1<<4; // Clears the UARTTXINTR interrupt.

	    } else if (*UART0_RIS & (1<<5))	{// UARTTXINTR - uart_send()
	        while (!(TX_BUF[0] == 0)) {
                r = TX_BUF[0];
                TX_BUF[0] = 0;
                while (TX_FIFO_FULL) asm volatile ("nop");
                *UART0_DR = r;
            }
            *UART0_ICR = 1<<5; // Clears the UARTRTINTR interrupt.
	    }
    }
    // local timer interrupt
    else if (interrupt_src & (1<<11)) {
        local_timer_handler();
    }
    // core timer interrupt
    else if (interrupt_src & (1<<1)) { // Physical Non Secure Timer Interrupt
        core_timer_handler();
    }
    else {
        uart_puts("interrupt_handler error.\n");
    }
    irq_reschedule();
    // uart_puts("++++++++++  interrupt_handler end  ++++++++++\n\r\n");
}

void irq_reschedule() 
{
    struct task* current = get_current();
    if (current->reschedule_flag == 1) {
        // uart_puts("IRQ reschedule...\n");
        // uart_puts("previous task: ");
        // uart_hex(current->task_id);
        // uart_puts("\n");
        current->reschedule_flag = 0;
        schedule();
        // uart_puts("after IRQ reschedule...\n");
    // } else {
    //     asm volatile("eret");
    }
}

#define UART_ENABLE_IRQ (unsigned int *)(MMIO_BASE + 0xb214)
void uart_irq_enable()
{
    *UART_ENABLE_IRQ = (1 << 25);
}

void timer_tick()
{
    struct task* current = get_current();
    current->counter--;
    // uart_puts("count: ");
    // uart_hex(current->counter);
    // uart_puts("\n");
	if (current->counter == 0) {
	    current->reschedule_flag = 1;
    }
}
