/*
 
  copyright (c) 2004, Mirek Kopertowski <m.kopertowski@post.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
*/

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include "exit.h"
#include "../parseopt/lex.h"
#include "../datastruct.h"
#include "../getstats.h"
#include "../conditions.h"
#include "../mainloop.h"
#include "communication.h"
#include "sighandlers.h"
#include "processtitle.h"


struct sigaction        schildact, stermact, susr1act, scontact;
sigset_t                sigblockset;

void schild_action (int snumber);
void sterm_action (int snumber); 
void susr1_parent_action (int snumber); 
void susr1_child_action (int snumber); 
void scont_parent_action (int snumber); 
void scont_child_action (int snumber); 


void signal_init (int process)
{

    sigemptyset(&sigblockset); 
    sigaddset(&sigblockset, SIGTERM);
    sigaddset(&sigblockset, SIGUSR1);

    switch (process) {
        case PARENT:
            memset (&schildact, 0, sizeof(schildact));
            schildact.sa_handler = &schild_action;
            schildact.sa_flags = SA_NOCLDSTOP;
            sigaction (SIGCHLD, &schildact, NULL);

            memset (&stermact, 0, sizeof(stermact));
            stermact.sa_handler = &sterm_action;
            sigaction (SIGTERM, &stermact, NULL);

            memset (&susr1act, 0, sizeof(susr1act));
            susr1act.sa_handler = &susr1_parent_action;
            sigaction (SIGUSR1, &susr1act, NULL);

            memset (&scontact, 0, sizeof(scontact));
            scontact.sa_handler = &scont_parent_action;
            sigaction (SIGCONT, &scontact, NULL);
            return;
        case CHILD:
            memset (&susr1act, 0, sizeof(susr1act));
            susr1act.sa_handler = &susr1_child_action;
            sigaction (SIGUSR1, &susr1act, NULL);             
            
            memset (&scontact, 0, sizeof(scontact));
            scontact.sa_handler = &scont_child_action;
            sigaction (SIGCONT, &scontact, NULL);
            return;
        default:
            return;
    }
}

void signals_handling (int onoff)
{
    switch (onoff) {
        case SIGBLOCK:
            sigprocmask (SIG_BLOCK, &sigblockset, NULL);                  
            return;
        case SIGUNBLOCK:
            sigprocmask (SIG_UNBLOCK, &sigblockset, NULL);
            return;
        default:
            return;
    }
}

/* local functions -----------------------------------------------------------*/

void schild_action (int snumber) 
{
    int     exitstatus, pid, i=0;

    pid = (int)wait (&exitstatus);
    
    while (*(ruleprocesses + i)!=pid) i++;
    
    *(ruleprocesses + i) = 0;
}

void sterm_action (int snumber) 
{
    int     i;
    
    /* deactive handling SIGCHLD */
    memset (&schildact, 0, sizeof(schildact));
    schildact.sa_handler = SIG_IGN;
    sigaction (SIGCHLD, &schildact, NULL);
    
    /* terminate child processes */
    for  (i=0; i<rulecount; i++)
        if (*(ruleprocesses + i)) kill ((pid_t)*(ruleprocesses + i), SIGTERM);

    /* detach, deallocate shared memory */
    shared_mem_detach (db_shm);
    shared_mem_del (db_shmid);
    shared_mem_detach (fs_shm);
    shared_mem_del (fs_shmid);
    shared_mem_detach (proc_shm);
    shared_mem_del (proc_shmid);
    shared_mem_detach (rules_shm);
    shared_mem_del (rules_shmid);

    /* deallocate semaphore */
    semaphore_del (semid);    

    bail_out (EXIT_OK, NULL);
}

void susr1_parent_action (int snumber) 
{
    int     i;
    
    /* send SIGUSR1 to child processes */
    for  (i=0; i<rulecount; i++)
        if (*(ruleprocesses + i)) kill ((pid_t)*(ruleprocesses + i), SIGUSR1);

    setproctitle (MAIN_PROCESS, STATE_STOPPED, NULL);  
    raise (SIGSTOP);
}

void susr1_child_action (int snumber) 
{
    setproctitle (RULE_PROCESS, STATE_STOPPED, rule->name);
    raise (SIGSTOP);
}

void scont_parent_action (int snumber)
{
    int     i;
    
    /* send SIGCONT to child processes */
    for  (i=0; i<rulecount; i++)
        if (*(ruleprocesses + i)) kill ((pid_t)*(ruleprocesses + i), SIGCONT);

    setproctitle (MAIN_PROCESS, STATE_NORMAL, NULL);
} 

void scont_child_action (int snumber)
{
    setproctitle (RULE_PROCESS, STATE_NORMAL, rule->name);
} 

