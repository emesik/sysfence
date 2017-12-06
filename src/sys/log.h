/*
 
  copyright (c) 2004, Michal Salaban <michal@salaban.info>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: log.h,v 1.2 2004/05/31 14:09:08 emes Exp $
 
*/

#define LOGLINEBUF      256
#define LOGVARBUF       64

void log_start ();
void log_rulehit (sf_rule *rule);
void log_end ();

/* $Id: log.h,v 1.2 2004/05/31 14:09:08 emes Exp $ */
