/*
 
  copyright (c) 2004, Mirek Kopertowski <m.kopertowski@post.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
*/

#define SEMAPHORE_SET   1
#define SEMAPHORE_RESET 0
                                 
int semaphore_init (int val);
int semaphore_op (int semid, int sem_op);
int semaphore_del (int semid);

#define semaphore_wait(S)       semaphore_op (S, -1)
#define semaphore_post(S)       semaphore_op (S, 1)

int shared_mem_init (int memsize);
void *shared_mem_attach (int shmid);
void shared_mem_detach (void *shm);
void shared_mem_del (int shmid);
