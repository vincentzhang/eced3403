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

//variable lists
PRIVATE unsigned long TXTimer;
FILE *fpin, *fpout; // input and output file 
BYTE data[LINE_LEN];// keep data read from the input file

int uart_time; // the time read from input file
BYTE UART_RECV_REG; // the input register
BYTE UART_TRAN_REG; // the output register
unsigned char UART_RECV_PENDING; // TRUE if the UART is still in the buffer

/* Constants */
INPUT_FILENAME = "UART_input.TXT";
OUTPUT_FILENAME = "UART_output.TXT";

if ((fpin = fopen(INPUT_FILENAME, "r")) == NULL)
{
     printf("No input file specified\n");
     exit(0);
}

if ((fpout = fopen(OUTPUT_FILENAME, "w")) == NULL)
{
     printf("No output file specified\n");
     exit(0);
}

int UART_device(BYTE reg_no, enum DEV_EM_IO cmd)
{
/* Emulate UART device port:
   - called after Port 3 or SIO have been read or written 
*/
  if (cmd == REG_RD)  // Z8 reading from UART device
// READ: Z8 read from UART
// Data on P30-P33(File in our case) -> Input Buffer -> Input Register -> BUS
// Reading from P3 returns the data on the input pins and in the output register
  {
    if (sys_clock >= uart_time)
    {
      if (!UART_RECV_PENDING) // if pending, do NOT do anything
      {
          if ( fgets(data, LINE_LEN, fpin) > 0 )
          { // if there's still content in the file, read and write to UART device
            // file format: time, device, data
            uart_time = data[0]; // save the next interrupt time
            UART_RECV_REG = data[2];
            UART_RECV_PENDING = TRUE;
            
            if ( UART_RECV_PENDING && (reg_mem[PORT3].contents & RCVDONE) ) // if still pending, must raise overrun
            {
              reg_mem[PORT3].contents|=RCVORUN; // raise OVERRUN
            }
            else
            {
              reg_mem[SIO] . contents = UART_RECV_REG; // load the input register content to SIO
              // clear the register and interrupt initialization
              UART_RECV_PENDING = FALSE; // nothing in the input register
              UART_RECV_REG = 0x00; // clear all the bits
              reg_mem[PORT3].contents &= ~RCVDONE; // no data in the input register
              reg_mem[IRQ].contents &= ~IRQ3; // clear interrupt bit
            }
          }
      } 
    }
  }
// Port 3 lines are fixed as four input
// (P30-P33) and four output (P34-P37) and do not have an input and output register for each bit.
  else if (cmd == REG_WR)
  {
      // TODO: write to PORT 
      if ( reg_mem[PORT3].contents & TXORUN ) // if Overrun    
      {
          reg_mem[PORT3].contents &= ~TXORUN; // clear OVERRUN
      }
      else // reload
      {
          if (reg_mem[PORT3].contents & TXDONE)   // if the previous transmit is done
          {
             reg_mem[SIO] . contents = 0x12;  // TODO: Write something to PORT3
          }     
          TXTimer = sys_clock + 100; 
          reg_mem[IRQ].contents &= ~IRQ3; // clear interrupt bit
      }
  }
}

void UART_check()
{
/* System clock has "ticked"
   - decrement TX timer if running
   - when timer reaches zero, "transmit" character 
*/
// int sysclock;
// what is TX timer?

   TXTimer--;
   if ( TXTimer == 0)
   {
      reg_mem[IRQ] . contents |= IRQ3;  /* Signal IRQ3 - UART interrupt */
   }
}
