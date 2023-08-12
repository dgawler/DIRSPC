
/*

   Simple program to display the disk space used by all directories
   below your current directory.

   Dean, 1-11-91.

*/



/****************************************************************************
Include files
****************************************************************************/
#include <stdio.h>
#include <dir.h>
#include <dos.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include <ctype.h>



/****************************************************************************
Define some constants for use within the program
****************************************************************************/
#define TRUE		1
#define FALSE		0
#define SCREEN_SIZE	4096
#define MAX_DIRS	512
#define kilobytes	1024
#define TOTAL_SPACE	1
#define AVAIL_SPACE	0
#define TLCG 218      /* Top Left Corner graphics char         */
#define TRCG 191      /* Top Right Corner graphics char        */
#define BLCG 192      /* Bottom Left Corner graphics char      */
#define BRCG 217      /* Bottom Right Corner graphics char     */
#define VLG  179      /* Vertical graphic char                 */
#define HLG  196      /* Horizontal graphic char               */
#define NUM_INTERVALS 50	/* Number of "fuel bar" intervals */




/****************************************************************************
This structure is used to hold info about each directory entry
****************************************************************************/
struct DIRECTORY_ENTRY
   {
      char entry[35];		/* Name of directory entry */
      long int size;		/* Size of entire directory structure */
   };



/****************************************************************************
Function prototypes
****************************************************************************/
int get_dir_spec(int, char * []);
int process_options(int, char * []);
long int disk_stats(int);
long int get_dir_size(char *);		/* Recursive routine which returns */
					/* The total size of the current   */
					/* sub-directory.                  */

void abandon(char *);
void banner(void);
void display_graphics(struct DIRECTORY_ENTRY [], int);
void display_help(void);
void display_results(struct DIRECTORY_ENTRY [], int);
void get_current_settings(void);
void open_data_file(char *);
void reset_indicator(void);
void reset_path(void);
void sort_dirs(struct DIRECTORY_ENTRY [], int);
void update_indicator(long int);
void write_results(struct DIRECTORY_ENTRY [], int);





/****************************************************************************
Global variables for use by all functions
****************************************************************************/
char file_spec[]="*.*";
char fname[50];
char home_dir[50], start_dir[50];
char screen_buffer[SCREEN_SIZE];
int BAD_OPTIONS=FALSE;
int GRAPHICS=FALSE;
int HELP=FALSE;
int PRINT=FALSE;
int SORT_BY_SIZE=FALSE;
int CHECKING_DRIVE=FALSE;
int home_drive, start_drive;
int WRITE_2_FILE=FALSE;
long int bytes_cluster=0L;
long int interval_size=0L;
long int total_files=0L;
long int total_used=0L;
FILE *data_file;






/****************************************************************************
Function to sort the list of directory structures as appropriate
****************************************************************************/
void sort_dirs(dirs,num_dirs)
   struct DIRECTORY_ENTRY dirs[];
   int num_dirs;
{
   char entry[50];			/* Temp storage for dir name        */
   int fnum=0,nnum=0;			/* LCV's		            */
   int sorted=FALSE;			/* Inidicates that list is sorted   */
   int needs_swapping=FALSE;		/* Indicates elements need swapping */
   long int temp=0L;			/* Temp storage for dir size	    */


   /* If we have only 1 entry, don't bother ! */
   if (num_dirs < 2)
      sorted=TRUE;

   /* While the list has not been sorted properly..... */
   while (! sorted)
     {
	/* Perform an insertion sort on the list */
	for (fnum=0;fnum<num_dirs-1;fnum++)
	   for (nnum=fnum+1;nnum<num_dirs;nnum++)
	      {
		sorted=TRUE;               /* Assume list is sorted */
		needs_swapping=FALSE;

		/* If they used the -s flag, sort elements by size */
		if (SORT_BY_SIZE)
		   {
		      if ((long) dirs[nnum].size > (long) dirs[fnum].size)
			 needs_swapping=TRUE;
		   }
		else
		   {
		      /* Otherwise, sort the list alphabetically */
		      if (strcmp(dirs[nnum].entry,dirs[fnum].entry) < 0)
			 needs_swapping=TRUE;
		   }

		/* If any elements need swapping, do so */
		if (needs_swapping)
		   {
		      needs_swapping=FALSE;   /* Reset flags for next pass */
		      sorted=FALSE;

		      /* Swap the long int fields first */
		      temp=dirs[fnum].size;
		      dirs[fnum].size=dirs[nnum].size;
		      dirs[nnum].size=temp;

		      /* Now swap the character string fields */
		      strcpy(entry,dirs[fnum].entry);
		      strcpy(dirs[fnum].entry,dirs[nnum].entry);
		      strcpy(dirs[nnum].entry,entry);
		   }
	      }
     }
}





