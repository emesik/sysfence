/*
 
  copyright (c) 2004, Michal Salaban <emes@pld-linux.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: parse.c,v 1.20 2004/06/09 10:45:57 emes Exp $
 
*/

#include <stdio.h>
#include <stdlib.h>
#include "lex.h"
#include "../sys/exit.h"
#include "../sys/xalloc.h"
#include "../datastruct.h"
#include "../getstats.h"
#include "../conditions.h"
#include "parse.h"
#include "../sys/users.h"
#include <syslog.h>

#define ERRMSGBUF 128

void * parse_error (tokdata *tok, const char *func, const char *desc)
{
    char    *buf = (char *) xalloc (NULL, ERRMSGBUF);
    
    snprintf (
            buf,
            ERRMSGBUF - 1,
            "at line %d before token %s, reported by function %s [%s]",
            tok->line,
            tok2string (tok->type),
            func,
            desc
            );
    
    bail_out (EXIT_PARSE, buf);
}

void collect_stats (sf_expression *ex, sf_list **statlist)
{
    sf_atomic   *atmp = (sf_atomic *) ex->arg1;
    
#ifdef DEBUG    
    syslog (LOG_DEBUG, "collect_stats (%x, %x)", ex, statlist);
#endif

    if (ex->type == RELATION) {
        collect_stats ((sf_expression *) ex->arg1, statlist);
        collect_stats ((sf_expression *) ex->arg2, statlist);
    } else add_def_to_list (statlist, &atmp->stat);
}

parserdata * get_logdata (tokdata *tok)
{
    /*  grammar:
     *  
     *  logdata := log [once]
     *
     */
    sf_logdata      *log;
    parserdata      *res = (parserdata *) xalloc (NULL, sizeof (parserdata));

    if (tok->type != KW_LOG) return NULL;
    tok ++;
    
    log = (sf_logdata *) xalloc (NULL, sizeof (sf_logdata));
    res->parsed = (void *) log;
    if (tok->type == KW_ONCE) {
        log->once = 1;
        tok ++;
    } else log->once = 0;

    res->ptr = tok;
    return res;
}

parserdata * get_rundata (tokdata *tok)
{
    /*  grammar:
     *
     *  rundata := run [once] "command"
     *
     */
    const char  *thisfuncname = "get_rundata()";
    
    parserdata  *res;
    sf_rundata  *run;   /* helper */

    if (tok->type != KW_RUN) return NULL;

    tok ++;

    /* that's run data. initialize structures */
    res = (parserdata *) xalloc (NULL, sizeof (parserdata));
    res->parsed = xalloc (NULL, sizeof (sf_rundata));
    run = (sf_rundata *) res->parsed;
    if (tok->type == KW_ONCE) {
        run->once = 1;
        tok ++;
    } else run->once = 0;

    if (tok->type != VA_STR) 
        parse_error (tok, thisfuncname, "run arg not string");
    run->command = tok->val;
    tok ++;

    res->ptr = tok;
    return res;
}

parserdata * get_step (tokdata *tok)
{
    /* grammar:
     *
     * stepdata := step <integer>
     *
     */
    const char  *thisfuncname = "get_step()";

    parserdata      *res = (parserdata *) xalloc (NULL, sizeof (parserdata));
    
    res->parsed = xalloc (NULL, sizeof (unsigned int));
    res->ptr = tok;

    if (tok->type != KW_STEP) {
        /* no stepping data, assuming default */
        *((unsigned int *) res->parsed) = DEFAULTSTEP;
        return res;
    }
    
    tok ++;

    if (tok->type != VA_INT)
        parse_error (tok, thisfuncname, "step val not int");
    
    res->parsed = tok->val;
    tok ++;
    res->ptr = tok;

    if (*((unsigned int *) res->parsed) < 1)
        bail_out (EXIT_VALUE, "stepping value must be positive");
    
    return res;
}
    

parserdata * expression_decompose (parserdata *left)
{
    const char  *thisfuncname = "expression_decompose()";
    
    sf_expression   *resexp;
    parserdata      *right, *res;
    tokdata         *tok = left->ptr;
    
    if ((tok->type == LO_OR) || (tok->type == LO_AND)) {
        /* this is compound expression */
        res = (parserdata *) xalloc (NULL, sizeof (parserdata));
        resexp = (sf_expression *) xalloc (NULL, sizeof (sf_expression));
        res->parsed = (void *) resexp;
        resexp->type = RELATION;
        /* set the expression we found before as first argument */
        resexp->arg1 = left->parsed;
        /* set operator */
        resexp->op = tok->type;
        tok ++;
        /* get next expression as second argument.
         * there must be one (we had an operator).
         */
        right = get_expression (tok);
        if (right == NULL)
            parse_error (tok, thisfuncname, "no right arg");
        /* second argument is ok */
        resexp->arg2 = right->parsed;
        res->ptr = right->ptr;

        return res;
        
    } else return left; /* we had no operator */
}

