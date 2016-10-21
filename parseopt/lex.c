/*
 
  copyright (c) 2004, Michal Salaban <emes@pld-linux.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: lex.c,v 1.13 2004/06/21 16:12:29 emes Exp $
 
*/

#include "../sys/exit.h"
#include "lex.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>

#define ERRMSGBUF       128

literal toktrans[] =
{
    /* Tokens table. To extend it, modify MAXREP in lex.h */
    { CO_NEQ,       {"!=",          "<>",           NULL,               NULL                } },
    { CO_LE,        {"<=",          "=<",           NULL,               NULL                } },
    { CO_LT,        {"<",           NULL,           NULL,               NULL                } },
    { CO_GE,        {">=",          "=>",           NULL,               NULL                } },
    { CO_GT,        {">",           NULL,           NULL,               NULL                } },
    { CO_EQ,        {"=",           "==",           NULL,               NULL                } },
    { LO_OR,        {"or",          "orelse",       "||",               NULL                } },
    { LO_AND,       {"and",         "andalso",      "&&",               NULL                } },
    { LI_SEP,       {",",           "|",            NULL,               NULL                } },
    { KW_IF,        {"if",          "when",         "on",               "rule"              } },
    { KW_RUN,       {"run",         "exec",         "execute",          "invoke"            } },
    { KW_LOG,       {"log",          NULL,          NULL,               NULL                } },
    { KW_ONCE,      {"once",        "onlyonce",     NULL,               NULL                } },
    { KW_STEP,      {"step",        "stepping",     NULL,               NULL                } },
    { BL_BEGIN,     {"{",           NULL,           NULL,               NULL                } },
    { BL_END,       {"}",           NULL,           NULL,               NULL                } },
    { ID_LA15,      {"la15",        "load15",       "loadavg15",        NULL                } },
    { ID_LA5,       {"la5",         "load5",        "loadavg5",         NULL                } },
    { ID_LA1,       {"la1",         "load1",        "loadavg1",         NULL                } },
    { ID_MFREE,     {"memfree",     "freemem",      "freememory",       NULL                } },
    { ID_MUSED,     {"memused",     "usedmem",      "usedmemory",       NULL                } },
    { ID_SFREE,     {"swapfree",    "freeswap",     NULL,               NULL                } },
    { ID_SUSED,     {"swapused",    "usedswap",     NULL,               NULL                } },
    { ID_FSFREE,    {"spacefree",   "freespace",    NULL,               NULL                } },
    { ID_FSAVAIL,   {"spaceavail",  "availspace",   "spaceavailable",   "availablespace"    } },
    { ID_FSUSED,    {"spaceused",   "usedspace",    NULL,               NULL                } },
    { ID_NPROC,     {"nproc",       "numproc",      "procnum",          "processes"         } },
    { PR_RUN,       {"run",         "running",      NULL,               NULL                } },
    { PR_STOP,      {"stop",        "stopped",      "traced",           NULL                } },
    { PR_SLEEP,     {"sleep",       "sleeping",     NULL,               NULL                } },
    { PR_UNINT,     {"unint",       "io",           "uninterruptible",  NULL                } },
    { PR_ZOMBIE,    {"zombie",      "defunct",      NULL,               NULL                } },
    { END,          {"\0",          NULL,           NULL,               NULL                } }
};

unsigned int    linenum = 1;

char * tok2string (token t)
{
    switch (t) {
        case CO_EQ:         return "=";
        case CO_NEQ:        return "!=";
        case CO_LE:         return "<=";
        case CO_LT:         return "<";
        case CO_GE:         return ">=";
        case CO_GT:         return ">";
        case LO_OR:         return "or";
        case LO_AND:        return "and";
        case LI_SEP:        return ",";
        case KW_IF:         return "if";
        case KW_RUN:        return "run";
        case KW_LOG:        return "log";
        case KW_ONCE:       return "once";
        case KW_STEP:       return "step";
        case BL_BEGIN:      return "{";
        case BL_END:        return "}";
        case ID_LA1:        return "LA1";
        case ID_LA5:        return "LA5";
        case ID_LA15:       return "LA15";
        case ID_MFREE:      return "MEMFREE";
        case ID_MUSED:      return "MEMUSED";
        case ID_SFREE:      return "SWAPFREE";
        case ID_SUSED:      return "SWAPUSED";
        case ID_FSFREE:     return "SPACEFREE";
        case ID_FSAVAIL:    return "SPACEAVAILABLE";
        case ID_FSUSED:     return "SPACEUSED";
        case ID_NPROC:      return "NPROC";
        case PR_RUN:        return "R";
        case PR_STOP:       return "T";
        case PR_SLEEP:      return "S";
        case PR_UNINT:      return "U";
        case PR_ZOMBIE:     return "Z";
        case VA_INT:        return "VA_INT";
        case VA_SIZ:        return "VA_SIZ";
        case VA_DBL:        return "VA_DBL";
        case VA_STR:        return "VA_STR";
        case END:           return "END";
    }
}

/*
 * Error reporting function.
 */

void lex_error (int errcode)
{
    size_t  bdis = dist_blank (lexptr);
    char    *ebuf = (char *) xalloc (NULL, bdis + ERRMSGBUF);

    /* terminate the string to print it clean.
     * (we're exiting so we can write to input buffer)
     */
    *(lexptr + bdis) = '\0';

    snprintf (ebuf, bdis + ERRMSGBUF,
            "illegal characters at line %d, near %s\n", linenum, lexptr);
    bail_out (errcode, ebuf);
}

/*
 * Character recognizing utilities.
 */

