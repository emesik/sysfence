/*
 
  copyright (c) 2004, Michal Salaban <michal@salaban.info>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
*/

/* $Id: sysfence.c,v 1.33 2006/04/30 11:32:37 emes Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <syslog.h>
#include "sys/xalloc.h"
#include "sys/exit.h"
#include "parseopt/lex.h"
#include "datastruct.h"
#include "getstats.h"
#include "conditions.h"
#include "parseopt/parse.h"
#include "parseopt/confread.h"
#include "mainloop.h"
#include "sys/log.h"
#include "sys/communication.h"
#include "sys/sighandlers.h"
#include "cp2memory.h"
#include "sys/processtitle.h"

const char      *usage  =
                "sysfence v0.17, 09-12-2017\n"
                "Usage: sysfence <config file> [<config file> ...] \n"
                "\n"
                "https://github.com/emesik/sysfence\n"
                "Report bugs to <michal@salaban.info>\n"
                "\n";

int main (int argc, char *argv[])
{
    sf_rule     **ruletab, **shmruletab;
    void        *shmruleptr;
    void        *tmpfsptr;

    int         i;
    int         total_rules_size = 0;

    int         pid;
    int         uid;

    if (argc < 2) bail_out (EXIT_NOCONF, NULL);
    
    log_start ();

    /* empty list */
    tmp_fs_db = NULL;
    
    /* read config file(s) */
    ruletab = read_config_files (argc - 1, &argv[1]);
    if (*ruletab == NULL) bail_out (EXIT_NORULE, NULL);

    /* count rules */
    rulecount = 0;
    while (*(ruletab + rulecount) != NULL) {
        rulecount ++;
    }
    syslog (LOG_INFO, "loaded %d rules", rulecount);

    /* check if we're running with r00t uid */
    uid = getuid ();
    if (uid)
        syslog (LOG_INFO,
                "running with UID %d, process data may be inaccurate",
                uid);

    /* open files for reading data and detach from console */
    open_files ();

    if (daemon (0, 0) != 0) bail_out (EXIT_IO, "daemon()");

    /* initialize semaphore */    
    semid = semaphore_init (SEMAPHORE_SET);
    if (semid < 0) bail_out (EXIT_SEM, NULL);
        
    /* count ruletab size */
    for (i = 0; i < rulecount; i++)
        total_rules_size += get_rule_size (*(ruletab + i));
   
    /* initialize shared memory for rules */
    rules_shmid = shared_mem_init (total_rules_size + rulecount * sizeof (void *));
    if (rules_shmid < 0) bail_out (EXIT_SHM, NULL);
    rules_shm = shared_mem_attach (rules_shmid);
    
    /* initialize shared memory for stats database */ 
    db_shmid = shared_mem_init (sizeof (sf_database));
    if (db_shmid < 0) bail_out (EXIT_SHM, NULL);
    db_shm = shared_mem_attach (db_shmid);
    main_db = (sf_database *) db_shm;
    
    if (tmp_fs_db) {
        /* initialize shared memory for fs database */ 
        fs_shmid = shared_mem_init (get_list_size (tmp_fs_db));
        if (fs_shmid < 0) bail_out (EXIT_SHM, NULL);
        fs_shm = shared_mem_attach (fs_shmid);

        /* copy fs db to shared memory */
        tmpfsptr = fs_shm;
        cp_list (&tmpfsptr, tmp_fs_db);
    } else {
        fs_shm = NULL;
    }

    /* initialize shared memory for proc database */ 
    proc_shmid = shared_mem_init (get_max_threads () * sizeof (sf_proc_stat));
    if (proc_shmid < 0) bail_out (EXIT_SHM, NULL);
    proc_shm = shared_mem_attach (proc_shmid);

    /* initialize database */
    main_db->fetch_step = get_min_step (ruletab);
    main_db->fs = (sf_list *) fs_shm;
    main_db->proc = proc_shm;

#ifdef DEBUG
    syslog (LOG_DEBUG, "main_db created @ %x: fs -> %x, proc -> %x",
            main_db, main_db->fs, main_db->proc
            );
#endif

    /* copy ruletab to shared memory */
    shmruletab = (sf_rule **) rules_shm;
    shmruleptr = (rules_shm + rulecount * sizeof (void *));
    for (i = 0; i < rulecount; i++) {
        *(shmruletab + i) = (sf_rule *) cp_rule (&shmruleptr, *(ruletab + i));
    }
    
    /* rule-watching processes start before data provider, so we must get fresh
    * stats before starting them */
    fetch_la (main_db);
    fetch_mem (main_db);
    fetch_fs (main_db);
    fetch_proc (main_db);

    assert (rulecount);
    ruleprocesses = (int *) xalloc (NULL, rulecount * sizeof (int));
    i = rulecount; 
    do {
        pid = (int)fork ();
        i--;
        if (pid) *(ruleprocesses + i) = pid;            
    } while ((pid !=0) && (i!=0));

    /* child */
    if (pid == 0) {        
        /* start handling signals */
        signal_init (CHILD);
                
        /* process title */
        initproctitle (argc, argv);
        rule = *(shmruletab + i);
        setproctitle (RULE_PROCESS, STATE_NORMAL, rule->name);
        
        /* one child process one ruleset */
        rule_watch_loop (*(shmruletab + i));
    } else /* parent */
    {        
        /* start handling signals */
        signal_init (PARENT);
        
        /* process title */
        initproctitle (argc, argv);
        setproctitle (MAIN_PROCESS, STATE_NORMAL, NULL);
        
        /* parent goes into data-providing loop */
        res_probe_loop (main_db);
     }

    /* remove compilation warning */
    return 0;
}

/* $Id: sysfence.c,v 1.33 2006/04/30 11:32:37 emes Exp $ */