parserdata * get_expression (tokdata *tok)
{
    /*  grammar:
     *
     *  expression := <atomic> [<op> <expression>]
     *             | <block_expression> [<op> <expression>]
     */
    const char  *thisfuncname = "get_expression()";
    
    parserdata      *left;

    left = get_block_expression (tok);
    if (left != NULL) /* this is a block */
        return expression_decompose (left);
    
    left = get_atomic_expression (tok);
    if (left != NULL) /* this is an atomic rule */
        return expression_decompose (left);        
    
    parse_error (tok, thisfuncname, "neither block nor atomic");
}

sf_list * get_uid_list_rec (tokdata **tok)
{
    sf_list     *res;
    uid_t       *tmpuid;
    char        *thisfuncname = "get_uid_list_rec()";

    switch ((*tok)->type) {
        case VA_INT:
            tmpuid = (uid_t *) xalloc (NULL, sizeof (uid_t));
            tmpuid = (uid_t *) (*tok)->val;
            break;
        case VA_STR:
            tmpuid = username2uid ((char *) (*tok)->val);
            if (!tmpuid) {
                parse_error (*tok, thisfuncname, "invalid username");
            }
            break;
        default:
            return NULL;
    }

    res = (sf_list *) xalloc (NULL, sizeof (sf_list));
    res->elsize = sizeof (uid_t);
    res->el = (void *) tmpuid;
    *tok = *tok + 1;
    
    if ((*tok)->type == LI_SEP) {
        /* separator?
         * go, fetch next uid
         */
        *tok = *tok + 1;
        res->next = get_uid_list_rec (tok);
    } else res->next = NULL;
    return res;
}

parserdata * get_uid_list (tokdata *tok)
{
    parserdata  *res = (parserdata *) xalloc (NULL, sizeof (parserdata));
    sf_list     *list = get_uid_list_rec (&tok);

    res->ptr = tok - 1;
    res->parsed = (void *) list;
    return res;
}

char get_states_rec (tokdata **tok, int first)
{
    char    thisstate = 0;
    char    *thisfuncname = "get_states_rec()";
    
    switch ((*tok)->type) {
        case PR_RUN:
            thisstate = PROC_RUN;
            break;
        case PR_STOP:
            thisstate = PROC_STOP;
            break;
        case PR_SLEEP:
            thisstate = PROC_SLEEP;
            break;
        case PR_UNINT:
            thisstate = PROC_UNINT;
            break;
        case PR_ZOMBIE:
            thisstate = PROC_ZOMBIE;
            break;
        default:
            /* we have already read some state descriptor(s)
             * and separator. so we must have another state
             * description here */
            if (!first) parse_error (*tok, thisfuncname, "expecting stste descriptor");
            else return 0;
    }

    *tok = *tok + 1;
    
    if ((*tok)->type == LI_SEP) {
        /* separator?
         * go, fetch next state
         */
        *tok = *tok + 1;
        thisstate |= get_states_rec (tok, 0);
    }

    return thisstate;
}

parserdata * get_states (tokdata *tok)
{
    parserdata  *res = (parserdata *) xalloc (NULL, sizeof (parserdata));
    char        *states = (char *) xalloc (NULL, sizeof (char));
    
    *states = get_states_rec (&tok, 1);

    if (*states == 0) *states = PROC_ANY;
    res->ptr = tok;
    res->parsed = (void *) states;
    return res;
}

