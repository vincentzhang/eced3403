/*
 Z8 Machine emulator with interrupt emulation - skeletal machine
 
 ECED 3403
 17 June 2014
*/

#include "Z8_IE.h"

/* Program and data memory size */
#define PD_MEMSZ  56536

/* Bus signals */
enum RDWR           {RD, WR};
enum MEM            {PROG, DATA};

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

// variables for register operations
BYTE flag;
int iret=0;//if meet iret:1 
BYTE x;
WORD dev[2];            	
int i;
            	int count;// how many interupts
            	BYTE y[6]={0,0,0,0,0,0};//STORE the interrupt byte
            	BYTE stackh,stackl;
				WORD stack;   // Stack Pointer
				BYTE pch,pcl; // PC
				int p01; // p01m
#ifdef IE_TEST
write_rm(IMR, INT_ENA | IRQ0); /* PORT 0 interrupts allowed */
write_rm(IRQ, 0);              /* No pending interrupt requests */
#endif



while (running && sanity < 24)

{
	
	
	
	
}
     
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
	
	
	if (iret==1) //  if meet iret set iret=1 enable IMR,coutinou responding next interrupt 
	{
		reg_mem[IMR] . contents |= INT_ENA;
			// pop stack to pc and flags
		bus(stack, &pcl,RD,DATA);
		stack++;
		bus(stack,&pch,RD, DATA);
		stack++;
		// update pc
		pc=pcl&pch<<8; 
		// update FLAGS register
		bus(stack,&flag,RD, DATA);
		write_rm(FLAGS,flag); 
	}
	
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
            	
			 
            	/* counting how many process we need
				compare x with 0x01,0x02.....0x05
				see how many bits are set */
				
				int count1[6]={0,0,0,0,0,0};//store which bit want to interrupt 
				
				x=reg_mem[IMR] . contents & reg_mem[IRQ] . contents& IRQ_MASK ;
			
				// determine interrupt source
				for(i=0;i<6;i++)
				{
					if(x & (BYTE)(pow(2, i)))
					{
						count++;
						y[i]=x;
						count1[i]=1; //store which bit want to interupt.
					}
				}
				
				// push Flags, PC lo, PC hi
				stackh=read_rm(SPH);// read high byte from re_memary address 
				stackl=read_rm(SPL);
				stack=stackh<<8&stackl;
				pch=HBYTE(pc);
				pcl=LBYTE(pc);
				
				
				/*initial the data memory already so write into data memory */   
			
				// DATA MEMORY
				flag = read_rm(FLAGS);		
				bus(stack,&flag,WR,DATA);// push Flags into data data memory
				stack--;
				bus(stack,&pcl,WR,DATA); // push pc into data memory
				stack--;
				bus(stack,&pch,WR,DATA);
				

				write_rm(SPH,stack>>8);
				write_rm(SPL,stack&0xff);// write back into SPH, SPL
				
				
				reg_mem[IMR] . contents &= ~INT_ENA;// clear interrupt status bit 7
				
				if(count==1)//if it is single interrupt
				{
					int j;

					for(j=0;j<6;j++)//find which bit of interuprt
					{	
					   if(count1[j]==1)
					   {
					  				   
						dev[0] = 2*j;
						dev[1] = 2*j+1;
						bus(dev[0],&pch,RD,DATA); // read the pc high to the device vector
                		bus(dev[1],&pcl,RD,DATA) ;//read pc light byte
						pc=pch<<8&pcl;   //the IRQn 
						prog_mem_fetch(); 
						break; 	// find the interrupt
						}
					}
				}
				
				else if(count>1) // multiple
				{		
					
					  /* we initialize the group a >b assume only timer and UART interrupt
					  	UART > TIMER, thus do uart first, and then timer
					  */
						/*UART FIRST*/ 
					  	dev[0] = 2*3;
						dev[1] = 2*3+1;
						bus(dev[0],&pch,RD,DATA); // read the pc high to the device vector
                		bus(dev[1],&pcl,RD,DATA) ;//read pc light byte
						pc=pch<<8&pcl;   //the IRQn 
						prog_mem_fetch();
						reg_mem[IRQ].contents&=~IRQ3;// CLEAR IRQ 3 BIT
				}
			
						
		}
				
					
				 
					
					
	}
				
				
                



     sys_clock++;
     sanity++;    
}


