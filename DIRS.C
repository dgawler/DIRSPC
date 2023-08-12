/*
   DU - Mark VI  !!

   Program to display the size of the directories in the specified
   directory.

   Dean, 1 June 1991.

*/

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <dir.h>
#include <process.h>
#include <ctype.h>



void main(int argc, char *argv[])
{

   int done=0;
   int home_drive=2;                  /* Set home drive to C by default */
   char cmd[]={"du"};
   char opt[]={"-s"};
   struct ffblk ffblk;


   /* Check args. They MUST specify a starting dir. */
   if (argc < 2)
	 {
	   printf("\nUsage: DIRS <drive>\n");
	   printf("   Eg: DIRS A:\n");
	   exit(1);
	 }

   /* Save current drive & directory information. */
   home_drive=getdisk();

   /* Attempt to change to the drive that they specified. */
   setdisk(((int) (toupper(argv[1][0]))) - 65);
   if (chdir("\\") != 0)
	 {
	   printf("\nCould not change to drive's root directory.\n\n");
	   setdisk(home_drive);
	   exit(1);
	 }

   /* Now do a listing of all directories in the current directory,
	  and get the size of each one. The "du" command must be on the
	  current path for this to work.                                */
   printf("\nDetermining size of directories on drive  %c\n",argv[1][0]);
   printf("============================================\n\n");
   done=findfirst("*.*",&ffblk,FA_DIREC);
   while (! done)
   {
	  if (ffblk.ff_attrib == FA_DIREC)
		 spawnlp(P_WAIT,cmd,cmd,opt,ffblk.ff_name,NULL);
	  done=findnext(&ffblk);
   }

   /* ET go home .....! */
   setdisk(home_drive);
}
