/*
 
  copyright (c) 2004, Michal Salaban <emes@pld-linux.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: confread.h,v 1.3 2004/05/23 20:52:23 emes Exp $
 
*/

#define TOKBUFFER       4096

char * readfile (char *fname);
tokdata * tokenized_conf (char *fname);
sf_rule ** read_config_files (int argc, char *argv[]);

/* $Id: confread.h,v 1.3 2004/05/23 20:52:23 emes Exp $ */