/****************************************************************************
This function provides a graphical display of the space used by directories
****************************************************************************/
void display_graphics(dirs,num_dirs)
   struct DIRECTORY_ENTRY dirs[];
   int num_dirs;
{
}





/****************************************************************************
Reset the indicator bar on the screen
****************************************************************************/
void reset_indicator()
{

   /* Draw a yellow box on a magenta background */
   textcolor(YELLOW);
   textbackground(MAGENTA);
   window(15,10,65,10);		/* Redefine active window on the screen  */
   clrscr();			/* Use CLS to repaint background magenta */
   window(1,1,80,25);		/* Redefine entire window for use		 */
   textbackground(BLACK);	/* Reset background color to black		 */

   /* Now print heading on the fuel bar */
   textcolor(WHITE);
   gotoxy(15,9);
   cprintf("Percentage complete");
   gotoxy(65,9);
   cprintf("\%");


   /* Initialise vars */
   total_used=disk_stats(1) - disk_stats(0);	/* Total space used up by files */
   interval_size=total_used / (long) NUM_INTERVALS;

}





/****************************************************************************
Update the indicator bar on the screen
*****************************************************************************/
void update_indicator(next_file_size)
   long int next_file_size;
{
    int x=0,y=0;
    float percent_complete=0.0;
    long int file_size=0L;
    long int num_clusters=0;
    static long int used_so_far=0L;
    static int nextx=0;
    static long int next_interval=1L;


    /* Each file must be a multiple of the cluster size, so work out how
       many clusters each file is currently occupying.					*/
    file_size = next_file_size + bytes_cluster;
    num_clusters = file_size / bytes_cluster;
    used_so_far += num_clusters * bytes_cluster;
    percent_complete=(float) (((float)used_so_far / (float)total_used) * 100.0);

    /* Check percentage complete */
    if (percent_complete > 100.0)
       percent_complete=100.0;

    /* If the currently calculated amount of used disk space exceeds the
       next percentage interval, update the "fuel bar" on the screen.    */
    if ((used_so_far > (next_interval * interval_size)) &&
	next_interval <= NUM_INTERVALS)
       {
	  x=wherex();           	/* Remember current cursor pos   */
	  y=wherey();
	  textcolor(WHITE);		/* Display percent complete stat */
	  gotoxy(60,9);
	  cprintf("%5.1f",percent_complete);

	  textcolor(LIGHTCYAN);		/* Reset color of text chars     */
	  gotoxy(15+nextx,10);		/* Go to next pos on the screen  */
	  cprintf("Û");			/* Print the graphic character   */
	  textcolor(LIGHTGREEN);	/* Reset textcolor		 */
	  gotoxy(x,y);			/* Return cursor to original pos */
	  nextx++;			/* Increment "fuel bar" x pos    */
	  next_interval += 1L;		/* Increment number of intervals */
       }
}





/****************************************************************************
General abort routine
****************************************************************************/
void abandon(message)
   char *message;
{
   int slen=0;

   /* Get length of message */
   slen=strlen(message);

   /* Clear last two lines so that we can print our message */
   gotoxy(1,23);
   printf("\n\n");
   gotoxy(1,24);
   clreol();
   gotoxy(1,25);
   clreol();

   /* Display the message is flashing red */
   gotoxy(40-slen/2,25);
   textcolor(RED + BLINK);
   cprintf("%s",message);

   /* Reset their path and exit */
   gotoxy(1,25);
   printf("\n");
   reset_path();
   exit (1);
}






