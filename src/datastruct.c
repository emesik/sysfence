/*
 
  copyright (c) 2004, Michal Salaban <emes@pld-linux.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: datastruct.c,v 1.14 2004/06/16 09:23:08 emes Exp $
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys/xalloc.h"
#include "sys/users.h"
#include "parseopt/lex.h"
#include "datastruct.h"
#ifdef DEBUG
#include <syslog.h>
#endif

void add_proc_entry_to_array (sf_database *db, sf_proc_stat *proc)
{
    sf_proc_stat    *arr = db->proc;

#ifdef DEBUG
    syslog (LOG_DEBUG,
            "add_proc_entry_to_array: uid=%d, state=%d",
            proc->uid,
            proc->state);
#endif
    
    // go to end of array
    arr += db->nr_proc;
    memcpy (arr, proc, sizeof (sf_proc_stat));
    db->nr_proc ++;
}

int get_proc_num (sf_database *db, char statemask, uid_t *uid)
{
    /* returns number of processes selecting by uid and state mask */

    int             count = 0;
    sf_proc_stat    *cur = db->proc;

    for (cur = db->proc;
         cur - db->proc < db->nr_proc;
         cur ++) {
        
        if (uid)
            if (cur->uid != *uid) continue;

        if (! (cur->state & statemask)) continue;

        count ++;
    }

    return count;
}

void add_def_to_list (sf_list **hd, sf_stat_def *def)
{
    /* adds stat def to list, discarding duplicates */

#ifdef DEBUG
    syslog (LOG_DEBUG, "adding def: %s @ %x", def_2_string (def), *hd);
#endif
    
    if (! *hd) {
        /* this is tail */
        sf_list     *new = (sf_list *) xalloc (NULL, sizeof (sf_list));

        new->elsize = sizeof (sf_stat_def);
        new->el = (void *) def;
        new->next = NULL;
        *hd = new;
    } else {
        /* this is existing entry */
        
        sf_stat_def *cur = (sf_stat_def *) (*hd)->el;

        /* if we're carrying a duplicate, drop it */
        if (equal_defs (def, cur)) return;
        else add_def_to_list (&(*hd)->next, def);
    }
}

int uid_in_list (sf_list *hd, uid_t uid)
{
    /* checks if uid is in the list */
    if (!hd) return 0;

    if (*((uid_t *)hd->el) != uid) return 0;
    
    return uid_in_list (hd->next, uid);
}

int equal_uid_lists (sf_list *l1, sf_list *l2)
{
    /*
     *  This function works in 2 * |l1| * |l2| time
     *  but is used during parsing process and for short lists (usually)
     *  so, there is no need to code it better. am i right? ;>
     */

    sf_list     *lp1 = l1, *lp2 = l2;

    while (lp1) {
        if (! uid_in_list (l2, *((uid_t *)lp1->el))) return 0;
        lp1 = lp1->next;
    }

    while (lp2) {
        if (! uid_in_list (l1, *((uid_t *)lp2->el))) return 0;
        lp2 = lp2->next;
    }

    return 1;
}

void add_fs_entry_to_list (sf_list **hd, char *path)
{
    /* adds new fs entry to list of paths being watched
     * used to register new paths while parsing config file
     */
    sf_list         *l = *hd;
    sf_fs_stat      *e;

#ifdef DEBUG
    if (! l)
        syslog (LOG_DEBUG, "add_fs_entry_to_list: %s", path);
#endif

    if (! l) {
        /* end-of-list reached, add entry */
        e = (sf_fs_stat *) xalloc (NULL, sizeof (sf_fs_stat));
        e->path = (char *) xalloc (NULL, strlen (path) + 1);
        strcpy (e->path, path);
        
        l = (sf_list *) xalloc (NULL, sizeof (sf_list));
        l->next = NULL;
        l->elsize = sizeof (sf_fs_stat);
        l->el = (void *) e;
        *hd = l;
        return;
    }

    e = (sf_fs_stat *) l->el;

    /* do not insert duplicates */
    if (strcmp (e->path, path) == 0) return;

    add_fs_entry_to_list (&(l->next), path);
}

sf_fs_stat * get_fs_entry_from_list (sf_list *hd, char *path)
{
    /* finds an entry in list of paths being watched */
    sf_fs_stat     *st;
   
    // not found
    if (! hd) return NULL;

    st = (sf_fs_stat *) hd->el;
    if (strcmp (st->path, path) == 0) return st;
    else return get_fs_entry_from_list (hd->next, path);
}    

