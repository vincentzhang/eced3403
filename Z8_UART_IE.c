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

// variable lists
PRIVATE unsigned long TXTimer;
BYTE uart_line[LINE_LEN];// keep data read from the input file
BYTE uart_char; // the data to be put in to SIO
int uart_time; // the time read from input file
BYTE uart_char_to_go; // the char to be sent to UART device
BYTE UART_RECV_REG; // the input register
BYTE UART_TRAN_REG; // the output register
unsigned char UART_RECV_PENDING; // TRUE if the UART is still in the buffer

/* open file and read first time value */
void UART_init()
{
 	 if ((uart_finput = fopen(INPUT_FILENAME, "r")) == NULL)
	{
	     printf("No input file specified\n");
	     uart_time = -1;
	}

	if ((uart_foutput = fopen(OUTPUT_FILENAME, "w")) == NULL)
	{
	     printf("No output file specified\n");
	     exit(0);
	}
	
	// read first time value
	if (fgets(uart_line, LINE_LEN, uart_finput) > 0)
	{
	   		uart_time = uart_line[0]; // read the next interrupt time
			uart_char = uart_line[2]; // read the data from uart
	}					
}

int UART_device(BYTE reg_no, enum DEV_EM_IO cmd)
{
/* Emulate UART device port:
   - called after Port 3 or SIO have been read or written 
*/
  if (cmd == REG_RD) // READ: Z8 read from UART
  {
       reg_mem[SIO].contents = uart_char; // load char to SIO
       reg_mem[PORT3].contents &= ~RCVDONE; // clear RCVDONE after loading char, no data received from UART
      
       // read next char
       // file format: time, device, data
  	   if (fgets(uart_line, LINE_LEN, uart_finput) > 0)
	   {
	     uart_time = uart_line[0]; // read the next interrupt time
	     uart_char = uart_line[2]; // read the data from uart
	   }

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

  else if (cmd == REG_WR) // write to UART
  {   
      uart_char_to_go = reg_mem[SIO].contents; // the chars waiting to be sent
      

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

   TXTimer--;
   if ( TXTimer == 0 )
   {
  	// when TXTimer reaches zero, signal IRQ3 interrupt
      reg_mem[IRQ] . contents |= IRQ3;  /* Signal IRQ3 - UART interrupt */
   }
   
   if ( sys_clock >= uart_time ) // time to signal interrupt ?
   {
      // Check for unread SIO (RCVDONE set -> RCVORUN)
      if (reg_mem[PORT3].contents & RCVDONE) {
	  	 reg_mem[PORT3].contents |= RCVORUN;
	  	 printf("Warning: PORT3 Overrun!!\n");
	  	 return; // do nothing when there's overrun
	  }
      reg_mem[PORT3].contents |= RCVDONE; // set RCVDONE
      reg_mem[IRQ] . contents |= IRQ3;  /* Signal IRQ3 - UART interrupt */
   }
   
    // when UART_check() finds system clock tick >= time, the character has 
    //  been received -- can signal RCVDONE
         if (sys_clock >= uart_output_time)
      {
	   	 fprintf(uart_fpout, "%i\n", uart_char_to_go);
	  // 	 port3 <- uart; 
	  //  	 IRQ < IRQ3 / 
		}
}
