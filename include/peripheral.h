#include "gpio.h"

/* PL011 UART registers */
#define UART0_DR        ((volatile unsigned int*)(MMIO_BASE+0x00201000)) // 0x??201000 data register
#define UART0_FR        ((volatile unsigned int*)(MMIO_BASE+0x00201018)) // 0x??201018 flag register
#define UART0_IBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201024)) // 0x??201024 Integer Baud rate divisor
#define UART0_FBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201028)) // 0x??201028 Fractional Baud rate divisor
#define UART0_LCRH      ((volatile unsigned int*)(MMIO_BASE+0x0020102C)) // 0x??20102C Line Control register
#define UART0_CR        ((volatile unsigned int*)(MMIO_BASE+0x00201030)) // 0x??201030 Control register
#define UART0_IMSC      ((volatile unsigned int*)(MMIO_BASE+0x00201038)) // 0x??201038 Interupt Mask Set Clear Register
#define UART0_RIS       ((volatile unsigned int*)(MMIO_BASE+0x0020103c)) // 0x??20103C Raw Interupt Status Register
#define UART0_ICR       ((volatile unsigned int*)(MMIO_BASE+0x00201044)) // 0x??201044 Interupt Clear Register

#define UARTBUF_SIZE 0x400
#define QUEUE_EMPTY(q) (q.tail == q.head)
#define QUEUE_FULL(q) ((q.tail + 1) % UARTBUF_SIZE == q.head)
#define QUEUE_POP(q) (q.head = (q.head + 1) % UARTBUF_SIZE)
#define QUEUE_PUSH(q) (q.tail = (q.tail + 1) % UARTBUF_SIZE)
#define QUEUE_GET(q) (q.buf[q.head])
#define QUEUE_SET(q, val) (q.buf[q.tail] = val)

// #define STACK_POP(S) 
// #define STACK_PUSH(S)
// #define STACK_EMPTY(S)
// #define STACK_FULL(S)

struct uart_buf
{
    int head;
    int tail;
    char buf[UARTBUF_SIZE];
} read_buf, write_buf;

char BUF[0x10];