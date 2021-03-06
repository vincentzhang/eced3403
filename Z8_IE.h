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

/* Program and data memory size */
#define PD_MEMSZ    65536

#define FALSE      0
#define TRUE       1

#define PRIVATE    static
#define BYTE       unsigned char
#define WORD       unsigned short
#define CARRY_BYTE unsigned short    /* Use to determine if carry set */

/* Constants */
#define INPUT_FILENAME "UART_input.TXT"
#define OUTPUT_FILENAME "UART_output.TXT"
FILE *uart_finput, *uart_foutput; // input and output file 

extern BYTE uart_line[];// keep data read from the input file
extern BYTE uart_char; // the data to be put in to SIO
extern int uart_time; // the time read from input file

/* Special register operations */
#define FLAG_Z(x)   ((x)<<6 | (FLAGS & 0xBF))   /* Set/clear Z bit */
#define RPBLK       (RP << 4)                   /* For RP | working register */

#define HBYTE(x)      ((x) >> 4)
#define LBYTE(x)      ((x) & 0x0F)

#define IRQ_MASK  0x3F             /* IRQ bits 0..5 in IMR */
enum IMR_BITS     {IRQ0 = 0x01, IRQ1 = 0x02, IRQ2 = 0x04, IRQ3=0x08, INT_ENA = 0x80};

/* Program and data memory */
extern BYTE memory[2][PD_MEMSZ]; 

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

/* UART bits */
#define TXDONE     0x04 // B2
#define TXORUN     0x08 // B3
#define RCVDONE    0x10 // B4
#define RCVORUN    0x20 // B5

/* Register memory functions */
extern void reg_mem_init();
extern void reg_mem_device_init(BYTE, int (*)(BYTE, enum DEV_EM_IO), BYTE);
void write_rm(BYTE, BYTE);
BYTE read_rm(BYTE);

/* Device entry points */
extern int TIMER_device(BYTE, enum DEV_EM_IO);
extern int UART_device(BYTE, enum DEV_EM_IO);
void UART_init();

extern unsigned long sys_clock;

#endif
