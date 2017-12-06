/*
 
  copyright (c) 2004, Michal Salaban <michal@salaban.info>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: exit.c,v 1.1 2004/05/26 17:30:42 mkoperto Exp $
 
*/

#include "exit.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

char *errormessage[] =
{
    [EXIT_IO]       = "I/O error on %s",
    [EXIT_MEM]      = "Out of memory",
    
    [EXIT_NOCONF]   = "No configuration given",
    [EXIT_NORULE]   = "No rules given",
   
    [EXIT_PARSE]    = "Parse error: %s",
    [EXIT_VALUE]    = "Invalid value: %s",
    [EXIT_OPTION]   = "Invalid option: %s",
    
    [EXIT_SHM]      = "Shared memory exists",
    [EXIT_SEM]      = "Semaphore exists",

    [EXIT_BUG]      = "Internal bug"
};

extern char *usage;

void bail_out (int excode, const char *details)
{
#ifdef DEBUG
    fprintf (stderr, "bail_out() exit code %d\n", excode);
#endif
    if (excode == EXIT_OK) exit (EXIT_OK);

    if (details != NULL)
        fprintf (stderr, errormessage[excode], details);
    else
        fprintf (stderr, errormessage[excode]);
    fprintf (stderr, "\n");

    if ((excode == EXIT_NOCONF) ||
        (excode == EXIT_VALUE) ||
        (excode == EXIT_OPTION))
            fprintf (stderr, usage);
    if (excode == EXIT_BUG)
        syslog (LOG_DEBUG, "%s %s", errormessage[excode], details);
    exit (excode);
} 

/* $Id: exit.c,v 1.1 2004/05/26 17:30:42 mkoperto Exp $ */
