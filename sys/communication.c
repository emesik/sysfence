/*
 
  copyright (c) 2004, Mirek Kopertowski <m.kopertowski@post.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
*/

#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};

int semaphore_op (int semid, int sem_op)
{
    static struct sembuf operation[1];
    
    operation[0].sem_num = 0;
    operation[0].sem_op = sem_op;
    operation[0].sem_flg = SEM_UNDO;

    return semop (semid, operation, 1);
}

int semaphore_init (int val)
{
    int semid;
    union semun argument;
    unsigned short values[1];

    /* create semaphore */
    semid = semget (IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    values[0] = val;
    argument.array = values;
    /* set semaphore value */
    semctl (semid, 0, SETALL, argument);

    return semid;
}

int semaphore_del (int semid)
{    
    union semun ignored_argument;

    /* delete semaphore - last process */
    return semctl (semid, 1, IPC_RMID, ignored_argument);
}

int shared_mem_init (int memsize)
{
    /* allocate shared memory */
    return shmget (IPC_PRIVATE, memsize, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
}

void *shared_mem_attach (int shmid)
{
    return shmat (shmid, NULL, 0);
}

void shared_mem_detach (void *shm)
{
    shmdt (shm);
}

void shared_mem_del (int shmid)
{
    shmctl (shmid, IPC_RMID, NULL);
}
