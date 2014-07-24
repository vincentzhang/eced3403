/*
  Sample S19 srecord-extraction program
  - extract bytes from an s-record in a file using sscanf()
  - sscanf() appears to require ints and unsigned ints only
  - can be used to verify assignment 1 records or act as part 
    of the loader in assignment 2
  - turn on diagnostics with define DEBUG
  
  ECED 3403
  23 July 14
*/
#include "Z8_IE.h"

#define DEBUG

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

load(char* s19file)
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
BYTE data;            		  /* bytes 5 through checksum byte */

if ((fp = fopen(s19file, "r")) == NULL)
{
     printf("No file specified\n");
     exit(0);
}

while (fgets(srec, LINE_LEN, fp) > 0)
{
#ifdef DEBUG
printf("\nsrec: %s\n", srec);
#endif
	  // srec stores the current line
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
          sscanf(&srec[pos], "%2x", &data); // save 1 byte to data
    /***************************** what i add **********************************************************************/    
		  switch(srtype)  // srtype is the number of the s19 Record Type
          {
           case 1: /*load s1 into program memory*/
           bus(address+pos-8,&data,WR,PROG);
           #ifdef DEBUG
           printf("!!Program Memory\n");
           #endif
           break;
          
           case 2: /*load s2 into data memory*/
      	   bus(address+pos-8,&data,WR,DATA);
      	   #ifdef DEBUG
      	   printf("!!Data Memory\n");
      	   #endif
      	   break;
      	  
      	   case 3:/*load s3 into register memory*/
      	   write_rm(al,data); // Register Memory can be represented by lower byte of the address
      	   #ifdef DEBUG
      	   printf("!!Register Memory\n");
      	   #endif
      	   break;
   /*********************************************************************************/			      	
			}		
		  chksum += data;
#ifdef DEBUG
printf("%02x ", data);    /* Convert two chars to byte */
#endif
	      pos += 2;    /* Skip 2 chars (1 "byte") in srec */
     }
     /* Read chksum byte */
     sscanf(&srec[pos], "%2x", &data);
     chksum += data;
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
 
