#include "Z8_emulator.h"
#include <stdio.h>
#include <stdlib.h>
/*testtest*/
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
BYTE src;        /* SRC operand - read if needed */
BYTE dst;        /* DST operand - read if needed */
BYTE high_nib;   /* MS nibble of instruction */
BYTE low_nib;    /* LS nibble of instruction */
BYTE sign;       /* Sign bit before arithmetic - for overflow */
int running;     /* TRUE until STOP instruction */
int sanity;      /* Limit on number of instruction cycles */
BYTE temp;
BYTE temp_rp;
BYTE temp_rp_0;
BYTE temp_rp_1;
BYTE temphigh;
BYTE templow;
BYTE pre_sign;
BYTE pre_carry; // value without carry bit
CARRY_BYTE post_carry; // value with carry bit
BYTE flag_d;       
running = TRUE;
sanity = 0;
BYTE test1;
BYTE test2;

while (running && sanity < 20)
{
     /* Get next instruction and extract nibbles */
     inst = prog_mem_fetch();
     high_nib = MSN(inst);
     low_nib = LSN(inst);
     int C=1,Z=1,S=1,V=1,D=1,H=1,F2=1,F1=1;// init flags
     /* Check for special cases: 
        - lnib: 0x08 through 0x0E - r or cc in hnib 
        - lnib: 0x0F - no-operand instructions */
     if (low_nib >= 0x08)
     {
          /* LD through INC and some no-operand instructions */
          switch(low_nib)
          {
          case 0x08: /* LD r, R */
               src = prog_mem_fetch();
               reg_mem[RPBLK | high_nib] = reg_mem[src];   
			   C=Z=S=V=D=H=F2=F1=0;// no flags affected      	   
          	   break; 
				  
          case 0x09: /* LD R,r */
               dst = prog_mem_fetch();
               reg_mem[dst] = reg_mem[RPBLK | high_nib];
			   C=Z=S=V=D=H=F2=F1=0;// no flags affected 
               break;
	
          case 0x0A: /* DJNZ r, dst */
               dst = prog_mem_fetch();
               src = RPBLK | high_nib;
               reg_mem[src] -= 1;
               if (reg_mem[src] != 0)
                   /* Reg != 0 -- repeat loop */
                   /* Signed extend dst to 16 bits if -ve */
                   pc = pc + SIGN_EXT(dst);
               C=Z=S=V=D=H=F2=F1=0;// no flags affected 
               break;
               
          case 0x0B: /* JR cc,dst */
          	   cc = high_nib;
          	   dst = prog_mem_fetch();
		  	   if(cc&MSN(FLAGS))
		  	   {	
				   pc = pc + SIGN_EXT(dst);
			   }	
			   else
			   	   pc = pc + 1;
			   C=Z=S=V=D=H=F2=F1=0;// no flags affected
			   break;
			   		     
          case 0x0C: /* LD r, IM */
               src = prog_mem_fetch();
               reg_mem[RPBLK | high_nib] = src;
               C=Z=S=V=D=H=F2=F1=0;// no flags affected
               break;
               
          case 0x0D: /* JP cc, dst */
		  	   cc = high_nib;
		  	   dst = prog_mem_fetch();
			   if(cc&MSN(FLAGS))
			   {	   
				   pc = pc + dst;
			   }
			   C=Z=S=V=D=H=F2=F1=0;// no flags affected
			   break;
			     
          case 0x0E: /* INC dst r */
               temp = RPBLK | high_nib;
               dst = reg_mem[temp];
               pre_sign = SIGN(dst); // sign of previous value
               reg_mem[temp] += 1;
			   sign = SIGN(reg_mem[temp]); // sign of post value
			   pre_carry = reg_mem[temp]; // last operation value
			   post_carry = dst + 1; // last operation value with carry byte
			   printf("********R dst: %02x***********\n", pre_carry);
			   C=D=H=F2=F1=0; 
               break;
               
          case 0x0F: /* STOP .. NOP */     
               switch (high_nib)
               {
               case 0x06: /* STOP - added instruction - not on opcode map */
               		//printf("*************here we go %02x***********\n", reg_mem[RPBLK|04]); 
                    running = FALSE;
               		C=Z=S=V=D=H=F2=F1=0;// no flags affected
               break;
               //case 0x07: /*HALT */
               		
               case 0x08: /* DI dst */
               		intr_ena = 0;
               		C=Z=S=V=D=H=F2=F1=0;// no flags affected
               break;
               
               case 0x09: /* EI dst */
               		intr_ena = 1;
               		C=Z=S=V=D=H=F2=F1=0;// no flags affected
               break;
               
               case 0x0A: /* RET */
               		temphigh = reg_mem[SPH];
               		templow = reg_mem[SPL];
               		pc = ((temphigh << 8) | templow);
               		reg_mem[SPL] += 2;
               		C=Z=S=V=D=H=F2=F1=0;// no flags affected
               break;
               
               case 0x0B: /* IRET */
               		temphigh = reg_mem[SPH];
               		templow = reg_mem[SPL];
               break;
               
               case 0x0C: /* RCF */
               		FLAGS = FLAG_C(0); // Set flag C to 0 
					C=Z=S=V=D=H=F2=F1=0;
                    printf("*********FLAGS: %02x*********\n", FLAGS); 
               break;
               
               case 0x0D: /* SCF */
                    FLAGS = FLAG_C(1); // Set flag C to one
                    C=Z=S=V=D=H=F2=F1=0;
                    printf("*********FLAGS: %02x*********\n", FLAGS);	
               break;
               
               case 0x0E: /* CCF */
               		//FLAGS = FLAG_C(1);
					FLAGS = (FLAGS + 0x80) & 0xFF;
					C=Z=S=V=D=H=F2=F1=0;
					printf("*********FLAGS: %02x*********\n", FLAGS);
			   break;
			   
               case 0x0F: /* NOP */
               		C=Z=S=V=D=H=F2=F1=0;// no flags affected
               break;
               }
          break;
          }
     }
     else if((low_nib>=2)&&(low_nib<=7))
     {
	 	if((high_nib>=0 && high_nib<=7) || (high_nib==10) || (high_nib==11))
		{ 
			printf("HI\n");						
				switch(low_nib)
				{
					case 0x02: /* r r */
						temp = prog_mem_fetch();
						dst = reg_mem[RPBLK|MSN(temp)];
						src = reg_mem[RPBLK|LSN(temp)];
						temp = RPBLK|MSN(temp);						
						break;
					case 0x03: /* r Ir */
						temp = prog_mem_fetch();
						dst = reg_mem[RPBLK|MSN(temp)];
						src = reg_mem[RPBLK|LSN(temp)];
						src = reg_mem[src]; // get the content of new location
						temp = RPBLK|MSN(temp); 
					break;
					case 0x04: /* R R */
						src = prog_mem_fetch();
						temp = prog_mem_fetch();
						src = reg_mem[src];
						dst = reg_mem[temp];
					break;
					case 0x05: /* R IR */
						src = prog_mem_fetch();
						temp = prog_mem_fetch();
						src = reg_mem[src]; // use src byte's content as new location 
						src = reg_mem[src]; // get the content of new location
						dst = reg_mem[temp];
					break;
					case 0x06: /* R IM */
						temp = prog_mem_fetch();
						src = prog_mem_fetch(); 
						dst = reg_mem[temp];
					break;
					case 0x07: /* IR IM */
						dst = prog_mem_fetch();
						src = prog_mem_fetch();
						temp = reg_mem[dst]; //use dst byte's content as new location
						dst = reg_mem[temp]; //get the content of the new location
					break;
				}
				/*common codes part*/
				pre_sign = SIGN(dst); // sign of previous value												
				switch(high_nib)
				{
					case 0x00: /* ADD */
						pre_carry = reg_mem[temp] = post_carry = dst + src; // last operation value
																			  // last operation value with carry byte
						//reg_mem[temp] = post_carry = dst + src;													
						FLAGS = FLAG_H((pre_carry&0x0F)!=((dst&0x0F)+(src&0X0F))); //set if a carry from the low-order nibble occurred 
						flag_d = 0;
						break;
					case 0x01: /* ADC */
						/* dst = dst + src +c
						   first, get the sum of dst and src without carry -> sum
						   second, get the sum of dst and src with carry,
						   then right shift 8 bits -> 0000000c,
						   sum = sum + 0000000c
						*/
						pre_carry = reg_mem[temp] = post_carry = ((dst + src)&0xFF) + ((dst + src)>>8);
						//reg_mem[temp] = post_carry = ((dst + src)&0xFF) + ((dst + src)>>8);
						FLAGS = FLAG_H((pre_carry&0x0F)!=((dst&0x0F)+(src&0X0F))); //set if a carry from the low-order nibble occurred 
						flag_d = 0;
						break;
					case 0x02: /* SUB */
						pre_carry = reg_mem[temp] = post_carry = dst - src;
						//reg_mem[temp] = post_carry = dst - src;
						FLAGS = FLAG_H((pre_carry&0x0F)!=((dst&0x0F)-(src&0X0F))); //set if a carry from the low-order nibble occurred 
						flag_d = 1;
						break;
					case 0x03: /* SBC */
						pre_carry = reg_mem[temp] = post_carry = ((dst - src)&0xFF) - ((dst - src)>>8); //similar idea as ADC
						//reg_mem[temp] = post_carry = ((dst - src)&0xFF) - ((dst - src)>>8); //similar idea as ADC
						FLAGS = FLAG_H((pre_carry&0x0F)!=((dst&0x0F)-(src&0X0F))); //set if a carry from the low-order nibble occurred  
						flag_d = 1;
						break;
					case 0x04: /* OR */
						pre_carry = reg_mem[temp] = post_carry = dst | src;
						//reg_mem[temp] = post_carry = dst | src;
						C=H=D=V=0;
						FLAGS = FLAG_V(0);
						break;
					case 0x05: /* AND */
						pre_carry = reg_mem[temp] = post_carry = dst & src;
						//reg_mem[temp] = post_carry = dst & src;
						C=H=D=V=0;
						FLAGS = FLAG_V(0);
						break;
					case 0x06: /* TCM */
						post_carry = (~dst) & src;
						C=H=D=V=0;
						FLAGS = FLAG_V(0);
						break;
					case 0x07: /* TM */
						post_carry = dst & src;
						C=H=D=V=0;
						FLAGS = FLAG_V(0);
						break;
					case 0x0A: /* CP */
						post_carry = dst - src;
						H=D=0;
						break;
					case 0x0B: /* XOR */
						pre_carry = reg_mem[temp] = post_carry = dst ^ src;
						//reg_mem[temp] = post_carry = dst ^ src;
						C=H=D=V=0;
						FLAGS = FLAG_V(0);
						break;
				}
				//printf("**********%04x***********\n", post_carry);
				/* Update flags */
				F2=F1=0; 		
        }
        else
		{
			switch(high_nib)
			{		
				case 0x08:/*LDE r, Irr 
							LDEI Ir, Irr 
						   */
					switch(low_nib)
					{
						case 0x02:/* LDE r, Irr */
							temp = prog_mem_fetch();
							dst = RPBLK|MSN(temp);
							src = RPBLK|LSN(temp);
							temp_rp_0 = reg_mem[src];
							temp_rp_1 = reg_mem[src+1];
							reg_mem[dst] = memory[DATA][(temp_rp_0<<8)|temp_rp_1];
							printf("*************here we go %02x***********\n", reg_mem[dst]); 
						break;
						
						case 0x03:/* LDEI Ir, Irr */
							temp = prog_mem_fetch();
							dst = RPBLK|MSN(temp);
							src = RPBLK|LSN(temp);
							dst = reg_mem[dst];
							temp_rp_0 = reg_mem[src];
							temp_rp_1 = reg_mem[src+1];
							reg_mem[dst] = memory[DATA][(temp_rp_0<<8)|(temp_rp_1)];
							reg_mem[dst+1] = memory[DATA][((temp_rp_0<<8)|(temp_rp_1))+1];
							printf("*************here we go %02x***********\n", reg_mem[dst]); 
							printf("*************here we go %02x***********\n", reg_mem[dst+1]); 
						break;				
					}	
					C=Z=S=V=D=H=F2=F1=0;// no flags affected
				break;			
				
				case 0x09:/*LDE Irr, r 
							LDEI Irr, Ir 
						   */
					switch(low_nib)
					{
						case 0x02:/* *LDE Irr, r */
							temp = prog_mem_fetch();
							src = RPBLK|MSN(temp);
							dst = RPBLK|LSN(temp);
							temp_rp_0 = reg_mem[dst];
							temp_rp_1 = reg_mem[dst];
							test1 = memory[DATA][(temp_rp_0<<8)|temp_rp_1]=reg_mem[src];
							printf("*************here we go %02x***********\n", test1); 
						break;
						
						case 0x03:/* LDEI Irr, Ir */
							temp = prog_mem_fetch();
							src = RPBLK|MSN(temp);
							dst = RPBLK|LSN(temp);
							src = reg_mem[src];
							temp_rp_0 = reg_mem[dst];
							temp_rp_1 = reg_mem[dst+1];
							test1 = memory[DATA][(temp_rp_0<<8)|(temp_rp_1)] = reg_mem[src]; 
							test2 = memory[DATA][((temp_rp_0<<8)|(temp_rp_1))+1] = reg_mem[src+1];
							printf("*************here we go %02x***********\n",test1); 
							printf("*************here we go %02x***********\n", test2); 
						break;				
					}	
					C=Z=S=V=D=H=F2=F1=0;// no flags affected
				break;	
				
				case 0x0C:/*LDC r, Irr 
							LDCI Ir, Irr 
						   */
					switch(low_nib)
					{
						case 0x02:/* LDC r, Irr */
							temp = prog_mem_fetch();
							dst = RPBLK|MSN(temp);
							src = RPBLK|LSN(temp);
							temp_rp_0 = reg_mem[src];
							temp_rp_1 = reg_mem[src+1];
							reg_mem[dst] = memory[PROG][(temp_rp_0<<8)|temp_rp_1];
							printf("*************here we go %02x***********\n", reg_mem[dst]);
						break;
						
						case 0x03:/* LDCI Ir, Irr */
							temp = prog_mem_fetch();
							dst = RPBLK|MSN(temp);
							src = RPBLK|LSN(temp);
							dst = reg_mem[dst];
							temp_rp_0 = reg_mem[src];
							temp_rp_1 = reg_mem[src+1];
							reg_mem[dst] = memory[PROG][(temp_rp_0<<8)|(temp_rp_1)];
							reg_mem[dst+1] = memory[PROG][((temp_rp_0<<8)|(temp_rp_1))+1];
							printf("*************here we go %02x***********\n", reg_mem[dst]); 
							printf("*************here we go %02x***********\n", reg_mem[dst+1]); 
						break;				
					}	
					C=Z=S=V=D=H=F2=F1=0;// no flags affected
				break;			
				
				case 0x0D:/*LDC Irr, r 
							LDCI Irr, Ir 
						   */
					switch(low_nib)
					{
						case 0x02:/* *LDC Irr, r */
							temp = prog_mem_fetch();
							src = RPBLK|MSN(temp);
							dst = RPBLK|LSN(temp);
							temp_rp_0 = reg_mem[dst];
							temp_rp_1 = reg_mem[dst];
							test1 = memory[PROG][(temp_rp_0<<8)|temp_rp_1]=reg_mem[src];
							printf("*************here we go %02x***********\n", test1); 
						break;
						
						case 0x03:/* LDCI Irr, Ir */
							temp = prog_mem_fetch();
							src = RPBLK|MSN(temp);
							dst = RPBLK|LSN(temp);
							src = reg_mem[src];
							temp_rp_0 = reg_mem[dst];
							temp_rp_1 = reg_mem[dst+1];
							test1 = memory[PROG][(temp_rp_0<<8)|(temp_rp_1)] = reg_mem[src]; 
							test2 = memory[PROG][((temp_rp_0<<8)|(temp_rp_1))+1] = reg_mem[src+1];
							printf("*************here we go %02x***********\n", test1); 
							printf("*************here we go %02x***********\n", test2); 
						break;				
					}	
					C=Z=S=V=D=H=F2=F1=0;// no flags affected
				break;	
				
				case 0x0E:
          		switch(low_nib)
          		{
					case 0x03: /* LD r Ir */
					temp = prog_mem_fetch();
					dst = reg_mem[RPBLK|MSN(temp)];
					src = reg_mem[RPBLK|LSN(temp)];
					reg_mem[RPBLK|MSN(temp)] = reg_mem[src];
					break;
						
					case 0x04: /* LD R,R */	
				  	src = prog_mem_fetch();
				  	dst = prog_mem_fetch();
				  	src = reg_mem[src];
				  	reg_mem[dst] = src;
                	break;
						            	
              		case 0x05: /* LD R,IR */
                  	src = prog_mem_fetch();
				  	dst = prog_mem_fetch();
				  	src = reg_mem[src];
				  	src = reg_mem[src];
				  	reg_mem[dst] = src;
                	break;
                		
              		case 0x06: /* LD R,IM */	
                  	dst = prog_mem_fetch();
				  	src = prog_mem_fetch();
				  	reg_mem[dst]=src;
                	break;   
						         	
              		case 0x07: /* LD IR,IM */
              		dst = prog_mem_fetch();
				  	src = prog_mem_fetch();
					dst = reg_mem[dst];
					reg_mem[dst] = src;
                	break;	          	
          		}
		  		C=Z=S=V=D=H=F2=F1=0;// no flags affected 
		  		break;
		  
		  		case 0x0F:
		  		switch(low_nib)
		  		{
		  			case 0x03: /* LD Ir r*/
		  			temp = prog_mem_fetch();
					dst = reg_mem[RPBLK|MSN(temp)];
					src = reg_mem[RPBLK|LSN(temp)];
					reg_mem[dst] = src;
					break;
						
					case 0x05: /* LD IR R*/
					src = prog_mem_fetch();
					dst = prog_mem_fetch();
					src = reg_mem[src];
					dst = reg_mem[dst];
					reg_mem[dst] = src;
					break;			
		  		}
		  		C=Z=S=V=D=H=F2=F1=0;// no flags affected
		  		break;      
     	  	} 	
		}
    }	
    else if (low_nib < 0x02) 
    {
		if(high_nib!=3 && high_nib !=8 && high_nib!=10)
		{
		
		switch(low_nib)
		{
			case 0x00: //R
				temp = prog_mem_fetch();
				dst = reg_mem[temp];
				reg_mem[temp] -= 1; 
				break; 
			case 0x01: //IR
				dst = prog_mem_fetch();
				temp = reg_mem[dst]; //use dst byte's content as new location
				dst = reg_mem[temp]; //get the content of the new location
			break;
		}
		/*common codes part */
		pre_sign = SIGN(dst); // sign of previous value
		switch(high_nib)
		{
			case 0x00: /* DEC */
				reg_mem[temp] = pre_carry = post_carry = dst - 1;
				C=H=D=F2=F1=0; 
			break;
			
			case 0x01: /* RLC */
				reg_mem[temp] = pre_carry = post_carry = dst<<1 | (FLAGS>>8);
				H=D=F2=F1=0;
			break;
			
			case 0x02: /* INC */
				reg_mem[temp] = pre_carry = post_carry = dst + 1;
				C=H=D=F2=F1=0; 
			break;
				
			case 0x04: /* DA */
				switch(FLAGS&0x8C)
				{
					case 0x00: /*D bit is 0, indicates ADD/ADC
								 C bit is 0 before DA
								 H bit is 0 before DA
								*/
						if(LSN(post_carry)<=9)
						{
							if(MSN(post_carry)<=9) post_carry += 0x00;
							else post_carry += 0x60;	
						}
						else
						{
							if(MSN(post_carry)<=8) post_carry += 0x06;
							else post_carry += 0x66;
						}
						printf("Here it is!!!!!\n");
					break;
					
					case 0x04: /*D bit is 0, indicates ADD/ADC
								 C bit is 0 before DA
								 H bit is 1 before DA
								*/
						if(LSN(post_carry)<=3)
						{
							if(MSN(post_carry)<=9) post_carry += 0x06;
							else post_carry += 0x66;	
						}
						else printf("0 Not for DA\n");
					break;
					
					case 0x80: /*D bit is 0, indicates ADD/ADC
								 C bit is 1 before DA
								 H bit is 0 before DA
							    */
						 if(MSN(post_carry)<=2)
						 {
						 	if(LSN(post_carry)<=9) post_carry += 0x60;
						 	else post_carry += 0x66;
						 }
						 else printf("1 Not for DA\n");
					break;
					
					case 0x08: /*D bit is 1, indicates SUB/SBC
								 C bit is 0 before DA
								 H bit is 0 before DA
								*/
						if((MSN(post_carry)<=9) && (LSN(post_carry)<=9)) post_carry += 0x00;
						else printf("2 Not for DA\n");
					break;
					
					case 0x0C: /*D bit is 1, indicates SUB/SBC
								 C bit is 0 before DA
								 H bit is 1 before DA
								*/
						if((MSN(post_carry)<=8) && (LSN(post_carry)>=6)) post_carry += 0xFA;
						else printf("3 Not for DA\n");
					break;
						 		
					case 0x88: /*D bit is 1, indicates SUB/SBC
								 C bit is 1 before DA
								 H bit is 0 before DA
								*/
						if((MSN(post_carry)>=7) && (LSN(post_carry)<=9)) post_carry += 0xA0;
						else printf("4 Not for DA\n");
					break;
					
					case 0x8C: /*D bit is 1, indicates SUB/SBC
								 C bit is 1 before DA
								 H bit is 1 before DA
								*/
						if((MSN(post_carry)>=6) && (LSN(post_carry)>=6)) post_carry += 0x9A;
						else printf("5 Not for DA\n");
					break;			   			
				}
				//printf("*************After %02x***********\n", post_carry); 	
			break;
			
			case 0x05: /* POP */
			break;
				
			case 0x06: /* COM */
				reg_mem[temp] = pre_carry = post_carry = ~dst;
				C=H=D=V=0;
				FLAGS = FLAG_V(0);
				//printf("*************After %02x***********\n", post_carry); 
			break;
			
			case 0x07: /* PUSH */
			break;
			
			case 0x09: /* RL */
				/* shift 1 bit left to replace the carry, 
					the original bit 0 is replaced with original value in bit 7 
				 */
				reg_mem[temp] = pre_carry = post_carry = (dst<<1)|(dst>>7);
				D=H=F2=F1; 
			break;
			
			case 0x0B: /* CLR */
				reg_mem[temp] = pre_carry = post_carry = 0;
				C=Z=S=V=D=H=F2=F1=0;// no flags affected
			break;
			
			case 0x0C: /* RRC */
				/* shift 1 bit right, the initial value of carry flag replaces bit 7 */
				reg_mem[temp] = pre_carry = post_carry = (dst>>1)|(FLAGS&0x80);
				/* the initial value of bit 0 replace the carry flag */
				FLAGS = FLAG_C(dst&0x01);
				C=H=D=F2=F1=0;
			break;
			
			case 0x0D: /* SRA */
				reg_mem[temp] = pre_carry = post_carry = ((dst>>1)|(dst&0x80));
            	FLAGS = FLAG_C(dst&0X01);
            	FLAGS = FLAG_V(0);
				C=V=H=D=F2=F1=0;
			break;
			
			case 0x0E: /* RR */
				reg_mem[temp] = pre_carry = post_carry = ((dst>>1)|((dst&0x01)<<7));
            	FLAGS = FLAG_C(dst&0X01);
				C=H=D=F2=F1=0;
			break;
			
			case 0x0F: /* SWAP */
				reg_mem[temp] = pre_carry = post_carry = (LSN(dst)<<4 | MSN(dst));
				C=V=H=D=F2=F1=0;
			break;
		}
			/*common codes part */
			//reg_mem[temp] -= 1;
			//sign = SIGN(reg_mem[temp]); // sign of post value
			//pre_carry = reg_mem[temp]; // last operation value
			//post_carry = dst -1; // last operation value with carry byte
			printf("*************here we go %02x***********\n", pre_carry); 
			
		}
		else if(high_nib==8 || high_nib==10)
		{
			
		}
		else
		{
			switch(low_nib)
			{
				case 0x00:
				{
					temp = prog_mem_fetch();
					temp = reg_mem[RPBLK|LSN(temp)];
					temp_rp = reg_mem[RPBLK|(LSN(temp)+1)];
					pc = (temp_rp<<8)|temp;
				}
			}

		}
				
    }
          

/* Update flags */
sign = SIGN(post_carry&0xFF); // sign of post value
//if(C) FLAGS = FLAG_C(post_carry != (post_carry&0xFF));
if(C) FLAGS = FLAG_C(CARRY(post_carry));
if(Z) FLAGS = FLAG_Z((post_carry&0xFF) == 0); 
if(S) FLAGS = FLAG_S(sign != 0 );
if(V) FLAGS = FLAG_V(pre_sign != sign); //if previous is not equal to new, overflow
if(D) FLAGS = FLAG_D(flag_d);
if(C+Z+S+V+D) printf("*********FLAGS: %02x*********\n", FLAGS);	
sanity++;
}

}

