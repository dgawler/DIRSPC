/*
   Program which processes a file that contains output from the
   DU command. The format of the file to be processed is:

		 xxx <tab or spaces> dir_name

   where  xxx  is the size of the entry (dir_name) in 512 byte blocks.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct data_fields
  {
	 char f1[80];		/* String representation of size of directory. */
	 char f2[80];		/* Character name of directory.                */
  };



void sort(struct data_fields [],int);




void sort(fields,ndir)
	 struct data_fields fields[];
	 int ndir;
{
  int array_changed=1,next;   /* Determines if changes were needed    */
  char tmp_str[60];           /* Temp storage while swapping elements */
  char tmp_num[20];

  while (array_changed)
	{
	  array_changed=0;		  /* Assume array doesn't need changes.   */

	  /* Cycle through the array one element at a time.               */

	  for (next=1;next<ndir+1;next++)

		  /* If the next element is < than total no. elements, check. */
		  if (next < ndir)

			 /* If next element is bigger than me, swap positions.    */
			 if (strcmpi(fields[next].f2,fields[next+1].f2) > 0)
			   {
				 strcpy(tmp_str,fields[next+1].f2);
				 strcpy(tmp_num,fields[next+1].f1);
				 strcpy(fields[next+1].f2,fields[next].f2);
				 strcpy(fields[next+1].f1,fields[next].f1);
				 strcpy(fields[next].f2,tmp_str);
				 strcpy(fields[next].f1,tmp_num);
				 array_changed=1;
			   }
	}
}



void main(int argc, char *argv[])
{
   FILE *infile;
   int nc=0,t=0,field_finished=0,ndir=0,f=0;
   double tot=0.0;
   char tline[80],tmp[80];
   struct data_fields fields[256];


   /* Make sure that they supplied the name of a file to process. */
   if ((infile=fopen(argv[1],"r")) == NULL){
	  printf("Error - file does not exist.\n");
	  exit (1);
   }


   /* Keep getting lines of data from the file until we hit EOF. */
   while (fgets(tline,80,infile) != NULL){
	   t=0;
	   f=1;				/* We want to start with 1st field from this line. */
	   ndir++;          /* Incr. number of entries read from file          */
	   for (nc=0;nc<strlen(tline);nc++)
		 {

		   /* If we found a <spc> or <tab> & we are still building a field,
			  we must have reached the end of the current field.           */

		   if (tline[nc] == '\t' || tline[nc] == ' ' ||
			   tline[nc] == '\r' || tline[nc] == '\n' && (! field_finished))
			 {
			   tmp[t]='\0';
			   if (f == 1 || f == 2)
				 {

				   /* Make sure that we are only storing fields 1 & 2 */
				   if (f == 1)
					  strcpy(fields[ndir].f1,tmp);
				   else
					  strcpy(fields[ndir].f2,tmp);
				 }
			   t=0;
			   f++;
			   field_finished=1;
			 }
		   else
			 {
			   /* Keep adding chars to field being built. */
			   tmp[t++] = tline[nc];
			   field_finished=0;
			 }
		 }
   }
   fclose(infile);

   /* Sort the list of directory entries alpabetically. */
   sort(fields,ndir);

   /* Now print a summary of what we found. */
   printf("\nSize of directories: \n\n");
   for (nc=1;nc<ndir+1;nc++)
	 {
	   printf("%-20s   %8.2f Mb\n",fields[nc].f2,
			  (double) (atof(fields[nc].f1) * 0.000512));
	   tot += (double) (atof(fields[nc].f1) * 0.000512);
	 }
   printf("==================================\n");
   printf("%-20s  %9.2f Mb\n","Total",tot);

}
