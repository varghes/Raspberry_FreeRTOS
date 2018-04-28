/*
    Copyright (C) 2018 Varghese Alphonse.
	License : MIT 
*/


#include <FreeRTOS.h>
#include <task.h>

//#include "Drivers/irq.h"
//#include "Drivers/gpio.h"



extern void PUT32 ( unsigned int, unsigned int );
extern unsigned int GET32 ( unsigned int );
#define ARM_TIMER_LOD 0x2000B400
#define ARM_TIMER_VAL 0x2000B404
#define ARM_TIMER_CTL 0x2000B408
#define ARM_TIMER_CLI 0x2000B40C
#define ARM_TIMER_RIS 0x2000B410
#define ARM_TIMER_MIS 0x2000B414
#define ARM_TIMER_RLD 0x2000B418
#define ARM_TIMER_DIV 0x2000B41C
#define ARM_TIMER_CNT 0x2000B420

#define SYSTIMERCLO 0x20003004

#define GPFSEL0 0x20200000
#define GPFSEL1 0x20200004
#define GPFSEL2 0x20200008

#define GPSET0  0x2020001C
#define GPCLR0  0x20200028
#define GPFSEL3 0x2020000C
#define GPFSEL4 0x20200010
#define GPSET1  0x20200020
#define GPCLR1  0x2020002C

#define GPPUD       0x20200094
#define GPPUDCLK0   0x20200098

#define IRQ_BASIC 0x2000B200
#define IRQ_PEND1 0x2000B204
#define IRQ_PEND2 0x2000B208
#define FIQ_ENABLE 0x2000B20C
#define IRQ_ENABLE_1    0x2000B210
#define IRQ_ENABLE_2     0x2000B214
#define IRQ_ENABLE_BASIC 0x2000B218
#define IRQ_DISABLE_1      0x2000B21C     
#define IRQ_DISABLE_2     0x2000B220      
#define IRQ_DISABLE_BASIC   0x2000B224    

#define  RASPI_IRQR2_UART0			(1<<25)

#define BCM2835_UART0_BASE  0x20201000
#define BCM2835_UART_SIZE    0x90
// The offsets for reach register for the UART.
#define BCM2835_UART0_DR     0x00
#define BCM2835_UART0_RSRECR 0x04
#define BCM2835_UART0_FR     0x18
#define BCM2835_UART0_ILPR   0x20
#define BCM2835_UART0_IBRD   0x24
#define BCM2835_UART0_FBRD   0x28
#define BCM2835_UART0_LCRH   0x2C
#define BCM2835_UART0_CR     0x30
#define BCM2835_UART0_IFLS   0x34
#define BCM2835_UART0_IMSC   0x38
#define BCM2835_UART0_RIS    0x3C
#define BCM2835_UART0_MIS    0x40
#define BCM2835_UART0_ICR    0x44
#define BCM2835_UART0_DMACR  0x48
#define BCM2835_UART0_ITCR   0x80
#define BCM2835_UART0_ITIP   0x84
#define BCM2835_UART0_ITOP   0x88
#define BCM2835_UART0_TDR    0x8C


#define BCM2835_DBGU_IE_SR_RXEMTY   0x10
#define BCM2835_DBGU_IE_SR_TXRDY    0x20







void init_uart(void)
{
	PUT32(BCM2835_UART0_BASE + BCM2835_UART0_CR, 0x00000000); // clear control register

	PUT32(GPPUD, 0x0);
	PUT32(GPPUDCLK0, (1<<14) | (1<<15));
	PUT32(GPPUDCLK0, 0x0);
	
	PUT32(BCM2835_UART0_BASE+ BCM2835_UART0_ICR, 0x7FF);//clear all interrupts
	PUT32(BCM2835_UART0_BASE + BCM2835_UART0_IBRD, 1);  // Baurd Rate divisior BAUDDIV = (FUARTCLK/(16 Baud rate))
	PUT32(BCM2835_UART0_BASE + BCM2835_UART0_FBRD, 40);// Fractional Baud rate divisor
	PUT32(BCM2835_UART0_BASE + BCM2835_UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));  // Line control register
		                                 //Word len:8         eps -parity

	PUT32(BCM2835_UART0_BASE+BCM2835_UART0_IMSC, (1<<1) | (1<<4)| (1<<6) |
		                      (1<<7) | (1<<8) | (1<<9) | (1<<10));
	PUT32(BCM2835_UART0_BASE + BCM2835_UART0_CR, (1<<0) | (1<<8) | (1<<9));
	PUT32(IRQ_ENABLE_2,1<<25);
		
}

