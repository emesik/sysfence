/*
 
  copyright (c) 2004, Michal Salaban <emes@pld-linux.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: conditions.c,v 1.8 2004/05/31 14:10:06 emes Exp $
 
*/
#include <stdio.h>
#include "sys/exit.h"
#include "parseopt/lex.h"
#include "datastruct.h"
#include "getstats.h"
#include "conditions.h"
#ifdef DEBUG
#include <syslog.h>
#endif

int ordcmp (int le, int eq, int gt, token op)
{
    switch (op) {
        case CO_LT:
            return le;
        case CO_LE:
            return (le || eq);
        case CO_GT:
            return gt;
        case CO_GE:
            return (gt || eq);
        case CO_EQ:
            return eq;
        case CO_NEQ:
            return ! eq;
        default:
            return 0;
    }
}

int logcmp (int a, token op, int b)
{
    switch (op) {
        case LO_OR:
            return (a || b);
        case LO_AND:
            return (a && b);
        default:
            return 0;
    }
}

int check_atomic (sf_atomic *at)
{
    sf_value        val = get_stat_value (&(at->stat));
    double          dv = *((double *) val.ptr),
                    dt = *((double *) at->thresh.ptr);
    long long int   iv = *((long long int *) val.ptr),
                    it = *((long long int *) at->thresh.ptr);

    if (at->thresh.type != val.type) bail_out (EXIT_BUG, "check_atomic(): type mismatch!");
    
    switch (at->thresh.type) {
        case DOUBLE:
#ifdef DEBUG
            syslog (LOG_DEBUG,
                    "cmp val=%f vs thr=%f: <:%d =:%d >:%d",
                    dv, dt,
                    (dv <  dt),
                    (dv == dt),
                    (dv >  dt)
                    );
#endif
            return ordcmp (
                    (dv <  dt),
                    (dv == dt),
                    (dv >  dt),
                    at->op
                    );
        case INTEGER:
#ifdef DEBUG
            syslog (LOG_DEBUG,
                    "cmp val=%llu vs thr=%llu: <:%d =:%d >:%d",
                    iv, it,
                    (iv <  it),
                    (iv == it),
                    (iv >  it)
                    );
#endif
            return ordcmp (
                    (iv <  it),
                    (iv == it),
                    (iv >  it),
                    at->op
                    );
        default:
            bail_out (EXIT_BUG, " check_atomic(): used type that is not implemented yet");
            return 0;       // to avoid warnings
    }
}

int check_expression (sf_expression *ex)
{
    switch (ex->type) {
        case ATOMIC:
            return check_atomic ((sf_atomic *) ex->arg1);
        case RELATION:
            return logcmp (
                    check_expression ((sf_expression *) ex->arg1),
                    ex->op,
                    check_expression ((sf_expression *) ex->arg2)
                    );
        default:
            return 0;
    }
}

/* $Id: conditions.c,v 1.8 2004/05/31 14:10:06 emes Exp $ */
