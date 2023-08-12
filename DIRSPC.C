
/*

   Simple program to display the disk space used by all directories on
   a particular drive.

   Dean, 1-11-91.

*/

#include <stdio.h>
#include <dir.h>
#include <dos.h>
#include <string.h>

void get_size(char *);

static long int dir_space=0L;

void get_size(path)
	 char *path;
{
   int done=0;
   struct ffblk ffblk;

   /* Find all further sub-dirs below us */
   chdir(path);

   /* Get space used by all files in current dir */
   done=findfirst("*.*",&ffblk,255);
   while (! done)
   {
      if (ffblk.ff_attrib != FA_DIREC)
	 printf("Attribute for %s is not FA_DIREC !!\n",ffblk.ff_name);

      if ((ffblk.ff_attrib == FA_DIREC) && (ffblk.ff_name[0] != '.'))
	  get_size(ffblk.ff_name);
      else
	  dir_space += ffblk.ff_fsize;

      done=findnext(&ffblk);
   }
   chdir("..");
}

void main()
{
   int done=0;
   struct ffblk ffblk;

   done=findfirst("dirspc.c",&ffblk,255);
   printf("FA_HIDDEN : %d\n",FA_HIDDEN);
   printf("FA_SYSTEM : %d\n",FA_SYSTEM);
   printf("FA_RDONLY : %d\n",FA_RDONLY);
   printf("FA_DIREC  : %d\n",FA_DIREC);
   printf("Attributes: %d\n",ffblk.ff_attrib);
   printf("FA_DIREC & FA_HIDDEN: %d\n",(FA_DIREC | FA_HIDDEN));

   while (! done)
     {
	if (((ffblk.ff_attrib == FA_DIREC) && (ffblk.ff_name[0] != '.')) ||
	   ((ffblk.ff_attrib == (FA_DIREC | FA_HIDDEN)) && (ffblk.ff_name[0] != '.')) ||
	   ((ffblk.ff_attrib == (FA_DIREC | FA_HIDDEN | FA_SYSTEM)) && (ffblk.ff_name[0] != '.')))
	{
	   get_size(ffblk.ff_name);
	   printf("%-14s %8ld kb\n",ffblk.ff_name,dir_space / 1024);
	   dir_space=0L;
	}
	done=findnext(&ffblk);
     }
}
