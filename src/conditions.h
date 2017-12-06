/*
 
  copyright (c) 2004, Michal Salaban <michal@salaban.info>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: conditions.h,v 1.9 2004/05/23 20:52:22 emes Exp $
 
*/
#include <stdio.h>

#define DEFAULTSTEP     10

/*
 *  Functions
 */

int check_atomic (sf_atomic *at);
int check_expression (sf_expression *ex);

/* $Id: conditions.h,v 1.9 2004/05/23 20:52:22 emes Exp $ */