void uart_send(int data )
{
	while ( GET32(BCM2835_UART0_BASE + BCM2835_UART0_FR) & BCM2835_DBGU_IE_SR_TXRDY ) { }
	PUT32(BCM2835_UART0_BASE + BCM2835_UART0_DR, (unsigned)data);
}
int uart_getchar(void)
{
	while ( GET32(BCM2835_UART0_BASE + BCM2835_UART0_FR) & BCM2835_DBGU_IE_SR_RXEMTY ) { }
	return GET32(BCM2835_UART0_BASE + BCM2835_UART0_DR);
}

void hexstrings ( unsigned int d )
{
    //unsigned int ra;
    unsigned int rb;
    unsigned int rc;

    rb=32;
    while(1)
    {
        rb-=4;
        rc=(d>>rb)&0xF;
        if(rc>9) rc+=0x37; else rc+=0x30;
        uart_send(rc);
        if(rb==0) break;
    }
    uart_send(0x20);
}
//------------------------------------------------------------------------
void hexstring ( unsigned int d )
{
    hexstrings(d);
    uart_send(0x0D);
    uart_send(0x0A);
}

//Print()
void print( char* ch)
{
	while (*ch!=0)
	{
		uart_send((unsigned int)*ch);
		ch++;
	}
	uart_send(0x0D);
	uart_send(0x0A);
}

void LED_Init(void)
{
	unsigned int ra;
	ra=GET32(GPFSEL4);  // green
    ra&=~(7<<21);
    ra|=1<<21;
    PUT32(GPFSEL4,ra);

    ra=GET32(GPFSEL3);  //red
    ra&=~(7<<15);
    ra|=1<<15;
    PUT32(GPFSEL3,ra);
}
void Green_Led_ON(void)
{
	PUT32(GPSET1,1<<15);
}
void Green_Led_OFF(void)
{
	PUT32(GPCLR1,1<<15);
}

void Red_Led_ON(void)
{
	PUT32(GPSET1,1<<3);
}

void Red_Led_OFF(void)
{
	 PUT32(GPCLR1,1<<3);
}


void task1(void *pParam) {
print(" task1 ");
	int i = 0;
	while(1) 
	{
		print(" task 1 run ");
		vTaskDelay(1000);
		Red_Led_ON();
		vTaskDelay(1000);
		Red_Led_OFF();
	}
}

void task2(void *pParam) {

print(" task2 ");
	int i = 0;
	while(1)
	{
		print(" task2 run ");
		vTaskDelay(3000);
		Green_Led_ON();
		vTaskDelay(3000);
		Green_Led_OFF();
	}
}


/**
 *	This is the systems main entry, some call it a boot thread.
 *
 *	-- Absolutely nothing wrong with this being called main(), just it doesn't have
 *	-- the same prototype as you'd see in a linux program.
 **/
void main (void)
{
	init_uart(); 
	LED_Init();
	xTaskCreate(task1, "LED_0", 128, NULL, 0, NULL);
	xTaskCreate(task2, "LED_1", 128, NULL, 0, NULL);
	print("2 tasks created ");
	vTaskStartScheduler();
	/*
	 *	We should never get here, but just in case something goes wrong,
	 *	we'll place the CPU into a safe loop.
	 */
	while(1) {
		;
	}
}
