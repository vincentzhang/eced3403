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


//variable for case test
BYTE src;        //SRC operand 
BYTE dst;        // DST operand 
BYTE hnib;   //MS nibble 
BYTE lnib;    // LS nibble 
int C=0,Z=0,S=0,V=0,F2=0,F1=0;//



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
	 inst = prog_mem_fetch();//get instruction
     	 hnib = HBYTE(inst);//get MS Nibble
     	 lnib = LBYTE(inst);//get LS nibble	
     	 
     	 /*check LS 2-7 the special case*//*----------------need to fill up*/
     	 
     	 
     	if (lnib >= 0x08)
	{
	  switch(lnib)
          {
          case 0x08: // LD r, R 
               src = prog_mem_fetch();
               reg_mem[RPBLK|hnib].contents = reg_mem[src].contents;//load the contents from source to dest
               sanity+=6; 
          	break;  
          case 0x09: // LD R,r 
               dst = prog_mem_fetch();
               reg_mem[dst].contents = reg_mem[(RP<<4)| hnib].contents;//
               sanity+=6; 
               break;               	
          case 0x0A: //DJNZ r, dst
               dst = prog_mem_fetch();
               src= (RPBLK| hnib;
               reg_mem[src].contents-=1;
               if (reg_me[src].contents!= 0)		 
			   pc = pc+SIGN_EXT(dst);//jump to the relative address is +127-128 
               break;
          
		  case 0x0B: /* JR cc,RA*/     
          	   cc = hnib;           //????????????????????
          	   dst = prog_mem_fetch();
          	   if(check_cc(cc)) //cc is true   
          		pc = pc+ SIGN_EXT(dst);
          	    break;               
          
		  case 0x0C: /* LD dst, IMM */
               src = prog_mem_fetch();
               reg_mem[RPBLK | hnib] = src;    
              break;
          
		  case 0x0D: // JP cc,DA
          	   cc = hnib;
		  	   dsth = prog_mem_fetch();  
		  	   dstl = prog_mm_fetch();
			   if(check_cc(cc))			   	   
				    pc = dsth<<8|dstl;//the condition true
			   break;              
          
		  case 0x0E: /* INC dst re r 6*/   //??????????????  not sure flag 
               dst = RPBLK | hnib; 
               sign = SIGN(reg_mem[dst]);//check the sign
               reg_mem[dst] += 1;
               
			  ///???????????   check example 
              
               break;
            
          case 0x0F: /* STOP .. NOP */
          switch (hnib)
               {
               case 0x06: // stop 6f  
                    running = FALSE;
               		break;
               //case 0x07: /*HALT */???????????????????????????????????????
               		
               case 0x08: /* DI dst */
               		reg_mem[IMR].contents &= ~INT_ENA;//disable interrrupt	
               		sanity+=6;
               		break;
               
               case 0x09: // EI  
               		reg_mem[IMR].contents |= INT_ENA;//IMR  0x80 bit 7 set
               		sanity+=6;
               		break;
               
               case 0x0A: /* RET     ??????????????????????
               		temphigh = reg_mem[SPH];
               		templow = reg_mem[SPL];
               		pc = ((temphigh << 8) | templow);
               		reg_mem[SPL] += 2;
               		C=Z=S=V=D=H=F2=F1=0;*/
               		sanity+=14;
               break;
               
               
			   case 0x0B: //IRET  ????????  INTERRUPT RETURN  check interrupt with below code
               		
					   
					pc = PC_saved ;
					reg_mem[FLAGS].contents= FLAGS_saved ;
					reg_mem[IMR] . contents |= INT_ENA;//reenable the interrupt
					printf("Interrupt return to %04x\n",pc);
					printf("Flag before interrupt %02x\n",FLAGS_saved);
               		sanity+=16;
            
               break;
               
               case 0x0C: //RCF 
               		reg_mem[FLAGS].contents	 = FLAG_C(0); // clear flag C to 0 
					sanity+=6; 
               		break;
               
               case 0x0D: //SCF 
                    reg_mem[FLAGS].contents	 = FLAG_C(1); // Set carry flag
					sanity+=6;	
               		break;
               
               case 0x0E: //CCF 
					int c1// c flag complement 
					c1=~C;
					reg_mem[FLAGS].contents	 = FLAG_C(c1);
					sanity+=6;
			        break;
			   
               case 0x0F: /* NOP */
               		sanity+=6;
               		break;
               }
          break;
          }
     }
  
  /*check low nibblee through 0x00 0x07*/
  else if((lnib>=0x02)&&(lnib<=0x07))
     {
	 	/*IF high nib through 0-7 and A,B low nibble could be 2-7*/  
		 if((hnib>=0x00 && hnib<=0x07) || (high_nib==0X0A)||(hnib==0x0B) )
		{ 						
				
				/*determide the address mode*/
				switch(lnib)
				{
					case 0x02: /* r r */
						ds = prog_mem_fetch();
						dst = reg_mem[RPBLK|HBYTE(ds)].contents;
						src = reg_mem[RPBLK|LBYTE(ds)].contents;
						temp = RPBLK|MSN(ds);	
						sanity+=6;					
						break;
					
					case 0x03: /* r Ir */
						ds = prog_mem_fetch();
						dst = reg_mem[RPBLK|HBYTE(ds)].contents;
						src = reg_mem[RPBLK|LBYTE(ds)].contents;
						src = reg_mem[src].contents; // get the content of location
						temp = RPBLK|HBYTE(ds);
						sanity+=6; 
						break;
					case 0x04: /* R R */
						src = prog_mem_fetch();
						dst = prog_mem_fetch();
						src = reg_mem[src].contents;
						dst = reg_mem[dst].contents;
						sanity+=10;
						break;
					case 0x05: /* R IR */
						src = prog_mem_fetch();
						dst = prog_mem_fetch();
						src = reg_mem[src].contents; // the content is new location 
						src = reg_mem[src].contents; 
						dst = reg_mem[dst].contents;
						sanity+=10;
						break;
					
					case 0x06: /* R IM */
						dst = prog_mem_fetch();
						src = prog_mem_fetch(); 
						dst = reg_mem[dst].contents;
						sanity+=10;
					    break;
					
					case 0x07: /* IR IM */
						dst = prog_mem_fetch();
						src = prog_mem_fetch();
						dst = reg_mem[dst].contents; // the content is new location 
						dst = reg_mem[dst].contents; //get the content
						sanity+=10;
					    break;
				}
				
				pre_sign = SIGN(dst); // sign of previous value												
				
				switch(hnib)
				{
					case 0x00: /* ADD */
						
						
						pre_carry = reg_mem[temp].contents = post_carry = dst + src; // last operation value
																			  // last operation value with carry byte
						//reg_mem[temp].contents = post_carry = dst + src;													
						reg_mem[FLAGS].contents	 = FLAG_H((pre_carry&0x0F)!=((dst&0x0F)+(src&0X0F))); //set if a carry from the low-order nibble occurred 
						flag_d = 0;
						break;
					case 0x01: /* ADC */
						/* dst = dst + src +c
						   first, get the sum of dst and src without carry -> sum
						   second, get the sum of dst and src with carry,
						   then right shift 8 bits -> 0000000c,
						   sum = sum + 0000000c
						*/
						pre_carry = reg_mem[temp].contents = post_carry = ((dst + src)&0xFF) + ((dst + src)>>8);
						//reg_mem[temp].contents = post_carry = ((dst + src)&0xFF) + ((dst + src)>>8);
						reg_mem[FLAGS].contents	 = FLAG_H((pre_carry&0x0F)!=((dst&0x0F)+(src&0X0F))); //set if a carry from the low-order nibble occurred 
						flag_d = 0;
						break;
					case 0x02: /* SUB */
						pre_carry = reg_mem[temp].contents = post_carry = dst - src;
						//reg_mem[temp].contents = post_carry = dst - src;
						reg_mem[FLAGS].contents	 = FLAG_H((pre_carry&0x0F)!=((dst&0x0F)-(src&0X0F))); //set if a carry from the low-order nibble occurred 
						flag_d = 1;
						break;
					case 0x03: /* SBC */
						pre_carry = reg_mem[temp].contents = post_carry = ((dst - src)&0xFF) - ((dst - src)>>8); //similar idea as ADC
						//reg_mem[temp].contents = post_carry = ((dst - src)&0xFF) - ((dst - src)>>8); //similar idea as ADC
						reg_mem[FLAGS].contents	 = FLAG_H((pre_carry&0x0F)!=((dst&0x0F)-(src&0X0F))); //set if a carry from the low-order nibble occurred  
						flag_d = 1;
						break;
					case 0x04: /* OR */
						pre_carry = reg_mem[temp].contents = post_carry = dst | src;
						//reg_mem[temp].contents = post_carry = dst | src;
						C=H=D=V=0;
						reg_mem[FLAGS].contents	 = FLAG_V(0);
						break;
					case 0x05: /* AND */
						pre_carry = reg_mem[temp].contents = post_carry = dst & src;
						//reg_mem[temp].contents = post_carry = dst & src;
						C=H=D=V=0;
						reg_mem[FLAGS].contents	 = FLAG_V(0);
						break;
					case 0x06: /* TCM */
						post_carry = (~dst) & src;
						C=H=D=V=0;
						reg_mem[FLAGS].contents	 = FLAG_V(0);
						break;
					case 0x07: /* TM */
						post_carry = dst & src;
						C=H=D=V=0;
						reg_mem[FLAGS].contents	 = FLAG_V(0);
						break;
					case 0x0A: /* CP */
						post_carry = dst - src;
						H=D=0;
						break;
					case 0x0B: /* XOR */
						pre_carry = reg_mem[temp].contents = post_carry = dst ^ src;
						//reg_mem[temp].contents = post_carry = dst ^ src;
						C=H=D=V=0;
						reg_mem[FLAGS].contents	 = FLAG_V(0);
						break;
				}
					
        }
     
 /*check if flag satisfy the condition
 yes return 1
 no return 0
 otherwise-1
 */
 int check_cc(BYTE cc)
 {
	switch(LBYTE(cc))
	{
		case 0x00://0000 F Always false
		return 0;
		
		case 0x01://0001 LT Less than (S XOR V) 1
		return (S^V);
		
		
		case 0x02://0010 LE Less than or equal (Z OR (S XOR V))=1
		return(Z | (S^V));
		
		case 0x03://0011 ULE Unsigned less than or equal (C OR Z) = 1

		return(C | Z);
		
		case 0x04://0100 OV Overflow V=1
		return V;
		
		case 0x05://0101 MI Minus S=1
		return S;
		
		case 0x06://0110 EQ Equal Z = 1
		return Z;
		
		case 0x07://0111 ULT Unsigned less than C=1
		return C;
		
		case 0x08://1000 (blank) Always true
		return 1;
		
		case 0x09://1001 GE Greater than or equal (S XOR V)= 0
		return(!(S^V));
		
		case 0x0A://1010 GT Greater Than (Z OR (S XOR V))=O
		return(!(Z | (S^V)));
		
		
		case 0x0B://1011 UGT Unsigned greater than (C=O AND Z=O)=1 
		return(!(C|Z));
		
		case 0x0C://1100 NOV No overflow V = 0  
		return(!V);
		
		case 0x0D://1101 PL Plus S= 0
		return(!S);
		
		case 0x0E://1110 NE Not equal Z = 0 
		return(!Z);
		
		case 0x0F://1111 UGE Unsigned greater than equal C=0;
		return(!C);
		
		
		default: 
		return-1;
	}
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


