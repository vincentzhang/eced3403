/*
 Z8 Interrupt Emulation - Mainline
 
 ECED 3403
 July 23, 2014
*/
#include "Z8_IE.h"

int main(int argc, char *argv[])
{ 
  	if (argc != 2)
	{
       printf("Format: parse filename\n");
       exit(0);
	   }

    /* load s19 code, should only take a file name as the input */
   	load(argv[1]); // load the s19 file specified by argv[1]

   	/* Initialize emulator */
   	reg_mem_init();
   	reg_mem_device_init(PORT3, UART_device, TXDONE);
   	reg_mem_device_init(PORT0, TIMER_device, 0x00);
   	reg_mem_device_init(SIO, UART_device, TXDONE);
   	
   	/* Initialize UART device */
	UART_init();

   	run_machine(); // execute the code in the memory

   	getchar();	
   	return 0;
}
