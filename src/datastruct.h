/*
 
  copyright (c) 2004, Michal Salaban <emes@pld-linux.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: datastruct.h,v 1.12 2004/06/16 09:23:08 emes Exp $
 
*/

/***************************************************************************
 *                                                                         *
 *      BIG RED WARNING!                                                   *
 *                                                                         *
 *  Changing anything?                                                     *
 *  Be sure that functions in...                                           *
 *      datastruct.c                                                       *
 *      cp2memory.c                                                        *
 *  ...will work properly!                                                 *
 *                                                                         *
 *  Other source files may also require adjustment :(                      *
 *                                                                         *
 ***************************************************************************/

#include    <sys/types.h>

#define     STRBUF          256

#define     STAT_MAX_ARGS     2

#define     _PR_RUN           0
#define     _PR_SLEEP         1
#define     _PR_STOP          2
#define     _PR_ZOMBIE        3
#define     _PR_UNINT         4
#define     _PR_UNKNOWN       5

#define     PROC_RUN          1
#define     PROC_SLEEP        2
#define     PROC_STOP         4
#define     PROC_ZOMBIE       8
#define     PROC_UNINT       16
#define     PROC_UNKNOWN     32
#define     PROC_ANY         63

/***************************************************************************
    All-purpose list
 ***************************************************************************/

typedef struct _sf_list {
    int                 elsize;
    void                *el;
    struct _sf_list     *next;
} sf_list;

/***************************************************************************
    Stats
 ***************************************************************************/

typedef enum {
    //      Label describing stat type
    ST_LOAD,
    ST_MEM,
    ST_SWAP,
    ST_FS,
    ST_PROC,

    ST_NONE
} sf_stat_label;

typedef enum {
    //      Label describing data type (used with void* pointers)
    DOUBLE,
    INTEGER,
    PERCENT
} sf_type;

typedef enum {
    //      Selector of value associated with resource
    VA_USED,
    VA_FREE,
    VA_AVAIL,
    VA_TOTAL,

    VA_LAST
} sf_res_state;

typedef union {
    //      Selector or option describing stat
    int             laminutes;      // load
    sf_res_state    resstat;        // resources
    char            procstates;     // processes
    sf_list         *uids;          // processes
    char            *path;          // filesystem
} sf_stat_arg;

typedef struct {
    //      Stat definition
    sf_stat_label   label;
    sf_stat_arg     arg[ STAT_MAX_ARGS ];
} sf_stat_def;

typedef struct {
    //      Value with known type
    sf_type         type;
    void            *ptr;
} sf_value;

typedef struct {
    //      Full stat (definition and value)
    sf_stat_def     stat;
    sf_value        val;
} sf_stat_value;

/***************************************************************************
    Rules
 ***************************************************************************/

typedef struct {
    //      Node (relation) of an expression
    enum {
        RELATION,
        ATOMIC
    }       type;

    void    *arg1;
    void    *arg2;
    token   op;
} sf_expression;

typedef struct {
    //      Atomic relation (between stat and threshold)
    sf_stat_def     stat;
    sf_value        thresh;
    token           op;
} sf_atomic;

typedef struct {
    //      Command to be run
    int         once;
    char        *command;
} sf_rundata;

typedef struct {
    //      Log action
    int         once;
} sf_logdata;

typedef struct {
    //      One rule
    char            *name;
    sf_expression   *expr;
    sf_rundata      *run;
    sf_logdata      *log;
    unsigned int    step;

    // indicate if expression was true after this check and previous one
    int             hit,
                    prevhit;
    // All stats that are watched by this rule, organized in list
    // (for fast logging)
    sf_list         *watchstats;
} sf_rule;

/***************************************************************************
    Stat values database
 ***************************************************************************/

typedef struct {
    //      Statistics of underlying filesystem in path
    char                        *path;
    unsigned long long int      val[ VA_LAST ];
} sf_fs_stat;

typedef struct {
    //      Statistics of single process
    char        state;
    uid_t       uid;
} sf_proc_stat;

typedef struct {
    //      Main statistics database
    int             fetch_step;
    
    double          load[3];
    long long int   mem[ VA_LAST ];
    long long int   swap[ VA_LAST ];
    sf_list         *fs;            // list of filesystems being watched
    int             nr_proc;        // number of process entries in array
    sf_proc_stat    *proc;          // array of processes
} sf_database;

/***************************************************************************
    Helper functions operating on sf_* structures
 ***************************************************************************/

void add_def_to_list (sf_list **hd, sf_stat_def *def);
int equal_defs (sf_stat_def *d1, sf_stat_def *d2);
int uid_in_list (sf_list *hd, uid_t uid);
void add_fs_entry_to_list (sf_list **hd, char *path);
void add_proc_entry_to_array (sf_database *db, sf_proc_stat *proc);
sf_fs_stat * get_fs_entry_from_list (sf_list *hd, char *path);
int get_proc_num (sf_database *db, char statemask, uid_t *uid);
char * def_2_string (sf_stat_def *def);
char * uids_2_str (sf_list *hd);
char * proc_states_2_str (int statemask);
int get_min_step (sf_rule **rule);

/* $Id: datastruct.h,v 1.12 2004/06/16 09:23:08 emes Exp $ */