/****************************************************************************
If the user specified -f <file>, try and open the data file
****************************************************************************/
void open_data_file(fname)
   char *fname;
{

   char ch;
   char cur_dir[50], orig_dir[50];
   int x=0,y=0;
   int file_ok=FALSE;
   int valid_ans=FALSE;
   int cur_drive=0, orig_drive=0;


   /* Temporarily change back to original dir to create data file */
   orig_drive=getdisk();
   strcpy(orig_dir,"\\");
   getcurdir(0,&orig_dir[1]);
   setdisk(home_drive);
   chdir(home_dir);


   /* While we have yet to successfully open a data file......... */
   while (! file_ok)
      {
	if ((data_file=fopen(fname,"r")) == NULL)
	  {
	    /* Dir entry isn't there. If no dir exists with same name,
	       create a new file.					  */

	    fclose(data_file);
	    cur_drive=getdisk();        /* Remember current disk settings */
	    strcpy(cur_dir,"\\");
	    getcurdir(0,&cur_dir[1]);

	    /* If it's a directory, display error message and abort */

	    if (chdir(fname) == 0)
	       abandon("This is a directory !  Cannot be written to.");
	    else
	       {
		 /* Otherwise, erase the file & open in WRITE mode */

		 setdisk(cur_drive);
		 chdir(cur_dir);
		 if ((data_file=fopen(fname,"w")) != NULL)
		    file_ok=TRUE;
		 else
		    abandon("Path for data file is invalid.");
	       }
	  }

	/* If we could open the file, see if the user wishes to erase it */
	else
	  {
	    fclose(data_file);
	    printf("\nFile exists. Overwrite ? ");
	    x=wherex();
	    y=wherey();
	    valid_ans=FALSE;

	    /* Prompt them for 'Y' or 'N' */
	    while (! valid_ans)
	       {
		  gotoxy(x,y);
		  clreol();
		  ch=toupper(getch());
		  if (ch == 'Y' || ch == 'N')
		     valid_ans=TRUE;
	       }
	    printf("%c\n",ch);

	    /* If they said "Yes", erase the file. */
	    if (ch == 'Y')
	       {
		  if ((data_file=fopen(fname,"w")) != NULL)
		     file_ok=TRUE;
		  else
		     abandon("Could not open file for writing.");
	       }
	    /* Otherwise, prompt for a new file name and recheck it. */
	    else
	       {
		  printf("\nEnter new file name for results data: ");
		  scanf("%s",fname);
	       }
	  }
      }


   /* Reset drive & path to original settings, as at beginning of function */
   setdisk(orig_drive);
   chdir(orig_dir);


}







/****************************************************************************
Initial screen banner - put my name up in lights !!
****************************************************************************/
void banner()
{

   clrscr();
   gotoxy(25,2);
   textcolor(YELLOW);
   cprintf("DIRSPACE 1.00   D.Gawler 1991");

   gotoxy(17,4);
   textcolor(LIGHTCYAN);
   cprintf("Building directory information, please wait....");
   textcolor(LIGHTGREEN);

}




/****************************************************************************
Main function that displays the results to the user
****************************************************************************/
void display_results(dirs,num_dirs)
   struct DIRECTORY_ENTRY dirs[];
   int num_dirs;
{
   int n=0;
   int next_dir=0;
   long int total=0L;

   /* First, determine total amount of space used by all directories */
   for (n=0;n<num_dirs;n++)
       total += dirs[n].size;

   /* Sort the directory list as appropriate */
   sort_dirs(dirs,num_dirs);

   /* If they choose -g, give them a grphical display */
   if (GRAPHICS)
      display_graphics(dirs,num_dirs);
   else
      {
	 /* Reposition cursor */
	 if (CHECKING_DRIVE)	/* So we don't muck up the bar graph ! */
	    gotoxy(1,13);
	 else
	    gotoxy(1,9);

	 /* Display a banner */
	 printf("\n   DIRSPACE 1.00   (C) 1991\n");
	 printf("\nSize of directories below  %c:%s\n",
		start_drive+'A',strupr(start_dir));
	 for (n=0;n<46;n++)
	     printf("~");
	 printf("\n");

	 /* Now print the results */
	 for (next_dir=0;next_dir<num_dirs;next_dir++)
	    {
	       printf("%-35s  %6ld kb",dirs[next_dir].entry,
		      dirs[next_dir].size);
	       if (strstr(dirs[next_dir].entry,"*") != NULL)
		  printf("   (starting directory)");
	       printf("\n");
	    }

	 /* Print total amount of space used by all dirs */
	 for (n=0;n<46;n++)
	     printf("-");
	 printf("\n%-20s %7ld %6s %7ld kb\n","Total space used by",
		total_files,"files:",total);

	 /* Finally, print total disk stats for reference */
	 printf("\n%-33s  %8ld kb\n","Total disk space on drive",
		disk_stats(TOTAL_SPACE) / kilobytes);
	 printf("%-33s  %8ld kb\n","Total free disk space",
		disk_stats(AVAIL_SPACE) / kilobytes);
      }

}







