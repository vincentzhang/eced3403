/*
 Z8 Interrupt Emulation - Mainline
 
 ECED 3403
 17 June 2014
*/

#include <stdio.h>
#include <stdlib.h>

#include "Z8_IE.h"

int main(int argc, char *argv[])
{ 
  
  char instring[LINE_LEN];
  FILE *fp;

/*read/load s19 code line by line*/
while (fgets(instring, LINE_LEN, fp) != NULL)
{
 
	instring[LINE_LEN-1] = NUL;
	load(instring);/* loader*/
  /* load() cannot be called this way.
  The way it works is that it opens a file *fp and read line by line and process it based on the first two letters (S1,S2,S3...).
  But what this load(instring) does is passing the line as an input argument, doesn't match the argument lister of load function. 
  We'll fix this later.
  */
	
}




/* Initialize emulator */
reg_mem_init();
reg_mem_device_init(PORT3, UART_device, TXDONE);
reg_mem_device_init(PORT0, TIMER_device, 0x00);



write_rm(PORT0,0x8f);
/*initialize stack for write on data memory*/
write_rm(P01M, 0X00);


run_machine();

getchar();	
return 0;
}
