/*
 
  copyright (c) 2004, Michal Salaban <emes@pld-linux.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: users.c,v 1.1 2004/06/03 22:00:42 emes Exp $
 
*/

#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include "xalloc.h"
#include "users.h"

uid_t * username2uid (char *name)
{
    struct passwd   *p = getpwnam (name);
    uid_t           *b;

    if (!p) return NULL;
    
    b = (uid_t *) xalloc (NULL, sizeof (uid_t));
    *b = p->pw_uid;
    return b;
}
    
char * uid2username (uid_t uid)
{
    struct passwd   *p = getpwuid (uid);
    char            *b;

    if (!p) return NULL;

    b = (char *) xalloc (NULL, strlen (p->pw_name) + 1);
    strcpy (b, p->pw_name);
    return b;
}

/* $Id: users.c,v 1.1 2004/06/03 22:00:42 emes Exp $ */
