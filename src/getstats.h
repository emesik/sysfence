/*
 
  copyright (c) 2004, Michal Salaban <michal@salaban.info>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
*/

/* $Id: getstats.h,v 1.11 2004/05/31 11:59:29 emes Exp $ */

#define         BUFSIZE             512
#define         PROCDIRNAMELEN      25

/* shared memory segments */
int             db_shmid, fs_shmid, proc_shmid, rules_shmid;
void            *db_shm, *fs_shm, *proc_shm, *rules_shm;

/* temporary fs list (will be copied to *fs_shm after db initialization */
sf_list         *tmp_fs_db;

sf_database     *main_db;

sf_value get_stat_value (sf_stat_def *def);

void stat_init (sf_database *db);
void fetch_la (sf_database *db);
void fetch_mem (sf_database *db);
void fetch_fs (sf_database *db);
void fetch_proc (sf_database *db);

long int get_max_threads ();

void open_files ();

char * fto_newline (char *b);
char * fto_notspace (char *b);
char * fto_space (char *b);

/* $Id: getstats.h,v 1.11 2004/05/31 11:59:29 emes Exp $ */
