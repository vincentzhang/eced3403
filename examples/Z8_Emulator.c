/*
 Z8 emulator 
 Supports program, data, and register memories, some FLAGS, and several 
 instructions.  Most instructions and clock are missing.
 
 ECED 3403
 12 June 2014
*/

#define DIAGNOSTICS

#include <stdio.h>

#define FALSE     0
#define TRUE      1

#define PD_MEMSZ    65536
#define RMSZ        256

#define SIGN(x)     (0x80 & (x))
#define CARRY(x)    ((0x100 & (x))
#define SIGN_EXT(x) (SIGN(x) ? (0xff00 | x) : x)

#define BYTE        unsigned char
#define CARRY_BYTE  unsigned short    /* Use to determine if carry set */
#define WORD        unsigned short

#define MSN(x)      ((x) >> 4)
#define LSN(x)      ((x) & 0x0F)

/* Special register operations */
#define FLAG_Z(x)   ((x)<<6 | (FLAGS & 0xBF))   /* Set/clear Z bit */
#define RPBLK       (RP << 4)                   /* For RP | working register */

/* Bus signals */
enum RDWR           {RD, WR};
enum MEM            {PROG, DATA};

/* Special reg_mem addresses */
#define FLAGS       reg_mem[0xFC]
#define RP          reg_mem[0xFD] 
#define SPH         reg_mem[0xFE]
#define SPL         reg_mem[0xFF]

/* Memory arrays */
BYTE memory[2][PD_MEMSZ];        /* PROG and DATA */
BYTE reg_mem[RMSZ];

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
BYTE src;        /* SRC operand - read if needed */
BYTE dst;        /* DST operand - read if needed */
BYTE high_nib;   /* MS nibble of instruction */
BYTE low_nib;    /* LS nibble of instruction */
BYTE sign;       /* Sign bit before arithmetic - for overflow */
int running;     /* TRUE until STOP instruction */
int sanity;      /* Limit on number of instruction cycles */

running = TRUE;
sanity = 0;

while (running && sanity < 20)
{
     /* Get next instruction and extract nibbles */
     inst = prog_mem_fetch();
     high_nib = MSN(inst);
     low_nib = LSN(inst);
     
     /* Check for special cases: 
        - lnib: 0x08 through 0x0E - r or cc in hnib 
        - lnib: 0x0F - no-operand instructions */
     if (low_nib >= 0x08)
     {
          /* LD through INC and some no-operand instructions */
          switch(low_nib)
          {
          case 0x0A: /* DJNZ r, dst */
               dst = prog_mem_fetch();
               src = RPBLK | high_nib;
               reg_mem[src] -= 1;
               if (reg_mem[src] != 0)
                   /* Reg != 0 -- repeat loop */
                   /* Signed extend dst to 16 bits if -ve */
                   pc = pc + SIGN_EXT(dst);
               break;
               
          case 0x0C: /* LD dst, IMM */
               dst = prog_mem_fetch();
               reg_mem[RPBLK | high_nib] = dst;
               break;
              
          case 0x0E: /* INC dst */
               dst = RPBLK | high_nib;
               sign = SIGN(reg_mem[dst]);
               reg_mem[dst] += 1;  
               /* Update flags */
               FLAGS = FLAG_Z(reg_mem[dst] == 0);
               /* 
               FLAG_S(SIGN(reg_mem[dst]));
               FLAG_V(SIGN(reg_mem[dst]) != sign); 
               */
               break;
               
          case 0x0F: /* STOP .. NOP */     
               switch (high_nib)
               {
               case 0x06: /* STOP - added instruction - not on opcode map */
                    running = FALSE;
               break;
               
               case 0x0F: /* NOP */
                    ;
               break;
               }
               break;
          }
     }
     else
     {
          /* Check high nibble (0x00 .. 0x0F) for opcode */ 
          switch(high_nib)
          {
          /*
          case 0x00:
              if (low_nib >= 0x02 && high_nib <= 0x07)
                 ADD inst
              else
              if (low_nib < 0x02)
                 DEC R and DEC IR
              else
                 others?
          */
          case 0x03:
              switch(low_nib)
              {
              case 0x01: /* SRP IMM */
                  RP = LSN(prog_mem_fetch());
                  break;
              }
              break;
          }
     }
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
memory[PROG][ldaddr++] = 0x31;  // SRP
memory[PROG][ldaddr++] = 0x05;  // 5
memory[PROG][ldaddr++] = 0x2C;  // LD R2
memory[PROG][ldaddr++] = 0x03;  // 3
memory[PROG][ldaddr++] = 0xFF;  // NOP
memory[PROG][ldaddr++] = 0x2A;  // DJNZ
memory[PROG][ldaddr++] = 0xFD;  // -3
memory[PROG][ldaddr++] = 0x6F;  // STOP

pc = 0x1000;

run_machine();

getchar();
}

main()
{
test_prog();
}