parserdata * get_atomic (tokdata *tok)
{
    const char  *thisfuncname = "get_atomic()";
    sf_atomic   *at = (sf_atomic *) xalloc (NULL, sizeof (sf_atomic));
    tokdata     *var = tok,
                *arg1 = NULL,
                *arg2 = NULL,
                *op,
                *val;
    sf_list     *uids = NULL;
    char        states = PROC_ANY;
    parserdata  *res, *subpar;
    double      *tmp;
    int         skiptok = 3;
                

    switch (var->type) {
        case ID_LA1:
        case ID_LA5:
        case ID_LA15:
        case ID_MUSED:
        case ID_MFREE:
        case ID_SUSED:
        case ID_SFREE:
            op = tok + 1;
            val = tok + 2;
            break;
        case ID_FSFREE:
        case ID_FSAVAIL:
        case ID_FSUSED:
            arg1 = tok + 1;
            op = tok + 2;
            val = tok + 3;
            skiptok ++;
            break;
        case ID_NPROC:
            /* try to get uids list */
            subpar = get_uid_list (tok + 1);
            if (subpar->parsed) uids = (sf_list *) subpar->parsed;
            tok = subpar->ptr;

            /* try to get states */
            subpar = get_states (tok + 1);
            if (subpar) {
                states = *((char *) subpar->parsed);
                tok = subpar->ptr;
            }
            
            skiptok = 2;
            op = tok;
            val = tok + 1;
            break;
        default:
            op = NULL;
    }
    
    tok += skiptok;
    
    switch (var->type) {
        case ID_LA1:
            at->thresh.type = DOUBLE;
            at->op = op->type;
            at->stat.label = ST_LOAD;
            at->stat.arg[0].laminutes = 1;
            break;
        case ID_LA5:
            at->thresh.type = DOUBLE;
            at->op = op->type;
            at->stat.label = ST_LOAD;
            at->stat.arg[0].laminutes = 5;
            break;
        case ID_LA15:
            at->thresh.type = DOUBLE;
            at->op = op->type;
            at->stat.label = ST_LOAD;
            at->stat.arg[0].laminutes = 15;
            break;
        case ID_MFREE:
            at->thresh.type = INTEGER;
            at->op = op->type;
            at->stat.label = ST_MEM;
            at->stat.arg[0].resstat = VA_FREE;
            break;
        case ID_MUSED:
            at->thresh.type = INTEGER;
            at->op = op->type;
            at->stat.label = ST_MEM;
            at->stat.arg[0].resstat = VA_USED;
            break;
        case ID_SFREE:
            at->thresh.type = INTEGER;
            at->op = op->type;
            at->stat.label = ST_SWAP;
            at->stat.arg[0].resstat = VA_FREE;
            break;
        case ID_SUSED:
            at->thresh.type = INTEGER;
            at->op = op->type;
            at->stat.label = ST_SWAP;
            at->stat.arg[0].resstat = VA_USED;
            break;
        case ID_FSFREE:
            at->thresh.type = INTEGER;
            at->op = op->type;
            at->stat.label = ST_FS;
            at->stat.arg[0].resstat = VA_FREE;
            at->stat.arg[1].path = (char *) arg1->val;
            break;
        case ID_FSAVAIL:
            at->thresh.type = INTEGER;
            at->op = op->type;
            at->stat.label = ST_FS;
            at->stat.arg[0].resstat = VA_AVAIL;
            at->stat.arg[1].path = (char *) arg1->val;
            break;
        case ID_FSUSED:
            at->thresh.type = INTEGER;
            at->op = op->type;
            at->stat.label = ST_FS;
            at->stat.arg[0].resstat = VA_USED;
            at->stat.arg[1].path = (char *) arg1->val;
            break;
        case ID_NPROC:
            at->thresh.type = INTEGER;
            at->op = op->type;
            at->stat.label = ST_PROC;
            at->stat.arg[0].uids = uids;
#ifdef DEBUG
            syslog (LOG_DEBUG, "%s", uids_2_str (uids));
#endif
            at->stat.arg[1].procstates = states;
            break;
        default:
            return NULL;
    }

    /* check arguments */
    switch (var->type) {
        case ID_FSFREE:
        case ID_FSAVAIL:
        case ID_FSUSED:
            if (arg1->type != VA_STR) {
                parse_error (tok, thisfuncname, "no path given");
            }
            /* register fs in database */
            add_fs_entry_to_list (&tmp_fs_db, (char *) arg1->val);
            break;
        default:
            break;  // to avoid warnings
    }
    
    switch (op->type) {
        /* allow only operators */
        case CO_EQ:
        case CO_NEQ:
        case CO_LE:
        case CO_LT:
        case CO_GE:
        case CO_GT:
            break;
        default:
            parse_error (tok, thisfuncname, "invalid operator");
    }

    /* check value type */
    if (val->type == VA_INT) {
        /* this is integer. convert it to type required by label */
        if (at->thresh.type == INTEGER) val->type = VA_SIZ;
        else if (at->thresh.type == DOUBLE) {
            /* long int 2 double */
            val->type = VA_DBL;
            tmp = xalloc (NULL, sizeof (double));
            *tmp = (double) *((long long int *) val->val);
            free (val->val);
            val->val = tmp;
        }
    }
        
    if (((at->thresh.type == INTEGER) && (val->type != VA_SIZ)) ||
         ((at->thresh.type == DOUBLE) && (val->type != VA_DBL)))
        parse_error (val, thisfuncname, "type mismatch");

    /* copy threshold value */
    at->thresh.ptr = val->val;

    res = (parserdata *) xalloc (NULL, sizeof (parserdata));
    res->ptr = tok;
    res->parsed = (void *) at;
    return res;
}

