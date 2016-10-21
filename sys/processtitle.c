/*
 
  copyright (c) 2004, Mirek Kopertowski <m.kopertowski@post.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  The idea is taken from sendmail.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "processtitle.h"

#define SPT_BUFSIZE     2048

#define max(A,B) ((A) > (B) ? (A) : (B))

extern char     **environ;

static char     **arg;
static int      arglength;
char            buf[SPT_BUFSIZE];
int             auxtitlelength;

void initproctitle (int argc, char **argv) 
{
    int         i;
    char        **envp = environ;

    /* determine the size of environ */
    for (i = 0; envp[i] != NULL; i++);
        
    /* copy environ to new place */
    environ = (char **) malloc(sizeof(char *) * (i + 1));
    for (i = 0; envp[i] != NULL; i++) environ[i] = strdup(envp[i]);
    environ[i] = NULL;

    /* determine the available space */
    arg = argv;
    if (i > 0)
        arglength = envp[i-1] + strlen(envp[i-1]) - argv[0];
    else
        arglength = argv[argc-1] + strlen(argv[argc-1]) - argv[0];
        
    /* auxiliary title length */
    auxtitlelength = max (strlen (STATE_STOPPED_NAME),
                            strlen (STATE_EXEC_NAME)) + 3;    
}	

void setproctitle (int process, int state, const char *rulename)
{
    static int     length;

    switch (process) {
        case MAIN_PROCESS:
            length = strlen (MAIN_PROCESS_NAME) + strlen (STATE_STOPPED_NAME);

            /* do nothing if title too long */
            if (length + 1 > SPT_BUFSIZE) return;

            switch (state) {
                case STATE_NORMAL:
                    sprintf (buf, "%s", MAIN_PROCESS_NAME);
                    break;
                case STATE_STOPPED:
                    sprintf (buf, "%s %s", MAIN_PROCESS_NAME, 
                             STATE_STOPPED_NAME);
                    break;
                default:
                    return;
            } 
            break;
        case RULE_PROCESS:
            if (!rulename) return;
            length = auxtitlelength + strlen(rulename);

            /* do nothing if title too long */
            if (length + 1 > SPT_BUFSIZE) return;

            switch (state) {
                case STATE_NORMAL:
                    sprintf (buf, "%s '%s'", RULE_PROCESS_NAME, rulename);
                    break;
                case STATE_STOPPED:
                    sprintf (buf, "%s %s '%s'", RULE_PROCESS_NAME, 
                             STATE_STOPPED_NAME, rulename);
                    break;
                case STATE_EXEC:
                    sprintf (buf, "%s %s '%s'", RULE_PROCESS_NAME, 
                             STATE_EXEC_NAME, rulename);
                    break;
                default:
                    return;
            } 
            break;           
        default:
            return;
    }
    
    /* cut title if too long */
    if (length > arglength - 2) {
        length = arglength - 2;
        buf[length] = '\0';
    }
	
    /* clear the memory area */    
    memset (arg[0], '\0', arglength);       
    
    strcpy (arg[0], buf);

    /* only one arg */
    arg[1] = NULL;
}
