/*
 Z8 Interrupt Emulation - Mainline
 
 ECED 3403
 17 June 2014
*/

#include <stdio.h>
#include <stdlib.h>

#include "Z8_IE.h"
void analyze(char *string)
{
  int i,j,k;
  int count = 0;
  int address;
  BYTE c[1];
  BYTE record[LINE_LEN];
  i = strlen(string)-1;
  //printf("length: %d\n",i);
  c[0] = string[0];
  c[1] = string[1];
  
  //printf("here: %s\n",c);
  for(j=2;j<(strlen(string)-1);j+=2)
  {
	c[0] = string[j];
  	c[1] = string[j+1];
  	//printf("here: %s\n",c);
  	record[count] = strtol(c, NULL, 16);
  	//printf("hex number: %02x\n",record[count]);//char to hex
  	count++;
  }
  //printf("count is %d\n", count);
  address = ((record[1]<<8)|(record[2]));
  //printf("addr is : %04x\n",address);
  for(k=3;k<(count-1);k++)
  {  
	  memory[PROG][address] = record[k];
	  //printf("record needs to be stored is %02x\n",record[k]);
	  //printf("new address is %04x\n",address);
	  //printf("hahaha is %02x\n",memory[PROG][address]);
	  address++;
  }
}

int main(int argc, char *argv[])
{
/*
 Mainline
 - Read records from file until EOF
 - parse contents of each record
*/

char instring[LINE_LEN];
FILE *fp;
if (argc != 2)
{
	printf("Format: parse filename\n");
	getchar(); /* Pause before closing window */
	exit(0);
}

if ((fp = fopen(argv[1], "r")) == NULL)
{
	printf("File %s could not be opened\n", argv[1]);
	getchar(); /* Pause before closing window */
	exit(0);
}
while (fgets(instring, LINE_LEN, fp) != NULL)
{
    /* Truncate lines too long */
	instring[LINE_LEN-1] = NUL;
	printf("%s\n", instring);
	analyze(instring);	
}
/* Initialize emulator */
reg_mem_init();
reg_mem_device_init(PORT3, UART_device, TXDONE);
reg_mem_device_init(PORT0, TIMER_device, 0x00);
//test_prog();
pc = 0x1000;
run_machine();	
getchar();	
return 0;
}
