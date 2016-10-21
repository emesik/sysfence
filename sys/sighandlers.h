/*
 
  copyright (c) 2004, Mirek Kopertowski <m.kopertowski@post.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
*/

#define     PARENT      0
#define     CHILD       1

#define     SIGBLOCK        0
#define     SIGUNBLOCK      1

void signal_init (int process);
void signals_handling (int onoff);
