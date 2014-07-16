/*
 Z8 Timer emulator:
 Emulates UART according to assignment 2 specifications
 Notes:
 - UART status changes signalled from file
 - Read file and time and char until clock tick >= time
 - File record: Time-of-char-arrival Char-read 
 - Check for unread SIO (RCVDONE set -> RCVORUN)
 - If RCVINT, signal IRQ3
 - RCVDONE set 
 - Clear RCVDONE when SIO read 
 - **** Make char-read available when SIO is read 
 
 - UART write (through SIO)
 - Check for overwrite (TXORUN)
 - Set UART timer to random value (base_value +|- random) and countdown
   on each clock tick -- set TXDONE when char finally written
   
 ECED 3403
 17 June 2014
*/

/* FILE *uart_file
   - mainline must call a UART function to open file and read first time 
     value
   - when UART_check() finds system clock tick >= time, the character has 
     been received -- can signal RCVDONE
   - must read next usrt file record once char arrival signalled
*/

#include "Z8_IE.h"

int UART_device(BYTE reg_no, enum DEV_EM_IO cmd)
{
/* Emulate UART device port:
   - called after Port 3 or SIO have been read or written 
*/

}

void UART_check()
{
/* System clock has "ticked"
   - decrement TX timer if running
   - when timer reaches zero, "transmit" character 
*/

}
