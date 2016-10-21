/*
 
  copyright (c) 2004, Michal Salaban <emes@pld-linux.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: parse.h,v 1.5 2004/06/05 12:49:41 emes Exp $
 
*/

#define MAXREP  4

typedef struct {
    tokdata     *ptr;
    void        *parsed;
} parserdata;

/*
 * Functions
 */

void * parse_error (tokdata *tok, const char *func, const char *desc);
parserdata * get_logdata (tokdata *tok);
parserdata * get_rundata (tokdata *tok);
parserdata * expression_decompose (parserdata *left);
parserdata * get_atomic_expression (tokdata *tok);
parserdata * get_expression (tokdata *tok);
parserdata * get_block_expression (tokdata *tok);
parserdata * get_ruleset (tokdata *tok);


/* $Id: parse.h,v 1.5 2004/06/05 12:49:41 emes Exp $ */
