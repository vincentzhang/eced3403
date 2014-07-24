/*
 Z8 Register Memory
 - Emulate register memory
 - Handle all valid registers (0..7F, E0..EF, F0..FF)
 - Register memory can be read/written
 - Accessing any register associated with a device will cause the device 
   emulator to be called with the register number and a RD/WR indication
   
 ECED 3403
 17 June 2014
*/

#include "Z8_IE.h"

/* Define register access */
#define RM_RDWR     (int(*)(BYTE, enum DEV_EM_IO))(-1)
#define RM_RDONLY   (int(*)(BYTE, enum DEV_EM_IO))(-2)
#define RM_USERP    (int(*)(BYTE, enum DEV_EM_IO))(-3)

#define RM_SIZE   256

/* Register memory */
struct reg_mem_el reg_mem[RM_SIZE];

void reg_mem_init()
{
/* Initialize register memory to default values */
int i;

/* Assume everything is RDWR - contents is unknown */
for(i=0; i<RM_SIZE; i++)
     reg_mem[i] . option = RM_RDWR;

/* 80..DF - not supported - make RD only and contents 0xFF */
for(i=0x80; i<0xE0; i++)
{
     reg_mem[i] . contents = 0xFF;
     reg_mem[i] . option = RM_RDONLY;
}

/* E0..EF - special code to indicate RP shift and prefix */
for(i=0xE0; i<0xF0; i++)
     reg_mem[i] . option = RM_USERP;

}

BYTE read_rm(BYTE reg_no)
{
/* Read specified byte and return value (RM_RDWR or RM_RDONLY)
   Extract working register and prefix RP (RM_USERP)
   Read byte and call device emulator function (none of the above)
   NOTE: RP must not equal 0x0E
*/

if (reg_mem[reg_no] . option == RM_USERP)
     reg_no = (reg_mem[RP] . contents << 4) | (reg_no - 0xE0);
     
/* option requires a cast to an int because switch doesn't support pointers */
switch((int) reg_mem[reg_no] . option)
{
case (int) RM_RDWR:
case (int) RM_RDONLY:
     return reg_mem[reg_no] . contents;

default:
     /* Call device emulator: option(reg_no, REG_RD)
        Device can update register contents
        Return device contents (after update)
     */
     reg_mem[reg_no] . option(reg_no, REG_RD);
     return reg_mem[reg_no] . contents;
}
}

void write_rm(BYTE reg_no, BYTE value)
{
/* Write value to register (RM_RDWR)
   Do nothing (RM_RDONLY)
   Extract working register and prefix RP then write value (RM_USERP)
   Write value and call device emulator function (none of the above)
*/
if (reg_mem[reg_no] . option == RM_USERP)
     /* E0..EF correct to RP | regno */
     reg_no = (reg_mem[RP] . contents << 4) | (reg_no - 0xE0);

switch((int)reg_mem[reg_no] . option)
{
case (int)RM_RDWR:
     reg_mem[reg_no] . contents = value;
     break;

case (int)RM_RDONLY:
     break;

default:
     /* Call device emulator: option(reg_no, REG_WR)
        Device can access register contents
     */
     reg_mem[reg_no] . contents = value;
     reg_mem[reg_no] . option(reg_no, REG_WR);
}

}

void reg_mem_device_init(BYTE reg_no, 
                         int (*dev_emulator)(BYTE, enum DEV_EM_IO), 
                         BYTE value)
{
/* Initialize register memory associated with an external device to 
   initial value (contents) and option (dev_emulator)
*/
reg_mem[reg_no] . contents = value;
reg_mem[reg_no] . option = dev_emulator;
}
