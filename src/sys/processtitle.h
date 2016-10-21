/*
 
  copyright (c) 2004, Mirek Kopertowski <m.kopertowski@post.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  The idea is taken from sendmail.

*/
#define     STATE_NORMAL    0
#define     STATE_EXEC      1
#define     STATE_STOPPED   2

#define     MAIN_PROCESS    1
#define     RULE_PROCESS    2

#define     MAIN_PROCESS_NAME   "sffetch"
#define     RULE_PROCESS_NAME   "sfwatch"

#define     STATE_EXEC_NAME     "EXEC"
#define     STATE_STOPPED_NAME  "STOPPED"

void initproctitle (int argc, char **argv);
void setproctitle (int process, int state, const char *rulename);
