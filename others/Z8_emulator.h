/*
 Z8 emulator 
 Supports program, data, and register memories, some FLAGS, and several 
 instructions.  Most instructions and clock are missing.
 
 ECED 3403
 12 June 2014
*/

#define DIAGNOSTICS

#define FALSE     0
#define TRUE      1

#define PD_MEMSZ    65536
#define RMSZ        256

#define SIGN(x)     (0x80 & (x))
#define CARRY(x)    ((0x100 & (x))>>8)
#define SIGN_EXT(x) (SIGN(x) ? (0xff00 | x) : x)

#define BYTE        unsigned char
#define CARRY_BYTE  unsigned short    /* Use to determine if carry set */
#define WORD        unsigned short

#define MSN(x)      ((x) >> 4)
#define LSN(x)      ((x) & 0x0F)

/* Special register operations */
#define FLAG_C(x)   ((x)<<7 | (FLAGS & 0X7F))   /* Set/clear C bit */
#define FLAG_Z(x)   ((x)<<6 | (FLAGS & 0xBF))   /* Set/clear Z bit */
#define FLAG_S(x)	((x)<<5 | (FLAGS & 0XDF))   /* Set/clear S bit */
#define FLAG_V(x)	((x)<<4 | (FLAGS & 0XEF))   /* Set/clear V bit */
#define FLAG_D(x)	((x)<<3 | (FLAGS & 0XF7)) 	/* Set/clear D bit */
#define FLAG_H(x)	((x)<<2 | (FLAGS & 0XFB))	/* Set/clear H bit */
#define FLAG_F2(x)	((x)<<1 | (FLAGS & 0XFD))	/* Set/clear F2 bit */
#define FLAG_F1(x)	((x)<<0 | (FLAGS & 0XFE))	/* Set/clear F1 bit */
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

int cc_check(BYTE cc);
BYTE cc; /* Condition code */