parserdata * get_atomic_expression (tokdata *tok)
{
    /* get atomic and encapsulate into expression with type = ATOMIC */
    parserdata      *at = get_atomic (tok);
    sf_expression   *ex;

    if (at == NULL) return NULL;
    ex = (sf_expression *) xalloc (NULL, sizeof (sf_expression));
    /* we'll use the same parserdata structure */

    ex->type = ATOMIC;
    ex->arg1 = at->parsed;
    at->parsed = ex;
    return at;
}

parserdata * get_block_expression (tokdata *tok)
{
    /*  grammar:
     * 
     *  block_expression := '{' <expression> '}'
     *
     */

    const char  *thisfuncname = "get_block_expression()";
    
    parserdata      *expr;

    /* simply pass the expression enclosed in block.
     * check if block is well formated.
     */
    if (tok->type != BL_BEGIN) return NULL;
    tok ++;
    expr = get_expression (tok);
    if (expr == NULL) return NULL;
    if (expr->ptr->type != BL_END)
        parse_error (tok, thisfuncname, "block not closed");

    expr->ptr ++;
    return expr;
}

parserdata * get_ruleset (tokdata *tok)
{
    /*  grammar:
     *
     *  ruleset := 'if' [<string>] <block_expression> <rundata> [<logdata>]
     *          |  'if' [<string>] <block_expression> <logdata> [<rundata>]
     */

    const char  *thisfuncname = "get_ruleset()";
    int         i;

    /* our result data */
    parserdata  *res = (parserdata *) xalloc (NULL, sizeof (parserdata)),
                *tmp, *run, *log;
    sf_rule     *rlst;  /* helper */

    /* the only way to return w/o error is to find end-of-file */
    if (tok->type == END) return NULL;
    
    /* create ruleset structure */
    res->parsed = (sf_rule *) xalloc (NULL, sizeof (sf_rule));
    rlst = (sf_rule *) res->parsed;
    /* reset fields */
    rlst->name = NULL;
    rlst->expr = NULL;
    rlst->run = NULL;
    rlst->log = NULL;
    rlst->hit = 0;
    rlst->prevhit = 0;
    rlst->watchstats = NULL;
    
    /* ruleset begins with IF */
    if (tok->type != KW_IF)
        parse_error (tok, thisfuncname, "no if keyword");
    else tok ++;

    /* then we have rule name */
    if (tok->type == VA_STR) {
        rlst->name = tok->val;
    } else {
        parse_error (tok, thisfuncname, "no rule name");
    }
    tok ++;

    /* next should be expression in block */
    tmp = get_block_expression (tok);
    if (tmp == NULL)
        parse_error (tok, thisfuncname, "no expr block");

    /* save expression and skip to next token */
    rlst->expr = (sf_expression *) tmp->parsed;
    tok = tmp->ptr;

    run = get_rundata (tok);
    log = get_logdata (tok);
    /* no log or run data? */
    if ((run == NULL) && (log == NULL))
        parse_error (tok, thisfuncname, "no action defined");

    /* this is full ruleset now but we try to get next action data */
    if (run != NULL) {
        /* save rundata && get logdata */
        rlst->run = (sf_rundata *) run->parsed;
        log = get_logdata (run->ptr);

        /* if we have log sequence, save it.
         * skip to next token
         */
        if (log != NULL) {
            tok = log->ptr;
            rlst->log = (sf_logdata *) log->parsed;
        } else tok = run->ptr;
    } else {
        rlst->log = (sf_logdata *) log->parsed;
        /* save logdata && get rundata */
        run = get_rundata (log->ptr);

        if (run != NULL) {
            tok = run->ptr;
            rlst->run = (sf_rundata *) run->parsed;
        } else tok = log->ptr;
    }

    /* get step length */
    tmp = get_step (tok);
    tok = tmp->ptr;
    rlst->step = *((unsigned int *) tmp->parsed);
    
    /* search expression tree for used stats */
    collect_stats (rlst->expr, &(rlst->watchstats));

    /* ok, now we got full ruleset structure.
     * we need only to skip to next token.
     */
    
    res->ptr = tok;
    return res;
}

/* $Id: parse.c,v 1.20 2004/06/09 10:45:57 emes Exp $ */
