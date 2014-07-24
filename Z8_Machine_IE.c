/*
 Z8 Machine emulator with interrupt emulation - skeletal machine
 
 ECED 3403
 17 June 2014
*/

#include "Z8_IE.h"

/* Program and data memory size */
#define PD_MEMSZ  56536

unsigned long sys_clock;

/* Memory arrays */
BYTE memory[2][PD_MEMSZ];        /* PROG and DATA */

/* Hidden registers */
WORD pc;              /* Program counter */
BYTE intr_ena;        /* Interrupt status */

void bus(WORD mar, BYTE *mbr, enum RDWR rdwr, enum MEM mem)
{
/* Bus emulation:
   - mar - 16-bit memory address register (W)
   - mbr - 8-bit memory buffer register (R|W)
   - rdwr - 1-bit read/write indication (RD or WR)
   - mem - 1-bit memory to access (PROG or DATA)
   Does not check for valid rdwr or mem values
*/

if (rdwr == RD)
   *mbr = memory[mem][mar];
else /* Assume WR */
   memory[mem][mar] = *mbr;

#ifdef DIAGNOSTICS
printf("Bus: %04x %01x %01x %01x\n", mar, *mbr, rdwr, mem);
#endif

}

BYTE prog_mem_fetch()
{
/* Call bus to access next location in program memory
   Use PC -- increment PC after access (points to next inst or next byte in 
   current instruction)
   Returns location as byte (mbr)
*/
BYTE mbr;  /* Memory buffer register */

bus(pc, &mbr, RD, PROG);

pc = pc + 1;

return mbr;
}

void run_machine()
{
/* Z8 machine emulator
   instruction fetch, decode, and execute 
*/
BYTE inst;       /* Current instruction */
int running;     /* TRUE until STOP instruction */
int sanity;      /* Limit on number of instruction cycles */
BYTE regval;     /* temp to emulate OR instruction */

running = TRUE;
sanity = 0;

#ifdef IE_TEST
write_rm(IMR, INT_ENA | IRQ0); /* PORT 0 interrupts allowed */
write_rm(IRQ, 0);              /* No pending interrupt requests */
#endif

while (running && sanity < 24)
{
#ifdef IE_TEST
printf("Time: %02d  IRQ: %02x\n", sys_clock, reg_mem[IRQ] . contents);
#endif
     /* Get next instruction and extract nibbles */
     /* ... */
     
#ifdef IE_TEST
     switch(sanity)
     {
     case 3:
            write_rm(PORT0, 0x83); /* Timer: continuous & 3x2 (6) cycles */
            break;
     case 11:
     case 16:
            /* Emulate ISR:
               - Acknowledge IRQ0 interrupt - emulate OR 
            */
            regval = read_rm(IRQ);
            regval &= ~IRQ0;      /* Mask off IRQ0 only */
            write_rm(IRQ, regval);
            break;
     case 12:
     case 17:
            /* Emulate ISR:
               - Emulate IRET - reenable interrupts */
            reg_mem[IMR] . contents |= INT_ENA;
            break;
            
     }
#endif
     
     /* Instruction cycle completed -- check for interrupts 
        - call device check
        - TRAPs - software-generated interrupts (SWI, SVC, etc) are caused
          by turning IRQ bit on (see section 2.6.4 in assignment)
        - IRET enables interrupts:
           reg_mem[IRQ] . contents |= INT_ENA;
        - Concurrent interrupts are possible if timer and uart interrupt at 
          same time
     */
     TIMER_check();
     UART_check();

     if ((reg_mem[IMR] . contents & INT_ENA) && reg_mem[IRQ] . contents != 0)
     {
          /* CPU interrupts enabled and one or more pending interrupts
	       - check interrupt mask (IMR) if device allowed 
          */
          if ((reg_mem[IMR] . contents & reg_mem[IRQ] . contents) & IRQ_MASK)
          {
#ifdef IE_TEST
printf("Interrupt on IMR: %02x\n", 
                  reg_mem[IMR] . contents & reg_mem[IRQ] . contents);
#endif
               /* At least one interrupt pending 
                - if single, proceed
                - if multiple, determine priority to select
                - push Flags, PC lo, and PC hi
                - clear interrupt status bit (IMR[7])
                - PC hi = device vector[n]
                - PC lo = device vector[n+1]
                - ISR entered on this clock tick
                - Pending interrupts remain until after IRET
                - IRET enables interrupt
                - update sys_clock with interrupt overhead
               */
               reg_mem[IMR] . contents &= ~INT_ENA;
          }
     }

     sys_clock++;
     sanity++;    
}

}

