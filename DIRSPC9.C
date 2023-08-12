
/*

   Simple program to display the disk space used by all directories
   below your current directory.

   Dean Gawler, 1-11-91.

   Added menus & pretty things on 20-5-92.

   Update on 25-5-92.

   Version 6, 2-6-92.

   Smoothed up on 8-6-92.

   Tightened up scrolling window functions on 26-6-92.

   Includes hidden files now, but not directories. 28-2-93.

   Added hidden directory support, modified the scroll window, removed
   all command line arguments (except drive and path specs), and
   generally tidied the program up. 12-4-93.

   Numbers now have commas inserted at the 000's point (eg. 1000 becomes
   1,000, etc....). 8-1-95.

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
#define TRUE            1
#define FALSE           0
#define IGNORE 0
#define RETRY 1
#define ABORT 2
#define SCREEN_SIZE     4096 /* Size of buffer for screen image.          */
#define MAX_DIRS        512  /* Max no. dirs we will search from cur dir. */
#define MAXPLEN         60   /* Max len of path/fname entered by user.    */
#define kilobytes       1024
#define SizeOfWindow    12       /* Height of display window for dir list */
#define WindowWidth     29       /* Width of display window for dir list  */
#define TOTAL_SPACE     1
#define AVAIL_SPACE     0
#define TLCG 218      /* Top Left Corner graphics char         */
#define TRCG 191      /* Top Right Corner graphics char        */
#define BLCG 192      /* Bottom Left Corner graphics char      */
#define BRCG 217      /* Bottom Right Corner graphics char     */
#define VLG  179      /* Vertical graphic char                 */
#define HLG  196      /* Horizontal graphic char               */
#define F1 59         /* ASCII char for function keys          */
#define F2 60
#define F3 61
#define F4 62
#define F5 63
#define F6 64
#define F7 65
#define F8 66
#define F9 67
#define F10 68
#define UP 72         /* ASCII char for up arrow               */
#define DOWN 80       /*       char for down arrow             */
#define LEFT 75       /*                left arrow             */
#define RIGHT 77      /*              right arrow              */
#define PGUP 73
#define PGDN 81
#define CR 13         /*       char for ENTER key              */
#define ESC 27        /*       char for ESC key                */
#define FF 12
#define ERR 2         /* Error return code for printer handler */




/****************************************************************************
This structure is used to hold info about each directory entry
****************************************************************************/
struct DIRECTORY_ENTRY
   {
      char entry[13];           /* Name of directory entry (not path!) */
      long int size;            /* Size of entire directory structure  */
   };

struct DIRECTORY_SIZE           /* Structure which is used to contain  */
   {                            /* the string conversion of the size   */
      char string[10];          /* of the subdirectory.                */
   };

struct ENTRY                    /* Contains the name of the directory  */
   {                            /* and its size in string format. Used */
      char string[32];          /* for scroll list.                    */
   };

struct HELP_INFO                /* Used to create an array of help     */
   {                            /* messages that are displayed when    */
      char line[52];            /* F1 is pressed.                      */
   };



/****************************************************************************
Function prototypes
****************************************************************************/
char *get_field(int);
int calc_dir_sizes(struct DIRECTORY_ENTRY []);
int directory(char);
int get_dir_spec(char *);
int get_file_name(char *);
int get_new_path(void);
int open_data_file(char *);
int reset_vars(struct DIRECTORY_ENTRY []);
long int disk_stats(int);
long int get_dir_size(char *);          /* Recursive routine which returns */
					/* The total size of the current   */
					/* sub-directory.                  */
void abandon(char *);
void banner(void);
void display_program_help(void);
void display_usage(void);
void display_results(struct DIRECTORY_ENTRY [], int);
void draw_box(int, int, int, int);
void draw_input_window(int,int,int,int);
void get_current_settings(void);
void help(void);
void insert_commas(char *);
void make_table(int, struct DIRECTORY_ENTRY [], struct ENTRY []);
void paint_screen(void);
void print_results(struct DIRECTORY_ENTRY [], int);
void process_options(int, char * []);
void reset_path(void);
void reset_screen_info(int []);
void reverse(char *);
void save_screen_info(int []);
void scroll_list(struct ENTRY [], int, struct DIRECTORY_ENTRY []);
void setup_help(void);
void sort_dirs(struct DIRECTORY_ENTRY [], int);
void update_display(long int, long int);
void where_are_we(char *, int);
void write_results(struct DIRECTORY_ENTRY [], int);




/****************************************************************************
Global variables for use by all functions
****************************************************************************/
char fname[30];
char home_dir[MAXDIR], start_dir[MAXDIR];
char screen_buffer[SCREEN_SIZE];
char top[]="ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป";
char sides[]="บ                                                                  บ";
char bottom[]="ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ";
char revision[]="DIRSPACE 5.0    (C) Dean Gawler 1995";
char file_spec[]="*.*";
int BAD_OPTIONS=FALSE;
int HELP=FALSE;
int PRINT=FALSE;
int SORT_BY_SIZE=TRUE;           /* Initially sort entries by size        */
int PRINTER_ERROR=FALSE;
int home_drive, start_drive;
int WRITE_2_FILE=FALSE;
int total_help_lines=0;
long int bytes_cluster=0L;
long int interval_size=0L;
long int total_files=0L;
long int total=0L;
long int total_used=0L;
long int total_space_used=0;
FILE *data_file;
struct HELP_INFO help_data[100];





/* ------------------------------------------------------------------------ */
/*                                                                          */
/*                       S C R E E N   U T I L I T I E S                    */
/*                                                                          */
/* ------------------------------------------------------------------------ */


/****************************************************************************
Initial screen banner - put my name up in lights !!
****************************************************************************/
void banner()
{
   int n=0;

   window(1,1,80,25);
   textbackground(BLACK);
   clrscr();

   gotoxy(7,1);
   textcolor(YELLOW);
   textbackground(LIGHTBLUE);
   cprintf("%s",top);              /* Display top of box          */

   for (n=0;n<6;n++)               /* Display sides of box        */
   {
       gotoxy(7,2+n);
       cprintf("%s",sides);
   }
   gotoxy(7,8);                    /* Bottom of box               */
   cprintf("%s",bottom);

   gotoxy(25,2);                   /* Display program revision    */
   cprintf("%s",revision);

   textcolor(WHITE);               /* Headings for final results  */
   gotoxy(32,4);
   cprintf("Files:");
   gotoxy(32,5);
   cprintf("Bytes:");

   window(8,2,73,10);
}




/*****************************************************************/
/* Display the results on the screen.                            */
/*****************************************************************/
void display_results(dirs,num_dirs)
   struct DIRECTORY_ENTRY dirs[];
   int num_dirs;
{
   struct ENTRY entries[MAX_DIRS];

   /* Sort the directory list as appropriate */
   sort_dirs(dirs,num_dirs);

   /* Make a table of entries */
   make_table(num_dirs,dirs,entries);

   /* Display the list of directories and sizes */
   paint_screen();
   scroll_list(entries,num_dirs,dirs);

}




