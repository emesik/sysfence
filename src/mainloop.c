/*
 
  copyright (c) 2004, Michal Salaban <michal@salaban.info>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
*/

/* $Id: mainloop.c,v 1.18 2004/06/09 10:45:56 emes Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "parseopt/lex.h"
#include "datastruct.h"
#include "getstats.h"
#include "conditions.h"
#include "sys/log.h"
#include "sys/communication.h"
#include "mainloop.h"
#include "sys/processtitle.h"
#include "sys/sighandlers.h"
#ifdef DEBUG
#include <syslog.h>
#endif

void rule_watch_loop (sf_rule *rule)
{
    while (1) {
        /* save result of this and previous hit */
        rule->prevhit = rule->hit;

        /* set semaphore while checking data and block signals */
        signals_handling (SIGBLOCK);        
        semaphore_wait (semid);
        rule->hit = check_expression (rule->expr);
        semaphore_post (semid);
        signals_handling (SIGUNBLOCK);

        if (rule->hit) {
            /* first - log if neccessary */
            if (rule->log != NULL) {
                if (! (rule->log->once && rule->prevhit)) {
                    signals_handling (SIGBLOCK);
                    semaphore_wait (semid);
                    log_rulehit (rule);
                    semaphore_post (semid);
                    signals_handling (SIGUNBLOCK);
                }
            }
            
            /* second - run command */
            if (rule->run != NULL) {
                if (! (rule->run->once && rule->prevhit)) {
                    setproctitle (RULE_PROCESS, STATE_EXEC, rule->name);
                    system (rule->run->command);
                    setproctitle (RULE_PROCESS, STATE_NORMAL, rule->name);
                }
            }
        } else {
#ifdef DEBUG                
            syslog (LOG_DEBUG, "Rule @%x NOT hit.\n", (int) rule);
#endif
        }

        sleep (rule->step);
    }
}
        

void res_probe_loop (sf_database *db)
{
    while (1) {
        
        /* set semaphore while changing data and block signals */
        signals_handling (SIGBLOCK);
        semaphore_wait (semid);
        fetch_la (db);
        fetch_mem (db);
        fetch_fs (db);
        fetch_proc (db);
        semaphore_post (semid);
        signals_handling (SIGUNBLOCK);

        sleep (db->fetch_step);
    }
}


/* $Id: mainloop.c,v 1.18 2004/06/09 10:45:56 emes Exp $ */
