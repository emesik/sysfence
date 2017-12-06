/*
 
  copyright (c) 2004, Michal Salaban <michal@salaban.info>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
*/

/* $Id: mainloop.h,v 1.5 2004/05/28 20:36:50 mkoperto Exp $ */

int         semid;
int         rule_shmid;
int         *ruleprocesses;
int         rulecount;
sf_rule     *rule;

void rule_watch_loop (sf_rule *rule);
void res_probe_loop (sf_database *db);

/* $Id: mainloop.h,v 1.5 2004/05/28 20:36:50 mkoperto Exp $ */