/********************************/
/* Draw a window on the screen. */
/********************************/
void draw_box(x,y,x1,y1)
     int x,y,x1,y1;
{
     int nc;

     /* Top line */
     gotoxy(x,y);
     cprintf("%c",TLCG);
     for (nc=x+1;nc<x1;nc++)
     {
	 gotoxy(nc,y);
	 cprintf("%c",HLG);
     }

     /* Right side */
     gotoxy(y1,x1);
     cprintf("%c",TRCG);
     for (nc=y+1;nc<y1;nc++)
     {
	 gotoxy(x1,nc);
	 cprintf("%c",VLG);
     }

     /* Bottom line */
     gotoxy(x1,y1);
     cprintf("%c",BRCG);
     for (nc=x1-1;nc>x;nc--)
     {
	 gotoxy(nc,y1);
	 cprintf("%c",HLG);
     }

     /* Left hand side */
     gotoxy(x,y1);
     cprintf("%c",BLCG);
     for (nc=y1-1;nc>y;nc--)
     {
	 gotoxy(x,nc);
	 cprintf("%c",VLG);
     }
}




/*********************************************/
/* Draw a small 1 line window for user input */
/*********************************************/
void draw_input_window(x1,y1,x2,y2)
   int x1,y1,x2,y2;
{
   /* Redefine active window & clear */
   window(x1,y1,x2,y2);
   textbackground(BLACK);
   textcolor(LIGHTMAGENTA);
   clrscr();

   /* Draw small window & redefine active window */
   window(x1,y1,x2+1,y2+1);
   draw_box(1,1,x2-x1,y2-y1+1);
   window(x1+1,y1+1,x2-2,y2-1);
}




/*********************************************/
/* Reset current screen info                 */
/*********************************************/
void reset_screen_info(window_params)
   int window_params[];
{

   /* Reset screen co-ords & attribs */
   window(window_params[0],window_params[1],window_params[2],
	  window_params[3]);
   textattr(window_params[4]);

}




/*********************************************/
/* Save all current screen parameters        */
/*********************************************/
void save_screen_info(window_params)
   int window_params[];
{
   struct text_info info;

   /* Ask system for window info. */
   gettextinfo(&info);

   /* Store current active window co-ordinates in global vars. */
   window_params[0]=info.winleft;
   window_params[1]=info.wintop;
   window_params[2]=info.winright;
   window_params[3]=info.winbottom;
   window_params[4]=info.attribute;

}




/*************************************************************/
/* Draw the output windows on the screen, and all of the     */
/* function key assignements.                                */
/*************************************************************/
void paint_screen()
{
   char tdir[50];      /* Temporary space for starting dir name. */
   char totfiles[14];  /* Total bytes used converted to a string. */
   char totspace[18];
   char totused[18];
   char totfree[18];

   /* Wipe strings. */
   memset(totfiles,'\0',14);
   memset(totspace,'\0',18);
   memset(totused,'\0',18);
   memset(totfree,'\0',18);

   /* Reposition cursor & clear new window */
   window(7,7,74,24);
   textcolor(YELLOW);
   textbackground(BLUE);
   clrscr();
   window(1,1,80,25);

   /* Draw a nice little window for the results ! */
   draw_box(7,7,38,24);
   draw_box(38,7,74,24);
   draw_box(7,7,38,9);
   draw_box(7,22,38,24);
   gotoxy(38,7);
   cprintf("ย");
   gotoxy(38,24);
   cprintf("ม");
   gotoxy(7,9);
   cprintf("ร");
   gotoxy(38,9);
   cprintf("ด");
   gotoxy(7,22);
   cprintf("ร");
   gotoxy(38,22);
   cprintf("ด");
   gotoxy(38,24);
   cprintf("ม");

   /* Redraw the double line bottom of the top box. */
   gotoxy(7,6);
   cprintf("%s",bottom);

   /* Add headings to columns */
   textcolor(LIGHTGREEN);
   gotoxy(9,8);
   cprintf("Directory name     Size (kb)");

   /* Add function key help */
   textcolor(LIGHTGREEN);
   gotoxy(9,23);    cprintf("TOTAL (kb)");
   gotoxy(40,8);   cprintf("Starting directory:");
   gotoxy(40,11);   cprintf("Total       :");
   gotoxy(40,12);   cprintf("Used        :");
   gotoxy(40,13);   cprintf("Free        :");
   gotoxy(40,15);   cprintf("F1");
   gotoxy(40,16);   cprintf("F2");
   gotoxy(40,17);   cprintf("F3");
   gotoxy(40,18);   cprintf("F4");
   gotoxy(40,19);   cprintf("F5");
   gotoxy(40,20);   cprintf("F6");
   gotoxy(40,21);   cprintf("Esc");

   /* Truncate starting dir name (if required) so it fits on screen. */
   memset(tdir,'\0',50);
   strncpy(tdir,start_dir,33);
   tdir[34]='\0';

   /* Convert disk stats to strings & insert commas where needed. */
   ltoa((disk_stats(TOTAL_SPACE) / kilobytes),totspace,10);
   ltoa((disk_stats(TOTAL_SPACE) - disk_stats(AVAIL_SPACE)) / kilobytes,
	totused,10);
   ltoa((disk_stats(AVAIL_SPACE) / kilobytes),totfree,10);
   ltoa(total,totfiles,10);

   insert_commas(totfiles);
   insert_commas(totspace);
   insert_commas(totused);
   insert_commas(totfree);

   /* Now display disk stats on the screen. */
   textcolor(YELLOW);
   gotoxy(40,9);    cprintf("%s",tdir);
   gotoxy(57,11);   cprintf("%13s kb",totspace);
   gotoxy(57,12);   cprintf("%13s kb",totused);
   gotoxy(57,13);   cprintf("%13s kb",totfree);
   gotoxy(24,23);   cprintf("%12s",totfiles);


   /* Display meaning of function keys */
   textcolor(LIGHTCYAN);
   gotoxy(44,15);   cprintf("Help");
   gotoxy(44,16);   cprintf("Change path");
   gotoxy(44,17);   cprintf("Change sort method");
   gotoxy(44,18);   cprintf("Re-calc directory sizes");
   gotoxy(44,19);   cprintf("Write data to file");
   gotoxy(44,20);   cprintf("Print results to LPT1:");
   gotoxy(44,21);   cprintf("Quit");

   /* Reset active window */
   window(9,10,38,25);

}




/****************************************************************************
Update the values on the screen
*****************************************************************************/
void update_display(total_files,next_file_size)
   long int total_files;
   long int next_file_size;
{
   char numfstr[MAXPLEN];
   char numbstr[MAXPLEN];

   /* Display the number of files that we have processed so far. */
   textcolor(CYAN);
   gotoxy(32,3);
   ltoa(total_files,numfstr,10);
   insert_commas(numfstr);
   cprintf("%s",numfstr);

   /* Display the total number of bytes being used by these files. */
   total_space_used += next_file_size;
   gotoxy(32,4);

   /* Add commas at appropriate point within string. */
   ltoa(total_space_used,numbstr,10);
   insert_commas(numbstr);
   cprintf("%s",numbstr);

}