int isnewline (int c)
{
    return ((c == '\n') || (c == '\r'));
}

int isdelim (int c)
{
    return (isspace (c) || (c == '#') || (c == '\0'));
}

int isstrbound (int c)
{
    return ((c == '\'') || (c == '\"'));
}

char * to_newline (char *ptr)
{
    while (! (isnewline (*ptr) || (*ptr == '\0')))
        ptr ++;
    
    /* skip M$ DOS and other weird double-character newlines */
    if (isnewline (*ptr) &&
        isnewline (*(ptr+1)) &&
        (*ptr != *(ptr+1)))
        ptr ++;
    linenum ++;
    ptr ++;
    return ptr;
}

char * to_nextmeaning (char *ptr)
{
    while (isblank (*ptr)) ptr ++;
    
    /* comment or line end. skip to next line */
    if ((*ptr == '#') || isnewline (*ptr))
        return to_nextmeaning (to_newline (ptr));

    return ptr;
}

int dist_blank (char *ptr)
{
    /* return distance do next blank character */
    int     i = 0;
    
    while (! (isspace(*ptr) || (*ptr == '\0'))) {
        i ++;
        ptr ++;
    }
    return i;
}

/*
 * Value parsing functions
 */

void * get_int ()
{
    long long int   *res =
        (long long int *) xalloc (NULL, sizeof (long long int));
    char            *endptr;

    *res = strtoll (lexptr, &endptr, 10);
    if (isdelim (*endptr) && (! errno)) {
        lexptr = endptr;
        return (void *) res;
    }

    free (res);
    return NULL;
}

void * get_double ()
{
    double  *res = (double *) xalloc (NULL, sizeof (double));
    char    *endptr;

    *res = strtod (lexptr, &endptr);
    if (isdelim (*endptr) && (! errno)) {
        lexptr = endptr;
        return (void *) res;
    }

    free (res);
    return NULL;
}

void * get_size ()
{
    long long int   *res = (long long int *) xalloc (NULL, sizeof (long long int));
    long int        mult;
    char            *endptr;

    *res = strtoll (lexptr, &endptr, 10);
    if (errno) return NULL;

    /* size in bytes */
    if (isdelim(*endptr)) mult = 1;
    /* kilobytes */
    else if (*endptr == 'k') mult = 1024;
    /* megabytes and so on... */
    else if (*endptr == 'M') mult = 1024 * 1024;
    else if (*endptr == 'G') mult = 1024 * 1024 * 1024;
    /*
     * following ones cause overflow
    else if (*endptr == 'T') mult = 1024 * 1024 * 1024 * 1024;
    else if (*endptr == 'P') mult = 1024 * 1024 * 1024 * 1024 * 1024;
     */
    else return NULL;

    if (mult > 1) endptr ++;
    if (! isdelim (*endptr)) return NULL;

    lexptr = endptr;
    *res *= mult;
    return (void *) res;
}

void *get_string ()
{
    char    *ptr = lexptr;
    char    *buf;
    size_t  blen;
    
    /* string begins with " or ' */
    if (! isstrbound (*ptr)) return NULL;

    do {
        ptr ++;
        /* string not closed */
        if (*ptr == '\0') return NULL;
        
        /* iterate until string boundary
         * i.e. the same, unescaped delimiter as on the beginning
         */
    } while ((*ptr != *lexptr) || (*(ptr - 1) == '\\'));
    
    /* ok, now we got lexptr pointing on first delim, ptr on last one */
    lexptr++;
    blen = ptr - lexptr + 1;

    /* copy to buffer, set lexptr after end, return string */
    buf = (char *) xalloc (NULL, blen);
    memcpy (buf, lexptr, blen);
    *(buf + blen - 1) = '\0';
    lexptr += blen;
    return (void *) buf;
}
    

/*
 * Tokenizing functions
 */

tokdata next_token ()
{
   int          itok, irep;
   int          complen;
   tokdata      res;

   lexptr = to_nextmeaning (lexptr);
   if (*lexptr == '\0') {
       res.type = END;
       res.line = linenum;
       return res;
   }
   complen = dist_blank (lexptr);

   res.line = linenum;
   itok = 0;
   while (toktrans[itok].type != END) {
       /* for each token... */
       irep = 0;
       while ((irep < MAXREP) && (toktrans[itok].repv[irep] != NULL)) {
           /* for each representation... */
           if (strncasecmp (lexptr, toktrans[itok].repv[irep], complen) == 0)
               /* matched. make sure if it's full match,
                * not a prefix only
                */
               if ((toktrans[itok].repv[irep][complen] == '\0') &&
                   isdelim (*(lexptr + complen))) {
                   /* matched. move pointer to end of token and return
                    * it in symbolic form
                    */
                   lexptr += complen;
                   res.type = toktrans[itok].type;
                   return res;
               }
           irep ++;
       }
       itok ++;
   }

   /* we had no match, so this can be a value */
   res.val = get_int ();
   if (res.val != NULL) {
       res.type = VA_INT;
       return res;
   }
   res.val = get_double ();
   if (res.val != NULL) {
       res.type = VA_DBL;
       return res;
   }
   res.val = get_size ();
   if (res.val != NULL) {
       res.type = VA_SIZ;
       return res;
   }
   res.val = get_string ();
   if (res.val != NULL) {
       res.type = VA_STR;
       return res;
   }

   /* cannot fetch value. this is an error */   
   lex_error (EXIT_PARSE);
}

/* $Id: lex.c,v 1.13 2004/06/21 16:12:29 emes Exp $ */