test_prog()
{
/* 
  Test program -- this is a slow (as opposed to quick) and dirty way to 
  exercise most of the above instructions.  Better to use the assembler.
  The program works.
*/
int ldaddr = 0x1000;

/***************************TEST*******************************/
/*
memory[PROG][ldaddr++] = 0xE6;
memory[PROG][ldaddr++] = 0xF0;//DST R
memory[PROG][ldaddr++] = 0x15;//SRC IMM

memory[PROG][ldaddr++] = 0xE6;  
memory[PROG][ldaddr++] = 0xF1; //DST R
memory[PROG][ldaddr++] = 0x27; //SRC IM

memory[PROG][ldaddr++] = 0x04;  
memory[PROG][ldaddr++] = 0xF1; //SRC 01
memory[PROG][ldaddr++] = 0xF0; //DST 0F

memory[PROG][ldaddr++] = 0x40;
memory[PROG][ldaddr++] = 0xF0; //DST 0F

memory[PROG][ldaddr++] = 0x6F;  //STOP
*/
 

memory[PROG][ldaddr++] = 0x6C; //R6 
memory[PROG][ldaddr++] = 0X30; //LD r6,#%30

memory[PROG][ldaddr++] = 0x7C;  
memory[PROG][ldaddr++] = 0xA2; //LD r7,#%A2

memory[PROG][ldaddr++] = 0x2C;  
memory[PROG][ldaddr++] = 0x22; //LD r2,#%A2

memory[PROG][ldaddr++] = 0xD2; //LDC @rr6, @R2
memory[PROG][ldaddr++] = 0x26;      
memory[PROG][ldaddr++] = 0x6F; //STOP


/* 
memory[PROG][ldaddr++] = 0xE6; //LD R IM
memory[PROG][ldaddr++] = 0x22; //R22
memory[PROG][ldaddr++] = 0xAB; //#%AB

memory[PROG][ldaddr++] = 0xE6; //LD R IM
memory[PROG][ldaddr++] = 0x23; //R23
memory[PROG][ldaddr++] = 0xC3; //#%AB

memory[PROG][ldaddr++] = 0x2C; // LD r2 #%22
memory[PROG][ldaddr++] = 0x22; //#%22

memory[PROG][ldaddr++] = 0x6C; // LD r6
memory[PROG][ldaddr++] = 0x40; //#%40

memory[PROG][ldaddr++] = 0x6C; // LD r7
memory[PROG][ldaddr++] = 0x40; //#%4A

memory[PROG][ldaddr++] = 0xD3; //LDC @rr6, @R2
memory[PROG][ldaddr++] = 0x26;

memory[PROG][ldaddr++] = 0x6F; //STOP
*/
pc = 0x1000;

run_machine();

getchar();
}

main()
{
test_prog();
}