int equal_defs (sf_stat_def *d1, sf_stat_def *d2)
{
    /* checks if two defs describe the same stat */

#ifdef DEBUG
    syslog (LOG_DEBUG,
            "equal_defs [%s] == [%s]",
            def_2_string (d1),
            def_2_string (d2)
           );
#endif

    if (d1->label != d2->label) return 0;
    switch (d1->label) {
        case ST_LOAD:
            return (d1->arg[0].laminutes == d2->arg[0].laminutes);
        case ST_MEM:
        case ST_SWAP:
            return (d1->arg[0].resstat == d2->arg[0].resstat);
        case ST_FS:
            return ((d1->arg[0].resstat == d2->arg[0].resstat)
                    &&
                    (strcmp (d1->arg[1].path, d2->arg[1].path) == 0));
        case ST_PROC:
            return ((d1->arg[1].procstates == d2->arg[1].procstates)
                    &&
                    equal_uid_lists (d1->arg[0].uids, d2->arg[0].uids));
        default:
            return 1;
    }
}

char * res_state_2_string (sf_res_state sta)
{
    switch (sta) {
        case VA_USED:
            return "used";
        case VA_FREE:
            return "free";
        case VA_AVAIL:
            return "available";
        default:
            return "";
    }
}

char * def_2_string (sf_stat_def *def)
{
    char    *buf = (char *) xalloc (NULL, STRBUF + 1);
    char    *buf2;

    *buf = '\0';

    switch (def->label) {
        case ST_LOAD:
            snprintf (buf,
                      STRBUF,
                      "loadavg(%d)",
                      def->arg[0].laminutes
                      );
            break;
        case ST_MEM:
            snprintf (buf,
                      STRBUF,
                      "%s memory",
                      res_state_2_string (def->arg[0].resstat)
                      );
            break;
        case ST_SWAP:
            snprintf (buf,
                      STRBUF,
                      "%s swap",
                      res_state_2_string (def->arg[0].resstat)
                      );
            break;
        case ST_FS:
            snprintf (buf,
                      STRBUF,
                      "%s space in %s",
                      res_state_2_string (def->arg[0].resstat),
                      def->arg[1].path
                      );
            break;
        case ST_PROC:
            buf2 = proc_states_2_str (def->arg[1].procstates);
            if (buf2) {
                snprintf (buf,
                          STRBUF,
                          "nproc %sin state(s) %s",
                          uids_2_str (def->arg[0].uids),
                          buf2
                          );
                free (buf2);
            } else {
                snprintf (buf,
                          STRBUF,
                          "nproc %s",
                          uids_2_str (def->arg[0].uids)
                        );
            }
        default:
            break;      // to avoid warnings
    }
    
    return buf;
}

char * uids_2_str (sf_list *hd)
{
    uid_t   uid;
    char    *buf = (char *) xalloc (NULL, STRBUF + 1),
            *buf2,
            *uname;
   
    *buf = '\0';
    if (hd) {
        uid = *((uid_t *) (hd->el));
        buf2 = uids_2_str (hd->next);
        
        /* try to get username */
        uname = uid2username (uid);
        if (!uname) {
            /* no username? print UID */
            uname = (char *) xalloc (NULL, STRBUF + 1);
            snprintf (uname, STRBUF, "%d", uid);
        }
        
        if (*buf2)
            snprintf (buf, STRBUF, "%s,%s", uname, buf2);
        else
            snprintf (buf, STRBUF, "%s ", uname);
        free (buf2);
    }
    return buf;
}

char * proc_states_2_str (int procmask)
{
    char    *buf = (char *) xalloc (NULL, STRBUF + 1);
    int     pos = 0;
    char    state[_PR_UNKNOWN] = {'R', 'S', 'T', 'Z', 'D'};
    int     sta = 0;
    
    if (procmask == PROC_ANY) return NULL;
    while (sta < _PR_UNKNOWN) {
        if (procmask & (1 << sta)) {
            *(buf + pos) = state[sta];
            pos ++;
        }
        sta ++;
    }
    *(buf + pos) = '\0';
    return buf;
}
    

int get_min_step (sf_rule **rule)
{
    sf_rule     *r;
    int         next;
    
    if (! *rule) return 10;     // 10 is default

    r = *rule;
    next = get_min_step (rule + 1);
    if (r->step < next) return r->step;
    else return next;    
}