/* ------------------------------------------------------------------------ */
/*                                                                          */
/*                E R R O R / H E L P     R O U T I N E S                   */
/*                                                                          */
/* ------------------------------------------------------------------------ */


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
Display some help text if they've gotten lost somewhere
****************************************************************************/
void display_usage()
{
   clrscr();

   textcolor(LIGHTCYAN);

   gotoxy(1,1);
   cprintf("%s\n",revision);
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
   cprintf("Usage:  DIRSPACE  [drive:][path]");
   gotoxy(1,12);
   cprintf("Eg:     DIRSPACE G:");
   gotoxy(1,13);
   cprintf("        DIRSPACE C:\DATA");
   gotoxy(1,15);

   reset_path();
   exit (1);

}




/*******************************/
/* Printer error handler       */
/*******************************/
#pragma warn -par
int error_handler(int errval, int ax, int bp, int si)
{
   /* If it wasn't a disk error, it was most likely the printer port */
   if (ax < 0)
      hardretn(NULL);

   return(NULL);
}
#pragma warn +par




/***************************************************/
/* Display program help in a window on the screen. */
/***************************************************/
void help()
{
   char ch=NULL;
   int window_params[5]={1,1,80,25,0};
   int sizeofwindow=15,index=0,n=0;
   int extended_key=FALSE;
   int update=TRUE;


   /* Remember current screen & settings. */
   gettext(10,5,70,22,screen_buffer);
   save_screen_info(window_params);


   /* Draw help window. */
   window(10,5,70,22);
   textcolor(BLUE);
   textbackground(CYAN);
   clrscr();
   window(10,5,71,22);
   draw_box(1,1,60,18);


   /* Draw titles around window border. */
   textcolor(WHITE);
   gotoxy(3,1);     cprintf(" HELP ");
   gotoxy(20,18);   cprintf("Quit ");
   gotoxy(40,18);   cprintf("More info ");
   textcolor(YELLOW);
   gotoxy(14,18);   cprintf(" Esc: ");
   gotoxy(28,18);   cprintf(" PGUP/PGDN: ");
   textcolor(BLUE);
   window(11,6,68,21);


   /* Now display help screens until they hit <ESC> */
   while (ch != ESC)
   {
     if (kbhit())                   /* If the keyboard has been hit... */
     {
	ch=getch();                 /* ... get a char from the buffer. */
	switch (ch)                 /* Now process the cbaracter.      */
	{

	   /* Was it an extended key ? */
	   case 0:
		extended_key=TRUE;
		break;


	   /* Display previous help page. */

	   case PGUP:
		if (! extended_key)
		   break;

		extended_key=FALSE;

		if (index > 0)
		{
		   index -= sizeofwindow;
		   if (index < 0)
		      index = 0;

		   update=TRUE;
		   extended_key=FALSE;
		}

		break;


	   /* Display next help page. */

	   case PGDN:
		if (! extended_key)
		   break;

		extended_key=FALSE;

		if (index+sizeofwindow < total_help_lines)
		{

		   index += sizeofwindow;
		   if (index > (total_help_lines-sizeofwindow))
		      index = (total_help_lines-sizeofwindow);

		   update=TRUE;
		   extended_key=FALSE;
		}

		break;


	   default:
		break;

	}
     }

     /* See if we need to update the screen. */
     if (update)
     {
	clrscr();
	for (n=0;n<sizeofwindow+1;n++)
	{
	   gotoxy(2,n+1);
	   cprintf("%s",help_data[0+index+n].line);
	}
	update=FALSE;
     }
   }


   /* Reset window parameters. */
   reset_screen_info(window_params);
   window(10,5,70,22);
/*   clrscr(); */
   puttext(10,5,70,22,screen_buffer);
   reset_screen_info(window_params);

}







/* ------------------------------------------------------------------------ */
/*                                                                          */
/*                         I / O     R O U T I N E S                        */
/*                                                                          */
/* ------------------------------------------------------------------------ */


/****************************************************************************
Check to see if next entry is a file or a directory
****************************************************************************/
int directory(attr)
    char attr;
{
    /* Simply check all combinations of hidden flags, read only flags, */
    /* system flags, etc.... to see what the entry is.                 */
    if ((attr == FA_DIREC) ||
	(attr == (FA_DIREC | FA_HIDDEN)) ||
	(attr == (FA_DIREC | FA_SYSTEM)) ||
	(attr == (FA_DIREC | FA_RDONLY)) ||
	(attr == (FA_DIREC | FA_HIDDEN | FA_SYSTEM)))
	return(TRUE);
    else
	return(FALSE);
}



/****************************************************************************
Get disk stats on current drive
****************************************************************************/
long int disk_stats(mode)
   int mode;                    /* 0=Available space, 1=Total space */
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




/************************************************/
/* Get a string from the user                   */
/************************************************/
char *get_field(field_len)
	int field_len;
{
   int x,y,num=0;
   char tmp[80],*t,ch;


   t=tmp;
   x=wherex();
   y=wherey();


   /* Collect characters until <CR> or <ESC> is hit, or the max.  */
   /* number of characters have been entered.                     */

   while ((ch=getch()) != '\r' && (num < field_len || ch == '\b'))
   {
	if (ch == ESC)              /* Return NULL string if ESC */
	   return NULL;
	if (ch == '\b')             /* Erase last char if BACKSPACE is hit */
	{
	   if (wherex() > x)
	   {
	      gotoxy(wherex()-1,y);
	      cprintf(" ");
	      gotoxy(wherex()-1,y);
	      --t;
	      --num;
	   }
	}
	else                        /* Otherwise add char to string */
	{
	   cprintf("%c",ch);
	   *t++=ch;
	   gotoxy(x+(++num),y);
	}
   }
   *t='\0';

   /* Clean up screen by clearing to end of line, but only if */
   /* chars were entered.                                     */
/*   if (num > 0) */
      clreol();

   return(tmp);
}




/*****************************************************************/
/* Ask the user to enter a new file name to write the data to.   */
/*****************************************************************/
int get_file_name(file_name)
   char *file_name;
{
   /* Position cursor & print prompt */
   gotoxy(2,1);
   textcolor(LIGHTGREEN);
   cprintf("File name:");
   textcolor(YELLOW);
   gotoxy(13,1);
   clreol();

   /* Now ask the user to enter a new file name for the output file. */
   memset(file_name,'\0',MAXPLEN);
   strcpy(file_name,get_field(MAXPLEN));
   strupr(file_name);

   /* If the length of the file name is 0, return the ABORT value to */
   /* signify that the user wants to abort this routine.             */
   if (! strlen(file_name))
      return(ABORT);

   return(TRUE);
}




