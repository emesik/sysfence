/*
 
  copyright (c) 2004, Mirek Kopertowski <m.kopertowski@post.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
*/
#include <string.h>
#include "parseopt/lex.h"
#include "datastruct.h"
#include "cp2memory.h"

int get_data_size (sf_type label)
{
    switch (label) {
        case INTEGER:
            return sizeof (long int);
        case DOUBLE:
        case PERCENT:
            return sizeof (double);
    }
    return 0;
}

int get_list_size (sf_list *list)
{
    int     size = sizeof (sf_list);
    
    if (! list) return 0;

    size += list->elsize;
    size += get_list_size (list->next);

    return size;
}    

int get_atomic_size (sf_atomic *atomic)
{
    int         size = sizeof (sf_atomic);
    
    switch (atomic->stat.label) {
        case ST_FS:
            // arg[1] is path
            size += strlen (atomic->stat.arg[1].path);
            break;
        case ST_PROC:
            // arg[0] is UID list
            size += get_list_size (atomic->stat.arg[0].uids);
            break;
        default:
            break;
            
    }
    return size;
}

int get_expression_size (sf_expression *expr)
{
    int         size = sizeof (sf_expression);

    switch (expr->type) {
        case ATOMIC:
            size += get_atomic_size ((sf_atomic *)expr->arg1);
            break;
        case RELATION:
            size += get_expression_size ((sf_expression *)expr->arg1);
            size += get_expression_size ((sf_expression *)expr->arg2);
            break;
    } 
    return size;
}

int get_rundata_size (sf_rundata *run)
{
    int     size = sizeof (sf_rundata);
    
    if (run->command) 
        size += (int)strlen (run->command) + 1;
   
    return size;
}


int get_logdata_size (sf_logdata *log)
{    
    return sizeof (sf_logdata);
}

int get_rule_size (sf_rule *rule) 
{
    int size = sizeof (sf_rule);
    
    if (rule->name)
        size += (int)strlen (rule->name) + 1;
    
    /* expression size */
    size += get_expression_size (rule->expr);

    /* rundata size */
    if (rule->run)
        size += get_rundata_size (rule->run);

    /* logdata size*/
    if (rule->log)
        size += get_logdata_size (rule->log);
    
    return size;
}

void *cp_list (void **buf, sf_list *list)
{
    sf_list     *l;

    if (! list) return NULL;
    
    /* copy list node */
    l = (sf_list *) *buf;
    memcpy (*buf, (void *) list, sizeof (sf_list));
    *buf += sizeof (sf_list);

    /* copy element linked to node */
    l->el = *buf;
    memcpy (*buf, list->el, list->elsize);
    *buf += list->elsize;

    /* copy list tail */
    l->next = (sf_list *) cp_list (buf, list->next);

    return (void *) l;
}

void *cp_atomic (void **buf, sf_atomic *atomic)
{
    sf_atomic   *a;
    int         ds;
    
    a = (sf_atomic *) *buf;
    memcpy (*buf, (void *) atomic, sizeof (sf_atomic));
    *buf += sizeof (sf_atomic);

    switch (atomic->stat.label) {
        // Copy variable-size data from sf_stat_def
        case ST_FS:
            // arg[1] is pathname
            a->stat.arg[1].path = (char *) *buf;
            strcpy (*buf, atomic->stat.arg[1].path);
            *buf += strlen (atomic->stat.arg[1].path) + 1;
            break;
        case ST_PROC:
            // arg[0] is UID list
            a->stat.arg[0].uids = (sf_list *) cp_list (buf, a->stat.arg[0].uids);
            break;
        default:
            break;
    }

    /* copy threshold */
    ds = get_data_size (atomic->thresh.type);
    memcpy (*buf, (void *) atomic->thresh.ptr, ds);
    buf += ds;

    return a;
}


void *cp_expression (void **buf, sf_expression *expr)
{
    void            *b1, *b2;
    sf_atomic       *a = (sf_atomic *) expr->arg1;
    sf_expression   *e; 

    switch (expr->type) {
        case ATOMIC:
            /* copy atomic */
            a = (sf_atomic *) cp_atomic (buf, a);

            /* copy expression itself */
            memcpy (*buf, (void *)expr, sizeof(sf_expression));
            e = (sf_expression *) *buf;
            e->arg1 = (void *) a;
            *buf += sizeof (sf_expression); 

            return (void *)e ; 
        case RELATION:
            /* copy child expressions */
            b1 = cp_expression (buf, (sf_expression *)expr->arg1);
            b2 = cp_expression (buf, (sf_expression *)expr->arg2);

            /* copy expression itself */
            memcpy (*buf, (void *)expr, sizeof (sf_expression));
            e = (sf_expression *) *buf;
            e->arg1 = b1;
            e->arg2 = b2;
            *buf += sizeof (sf_expression);

            return (void *)e;
        default:
            return 0;
    } 
}

void *cp_rundata (void **buf, sf_rundata *run)
{
    sf_rundata *r;

    /* copy rundata */
    memcpy (*buf, (void *)run, sizeof (sf_rundata));
    r = (sf_rundata *) *buf;
    *buf += sizeof (sf_rundata);
    
    /* copy command */
    if (run->command) {
        strcpy (*buf, run->command);
        r->command = (char *) *buf;
        *buf += (int)strlen (run->command) + 1;
    }
    
    return (void *)r;
}


void *cp_logdata (void **buf, sf_logdata *log)
{
    void *b;

    /* copy logdata */
    memcpy (*buf, (void *)log, sizeof (sf_logdata));
    b = *buf;
    *buf += sizeof (sf_logdata);
    
    return b;
}

void *cp_rule (void **buf, sf_rule *rule) 
{
    sf_rule     *r;

    /* copy rule */
    memcpy (*buf, (void *)rule, sizeof (sf_rule));
    r = (sf_rule *) *buf;
    *buf += sizeof (sf_rule);

    /* copy name */
    if (rule->name) {
        strcpy (*buf, rule->name);
        r->name = (char *) *buf;
        *buf += (int)strlen (rule->name) + 1;
    }

    /* copy expression */
    r->expr = (sf_expression *) cp_expression (buf, rule->expr);

    /* copy rundata */
    if (r->run)
        r->run = (sf_rundata *) cp_rundata (buf, rule->run);

    /* copy logdata */
    if (r->log)
        r->log = (sf_logdata *) cp_logdata (buf, rule->log);

    /* copy watched stats list */
    if (r->watchstats)
        r->watchstats = (sf_list *) cp_list (buf, rule->watchstats);
    
    return (void *)r;
}

/*
   $Id: cp2memory.c,v 1.4 2004/06/04 12:13:47 emes Exp $
*/
