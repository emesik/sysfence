/*
 
  copyright (c) 2004, Michal Salaban <michal@salaban.info>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: lex.h,v 1.7 2004/06/05 12:49:41 emes Exp $
 
*/

#define MAXREP  4

char            *lexptr;


typedef enum
{
    /* comparison operators */
    CO_EQ,      CO_NEQ,     CO_LE,      CO_LT,      CO_GE,      CO_GT,
    /* logical operators */
    LO_OR,      LO_AND,
    /* list separator */
    LI_SEP,
    /* keywords */
    KW_IF,      KW_RUN,     KW_LOG,     KW_ONCE,    KW_STEP,
    /* block beginning/end */
    BL_BEGIN,   BL_END,
    /* identifiers */
    ID_LA1,     ID_LA5,     ID_LA15,
    ID_MFREE,   ID_MUSED,
    ID_SFREE,   ID_SUSED,
    ID_FSFREE,  ID_FSAVAIL, ID_FSUSED,
    ID_NPROC,
    /* process states */
    PR_RUN,     PR_STOP,    PR_SLEEP,   PR_UNINT,   PR_ZOMBIE,
    /* values */
    VA_INT,     VA_SIZ,     VA_DBL,     VA_STR,
    /* special token, must be last */
    END
} token;

typedef struct
{
    token   type;
    char    *repv[MAXREP];  /* representations table */
} literal;

typedef struct
{
    token           type;
    unsigned int    line;
    void            *val;
} tokdata;

/*
 * Functions
 */

tokdata next_token ();
char * tok2string (token t);

/* $Id: lex.h,v 1.7 2004/06/05 12:49:41 emes Exp $ */
