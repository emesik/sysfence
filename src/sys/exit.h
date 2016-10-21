/*
 
  copyright (c) 2004, Michal Salaban <emes@pld-linux.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: exit.h,v 1.1 2004/05/26 17:30:42 mkoperto Exp $
 
*/
#include <stdio.h>

#define EXIT_OK         0
#define EXIT_IO         1
#define EXIT_MEM        2

#define EXIT_NOCONF     11
#define EXIT_NORULE     12

#define EXIT_PARSE      21
#define EXIT_VALUE      22
#define EXIT_OPTION     23

#define EXIT_SHM        31
#define EXIT_SEM        32

#define EXIT_BUG        41

void bail_out (int excode, const char *details);

/* $Id: exit.h,v 1.1 2004/05/26 17:30:42 mkoperto Exp $ */
