/*
 
  copyright (c) 2004-2006, Michal Salaban <emes/at/pld-linux/dot/org>,
                     2006, Paul Stynen <mammooth/at/scarlet/dot/be>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
*/

/* $Id: getstats.c,v 1.26 2006/04/30 12:21:35 emes Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <syslog.h>
#include "sys/xalloc.h"
#include "sys/exit.h"
#include "parseopt/lex.h"
#include "datastruct.h"
#include "getstats.h"

sf_value get_stat_fs (sf_stat_def *def, sf_list *fslist)
{
    sf_value        res;
    sf_fs_stat      *fs = get_fs_entry_from_list (fslist, def->arg[1].path);
    sf_res_state    slctr = def->arg[0].resstat;

#ifdef DEBUG
    syslog (LOG_DEBUG,
            "get_stat_fs(%s): total = %llu, used = %llu, free = %llu, avail = %llu",
            def->arg[1].path,
            fs->val[VA_TOTAL],
            fs->val[VA_USED],
            fs->val[VA_FREE],
            fs->val[VA_AVAIL]
            );
#endif

    res.type = INTEGER;
    res.ptr = &(fs->val[slctr]);
    return res;
}

sf_value get_nproc (sf_stat_def *def, sf_database *db)
{
    long long int   *nproc = (long long int *) xalloc (NULL, sizeof (long int));
    sf_value        res = { INTEGER, nproc };
    sf_list         *uids = def->arg[0].uids;
    char            statemask = def->arg[1].procstates;

    if (! uids) *nproc = get_proc_num (db, statemask, NULL);
    else *nproc = 0;
    
    while (uids) {
        *nproc = *nproc + get_proc_num (db, statemask, (uid_t *) uids->el);
        uids = uids->next;
    }
#ifdef DEBUG
    syslog (LOG_DEBUG, "%s = %lu", def_2_string (def), *nproc);
#endif
    return res;
}

sf_value get_stat_value (sf_stat_def *def)
{
    sf_value        res;
    sf_res_state    selector = def->arg[0].resstat;

#ifdef DEBUG
    syslog (LOG_DEBUG, "get_stat_value(%s)", def_2_string(def));
#endif

    switch (def->label) {
        case ST_PROC:
            res = get_nproc (def, main_db);
            break;
        case ST_FS:
            res = get_stat_fs (def, main_db->fs);
            break;
        case ST_MEM:
            res.type = INTEGER;
            res.ptr = (void *) &(main_db->mem[selector]);
            break;
        case ST_SWAP:
            res.type = INTEGER;
            res.ptr = (void *) &(main_db->swap[selector]);
            break;
        case ST_LOAD:
            res.type = DOUBLE;
            switch (def->arg[0].laminutes) {
                case 1:
                    res.ptr = (void *) &(main_db->load[0]);
                    break;
                case 5:
                    res.ptr = (void *) &(main_db->load[1]);
                    break;
                case 15:
                    res.ptr = (void *) &(main_db->load[2]);
                    break;
                default:
                    bail_out (EXIT_BUG, "get_stat_value(): invalid minutes for LA");
            }
            break;
        default:
            bail_out (EXIT_BUG, "get_stat_value(): invalid stat family");
    }
    return res;
}

const char      *procdir = "/proc";
const char      *maxtfile = "/proc/sys/kernel/threads-max";
const char      *lafile = "/proc/loadavg";
int             lafha;
const char      *memfile = "/proc/meminfo";
FILE*           memfha;
    
char            fbuf[ BUFSIZE ];

void fetch_la (sf_database *db)
{
    char        *one, *two, *three;

    lseek (lafha, 0, SEEK_SET);
    read (lafha, (char *) &fbuf, BUFSIZE);

    one   = &fbuf[0];
    two   = index (one, ' ') + 1;
    three = index (two, ' ') + 1;
    
    db->load[0] = atof (one);
    db->load[1] = atof (two);
    db->load[2] = atof (three);

#ifdef DEBUG
    syslog (LOG_DEBUG, "fetch_la(): la1=%0.2f la5=%0.2f la15=%0.2f\n",
            db->load[0], db->load[1], db->load[2]);
#endif
}

void fetch_mem (sf_database *db)
{
    char                *ptr;
    const char          *items[6] = {"MemTotal",
                                     "MemFree",    
                                     "Buffers",
                                     "Cached", 
                                     "SwapTotal", 
                                     "SwapFree"};
    unsigned int found_items = 0;
    long long int swap_total = 0;
    long long int mem_buffered = 0;
    long long int mem_cached = 0;
    long long int mem_free = 0;
    
    clearerr(memfha); // clear previous EOF
    memfha = freopen(memfile, "r", memfha); // reopen the file
    
    while (!feof(memfha) && found_items < 6) {
      /* scanning /proc/meminfo for needed values */
      unsigned int i;
      fgets(fbuf, BUFSIZE, memfha);
      for (i = 0; i < 6; i++) {
        unsigned int length = strlen(items[i]);
        ptr = fto_notspace(fbuf + length + 1);
        if (strncmp(fbuf, items[i], length) == 0) {
           switch (i) {
             case 0 :
                db->mem[VA_TOTAL] = atoll (ptr) * 1024;
                found_items ++;
                break;
             case 1 :
                mem_free = atoll (ptr) * 1024;
                found_items ++;
                break;
             case 2 :
                mem_buffered = atoll (ptr) * 1024;
                found_items ++;
                break;
             case 3 :
                mem_cached = atoll (ptr) * 1024;
                found_items ++;
                break;
             case 4 :
                swap_total = atoll (ptr) * 1024;
                found_items ++;
                break;
             case 5 :
                db->swap[VA_FREE] = atoll (ptr) * 1024;
                found_items ++;
                break;                             
           }
        }
      }    
    }
    
    if (found_items != 6) {
      syslog (LOG_ERR, "fetch_mem() incorrect nr of items found: %d",
              found_items );    
    }
   
    /* calculate real values, i.e. substract buffers and cache size */
    db->mem[VA_FREE] = mem_free + mem_buffered + mem_cached;
    db->swap[VA_USED] = swap_total - db->swap[VA_FREE];
    db->mem[VA_USED] = db->mem[VA_TOTAL] - db->mem[VA_FREE];

