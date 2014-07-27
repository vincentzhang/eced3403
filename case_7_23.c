/*
 Z8 Machine emulator with interrupt emulation - skeletal machine
 
 ECED 3403
 17 June 2014
*/

#include "Z8_IE.h"

unsigned long sys_clock;

/* Memory arrays */
BYTE memory[2][PD_MEMSZ];        /* PROG and DATA */

/* Hidden registers */
WORD pc;              /* Program counter */
BYTE intr_ena;        /* Interrupt status */

/*define the for flag*/
int C=reg_mem[FLAGS].contents>>7; //carry flag bit 7 
int Z=(reg_mem[FLAGS].contents>>6)&0X01;//zero flag bit 6 
int S=(reg_mem[FLAGS].contents>>5)&0X01;//Negetive Flag bit 5  
int V=(reg_mem[FLAGS].contents>>4)&0X01;// Flag Vbit 4 
int D=(reg_mem[FLAGS].contents>>3)&0X01;//flag D bit 3
int H=(reg_mem[FLAGS].contents>>2)&0X01;//Flag H Bit2
int F2=(reg_mem[FLAGS].contents>>1)&0X01;//F2 bit1
int F1=reg_mem[FLAGS].contents&0X01;//F1 bit0

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

 /* check if flag satisfy the condition
    yes return 1
    no return 0
    otherwise -1
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
		return -1;
	}
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
sys_clock = 0;

//variable for case test
BYTE src;        // SRC operand 
BYTE dst;        // DST operand 
BYTE hnib;       // MS nibble 
BYTE lnib;    	 // LS nibble 
BYTE cc;    	 //condition code
BYTE dsth; 		 // 16 bit dst high byte part 
BYTE dstl; 		 //16 bit dst low byte part
BYTE ds; 		 // 4bit dst + 4bit src
BYTE temp;		 // is the address of register to store the result

BYTE result; 	 // the 8 bit result without carry bit  
CARRY_BYTE ctest; // value with carry bit
//BYTE htest // 8 bit for checking low nibble H flag
BYTE adr1;		 // address for Irr 
BYTE adr2; 		 // address for Irr 


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

unsigned int C=0,Z=0,S=0,V=0,F2=0,F1=0;//Initial flag all clear
while (running && sanity < 24)
{	
	 inst = prog_mem_fetch();//get instruction
	 hnib = HBYTE(inst);//get MS Nibble
	 lnib = LBYTE(inst);//get LS nibble	
     	
	/* Check for lnib: 
	 case:0x08 - 0x0E 
	 case 0x0F - no-operand instruction*/
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
		  case 0x0B: /* JR cc,RA (Jump Relative based on condition codes)*/     
          	   // first byte: cc|opcode
          	   // second byte: dst
          	   // third byte address: PC. 
          	   cc = hnib;           // Fixed. ????????
          	   dst = prog_mem_fetch(); // pc has been incremented in this command
          	   if(check_cc(cc)) // if cc is true   
			     pc = pc + SIGN_EXT(dst);
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
		  case 0x0E: /* INC dst re r 6*/  //??????????????  not sure flag  
               dst = RPBLK | hnib; 
               sign = SIGN(reg_mem[dst]);//check the sign
               reg_mem[dst] += 1;
               
			   ///???????????   check example 
               // ZVS are affected
               /* Update flags */
               FLAGS = FLAG_Z(reg_mem[dst] == 0);
               /* 
               FLAG_S(SIGN(reg_mem[dst]));
               FLAG_V(SIGN(reg_mem[dst]) != sign); 
               */
               
               break;
            
          case 0x0F: /* STOP .. NOP */ 
          	   switch (hnib)
               {
	   		   		  case 0x06: // stop 6f  
                      running = FALSE;
               		  break;
               		  
				 	  //case 0x07: /*HALT suspends execution and waits for an interrupt.*/???????????????
               		
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
					        // TODO:
					   		// FLAGS <- @SP
					   		// SP <- SP+1
					   		// PC <- @SP
					   		// SP <- SP+2
					   		// IMR(7) <- 1, set Bit7 of IMR to 1, already done
					   		
							pc = PC_saved; // where is PC_saved defined??
							reg_mem[FLAGS].contents= FLAGS_saved ; // and where defined?
							reg_mem[IMR] . contents |= INT_ENA; // IMR(7)<-1, reenable the interrupt
							#ifdef DIAGNOSTICS
							printf("Interrupt return to %04x\n",pc);
							printf("Flag before interrupt %02x\n",FLAGS_saved);
							#endif
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
						dst = reg_mem[RPBLK|HBYTE(ds)].contents;//the content of register 
						src = reg_mem[RPBLK|LBYTE(ds)].contents; 
						temp = RPBLK|MSN(ds);// reg_no address of register file   	
						sanity+=6;					
						break;
					
					case 0x03: /* r Ir */
						ds = prog_mem_fetch();
						dst = reg_mem[RPBLK|HBYTE(ds)].contents; // the content of registr as dst
						src = reg_mem[RPBLK|LBYTE(ds)].contents; // the content is a address 
						src = reg_mem[src].contents; // get the content of neww address is operand
						temp = RPBLK|HBYTE(ds);
						sanity+=6; 
						break;
					
					case 0x04: /* R R */
						src = prog_mem_fetch();
						dst = prog_mem_fetch();
						temp=dst; // the address for storing destination
						src = reg_mem[src].contents; 
						dst = reg_mem[dst].contents;//the operand
						sanity+=10;
						break;
					
					case 0x05: /* R IR */
						src = prog_mem_fetch();
						dst = prog_mem_fetch();
						temp=dst;
						src = reg_mem[src].contents; // the content is new address 
						src = reg_mem[src].contents;// the conetent of new address
						dst = reg_mem[dst].contents;  
						sanity+=10;
						break;
					
					case 0x06: /* R IM */
						dst = prog_mem_fetch();
						temp=dst;
						src = prog_mem_fetch(); 
						dst = reg_mem[dst].contents;
						sanity+=10;
					    break;
					
					case 0x07: /* IR IM */
						dst = prog_mem_fetch();
						src = prog_mem_fetch();
						dst = reg_mem[dst].contents; // the content is new location 
						temp=dst;
						dst = reg_mem[dst].contents; //get the content
						sanity+=10;
					    break;
				}
				
												
				
				switch(hnib)
				{
					case 0x00: /* ADD *////????????????????????????????
						
						result=dst+src; 
						reg_mem[temp].contents=result;// store the result back to the dst add
					    ctest = dst + src; // 16 bit 
						if(ctest-result==0)//if they r equal no carry flag
						 FLAG_C(0);
						 else 
						 FLAG_C(1); // flag c set										
			 			
						if(LBYTE(result)!=(LBYTE(dst)+LBYTE(src)))
			 			FLAG_H(1);  //set if a carry from the low-order nibble occurred 
			 			else
			 			FLAG_H(0);
						
						FLAG_D(0);// D always reset to zero 
						break;
					
					case 0x01: /* ADC */ //  ???????????????????????????????????
						/* dst = dst + src +c
						   first, get the sum of dst and src without carry -> sum
						   second, get the sum of dst and src with carry,
						   then right shift 8 bits -> 0000000c,
						   sum = sum + 0000000c
						*/
						result=reg_mem[temp].contents= ctest = ((dst + src)&0xFF) + ((dst + src)>>8); //??????????????????????????????????
						//reg_mem[temp].contents = post_carry = ((dst + src)&0xFF) + ((dst + src)>>8);
						
						if(LBYTE(result)!=(LBYTE(dst)+LBYTE(src)))
			 			reg_mem[FLAGS].contents	 = FLAG_H(1);  //set if a carry from the low-order nibble occurred 
			 			else
			 			reg_mem[FLAGS].contents	 = FLAG_H(0);
						
						reg_mem[FLAGS].contents	 = FLAG_D(0);// D always reset to zero 
						break;
					
					case 0x02: /* SUB */
						result = reg_mem[temp].contents = ctest = dst - src;
						
					if(LBYTE(result)!=(LBYTE(dst)-LBYTE(src)))
			 			FLAG_H(1);  //set if a carry from the low-order nibble occurred 
			 			else
			 			reg_mem[FLAGS].contents	 = FLAG_H(0);
						
						reg_mem[FLAGS].contents	 = FLAG_D(1);// D always set to 1
						break;
					case 0x03: /* SBC */
						pre_carry = reg_mem[temp].contents = post_carry = ((dst - src)&0xFF) - ((dst - src)>>8); //similar idea as ADC  ?????????????
						//reg_mem[temp].contents = post_carry = ((dst - src)&0xFF) - ((dst - src)>>8); //similar idea as ADC
						
						regif(LBYTE(result)!=(LBYTE(dst)-LBYTE(src)))
			 			reg_mem[FLAGS].contents	 = FLAG_H(1);  //set if a carry from the low-order nibble occurred 
			 			else
			 			reg_mem[FLAGS].contents	 = FLAG_H(0);
						
						reg_mem[FLAGS].contents	 = FLAG_D(1);// D always set to 1
						break;
						break;
					
					case 0x04: /* OR */
						result = reg_mem[temp].contents = ctest = dst | src;
						reg_mem[FLAGS].contents	 = FLAG_V(0);
						break;
					
					case 0x05: /* AND */
						result = reg_mem[temp].contents = ctest = dst & src;
						//reg_mem[temp].contents = post_carry = dst & src;
						reg_mem[FLAGS].contents	 = FLAG_V(0);
						break;
					case 0x06: /* TCM */
						result = (~dst) & src;
						reg_mem[FLAGS].contents	 = FLAG_V(0);
						break;
					
					case 0x07: /* TM */
						result = dst & src;
						reg_mem[FLAGS].contents	 = FLAG_V(0);
						break;
					case 0x0A: /* CP */
						result = dst - src;
						break;
					case 0x0B: /* XOR */
						result = reg_mem[temp].contents = ctest = dst ^ src;
						//reg_mem[temp].contents = post_carry = dst ^ src;
						
						reg_mem[FLAGS].contents	 = FLAG_V(0);
						break;
				}
		}
				
		 else
		{
			switch(hnib)
			{		
				case 0x08:/*LDE r, Irr ;LDEI Ir, Irr*/ 
						   
					switch(lnib)
					{
						case 0x02:/* LDE r, Irr */
							ds = prog_mem_fetch();
							dst = RPBLK|HBYTE(ds); // dst is address
							src = RPBLK|LSN(ds); // scr is address to register file
							adr1= reg_mem[src].contents; // the new address1 
							adr2 = reg_mem[src+1].contents; //the new ad
							/*lad the exteernal data from data mem into address dst  */
							reg_mem[dst].contents = memory[DATA][(adr1<<8)|adr2];
							sanity+=12; 
							break;
						
						case 0x03:/* LDEI Ir, Irr */
							ds= prog_mem_fetch();
							dst = RPBLK|HBYTE(ds);
							src = RPBLK|LBYTE(ds);
							dst = reg_mem[dst].contents;//  address
							adr1= reg_mem[src].contents;//
							adr2 = reg_mem[src+1].contents;
							/*r<----r+1*/
							reg_mem[dst].contents = memory[DATA][(adr1<<8)|(adr2)];
							reg_mem[dst+1].contents = memory[DATA][((adr1<<8)|(adr2))+1];
							sanity+=18;
						break;				
					}	
				
				break;			
				
				case 0x09:/*LDE Irr, r 
							LDEI Irr, Ir 
						   */
					switch(low_nib)
					{
						case 0x02:/* *LDE Irr, r */
							ds= prog_mem_fetch();
							src = RPBLK|HBYTE(ds);
							dst = RPBLK|LBYTE(ds);
							adr1= reg_mem[dst].contents;
							adr2 = reg_mem[dst].contents;
							reg_mem[src].contents = memory[DATA][(adr1<<8)|adr2];
							
							sanity+=12; 
						break;
						
						case 0x03:/* LDEI Irr, Ir */
							ds = prog_mem_fetch();
							src = RPBLK|HBYTE(ds);
							dst = RPBLK|LBYTE(ds);
							
							src = reg_mem[src].contents;//address of src
							adr1 = reg_mem[dst].contents;
							adr2 = reg_mem[dst+1].contents;
							reg_mem[src].contents = memory[DATA][(adr1<<8)|(adr2)]; 
							reg_mem[src+1].contents = memory[DATA][((adr1<<8)|(adr2))+1];
						 
							sanity+=18;
						break;				
					}	
				
				break;	
				
				case 0x0C: /*LDC r, Irr ; LDCI Ir, Irr */
					switch(lnib)
					{
						case 0x02:/* LDC r, Irr */
							ds = prog_mem_fetch();
							dst = RPBLK|HBYTE(ds);
							src = RPBLK|LBYTE(ds);
							/*IRR*/
							adr1= reg_mem[src].contents;
							adr2= reg_mem[src+1].contents;
							reg_mem[dst].contents = memory[PROG][(adr1<<8)|adr2];// load src from program meminto dst
						
							sanity+=12;
							break;
						
						case 0x03:/* LDCI Ir, Irr */
							ds = prog_mem_fetch();
							dst = RPBLK|HBYTE(ds);
							src = RPBLK|LBYTE(ds);
							dst = reg_mem[dst].contents;//address
							adr1= reg_mem[src].contents;//Irr address
							adr2= reg_mem[src+1].contents;//Irr address
							reg_mem[dst].contents = memory[PROG][(adr1<<8)|(adr2)];
							reg_mem[dst+1].contents = memory[PROG][((adr1<<8)|(adr2))+1];//r+1 rr+1
							
							sanity+=18; 
							break;				
					}	
					
				break;			
				
				case 0x0D:/*LDC Irr, r; LDCI Irr, Ir 
						   */
					switch(lnib)
					{
						case 0x02:/* *LDC Irr, r */
							ds = prog_mem_fetch();
							src = RPBLK|HBYTE(ds);
							dst = RPBLK|LBYTE(ds);
							adr1= reg_mem[dst].contents;
							adr2 = reg_mem[dst].contents;
							reg_mem[src].contents = memory[PROG][(adr1<<8)|adr2];
							sanity+=12;
							break;
						
						case 0x03:/* LDCI Irr, Ir */
							ds = prog_mem_fetch();
							src = RPBLK|HBYTE(ds);
							dst = RPBLK|LBYTE(ds);
							src = reg_mem[src].contents;//
							adr1 = reg_mem[dst].contents;
							adr2 = reg_mem[dst+1].contents;
							reg_mem[src].contents = memory[PROG][(adr1<<8)|(adr2)];
							reg_mem[src+1].contents = memory[PROG][((adr1<<8)|(adr2))+1];//r+1 rr+1
							sanity+=18; 
							break;				
					}	
					
				break;	
				
				case 0x0E:
          		switch(lnib)
          		{
					case 0x03: /* LD r Ir */
					ds= prog_mem_fetch();
					src=reg_mem[RPBLK|LBYTE(ds)].contents;// adress
					reg_mem[RPBLK|HBYTE(ds)].contents = reg_mem[src].contents;// load the src into dst
					sanity+=6;
					break;
						
					case 0x04: /* LD R,R */	
				  	src = prog_mem_fetch();//adress
				  	dst = prog_mem_fetch();//adress
				    reg_mem[dst].contents = reg_mem[src].contents;// load content of src into destination 
				  	
					sanity+=10;
                	break;
						            	
              		case 0x05: /* LD R,IR */
                  	src = prog_mem_fetch();
				  	dst = prog_mem_fetch();
				  	src = reg_mem[src].contents;//addrress
				  	reg_mem[dst].contents = reg_mem[src].contents;
				  
				  	sanity+=10;
                	break;
                		
              		case 0x06: /* LD R,IM */	
                  	dst = prog_mem_fetch();// address
				  	src = prog_mem_fetch();//value
				  	reg_mem[dst].contents=src;
				  	sanity+=10;
                	break;   
						         	
              		case 0x07: /* LD IR,IM */
              		dst = prog_mem_fetch();
				  	src = prog_mem_fetch();
					dst = reg_mem[dst].contents;//address
					reg_mem[dst].contents = src;
					sanity+=10;
                	break;	          	
          		}
          	
		  		break;
		  
		  		case 0x0F:
		  		switch(lnib)
		  		{
		  			case 0x03: /* LD Ir r*/
		  			ds = prog_mem_fetch();
					dst = reg_mem[RPBLK|HBYTE(ds)].contents;
					src = reg_mem[RPBLK|LBYTE(ds)].contents;
					//reg_mem[dst].contents = src;
					sanity+=6;
					break;
						
					case 0x05: /* LD IR R*/
					src = prog_mem_fetch();
					dst = prog_mem_fetch();
					src = reg_mem[src].contents;
					dst = reg_mem[dst].contents;
					//reg_mem[dst].contents = src;
					sanity+=10;
					break;			
		  		}
		  		write_rm(dst,src);
		  		
		  		break;      
     	  	} 	
		}
    }				
	 else if (lnib < 0x02) 
    {	/* the rest will be the same instruction but R an IR*/
		if(high_nib!=3 && high_nib !=8 && high_nib!=10)
		{
		switch(low_nib)
		{
			case 0x00: //R
				dst = prog_mem_fetch();//address
				dst = reg_mem[temp].contents;// content
				break; 
			case 0x01: //IR
				dst = prog_mem_fetch();
				dst = reg_mem[dst].contents; //new address
				dst = reg_mem[dst].contents; //the content
				break;
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



