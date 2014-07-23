/*
  Sample S19 srecord-extraction program
  - extract bytes from an s-record in a file using sscanf()
  - sscanf() appears to require ints and unsigned ints only
  - can be used to verify assignment 1 records or act as part 
    of the loader in assignment 2
  - turn on diagnostics with define DEBUG
  
  ECED 3403
  28 June 14
*/
#include <stdio.h>
#include <stdlib.h>
#include "Z8_IE.h"

#define DEBUG
#define LINE_LEN   256

enum SREC_ERRORS   {MISSING_S, BAD_TYPE, CHKSUM_ERR};

char *srec_diag[] = {
"Invalid srec - missing 'S'",
"Invalid srec - bad rec type",
"\nInvalid srec - chksum error"};

char srec[LINE_LEN];
FILE *fp;

void srec_error(enum SREC_ERRORS serr)
{
/* Display diagnostic and abort */
printf("%s: %s\n", srec_diag[serr], srec);
getchar();
fclose(fp);
exit(0);
}

load(int argc, char *argv[])
{
/* Read and process s-records */
/* Record access variables */
unsigned pos;
int i;
/* Record fields */
int srtype;                   /* byte 1 - 0..9 */
unsigned int length;          /* byte 2 */
unsigned int ah, al, address; /* bytes 3 and 4 */
signed char chksum;           /* checksum tally */
unsigned int byte;            /* bytes 5 through checksum byte */

if (argc != 2)
{
     printf("Format: parse filename\n");
     exit(0);
}

if ((fp = fopen(argv[1], "r")) == NULL)
{
     printf("No file specified\n");
     exit(0);
}

while (fgets(srec, LINE_LEN, fp) > 0)
{
#ifdef DEBUG
printf("\nsrec: %s\n", srec);
#endif
     /* Should check min srec length */
     if (srec[0] != 'S')
          srec_error(MISSING_S);        
     /* Check srec type */
     srtype = srec[1] - '0';
     if (srtype < 0 || srtype > 9)
          srec_error(BAD_TYPE);
          
     /* Get length (2 bytes) and two address bytes */
     sscanf(&srec[2], "%2x%2x%2x", &length, &ah, &al);
     address = ah << 8 | al;
#ifdef DEBUG
printf("len: %02x ah: %02x al: %02x\n", length, ah, al);
printf("Address: %04x\n", address);
#endif

     chksum = length + ah + al;
     
     length -= 3; /* Ignore length, address and checksum bytes */
     pos = 8;     /* First byte in data */
     /* Read data bytes */
     for (i=0; i<length; i++)
     {
          sscanf(&srec[pos], "%2x", &byte);
    /***************************** what i add **********************************************************************/    
		  switch(srec[1])
          {
          case 1: /*load s1 into program memory*/
          bus(address,&srec[pos],WR,PROG);
          printf("!!!program");
          break;
          
          case 2: /*load s2 into data memory*/
      	  bus(address,&srec[pos],WR,DATA);
      	  printf("!!data\n");
      	  
		  break;
      	  
      	  case 3:/*load s3 into register*/
      	  write_rm(al,srec[pos]);
      	  break;
/*********************************************************************************/			      	
			}		
		  chksum += byte;
#ifdef DEBUG
printf("%02x ", byte);    /* Convert two chars to byte */
#endif
	      pos += 2;    /* Skip 2 chars (1 "byte") in srec */
     }
     /* Read chksum byte */
     sscanf(&srec[pos], "%2x", &byte);
     chksum += byte;
     /* Valid record? */
     if (chksum != -1)
          srec_error(CHKSUM_ERR);
}
#ifdef DEBUG
printf("\n File read - no errors detected\n");
#endif
fclose(fp);
getchar();
}
 