/****************************************************************************
Write all results to a data file
****************************************************************************/
void write_results(dirs,num_dirs)
   struct DIRECTORY_ENTRY dirs[];
   int num_dirs;
{
   int n=0;
   int next_dir=0;
   long int total=0L;

   /* First, determine total amount of space used by all directories */
   for (n=0;n<num_dirs;n++)
       total += dirs[n].size;

   /* Sort the directory list as appropriate */
   sort_dirs(dirs,num_dirs);

   /* Write a banner to the file */
   fprintf(data_file,"\n\n   DIRSPACE 1.00   (C) 1991\n");
   fprintf(data_file,"\nSize of directories below  %c:%s\n",
	 start_drive+'A',strupr(start_dir));
   for (n=0;n<46;n++)
	fprintf(data_file,"~");
   fprintf(data_file,"\n");

   /* Now print the results to the file */
   for (next_dir=0;next_dir<num_dirs;next_dir++)
      {
	fprintf(data_file,"%-35s  %6ld kb",dirs[next_dir].entry,
	       dirs[next_dir].size);
	if (strstr(dirs[next_dir].entry,"*") != NULL)
	    fprintf(data_file,"   (starting directory)");
	fprintf(data_file,"\n");
      }

   /* Print total amount of space used by all dirs */
   for (n=0;n<46;n++)
       fprintf(data_file,"-");
   fprintf(data_file,"\n%-20s %7ld %6s %7ld kb\n","Total space used by",
	  total_files,"files:",total);

   /* Finally, print total disk stats for reference */
   fprintf(data_file,"\n%-33s  %8ld kb\n","Total disk space on drive",
	  disk_stats(TOTAL_SPACE) / kilobytes);
   fprintf(data_file,"%-33s  %8ld kb\n","Total free disk space",
	 disk_stats(AVAIL_SPACE) / kilobytes);

   /* Close the data file */
   fflush(data_file);
   fclose(data_file);

}






/****************************************************************************
Get disk stats on current drive
****************************************************************************/

long int disk_stats(mode)
   int mode;			/* 0=Available space, 1=Total space */
{
   long int disk_total=0L, disk_avail=0L;
   long int space=0L;
   struct dfree disk;


   /* Call routine and get clusters, bytes/sector, sectors/cluster */
   getdfree(0,&disk);

   /* How many bytes per cluster ? */
   bytes_cluster=(long) disk.df_sclus * (long) disk.df_bsec;

   /* Calculate total amount of disk space on disk */
   disk_total=(long) disk.df_total * (long) bytes_cluster;

   /* Calculate total amount of disk space available */
   disk_avail=(long) disk.df_avail * (long) bytes_cluster;

   /* Which value did they want ? */
   if (mode == TOTAL_SPACE)
      space=(long) disk_total;
   else
      space=(long) disk_avail;

   return(space);

}





/****************************************************************************
Save the current settings, such as current dir/drive, screen info etc....
****************************************************************************/
void get_current_settings()
{

   /* Save screen image */
   gettext(1,1,80,25,screen_buffer);

   /* Set the correct drive specifier in the string */
   home_drive=getdisk();

   /* Set the correct path within the string */
   strcpy(home_dir,"\\");
   getcurdir(0,&home_dir[1]);

}




/****************************************************************************
Reset their path to the original settings
****************************************************************************/
void reset_path()
{
   /* Reset original drive and directory. */
   setdisk(home_drive);
   chdir(home_dir);
}





/****************************************************************************
Display some help text if they've gotten lost somewhere
****************************************************************************/
void display_help()
{
   clrscr();

   textcolor(LIGHTCYAN);

   gotoxy(1,1);
   cprintf("DIRSPACE  1.0      (C) Dean Gawler    1991\n");
   textcolor(LIGHTGREEN);

   gotoxy(1,3);
   cprintf("DIRSPACE  calculates the total amount of disk space being");
   gotoxy(1,4);
   cprintf("used by all sub-directories below your current working");
   gotoxy(1,5);
   cprintf("directory. This is the default if no options are given.");

   gotoxy(1,7);
   cprintf("In addition, you can provide DIRSPACE with either a drive");
   gotoxy(1,8);
   cprintf("and/or starting directory that you wish to produce a directory");
   gotoxy(1,9);
   cprintf("summary for.");

   textcolor(YELLOW);
   gotoxy(1,11);
   cprintf("Usage:  DIRSPACE  [drive:][path] [option]");
   gotoxy(1,12);
   cprintf("            Options:  -G         Graphical screen display (unavailable)");
   gotoxy(1,13);
   cprintf("                      -F <file>  Write results to <file>");
   gotoxy(1,14);
   cprintf("                      -H         This help screen");
   gotoxy(1,15);
   cprintf("                      -P         Send output to printer (unavailable)");
   gotoxy(1,16);
   cprintf("                      -S         Sort according to size");

   gotoxy(1,18);
   cprintf("Eg:     DIRSPACE G: -P -G");
   gotoxy(1,19);
   cprintf("        DIRSPACE G:\REV2 -S");
   gotoxy(1,21);

   reset_path();
   exit (1);

}




