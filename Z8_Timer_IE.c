/*
 Z8 Timer emulator
 Requirements (from assignment):
 A single timer is to be supported using IRQ0. The Port 0 (P0) register 
 is to be interpreted as follows:
 - Bits 0 through 6: Timer delay count divided by 2 (i.e., the delay count 
   is multiplied by two when it is loaded into the emulated timer device). 
   Each cycle (system clock "tick" decrements the count by one).
 - Bit 7: Single shot (0) or continuous (1) timer interrupts. If continuous, 
   the timer value is reloaded from P0.
 - If IRQ0 is cleared, the timer is disabled.
 The timer can be disabled by clearing P0. In order to reset the timer, P0 
 must first be cleared and then the new value written to the register.
*/

#include "Z8_IE.h"

PRIVATE WORD tdc;       /* Timer delay count */
PRIVATE BYTE treload;   /* Timer reload? T|F */
PRIVATE BYTE trunning;  /* Timer running? T|F */

int TIMER_device(BYTE reg_no, enum DEV_EM_IO cmd)
{
/* Emulate timer device port:
   - called after Port 0 has been read or written (reads are ignored)
   - write to port implies change - check value
   - interrupts will occur only if IMR(0) is set
*/
if (cmd == REG_RD)
     return;

/* Write to port -- check port value */
if (reg_mem[PORT0] . contents == 0)
     /* Timer off */
     trunning = FALSE;
else
{
     /* Non-zero write - extract tdc and treload */
     tdc = (reg_mem[PORT0] . contents & 0x7F) << 1;
     treload = (reg_mem[PORT0] . contents & 0x80) == 0x80;
     trunning = TRUE;
}

#ifdef DIAGNOSTIC
printf("tdc: %x treload: %x trunning: %x\n", tdc, treload, trunning);
printf("Port[0]: %02x\n", reg_mem[PORT0] . contents);
#endif
}

void TIMER_check()
{
/* System clock has "ticked"
   - decrement timer if running
   - when timer reaches zero, reload if required otherwise stop timer
*/
if (!trunning)
     return;

tdc--;

if (tdc == 0)
{
     reg_mem[IRQ] . contents |= IRQ0;  /* Signal IRQ0 - timer interrupt */
     if (treload)
          tdc = (reg_mem[PORT0] . contents & 0x7F) << 1;
     else
          trunning = FALSE;
}

}