/****************************************************************************
If the user specified -f <file>, try and open the data file
****************************************************************************/
int open_data_file(fname)
   char *fname;
{

   char cur_dir[MAXDIR], orig_dir[MAXDIR];
   int file_ok=FALSE;
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
	    /* Dir entry isn't there. If no dir exists with same name,    */
	    /* create a new file.                                         */

	    fclose(data_file);
	    cur_drive=getdisk();        /* Remember current disk settings */
	    strcpy(cur_dir,"\\");
	    getcurdir(0,&cur_dir[1]);

	    /* If it's a directory, display error message and abort */
	    if (chdir(fname) == 0)
	       return(FALSE);
	    else
	    {
		 /* Otherwise, erase the file & open in WRITE mode */
		 setdisk(cur_drive);
		 chdir(cur_dir);
		 if ((data_file=fopen(fname,"w")) != NULL)
		    file_ok=TRUE;
		 else
		    return (FALSE);
	    }
	}

	/* If we could open the file, see if the user wishes to erase it */
	else
	{
	    fclose(data_file);
	    if ((data_file=fopen(fname,"w")) != NULL)
		file_ok=TRUE;
	    else
		return (FALSE);
	}
   }

   /* Reset drive & path to original settings, as at beginning of function */
   setdisk(orig_drive);
   chdir(orig_dir);
   return(TRUE);            /* Path was OK, so return positive value */

}




