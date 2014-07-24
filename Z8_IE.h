/*
 Z8 Interrupt emulation header file
 
 ECED 3403
 July 23, 2014
*/

#ifndef Z8_IE_H
#define Z8_IE_H

#include <stdio.h>
#include <stdlib.h>

#define DIAGNOSTIC
#define IE_TEST        /* IE test */
#define LINE_LEN   256

#define FALSE      0
#define TRUE       1

#define PRIVATE    static
#define BYTE       unsigned char
#define WORD       unsigned short

#define IRQ_MASK  0x3F             /* IRQ bits 0..5 in IMR */
enum IMR_BITS     {IRQ0 = 0x01, IRQ1 = 0x02, IRQ2 = 0x04, IRQ3=0x08, INT_ENA = 0x80};

/* Program and data memory */
extern BYTE memory[][]; 

/* Bus signals */
enum RDWR           {RD, WR};
enum MEM            {PROG, DATA};

/* Bus function */
void bus(WORD, BYTE* , enum RDWR, enum MEM);

/* Register memory */
enum DEV_EM_IO    {REG_RD, REG_WR};

struct reg_mem_el
{
BYTE contents;
int (*option)(BYTE, enum DEV_EM_IO);       
};

extern struct reg_mem_el reg_mem[];

/* Devices in register memory */
#define PORT0     0x00
#define PORT1     0x01
#define PORT2     0x02
#define PORT3     0x03
#define SIO       0xF0
#define P01M      0xF8
#define IPR       0xF9
#define IRQ       0xFA
#define IMR       0xFB

/* Special reg_mem addresses */
#define FLAGS     0xFC
#define RP        0xFD 
#define SPH       0xFE
#define SPL       0xFF

/* Register memory functions */
extern void reg_mem_init();
extern void reg_mem_device_init(BYTE, int (*)(BYTE, enum DEV_EM_IO), BYTE);

/* Device entry points */
extern int TIMER_device(BYTE, enum DEV_EM_IO);
extern int UART_device(BYTE, enum DEV_EM_IO);

/* UART bits */
#define TXDONE     0x04 // B2
#define TXORUN     0x08 // B3
#define RCVDONE    0x10 // B4
#define RCVORUN    0x20 // B5

extern unsigned long sys_clock;

#endif