#ifdef DEBUG
    syslog (LOG_DEBUG, "fetch_mem(): memfree=%llu memused=%llu swapfree=%llu swapused=%llu\n",
            db->mem[VA_FREE],
            db->mem[VA_USED],
            db->swap[VA_FREE],
            db->swap[VA_USED]
            );
#endif
}

void fetch_pathspace (sf_fs_stat *fs) 
{
    struct statfs   buf;

    if (statfs (fs->path, &buf) == -1) {
        fs->val[VA_TOTAL] = 0;
        fs->val[VA_AVAIL] = 0;
        fs->val[VA_FREE]  = 0;
        fs->val[VA_USED]  = 0;
#ifdef DEBUG
        syslog (LOG_DEBUG, "fetch_pathspace(): error on statfs() on %s", fs->path);
#endif
        return;
    }

    /* block size in KB */
//    bsizeKB = buf.f_bsize / 1024;

    /* sizes are in kilobytes */    
    fs->val[VA_TOTAL] = (long long int) buf.f_blocks * (long long int) buf.f_bsize;
    fs->val[VA_AVAIL] = (long long int) buf.f_bavail * (long long int) buf.f_bsize;
    fs->val[VA_FREE]  = (long long int) buf.f_bfree * (long long int) buf.f_bsize;
    fs->val[VA_USED]  =
        ((long long int) buf.f_blocks - (long long int) buf.f_bfree) * (long long int) buf.f_bsize;
        
#ifdef DEBUG
	syslog (LOG_DEBUG,
            "fetch_pathspace(): path %s bsize=%lu btotal=%lu bfree=%lu bavail=%lu",
            fs->path,
            buf.f_bsize,
            buf.f_blocks,
            buf.f_bfree,
            buf.f_bavail
           );
    syslog (LOG_DEBUG,
            "fetch_pathspace(): path %s total=%lld used=%lld free=%lld avail=%lld",
            fs->path,
            fs->val[VA_TOTAL],
            fs->val[VA_USED],
            fs->val[VA_FREE],
            fs->val[VA_AVAIL]
            );
#endif	
}

