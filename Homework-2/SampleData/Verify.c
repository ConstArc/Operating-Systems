// Author: A. Delis ad@di.uoa.gr
#include <stdio.h>
#include <unistd.h>

#define SIZEofBUFF 20

typedef struct{
	int  	custid;
	char 	LastName[SIZEofBUFF];
	char 	FirstName[SIZEofBUFF];
	int	balance;
} MyRecord;

int main (int argc, char** argv) {
   FILE *fpb;
   MyRecord rec;
   long lSize;
   int numOfrecords, i;

   if (argc!=2) {
      	printf("Correct syntax is: %s BinaryFile\n", argv[0]);
      	return(1);
   	}
   fpb = fopen (argv[1],"rb");
   if (fpb==NULL) {
      	printf("Cannot open binary file\n");
      	return 1;
   	}

   // check number of records
   fseek (fpb , 0 , SEEK_END);
   lSize = ftell (fpb);
   rewind (fpb);
   numOfrecords = (int) lSize/sizeof(rec);

   printf("Records found in file %d \n", numOfrecords);
   sleep(5);

   for (i=0; i<numOfrecords ; i++) {
      	fread(&rec, sizeof(rec), 1, fpb);
      	printf("%d %s %s  %d \n", \
		rec.custid, rec.LastName, \
                rec.FirstName, rec.balance);
   	}

   fclose (fpb);
}