/****************************************************************************
Print all results
****************************************************************************/
void print_results(dirs,num_dirs)
   struct DIRECTORY_ENTRY dirs[];
   int num_dirs;
{
   char *device="LPT1";
   int n=0;
   int next_dir=0;
   FILE *printer;


   /* Try and print a banner. If there is a printer fault, our error   */
   /* handler will return NULL.                                         */

   PRINTER_ERROR=FALSE;
   if ((printer=fopen(device,"w")) == NULL)
   {
      PRINTER_ERROR=TRUE;
      gotoxy(1,1);
      clreol();
      textcolor(RED+BLINK);
      gotoxy(12,1);
      cprintf("Could not open printer !");
      delay(0);
      delay(1500);
      gotoxy(1,1);
      clreol();
   }


   /* Check again for a printer error. Quit if an error occurred. */

   if (! PRINTER_ERROR)
   {
      if (fprintf(printer,"\n\n   %s\n",revision) == NULL)
      {
	 PRINTER_ERROR=TRUE;

	 gotoxy(1,1);
	 clreol();
	 textcolor(RED+BLINK);
	 gotoxy(17,1);
	 cprintf("Printer error !");
	 delay(0);
	 delay(1500);
	 gotoxy(1,1);
	 clreol();
      }
   }


   if (! PRINTER_ERROR)
   {

      /* Sort the directory list as appropriate */
      sort_dirs(dirs,num_dirs);

      fprintf(printer,"\nSize of directories below  %s\n",strupr(start_dir));

      for (n=0;n<46;n++)
	  fprintf(printer,"~");
      fprintf(printer,"\n");

      /* Now print the results */
      for (next_dir=0;next_dir<num_dirs;next_dir++)
      {
	 fprintf(printer,"%-35s  %6ld kb",dirs[next_dir].entry,
		 dirs[next_dir].size);

	 if (dirs[next_dir].entry[0] == '.' &&
	    (strlen(dirs[next_dir].entry) == 1))
	    fprintf(printer,"   (starting directory)");

	 if (dirs[next_dir].entry[0] == '.' &&
	    dirs[next_dir].entry[1] == '.')
	    fprintf(printer,"   (parent directory)");

	 fprintf(printer,"\n");
      }

      /* Print total amount of space used by all dirs */
      for (n=0;n<46;n++)
	  fprintf(printer,"-");

      fprintf(printer,"\n%-20s %7ld %6s %7ld kb\n","Total space used by",
	     total_files,"files:",total);

      /* Finally, print total disk stats for reference */
      fprintf(printer,"\n%-33s  %8ld kb\n","Total disk space on drive",
	     disk_stats(TOTAL_SPACE) / kilobytes);

      fprintf(printer,"%-33s  %8ld kb\n","Total free disk space",
	     disk_stats(AVAIL_SPACE) / kilobytes);

      /* Print a Form Feed and then flush & close printer device */
      fprintf(printer,"%c\n",FF);
      fflush(printer);
      fclose(printer);

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


   /* Sort the directory list as appropriate */
   sort_dirs(dirs,num_dirs);

   /* Write a banner to the file */
   fprintf(data_file,"\n\n   %s\n",revision);
   fprintf(data_file,"\nSize of directories below  %s\n",strupr(start_dir));
   for (n=0;n<46;n++)
	fprintf(data_file,"~");
   fprintf(data_file,"\n");

   /* Now print the results to the file */
   for (next_dir=0;next_dir<num_dirs;next_dir++)
   {
      fprintf(data_file,"%-35s  %6ld kb",dirs[next_dir].entry,
	     dirs[next_dir].size);

      if (dirs[next_dir].entry[0] == '.' &&
	 (strlen(dirs[next_dir].entry) == 1))
	 fprintf(data_file,"   (starting directory)");

      if (dirs[next_dir].entry[0] == '.' && dirs[next_dir].entry[1] == '.')
	 fprintf(data_file,"   (parent directory)");

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





/* ------------------------------------------------------------------------ */
/*                                                                          */
/*               M I S C    S E T U P    R O U T I N E S                    */
/*                                                                          */
/* ------------------------------------------------------------------------ */


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
Insert commas into a numeric string.      
****************************************************************************/
void insert_commas(string)
   char *string;
{
   static char tmp[MAXPLEN];
   static int i,n,slen;

   slen=strlen(string);
   reverse(string);
   memset(tmp,'\0',MAXPLEN);

   for (n=0,i=0;n<slen;n++)
   {
       tmp[i++] = string[n];
       if (((n+1) % 3) == 0)
	  tmp[i++] = ',';
   }

   reverse(tmp);
   if (tmp[0] == ',')
      strcpy(string,&tmp[1]);
   else
      strcpy(string,tmp);
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




/**********************************************/
/* Reset all variables                        */
/**********************************************/
int reset_vars(dirs)
   struct DIRECTORY_ENTRY dirs[];
{
   int n=0;

   for (n=0;n<MAX_DIRS;n++)             /* Array of directory entries */
   {
      memset(dirs[n].entry,'\0',13);
      dirs[n].size=0L;
   }

   bytes_cluster=0L;           /* Various totals etc.... */
   total_files=0L;
   total=0L;
   total_used=0L;
   total_space_used=0L;

   return(0);                  /* Number of directories is 0 */
}




/***************************/
/* Help !!!!!!!!           */
/***************************/
void setup_help()
{

   /* A BAD way to code a help module, but it will do until i can think */
   /* of a better way.                                                  */

   strcpy(help_data[0].line,"Entering other subdirectories");
   strcpy(help_data[1].line,"-----------------------------");
   strcpy(help_data[2].line,"Position the cursor bar on the subdirectory that");
   strcpy(help_data[3].line,"you wish to examine, and press RETURN. The program");
   strcpy(help_data[4].line,"will then determine the size of all of the");
   strcpy(help_data[5].line,"subdirectories  within  this directory.");
   strcpy(help_data[6].line," ");
   strcpy(help_data[7].line,"Pressing  RETURN  on  '..'  will force the program");
   strcpy(help_data[8].line,"to go up one level.");
   strcpy(help_data[9].line," ");
   strcpy(help_data[10].line,"Sort Order");
   strcpy(help_data[11].line,"----------");
   strcpy(help_data[12].line,"By default, subdirectories are sorted by size, not");
   strcpy(help_data[13].line,"alphabetical order. Pressing  F3  will toggle");
   strcpy(help_data[14].line,"between the available sorting methods. The current");
   strcpy(help_data[15].line,"sorting method will be used when examining other");
   strcpy(help_data[16].line,"subdirectories or drives.");
   strcpy(help_data[17].line," ");
   strcpy(help_data[18].line,"Searching other disk drives");
   strcpy(help_data[19].line,"---------------------------");
   strcpy(help_data[20].line,"Use the  F2  option to Change Path, and simply ");
   strcpy(help_data[21].line,"give the new path as being  D:  (or whatever drive");
   strcpy(help_data[22].line,"is required). By default, the program will");
   strcpy(help_data[23].line,"automatically start at the root directory.");
   strcpy(help_data[24].line," ");
   strcpy(help_data[25].line,"Network compatibility");
   strcpy(help_data[26].line,"---------------------");
   strcpy(help_data[27].line,"The program has been tested on Novell Netware");
   strcpy(help_data[28].line,"versions 2.12, 2.15 and 3.11. No guarantees are");
   strcpy(help_data[29].line,"made concerning the compatibility or reliability");
   strcpy(help_data[30].line,"of this program with any particular network or PC.");
   strcpy(help_data[31].line," ");
   strcpy(help_data[32].line,"You must use this program at your own risk, as the");
   strcpy(help_data[33].line,"programmer will not accept any responsibility for ");
   strcpy(help_data[34].line,"any loss of data or damage caused by this program.");
   strcpy(help_data[35].line,"              YOU HAVE BEEN WARNED !!");
   strcpy(help_data[36].line," ");
   strcpy(help_data[37].line,"Program Bugs");
   strcpy(help_data[38].line,"------------");
   strcpy(help_data[39].line,"Bugs! In MY programs! Never!!!!  Seriously, the");
   strcpy(help_data[40].line,"program ONLY prints to LPT1:. Also, it doesn't");
   strcpy(help_data[41].line,"recognize when there is a fault on LPT1: (ie. paper");
   strcpy(help_data[42].line,"out, disconnected printer, etc...), but the program");
   strcpy(help_data[43].line,"won't crash - you just won't get any output.");
   strcpy(help_data[44].line," ");
   strcpy(help_data[45].line," ");
   strcpy(help_data[46].line,"Licensing");
   strcpy(help_data[47].line,"---------");
   strcpy(help_data[48].line,"Version 4.3 is Public Domain. No royalties need");
   strcpy(help_data[49].line,"to be paid for its use. Then again, there is no");
   strcpy(help_data[50].line,"support of any shape or form, so I can't really");
   strcpy(help_data[51].line,"charge for it !  Use this program AT YOUR OWN");
   strcpy(help_data[52].line,"RISK.");
   strcpy(help_data[53].line," ");
   strcpy(help_data[54].line,"(C) 1993  D. Gawler (Living Legend Software)");
   total_help_lines=55;
}






/*********************************************************/
/* Calculate size of all sub-directories                 */
/*********************************************************/
int calc_dir_sizes(dirs)
   struct DIRECTORY_ENTRY dirs[];
{
   int done=FALSE;              /* Logical flag which stops processing    */
   int num_dirs=0;              /* Total number of dirs found             */
   long int top_dir_size=0L;    /* Space used by files in starting dir    */
   struct ffblk ffblk;


   /* Display message on the screen for user */
   banner();

   /* Force system to calc cluster size */
   disk_stats(TRUE);

   /* Get a list of directory entries. */
   done=findfirst(file_spec,&ffblk,255);

   /* While not finished processing directory entries...... */
   while (! done)
   {
      /* Never process entries if they start with a '.', as this means */
      /* that the entry is the current or parent directory.            */

      if (ffblk.ff_name[0] != '.')
      {
	 /* If entry is a sub-dir, process all entries within it. */
	 if (directory(ffblk.ff_attrib))
	 {
	    dirs[num_dirs].size=get_dir_size(ffblk.ff_name) / (long) kilobytes;
	    strcpy(dirs[num_dirs].entry,ffblk.ff_name);
	    num_dirs++;
	 }
	 else   /* Otherwise entry is a file, so add to size of dir. */
	 {
	    top_dir_size += ffblk.ff_fsize;
	    total_files += 1L;
	 }
      }

      /* Update display */
      update_display(total_files,ffblk.ff_fsize);

      done=findnext(&ffblk);            /* Now process the next entry. */
   }

   /* Add root dir to our table of subdirs */
   dirs[num_dirs].size=(long) top_dir_size / (long) kilobytes;
   strcpy(dirs[num_dirs].entry,".");         /* Flag starting dir with "." */
   num_dirs++;

   /* Add parent directory so that they can change up */
   dirs[num_dirs].size=0L;
   strcpy(dirs[num_dirs].entry,"..");
   num_dirs++;

   return(num_dirs);

}




/****************************************************************************
Get the total size of the current directory
****************************************************************************/
long int get_dir_size(path)
	 char *path;
{
   char cur_dir[MAXDIR], tmp_dir[50];
   int done=0;
   long int dir_space=0L;       /* Space used by files in this sub-dir */
   struct ffblk ffblk;          /* File structure                      */


   /* Change to the sub-dir name that was passed to this routine */
   chdir(path);

   /* Tell user which directory we are processing */
   memset(tmp_dir,'\0',50);
   getcurdir(0,cur_dir);
   strncpy(tmp_dir,cur_dir,48);  /* Only want to display 50 chars of path */
   gotoxy(4,6);

   /* Set up screen colors etc.... */
   textbackground(LIGHTBLUE);
   textcolor(LIGHTGREEN);
   clreol();
   cprintf("Processing  %c:\\%s",start_drive+'A',tmp_dir);

   /* Get a list of all directory entries within this sub-dir */
   done=findfirst("*.*",&ffblk,255);

   /* While there are still directory entries to process ......*/
   while (! done)
   {
      /* Never process entries if they start with a '.', as this means */
      /* that the entry is the current or parent directory.            */
      if (ffblk.ff_name[0] != '.')
      {
	 if (directory(ffblk.ff_attrib))
	    dir_space += get_dir_size(ffblk.ff_name);   /* Yes, so get size of this sub-directory. */
	 else
	 {
	    dir_space += ffblk.ff_fsize;                /* No, so add file size to total directory size. */
	    total_files += 1L;
	 }
      }

      update_display(total_files,ffblk.ff_fsize);
      done=findnext(&ffblk);            /* Process next directory entry. */
   }

   chdir("..");             /* Change back to calling sub-directory.       */
   return (long) dir_space; /* Return size of sub-dir to calling function. */
}




/****************************************************************************
Function which extracts the specified path from the command line argument
****************************************************************************/
int get_dir_spec(path)
    char *path;
{
   int BAD_FSPEC=FALSE;

   /* The first arg MUST be the drive and/or dir-spec, so check arg 2 */
   memset(start_dir,'\0',MAXDIR);
   strcpy(start_dir,"X:");

   /* Find out the drive to be searched. */
   if (path[1] == ':')
      start_drive=toupper(path[0]) - 'A';
   else
      start_drive=getdisk();

   /* Add drive letter to start_dir (eg. "A:") */
   start_dir[0]=start_drive + 'A';

   /* Simply add remainder of argument to start_dir. */
   if (path[1] == ':')
      strcat(start_dir,&path[2]);
   else
      strcat(start_dir,path);

   /* If start_dir only contains the drive letter, add a "\" to make */
   /* it scan from the root directory.                               */
   if (strlen(start_dir) == 2 && start_dir[1] == ':')
      strcat(start_dir,"\\");

   /* Check that the specified directory actually exists */
   setdisk(start_drive);
   if (getdisk() != start_drive)
      BAD_FSPEC=TRUE;

   if (chdir(start_dir) != 0)
      BAD_FSPEC=TRUE;

   /* If we had a bad file spec, say so and quit. */
   if (BAD_FSPEC)
      return(FALSE);

   /* Otherwise, we successfully changed to their specified drive */
   /* and directory, so set a positive flag.                      */
   return(TRUE);
}




/*************************************************/
/* Ask the user to enter a new path              */
/*************************************************/
int get_new_path()
{
   char new_path[MAXPLEN];
   int quit=FALSE;
   int valid_path=FALSE;

   /* Position cursor & print prompt */
   gotoxy(2,1);
   textcolor(LIGHTGREEN);
   memset(new_path,'\0',MAXPLEN);
   cprintf("New path:");
   textcolor(YELLOW);

   /* Ask use for another drive/path that they wish to check. */
   while (! valid_path)
   {
      gotoxy(12,1);
      clreol();
      strcpy(new_path,get_field(MAXPLEN-1));
      strupr(new_path);

      /* If the length of new_path is 0, assume user wants to quit */
      /* this function.                                            */
      if (strlen(new_path) > 0)
	 valid_path=get_dir_spec(new_path);
      else
      {
	 valid_path=TRUE;
	 quit=TRUE;
      }
   }
   if (quit)
      return(ABORT);
   else
      return (valid_path);
}




/****************************************************************/
/* Concatenate the directory name and its size into one string. */
/****************************************************************/
void make_table(num_dirs,dirs,entries)
   int num_dirs;
   struct DIRECTORY_ENTRY dirs[];
   struct ENTRY entries[];
{
   int n=0;
   int next_dir=0;
   struct DIRECTORY_SIZE dir_sizes[MAX_DIRS];


   /* Initialize all entries */
   for (n=0;n<MAX_DIRS;n++)
       memset(entries[n].string,'\0',32);

   /* Convert long integer of total dir size to a string. */
   for (next_dir=0;next_dir<num_dirs;next_dir++)
   {
       ltoa(dirs[next_dir].size,dir_sizes[next_dir].string,10);
       insert_commas(dir_sizes[next_dir].string);
   }

   /* Concatenate strings */
   for (next_dir=0;next_dir<num_dirs;next_dir++)
   {
       strcpy(entries[next_dir].string,dirs[next_dir].entry);

       for (n=0;n<(12-strlen(dirs[next_dir].entry));n++)
	   strcat(entries[next_dir].string," ");

       for (n=0;n<(8-strlen(dir_sizes[next_dir].string));n++)
	   strcat(entries[next_dir].string," ");

       strcat(entries[next_dir].string,"      ");
       strcat(entries[next_dir].string,dir_sizes[next_dir].string);
   }
}




/****************************************************************************
Function to process and validate command line switches
****************************************************************************/
void process_options(argc, argv)
     int argc;
     char *argv[];
{
     int valid_path=FALSE;

   /* Set defaults if no args supplied */
   if (argc < 2)
   {
	start_dir[0]=getdisk() + 'A';
	start_dir[1]=':';
	start_dir[2]='\\';                      /* Dir to start in  */
	getcurdir(0,&start_dir[3]);
	start_drive=getdisk();                  /* Disk to start on */
   }
   else
   {
      /* They must be specifiying a path, so validate it. */
      valid_path=get_dir_spec(argv[1]);

      /* If their options were bad, display the help screen and quit */
      if (! valid_path)
	 display_usage();
   }
}



/***********************************************************/
/* Reverse a string.                                       */
/***********************************************************/
void reverse(string)
   char *string;
{
   static char tmp[MAXPLEN];
   static int i,n,slen;

   slen=strlen(string);
   memset(tmp,'\0',MAXPLEN);

   for (n=slen-1,i=0;n>=0;n--)
       tmp[i++] = string[n];

   memset(string,'\0',slen);
   strcpy(string,tmp);
}



/***********************************************************/
/* Display list of results on the screen, and let the user */
/* scroll through the list.                                */
/***********************************************************/
void scroll_list(entries,num_dirs,dirs)
   struct ENTRY entries[];
   struct DIRECTORY_ENTRY dirs[];
   int num_dirs;
{
   char screen_buffer[4096];
   char ch, cursor_bar[80];
   char file_name[MAXPLEN], tmp[MAXDIR];
   int status=FALSE;
   int extended_key=FALSE;
   int move_bar=FALSE, modified_table=FALSE, valid_file=FALSE;
   int n=0;
   int start_table=0;
   int start_window=0;
   int bar_pos=0, last_bar_pos=0;
   int array_start=0;
   int array_end=num_dirs-1;
   int window_params[5]={1,1,80,25,0};


   /* Develop a 'cursor_bar' line for use as a cursor bar */

   memset(cursor_bar,' ',80);
   cursor_bar[WindowWidth-1]='\0';


   /* Display the first screen full of directory names on the screen. */

   textcolor(WHITE);
   for (n=start_table;(n < start_table+SizeOfWindow) && (n < num_dirs);n++)
   {
      gotoxy(2,1+n);
      cprintf("%s",entries[n].string);
   }


   /* Now wait for the user to hit a key. */

   move_bar=TRUE;                   /* Assume screen refresh is needed */
   ch=' ';
   while (ch != ESC)
   {
     gotoxy(WindowWidth-1,bar_pos+1);

     if (kbhit())                   /* If the keyboard has been hit... */
     {
	ch=getch();                 /* ... get a char from the buffer. */
	switch (ch)                 /* Now process the cbaracter.      */
	{

	   /* Was it an extended key ? */
	   case 0:
		extended_key=TRUE;
		break;


	   /* If they hit the PGUP, move the bar up by a screen full. */
	   case PGUP:

		if (! extended_key)
		   break;


		/* If bar is at top of table, just quit. */

		if (bar_pos == start_window && start_window == start_table)
		{
		   extended_key=FALSE;
		   break;
		}

		/* If bar is somewhere within the window, other than at  */
		/* the first line, move the bar to the first line of the */
		/* window.                                               */

		if (bar_pos > start_window)
		{
		   bar_pos = start_window;
		   move_bar=TRUE;
		}

		/* If they have reached the top of the screen, see if */
		/* there are any previous entries to be displayed.    */

		else if (bar_pos == start_window)
		{
		   start_table -= SizeOfWindow;
		   modified_table=TRUE;
		   if (start_table < array_start)
		      start_table = array_start;
		}
		extended_key=FALSE;

	   /* They hit a down arrow, so try and move the bar down. */
	   case PGDN:

		if (! extended_key)
		   break;

		/* If they're at the end of the array already, don't  */
		/* update the screen.                                 */

		if (((bar_pos == start_window+SizeOfWindow-1) &&
		     (start_table+SizeOfWindow-1 == array_end)) ||
		     (start_window+bar_pos == num_dirs-1))
		{
		   extended_key=FALSE;
		   break;
		}

		/* If the bar isn't at the bottom of the window, move */
		/* it there.                                          */

		if (bar_pos < start_window+SizeOfWindow-1)
		{
		   if (start_window+SizeOfWindow < num_dirs-1)
		   {
		      bar_pos  = start_window+SizeOfWindow-1;
		      move_bar = TRUE;
		   }
		   else
		   {
		      bar_pos = start_window+num_dirs-1;
		      move_bar = TRUE;
		   }
		}

		/* If they have reached the bottom of the screen, see if */
		/* there are any further entries to be displayed.        */

		else if (bar_pos == start_window+SizeOfWindow-1)
		{
		   start_table += SizeOfWindow;
		   modified_table=TRUE;
		   if (start_table > array_end-SizeOfWindow+1)
		      start_table = array_end-SizeOfWindow+1;
		}
		extended_key=FALSE;
		break;

	   /* If they hit an up arrow, move the bar up. */
	   case UP:
		if (! extended_key)
		   break;

		if (bar_pos > start_window)
		{
		   bar_pos--;
		   move_bar=TRUE;
		}

		/* If they have reached the top of the screen, see if */
		/* there are any previous entries to be displayed.    */

		else if (bar_pos == start_window)
		{
		   if (start_table > array_start)
		   {
		      start_table--;
		      modified_table=TRUE;
		   }
		}
		extended_key=FALSE;
		break;


	   /* They hit PGDN, so try and move the bar down a screenful. */
	   case DOWN:

		if (! extended_key)
		   break;

		if (bar_pos < start_window+SizeOfWindow-1)
		{
		   if (start_window+bar_pos < num_dirs-1)
		   {
		      bar_pos++;
		      move_bar=TRUE;
		   }
		}

		/* If they have reached the bottom of the screen, see if */
		/* there are any further entries to be displayed.        */

		else if (bar_pos == start_window+SizeOfWindow-1)
		{
		   if (start_table+SizeOfWindow-1 < array_end)
		   {
		      start_table++;
		      modified_table=TRUE;
		   }
		}
		extended_key=FALSE;
		break;

	   case F1:
		/* Display system help */
		if (! extended_key)
		   break;

		help();
		break;

	   case F2:
		/* Get a new drive path from the user */

		if (! extended_key)
		   break;

		/* Save screen info. */
		memset(screen_buffer,'\0',4096);
		save_screen_info(window_params);
		window(1,1,80,25);
		gettext(9,14,71,18,screen_buffer);
		draw_input_window(10,15,70,17);

		/* Prompt user for new path. */
		status=FALSE;
		status=get_new_path();

		/* Reset screen to how it was. */
		window(1,1,80,25);
		puttext(9,14,71,18,screen_buffer);
		reset_screen_info(window_params);

		/* If the get_new_path returned a NULL value into the */
		/* abort variable, break out of routine.              */
		if (status == ABORT)
		   break;

		/* Otherwise, continue processing the new path.       */
		/* First, reset all vars */
		num_dirs=reset_vars(dirs);

		/* Reset initial drive and directory */
		setdisk(start_drive);
		chdir(start_dir);

		/* Redraw initial banner & re-calc all dir sizes */
		banner();
		num_dirs=calc_dir_sizes(dirs);
		array_end=num_dirs-1;

		/* Re-sort & display the list */
		sort_dirs(dirs,num_dirs);
		make_table(num_dirs,dirs,entries);
		paint_screen();

		/* Reset flags & pointers to redisplay table */
		modified_table=TRUE;
		start_table=0;
		start_window=0;
		bar_pos=0;
		last_bar_pos=0;
		extended_key=FALSE;
		break;

	   case F3:
		/* Change the sort order */

		if (! extended_key)
		   break;

		if (SORT_BY_SIZE)
		   SORT_BY_SIZE=FALSE;
		else
		   SORT_BY_SIZE=TRUE;

		/* Re-sort & display the list */
		sort_dirs(dirs,num_dirs);
		make_table(num_dirs,dirs,entries);

		/* Reset flags & pointers to redisplay table */
		modified_table=TRUE;
		start_table=0;
		start_window=0;
		bar_pos=0;
		last_bar_pos=0;
		extended_key=FALSE;
		break;

	   case F4:
		/* Recalc size of directories */

		if (! extended_key)
		   break;

		/* Reset all vars */
		num_dirs=reset_vars(dirs);

		/* Reset initial drive and directory */
		setdisk(start_drive);
		chdir(start_dir);

		/* Redraw initial banner & re-calc all dir sizes */
		banner();
		num_dirs=calc_dir_sizes(dirs);

		/* Re-sort & display the list */
		sort_dirs(dirs,num_dirs);
		make_table(num_dirs,dirs,entries);
		paint_screen();

		/* Reset flags & pointers to redisplay table */
		modified_table=TRUE;
		start_table=0;
		start_window=0;
		bar_pos=0;
		last_bar_pos=0;
		extended_key=FALSE;
		break;

	   case F5:
		/* Write all output data to a file */

		if (! extended_key)
		   break;

		/* Save current screen image */
		memset(screen_buffer,'\0',4096);
		save_screen_info(window_params);
		window(1,1,80,25);
		gettext(9,14,71,18,screen_buffer);
		draw_input_window(10,15,70,17);

		valid_file=FALSE;
		while (! valid_file)
		{
		   /* If the user presses <ESC> for the file name,   */
		   /* or just <CR>, abort will contain a NULL value. */
		   /* In this case, abort this routine.              */
		   status=FALSE;
		   status=get_file_name(file_name);
		   if (status == ABORT)
		      break;
		   valid_file=open_data_file(file_name);
		}

		/* Write data to file */
		window(1,1,80,25);
		puttext(9,14,71,18,screen_buffer);
		reset_screen_info(window_params);
		write_results(dirs,num_dirs);
		extended_key=FALSE;
		break;

	   case F6:
		/* Print all output */

		if (! extended_key)
		   break;

		/* Save current screen image */
		memset(screen_buffer,'\0',4096);
		save_screen_info(window_params);
		window(1,1,80,25);
		gettext(1,1,80,25,screen_buffer);
		draw_input_window(15,15,65,17);
		textcolor(LIGHTGREEN);
		gotoxy(8,1);
		cprintf("Printing data, please wait....");

		/* Print data */
		print_results(dirs,num_dirs);
		window(1,1,80,25);
		puttext(1,1,80,25,screen_buffer);
		reset_screen_info(window_params);
		extended_key=FALSE;
		break;

	   case CR:

		/* Make sure they haven't chosen the current directory. */
		if (dirs[start_table+bar_pos].entry[0] == '.' &&
		   (strlen(dirs[start_table+bar_pos].entry) == 1))
		   break;

		/* If they are at the root and they've picked '..' */
		/* don't do anything.                              */
		if (strcmp(dirs[start_table+bar_pos].entry,"..") == 0)
		{
		   getcurdir(0,tmp);
		   if (strlen(tmp) == 0)   /* Root dir returns zero length */
		      break;
		}

		/* Get current drive and directory */
		start_drive=getdisk();
		where_are_we(start_dir,MAXDIR);

		/* Append the name of the directory that the cursor     */
		/* bar is sitting on, onto the end of our current path. */
		if ((strlen(start_dir) == 3) && ((start_dir[2] == '\\')))
		   strcat(start_dir,dirs[start_table+bar_pos].entry);
		else
		{
		   strcat(start_dir,"\\");
		   strcat(start_dir,dirs[start_table+bar_pos].entry);
		}

		chdir(start_dir);

		/* Remember where we are. */
		where_are_we(start_dir,MAXDIR);

		/* Reset all vars */
		num_dirs=reset_vars(dirs);

		/* Redraw initial banner & re-calc all dir sizes */
		banner();
		num_dirs=calc_dir_sizes(dirs);
		array_end=num_dirs-1;

		/* Re-sort & display the list */
		sort_dirs(dirs,num_dirs);
		make_table(num_dirs,dirs,entries);
		paint_screen();

		/* Reset flags & pointers to redisplay table */
		modified_table=TRUE;
		start_table=0;
		start_window=0;
		bar_pos=0;
		last_bar_pos=0;
		break;

	   default :
		break;
	}
     }


     /* If the range of entries to be displayed has been changed by */
     /* scrolling off the screen, update the display.               */

     if (modified_table)
     {
	textbackground(BLUE);
	textcolor(WHITE);
	for (n=start_window;n<start_window+SizeOfWindow;n++)
	{
	   gotoxy(1,1+n);
	   cprintf("%s",cursor_bar);
	   gotoxy(2,1+n);
	   cprintf("%s",entries[start_table+n].string);
	}
	modified_table=FALSE;
	move_bar=TRUE;
     }


     /* If the cursor bar has moved, update the display. */

     if (move_bar)
     {
	/* Re-set the last entry to 'normal' screen attributes. */
	textcolor(WHITE);
	textbackground(LIGHTBLUE);
	gotoxy(1,start_window+last_bar_pos+1);
	cprintf("%s",cursor_bar);
	gotoxy(2,start_window+last_bar_pos+1);
	cprintf("%s",entries[start_table+last_bar_pos].string);

	/* Now highlight the new entry that the cursor bar is on. */
	textbackground(RED);
	textcolor(WHITE);
	gotoxy(1,start_window+bar_pos+1);
	cprintf("%s",cursor_bar);
	gotoxy(2,start_window+bar_pos+1);
	cprintf("%s",entries[start_table+bar_pos].string);

	/* Reset all flags. */
	last_bar_pos=bar_pos;           /* Remember where cursor_bar was */
	move_bar=FALSE;
     }
   }
}



/****************************************************************************
Function to sort the list of directory structures as appropriate
****************************************************************************/
void sort_dirs(dirs,num_dirs)
   struct DIRECTORY_ENTRY dirs[];
   int num_dirs;
{
   char entry[30];                      /* Temp storage for dir name        */
   int n=0;
   int fnum=0,nnum=0;                   /* LCV's                            */
   int sorted=FALSE;                    /* Inidicates that list is sorted   */
   int needs_swapping=FALSE;            /* Indicates elements need swapping */
   long int temp=0L;                    /* Temp storage for dir size        */


   /* First, determine total amount of space used by all directories */
   total=0L;
   for (n=0;n<num_dirs;n++)
       total += dirs[n].size;

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

   /* ALWAYS move '..' to the top of the list so that it easier for the */
   /* user to "pop up" a level.                                         */

   fnum=0;
   while (strcmp(dirs[fnum].entry,"..") != 0)
	 fnum++;

   for (nnum=fnum;nnum>0;nnum--)
   {
      /* Move the long int fields down the array one position */
      dirs[nnum].size=dirs[nnum-1].size;

      /* Move the character string fields down the array 1 pos. */
      strcpy(dirs[nnum].entry,dirs[nnum-1].entry);
   }
   dirs[0].size=0L;
   strcpy(dirs[0].entry,"..");
}




/*************************************************/
/* Where are we at the moment ???                */
/*************************************************/
void where_are_we(path,path_len)
     char *path;
     int path_len;
{
   /* Returns a string such as  "C:\DOS"  which tells us where we are. */

   /* Wipe existing path */
   memset(path,'\0',path_len);

   /* Set 'path' to current drive & path */
   strcpy(path,"X:\\");
   path[0]=getdisk() + 'A';
   getcurdir(0,&path[3]);

}







/* ------------------------------------------------------------------------ */
/*                                                                          */
/*                         M A I N     F U N C T I O N                      */
/*                                                                          */
/* ------------------------------------------------------------------------ */

void main(int argc, char **argv)
{
   int num_dirs=0;              /* Total number of dirs found             */
   struct DIRECTORY_ENTRY dirs[MAX_DIRS];       /* Stores dir name & size */


   /* Install our error handler */
   harderr(error_handler);

   /* Save current settings. */
   get_current_settings();

   /* Reset all variables */
   num_dirs=reset_vars(dirs);

   /* Setup help data */
   setup_help();

   /* Check command line arguments, if any were supplied. */
   process_options(argc,argv);

   /* Calculate the size of the directories */
   where_are_we(start_dir,MAXDIR);
   num_dirs=calc_dir_sizes(dirs);

   /* Store results in file if flag was set */
   if (WRITE_2_FILE)
      write_results(dirs,num_dirs);

   /* Print results if command line arg was set. */
   if (PRINT)
      print_results(dirs,num_dirs);

   /* Display our findings */
   display_results(dirs,num_dirs);

   /* Restore original settings */
   reset_path();

   /* Quit */
   textcolor(LIGHTGRAY);
   window (1,1,80,25);
   gotoxy(1,24);
}

