#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "platform.h"
#include "dcl.h"
#include "termio.h"

// HLPVMS provides extensive help from a VMS help file.
#define	MAX_TOP	100
#define MAX_BUF	1024
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

// S_BLANKS_DELETE replaces consecutive blanks by one blank.
char *hlp_trim_blank(char *s)
{
	char	*tmp 	= strdup(s);
	char    *ptr    = tmp;
	int		lens 	= (int)strlen(s);
	int		i		= 0;
    char    newchr = ' ';
    char    oldchr = ' ';

	if (tmp) {
    	for(i = 0;i < lens; i++) {
    		oldchr = newchr;
    		newchr = s[i];
    		if (newchr == '\t') {
    			newchr = ' ';
    		}
        	if (oldchr != ' ' || newchr != ' ') {
                if (isprint((int)newchr)) {
                    *ptr++ = newchr;
                }
    		}
    	}
        *ptr = 0;
        strcpy(s, tmp);
        free(tmp);
    }
    return(s);
}
// trim trailing spaces
char *hlp_trim(char *s)
{
	int		i 		= (int)strlen(s);

	for (; i >= 0; i--) {
		if (isspace((int)s[i])) {
			s[i] = 0;
		}
		else {
			break;
		}
	}
	return(s);
}


int hlpvms(char *help_file)
{
    char    buffer[MAX_BUF + 1];
    char    in_line[MAX_BUF + 1];
    char    choice[80];
    int     i = 0;
    int     iline = 0;
    int     itop = 0;
    int     jerror = 0;
    char    lab;
    int     lenc = 0;
    int     level = 0;
    char    levelc[MAX_TOP][80];
    int     levelm[10];
    int     levelo = 0;
    int     levelt[MAX_TOP];
    int     move = 0;
    int     nline = 0;
    int     ntop = 0;
    int     num = 0;
    char    output[80];
    char    prompt[80];
    FILE    *fp;
    int		pos = 0;
    int		w   = 0;

    //  Open help file
    fp = fopen(help_file, "r");
    if (fp == NULL) {
    	(void) dcl_printf(dcl[D].SYS_OUTPUT,"HLPVMS - ERROR Could not open help file.");
        return(DCL_ERROR);
    }

    levelo = 0;
    level = 1;
    iline = 1;

    //
    //  Move to beginning of current topic by reading MOVE lines from
    //  the top of the file.  Record this position, corresponding to
    //  the current LEVEL, in LEVELM, in case we later want to back up.
    //
    //  Print out the heading line of this topic.
    //
    while(1) {
        jerror = 0;
        move = iline;
        levelm[level] = iline;

        for (i = 1; i < move; i++) {
            if (fgets(buffer, MAX_BUF, fp) == NULL) {
    			(void) dcl_printf(dcl[D].SYS_OUTPUT,"HLPVMS - ERROR Unexpected end of file, or other I/O error.");
        		return(DCL_ERROR);
            }
        }
        if (fgets(buffer, MAX_BUF, fp) == NULL) {
   			(void) dcl_printf(dcl[D].SYS_OUTPUT,"HLPVMS - ERROR Unexpected end of file, or other I/O error.");
       		return(DCL_ERROR);
        }
        lab = *buffer;
        if (buffer[strlen(buffer)-1] == '\n') buffer[strlen(buffer)-1] = 0;
        strcpy(in_line, isdigit((int)lab) ? &buffer[1] : buffer);
        dcl_printf(dcl[D].SYS_OUTPUT,"%s\n", " ");
        dcl_printf(dcl[D].SYS_OUTPUT,"%s\n", hlp_trim_blank(in_line));
        dcl_printf(dcl[D].SYS_OUTPUT,"%s\n", " ");
        nline = 3;
        //
        //  If 'going down' or redisplaying, (as opposed to backing up),
        //  display information available under the current topic.
        //
        //  We stop printing when we hit a numeric label.
        //
        //  If this label is less than or equal to current level, there are
        //  no subtopics.
        //
        //  Otherwise, we now move ahead to print out the list of subtopics
        //  available for this topic.
        //
        if (level >= levelo) {
            ntop = -1;

            while(1) {
                if (fgets(buffer, MAX_BUF, fp) == NULL) goto label_50;
                if (feof(fp)) {
                    goto label_50;
                }
        		if (buffer[strlen(buffer)-1] == '\n') buffer[strlen(buffer)-1] = 0;
                lab = *buffer;
                strcpy(in_line, isdigit((int)lab) ? &buffer[1] : buffer);
                move = move + 1;

                if (isdigit((int)lab)) {
                    num = lab;
                    if (num <= level) {
                        goto label_50;
                    }
                    ntop = 0;
                    break;
                }

                if (nline >= 24) {
                	int x = tio_wherex();
                	int y = tio_wherey();
                    dcl_printf(dcl[D].SYS_OUTPUT,"Press RETURN to continue ...");
                    tio_getch();
                    tio_gotoxy(x,y);
                    tio_clreol();
                    nline = 0;
                }

                nline = nline + 1;
                dcl_printf(dcl[D].SYS_OUTPUT,"%s\n", hlp_trim(in_line));

            }
        }
        else {
            ntop = 0;
            *in_line = 0;
            lab = ' ';
        }
        //
        //  Locate each subtopic by examining column 1, searching for
        //  integer label.
        //
        //  Assuming we are at level LEVEL, we are searching for labels
        //  equal to LEVEL+1.  As we encounter each such label, we want to
        //  store the rest of the line as a subtopic.  We ignore labels
        //  greater than LEVEL+1 because these are sub-subtopics, and we
        //  cease our search when we reach a label less than or equal to
        //  LEVEL.
        //
//label_40:

        while(1) {
            if (isdigit((int)lab)) {
                num = lab - 0x30;
                if (num <= level) {
                    break;
                }
                if (num == level+1) {
                    if (ntop == MAX_TOP+1) {
                        dcl_printf(dcl[D].SYS_OUTPUT,"HELP - Warning  Maximum number of topics reached!\n");
                    }
                    else if (ntop > MAX_TOP) {
                    }
                    else {
                        ntop = ntop + 1;
                        if (ntop == 1) {
                            dcl_printf(dcl[D].SYS_OUTPUT," \n");
                            dcl_printf(dcl[D].SYS_OUTPUT,"Help is available on:\n");
                            dcl_printf(dcl[D].SYS_OUTPUT," \n");
                            pos = 0;
                        }
                        w = (int)strlen(hlp_trim_blank(in_line));
            			w = w / 8;
            			w = w + 1;
            			w = w * 8;
            			if (pos + w > 72) {
            				dcl_printf(dcl[D].SYS_OUTPUT,"\n");
            				pos = 0;
            			}
            			dcl_printf(dcl[D].SYS_OUTPUT,"%-*.*s",w,w,in_line);
            			pos = pos + w;
                        levelt[ntop] = move;
                        strcpy(levelc[ntop], in_line);
                    }
                }
            }
            if (fgets(buffer, MAX_BUF, fp)==NULL) break;;
            if (feof(fp)) {
                break;
            }
        	if (buffer[strlen(buffer)-1] == '\n') buffer[strlen(buffer)-1] = 0;
            lab = *buffer;
            strcpy(in_line, &buffer[1]);

        	move = move + 1;
    	}

label_50:
        //
        //  Display subtopics.
        //
        //printf("%s\n", " ");
        //printf("%s\n", "RETURN to back up, ? to redisplay.");
        //
        //  Prompt for user choice of new topic, exit, or back up.
        //
label_60:
       nline = 0;
        if ( ntop > 0 ) {
            strcpy(prompt, "Enter topic you want help on, or RETURN or ?");
        }
        else {
            strcpy(prompt, "Enter RETURN or ?");
        }
        dcl_printf(dcl[D].SYS_OUTPUT,"\n");
        dcl_printf(dcl[D].SYS_OUTPUT,"%s ", prompt);

        nline = 0;
        memset(choice, 0, sizeof(choice));
        (void) kbdread(choice,16,data_stack,0);
        hlp_trim_blank(choice);
        lenc = (int)strlen(choice);
        tio_clrscr();

        if ( lenc <= 0 ) {
            *choice = '!';
        }
        //
        //  Consider ending this help session.
        //
        if (*choice == '!' && level == 1 ) {
            fclose(fp);
            return(DCL_OK);
        }
        //
        //  User wants to back up to a supertopic.  We must rewind.
        //
        rewind(fp);
        levelo = level;

        if (*choice == '!' ) {
            level = level - 1;
            iline = levelm[level];
        }
        //
        //  Redisplay current topic.
        //
        else if (*choice == '?') {
        }
        //
        //  User wants to go down to a subtopic.
        //
        else {
            itop = 0;
            for (i = 0; i <= ntop; i++) {
                if (strncasecmp(choice, levelc[i], strlen(choice))==0) {
                    itop = i;
                    break;
                }
            }
            if (itop == 0) {
                sprintf(output, "Sorry, no help available on %s.", choice);
                dcl_printf(dcl[D].SYS_OUTPUT,"%s\n", output);
                jerror = jerror + 1;
                goto label_60;
            }
            else {
                level = level + 1;
                iline = levelt[itop];
            }
        }
    } //  end while(1)

    return(DCL_OK);
}