/****************************************************************************
Function which extracts the specified path from the command line argument
****************************************************************************/
int get_dir_spec(argc, argv)
    int argc;
    char *argv[];
{
   int BAD_FSPEC=FALSE;

   /* The first arg MUST be the drive and/or dir-spec, so check arg 2 */
   if (argv[1][1] == ':')
      {
	start_drive=toupper(argv[1][0]) - 'A';
	setdisk(start_drive);
	if (getdisk() != start_drive)
	   BAD_FSPEC=TRUE;

	/* If they specified more than just "C:", extract dir name */
	if (strlen(argv[1]) > 2 && argv[1][1] == ':')
	   strcpy(start_dir,&argv[1][2]);	/* Extract dir name */
	else if (strlen(argv[1]) <= 2 && argv[1][1] == ':')
	   {
	     strcpy(start_dir,"\\");
	     CHECKING_DRIVE=TRUE;
	   }
	else
	   strcpy(start_dir,".");	/* Otherwise, use current dir */
      }
  else
      {
	 start_drive=getdisk();		/* Use current drive */
	 strcpy(start_dir,argv[1]);
      }

  /* Check that the specified directory actually exists */
  if (chdir(start_dir) != 0)
     BAD_FSPEC=TRUE;

  /* If we had a bad file spec, say so and quit. */
  if (BAD_FSPEC)
     {
	textcolor(YELLOW);
	cprintf("\n\nBad drive or file spec. Please try again.\n\n");
	exit (1);
     }

  return(TRUE);
}




/****************************************************************************
Function to process and validate command line switches
****************************************************************************/
int process_options(argc, argv)
     int argc;
     char *argv[];
{
     int n=0;
     int start_arg=1;
     int valid_path=FALSE;

   /* Set defaults if no args supplied */
   if (argc < 2)
      {
	   strcpy(&start_dir[0],"\\");		/* Dir to start in  */
	   getcurdir(0,&start_dir[1]);
	   start_drive=getdisk();		/* Disk to start on */
	   return (TRUE);
      }


   /* If the 2nd char of the 1st argument is ":", they must be trying
      to specify a drive and/or path that they wish to start in, so try
      and extract this from the command line.				*/

   if ((argv[1][1] == ':') || (strpbrk(argv[1],":/-") == NULL))
      valid_path=get_dir_spec(argc,argv);
   else
      start_drive=getdisk();

   /* If we extract a valid drive/dir, skip to the 2nd argument */
   if (valid_path)
      start_arg=2;

   /* Now process ALL remaining arguments on the command line */
   for (n=start_arg;n<argc;n++)
       {
	  /* Check 1st char of next argument. Only "-" or "/" are valid */
	  switch (argv[n][0])
	     {
		case '-' :
		case '/' :

		    /* Now check 2nd char of next argument. Valid chars are
		       options that are included below.                    */
		    switch (toupper(argv[n][1]))
		       {
			  case 'H' :
			  case '?' :
			      display_help();	/* They want help    */
			      break;
			  case 'F' :
			      WRITE_2_FILE=TRUE;
			      strcpy(fname,argv[n+1]);
			      n++;
			      open_data_file(fname);
			      break;
			  case 'G' :
			      GRAPHICS=TRUE;	/* Graphics display  */
			      break;
			  case 'P' :
			      PRINT=TRUE;	/* Send results to printer */
			      break;
			  case 'S' :
			      SORT_BY_SIZE=TRUE;   /* Sort results by size */
			      break;
			  default  :
			      BAD_OPTIONS=TRUE;   /* Otherwise, bad argument */
			      break;
		       }
		    break;
		default :
		    BAD_OPTIONS=TRUE;
		    break;
	     }
       }

     /* If their options were bad, display the help screen and quit */
     if (BAD_OPTIONS)
	display_help();

     return (TRUE);
}



