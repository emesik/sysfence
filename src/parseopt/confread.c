/*
 
  copyright (c) 2004, Michal Salaban <michal@salaban.info>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: confread.c,v 1.7 2004/06/05 12:49:41 emes Exp $
 
*/

#include "../sys/exit.h"
#include "../sys/xalloc.h"
#include "lex.h"
#include "../datastruct.h"
#include "../getstats.h"
#include "../conditions.h"
#include "parse.h"
#include "confread.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>

#define RULESETS    64

char * readfile (char *fname)
{
    char        *buf;
    int         fha = open (fname, O_RDONLY);
    off_t       flen;

    if (errno != 0) bail_out (EXIT_IO, fname);

    flen = lseek (fha, 0, SEEK_END);
    lseek (fha, 0, SEEK_SET);
#ifdef DEBUG
    syslog (LOG_DEBUG, "%s: %d bytes to read\n", fname, flen);
#endif

    buf = (char *) xalloc (NULL, flen + 1);
    read (fha, buf, flen);
    if (errno != 0) bail_out (EXIT_IO, fname);
    *(buf + flen) = '\0';

    return buf;
}


tokdata * tokenized_conf (char *fname)
{
    tokdata     tok,
                *res;
    int         i = 0,
                tbufsize = TOKBUFFER;
    char        *txtbuf = readfile (fname);
    
    /* allocate buffer for tokens */
    res = (tokdata *) xalloc (NULL, tbufsize * sizeof (tokdata));
    
    lexptr = txtbuf;
    do {
        tok = next_token();
        *(res + i) = tok;
        i++;
        if (i > TOKBUFFER) {
            /* we're running out of buffer space?
             * let's extend it by next TOKBUFFER entries
             */
            tbufsize += TOKBUFFER;
            res = (tokdata *) xalloc (
                                (void *) res,
                                tbufsize * sizeof(tokdata)
                                );
        }
    } while (tok.type != END);

    /* we don't need text config anymore */
    free (txtbuf);
    return res;
}

sf_rule ** read_config_files (int argc, char *argv[])
{
    int         i = 0,
                setsize = RULESETS,
                setpos = 0;
    sf_rule     **set = (sf_rule **) 
                        xalloc (NULL, setsize * sizeof (sf_rule *));
    parserdata  *rule;
    tokdata     *stok, *tok;

    while (i < argc) {
        /* for every file */
        stok = tokenized_conf (argv[i]);
        tok = stok;
        while (1) {
            /* get rulesets */
            rule = get_ruleset (tok);
            if (!rule) break;
            tok = rule->ptr;
            *(set + setpos) = (sf_rule *) rule->parsed;
            setpos ++;
            if (setpos == setsize - 1) {
                /* if we're running out of buffer, resize it */
                setsize += RULESETS;
                set = (sf_rule **) xalloc (set, setsize * sizeof (sf_rule *));
            }
            
        }
        i ++;
        free (stok);
    }
    /* put NULL to terminate buffer */
    *(set + setpos) = NULL;
    return set;
}


/* $Id: confread.c,v 1.7 2004/06/05 12:49:41 emes Exp $ */