void fetch_fs (sf_database *db)
{
    sf_list         *hd = db->fs;
    sf_fs_stat      *fs;

    // Iterate through list of watched paths
    while (hd) {
        fs = (sf_fs_stat *) hd->el;
        if (fs)
            fetch_pathspace (fs);
        hd = hd->next;
    }
}

char get_proc_state (sf_database *db, char *dir)
{
    char            fnamebuf[ PROCDIRNAMELEN + 1 ];
    int             statfha;
    char            state;
    char            *ptr = &fbuf[0];

    snprintf (&fnamebuf[0], PROCDIRNAMELEN, "%s/status", dir);
    statfha = open (&fnamebuf[0], O_RDONLY);

    /* cannot open file? process state is unknown for us */
    if (statfha < 0) return PROC_UNKNOWN;
    else {
        read (statfha, ptr, BUFSIZE);
        /* second line looks like this:
         *
         * Status:  L
         *
         * where L is letter indicating process' state
         */
        ptr = fto_notspace (fto_space (fto_newline (ptr)));
        switch (*ptr) {
            case 'R':
                state = PROC_RUN;
                break;
            case 'S':
                state = PROC_SLEEP;
                break;
            case 'Z':
                state = PROC_ZOMBIE;
                break;
            case 'T':
                state = PROC_STOP;
                break;
            case 'D':
                state = PROC_UNINT;
                break;
            default:
                state = PROC_UNKNOWN;
        }
        close (statfha);
    }
    return state;
}

void fetch_proc (sf_database *db)
{
    DIR             *dh;
    struct dirent   *de;
    struct stat     statbuf;
    long long int   pid;
    char            fnamebuf[ PROCDIRNAMELEN + 1 ];
    sf_proc_stat    res;

    /* reset db */
    db->nr_proc = 0;
    
    dh = opendir (procdir);
    if (!dh) bail_out (EXIT_IO, procdir);

    while (1) {
        de = readdir (dh);
        if (!de) break;

        /* check only dirs */
        if (de->d_type != DT_DIR) continue;

        /* try to convert file name to int */
        pid = atoll (de->d_name);
        /* zero means it's not a process' dir */
        if (!pid) continue;
        
        /* get real dir name */
        snprintf (&fnamebuf[0], PROCDIRNAMELEN, "%s/%llu", procdir, pid);
        stat (&fnamebuf[0], &statbuf);
        res.uid = statbuf.st_uid;
        res.state = get_proc_state (db, &fnamebuf[0]);

        /* add process */
        add_proc_entry_to_array (db, &res);
    }    
    
    closedir (dh);
    return;
}


void open_files ()
{
    lafha = open (lafile, O_RDONLY);
    if (lafha < 0) bail_out (EXIT_IO, lafile);
    
    memfha = fopen(memfile, "r");
    if (memfha == 0) bail_out (EXIT_IO, memfile);
}

long int get_max_threads ()
{
    int     fh = open (maxtfile, O_RDONLY);
    int     mt;

    if (fh < 0) bail_out (EXIT_IO, maxtfile);
    read (fh, &fbuf[0], BUFSIZE);

    mt = atol (&fbuf[0]);

    close (fh);

    syslog (LOG_NOTICE, "maximum number of threads in system: %d", mt);
    
    return  mt;
}    
    

/* fast text parsing helpers */

char * fto_newline (char *b)
{
    do {
        b++;
    } while ((*b != '\n') && 
             (*b != '\r') && 
             (*b != '\0'));
    /* skip to new line */
    if (*b != '\0') b++;
    return b;
}

char * fto_notspace (char *b)
{
    do {
        b++;
    } while ((*b == ' ') || 
             (*b == '\t') || 
             (*b == '\n') || 
             (*b == '\r'));
    return b;
}

char * fto_space (char *b)
{
    do {
        b++;
    } while ((*b != ' ') && 
             (*b != '\t') && 
             (*b != '\n') && 
             (*b != '\r') &&
             (*b != '\0'));
    return b;
}


/* $Id: getstats.c,v 1.26 2006/04/30 12:21:35 emes Exp $ */