/****************************************************************************
Get the total size of the current directory
****************************************************************************/
long int get_dir_size(path)
	 char *path;
{
   char cur_dir[50];
   int done=0;
   long int dir_space=0L;	/* Space used by files in this sub-dir */
   struct ffblk ffblk;		/* File structure 		       */


   /* Change to the sub-dir name that was passed to this routine */
   chdir(path);

   /* Tell user which directory we are processing */
   gotoxy(17,6);
   getcurdir(0,cur_dir);
   clreol();
   cprintf("Processing  %c:\\%s",start_drive+'A',cur_dir);

   /* Get a list of all directory entries within this sub-dir */
   done=findfirst("*.*",&ffblk,FA_DIREC);

   /* While there are still directory entries to process ......*/
   while (! done)
      {
	 /* Is the next entry another sub-directory ? */

	 if ((ffblk.ff_attrib == FA_DIREC) &&
	     (strstr(".",ffblk.ff_name) == NULL) &&	/* Remember to skip current & parent dir entries. */
	     (strstr("..",ffblk.ff_name) == NULL))
	     dir_space += get_dir_size(ffblk.ff_name);	/* Yes, so get size of this sub-directory. */
	 else
	     {
		if (strstr(".",ffblk.ff_name) == NULL &&
		    strstr("..",ffblk.ff_name) == NULL)
		    {
		       dir_space += ffblk.ff_fsize;		/* No, so add file size to total directory size. */
		       total_files += 1L;
		    }
	     }

	 /* If we're checking a full drive, update indicator bar */
	 if (CHECKING_DRIVE)
	    update_indicator(ffblk.ff_fsize);

	 done=findnext(&ffblk);		/* Process next directory entry. */
      }

   chdir("..");			/* Change back to calling sub-directory. */
   return (long) dir_space;     /* Return size of sub-dir to calling function. */
}




/****************************************************************************
The main function !!
****************************************************************************/
void main(int argc, char **argv)
{
   int done=FALSE;		/* Logical flag which stops processing    */
   int num_dirs=0;		/* Total number of dirs found 	          */
   long int top_dir_size=0L;	/* Space used by files in starting dir    */
   struct ffblk ffblk;
   struct DIRECTORY_ENTRY dirs[MAX_DIRS];	/* Stores dir name & size */

   /* Save current settings. */
   get_current_settings();

   /* Check command line arguments, if any were supplied. */
   process_options(argc,argv);

   /* Get a list of directory entries. */
   done=findfirst(file_spec,&ffblk,FA_DIREC);

   /* Display message on the screen for user */
   banner();

   /* Start "Percentage complete" indicator */
   if (CHECKING_DRIVE)
      {
	 disk_stats(1);		/* Force system to calc cluster size */
	 reset_indicator();
      }

   /* While not finished processing directory entries...... */
   while (! done)
      {
	 /* If next entry is a sub-directory.... */

	 if ((ffblk.ff_attrib == FA_DIREC) &&
	     (strstr(".",ffblk.ff_name) == NULL) &&	/* Skip "." and ".." entries */
	     (strstr("..",ffblk.ff_name) == NULL))
	     {                                     	/* ... get its total size. */
		dirs[num_dirs].size=get_dir_size(ffblk.ff_name) / (long) kilobytes;
		strcpy(dirs[num_dirs].entry,ffblk.ff_name);
		num_dirs++;
	     }
	 else
	     {
		if (strstr(".",ffblk.ff_name) == NULL &&
		    strstr("..",ffblk.ff_name) == NULL)
		    {
		      top_dir_size += ffblk.ff_fsize;
		      total_files += 1L;
		    }
	     }

	 /* Update fuel bar if a whole drive is being checked */
	 if (CHECKING_DRIVE)
	    update_indicator(ffblk.ff_fsize);

	 done=findnext(&ffblk);		/* Now process the next entry. */
      }


   /* Add root dir to our table of subdirs */
   dirs[num_dirs].size=(long) top_dir_size / (long) kilobytes;
   strcpy(dirs[num_dirs].entry,start_dir);
   strcat(dirs[num_dirs].entry," *");	     /* Flag starting dir with "*" */
   num_dirs++;

   /* Store results in file if flag was set */
   if (WRITE_2_FILE)
      write_results(dirs,num_dirs);

   /* Display our findings */
   display_results(dirs,num_dirs);

   /* Restore original settings */
   reset_path();

}

