/*
 
  copyright (c) 2004, Michal Salaban <michal@salaban.info>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: log.c,v 1.5 2004/06/05 12:49:41 emes Exp $
 
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include "xalloc.h"
#include "../parseopt/lex.h"
#include "../datastruct.h"
#include "../getstats.h"
#include "../conditions.h"
#include "log.h"

#define     BIGSTRBUF   1024

char * stats_2_string (sf_list *hd)
{
    char        *gluebuf, *thisbuf, *nextbuf, *defbuf;
    sf_value    val;

    if (!hd) return NULL;

    thisbuf = (char *) xalloc (NULL, STRBUF + 1);
    defbuf = def_2_string ((sf_stat_def *) hd->el);
    val = get_stat_value ((sf_stat_def *) hd->el);
#ifdef DEBUG
   syslog (LOG_DEBUG,
           "%s = type: %x, ptr: %x",
           defbuf,
           val.type,
           (long int) val.ptr);
#endif
    nextbuf = stats_2_string (hd->next);

    switch (val.type) {
        case INTEGER:
            snprintf (thisbuf, STRBUF, "%s = %llu", defbuf, *((long long int *) val.ptr));
            break;
        case DOUBLE:
            snprintf (thisbuf, STRBUF, "%s = %.2f", defbuf, *((double *) val.ptr));
            break;
        default:
#ifdef DEBUG
            syslog (LOG_DEBUG, "stats_2_string(): invalid val type: %d", val.type);
#endif
            break;      // to avoid warnings
    }

    free (defbuf);
    
    if (nextbuf) {
        gluebuf = (char *) xalloc (NULL, BIGSTRBUF + 1);
        snprintf (gluebuf, BIGSTRBUF, "%s, %s", thisbuf, nextbuf);
        free (thisbuf);
        free (nextbuf);
        return gluebuf;
    } else return thisbuf;
}

void log_start ()
{
    openlog ("sysfence",
            LOG_NDELAY | LOG_PID | LOG_CONS,
            LOG_DAEMON);
}

void log_rulehit (sf_rule *rule)
{
    char        *stattxt = stats_2_string (rule->watchstats);    
    
    syslog (LOG_WARNING, "%s: %s", rule->name, stattxt);
    free (stattxt);
}

void log_end ()
{
    syslog (LOG_INFO, "exiting");

    closelog ();
}

/* $Id: log.c,v 1.5 2004/06/05 12:49:41 emes Exp $ */
