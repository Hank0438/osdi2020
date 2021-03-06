#include "../include/peripheral.h"
#include "../include/gpio.h"
#include "../include/mailbox.h"
#include "../include/sprintf.h"

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;

    /* initialize UART */
    *UART0_CR = 0;         // turn off UART0

    /* set up clock for consistent divisor values */
    mbox[0] = 9*4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = MBOX_TAG_SETCLKRATE; // set clock rate
    mbox[3] = 12;
    mbox[4] = 8;
    mbox[5] = 2;           // UART clock
    mbox[6] = 4000000;     // 4Mhz
    mbox[7] = 0;           // clear turbo
    mbox[8] = MBOX_TAG_LAST;
    mbox_call(MBOX_CH_PROP);

    /* map UART0 to GPIO pins */
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(4<<12)|(4<<15);    // alt0
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup

    *UART0_ICR = 0x7FF;    // clear interrupts
    *UART0_IBRD = 2;       // 115200 baud
    *UART0_FBRD = 0xB;
    *UART0_LCRH = 0b110<<4; // 8n1, enable FIFO
    /*
     Program the control registers as follows:
        1. Disable the UART.
        2. Wait for the end of transmission or reception of the current character.
        3. Flush the transmit FIFO by setting the FEN bit to 0 in the Line Control
        Register, UART_LCRH.
        4. Reprogram the Control Register, UART_CR.
        5. Enable the UART.
     */
    *UART0_CR = 0x301;     // enable Tx, Rx, UART

    // enable interrupt
    *UART0_IMSC = 0b11 << 4;		// Tx, Rx
}

/**
 * Send a character
 */
void uart_send(unsigned int c) {
    /* wait until we can send */
    do{ asm volatile("nop"); } while(TX_FIFO_FULL); // TX FIFO is full
    /* write the character to the buffer */
    *UART0_DR=c;
}

/**
 * Receive a character
 */
char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do{ asm volatile("nop"); } while(*UART0_FR&0x10); // RX FIFO is empty
    /* read it and return */
    r=(char)(*UART0_DR);
    /* convert carrige return to newline */
    return r=='\r'?'\n':r;
}

/**
 * Display a string
 */
void uart_puts(char *s) 
{
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

void uart_readline(char* buf) 
{
    // char buf[0x100];
    char tmp;
    long len = 0;
    while (1) {
        tmp = uart_getc();
        buf[len++] = tmp;
        if((tmp == '\n') | (tmp =='\0'))
            break;
    }
    buf[len++] = '\0';
}

/*
 * Transfer int to char
 */
char uart_i2c(unsigned int d) 
{
    return (d<10) ? d+48 :0;
}

/**
 * Display a binary value in hexadecimal
 */
void uart_hex(unsigned int d) 
{
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
}

/**
 * Compare two string and return 0 if they are identical
 */ 
int uart_strncmp(const char *cs, const char *ct, int len)
{
    unsigned char c1, c2;
    while (len--) {
        c1 = *cs++;
        c2 = *ct++;
        if (c1 != c2) return c1 < c2 ? -1 : 1;
        if (!c1) break;
    }
    return 0;
}

/**
 * Copy memory by address and size
 */ 
void uart_memcpy (void *src, void *dst, int len)
{
    char *s = src;
    char *d = dst;
    while (len--) {
        *d++ = *s++;
        //if (*s == 0) break;
    }
}


/**
 * Transfer string to int
 */
/*
unsigned int uart_c2i(char *s) 
{
    return (d<10) ? d+48 :0;
}
*/
int uart_atoi(char *dst, int d) 
{
    int sign, i;
    char tmpstr[19];
    // check input
    sign=0;
    if(d<0) {
        d*=-1;
        sign++;
    }
    if(d>2147483647) {
        d=2147483647;
    }
    // convert to string
    i=18;
    tmpstr[i]=0;
    do {
        tmpstr[--i]='0'+(d%10);
        d/=10;
    } while(d!=0 && i>0);
    if(sign) {
        tmpstr[--i]='-';
    }
    uart_memcpy(&tmpstr[i], dst, 18-i);
    dst[18-i] = 0;
    return 18-i;
}

void uart_memset (void *dst, char s, int len)
{
    char *d = dst;
    while (len--) {
        *d++ = s;
    }
}

/**
 * Display a string
 */
extern volatile unsigned char __kernel_end;
void printf(char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    // we don't have memory allocation yet, so we
    // simply place our string after our code
    char *s = (char*)&__kernel_end;
    // char buf[0x100];
    // char *s = buf;
    // use sprintf to format our string
    vsprintf(s,fmt,args);
    // print out as usual
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}