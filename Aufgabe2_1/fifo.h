#include <pthread.h>
#include <semaphore.h>

#ifndef _FIFO_H
#define _FIFO_H

struct Fifo {
    
    int head;
    int tail;

    char *buffer;
    int bufferSize;
    int currentSize;

    sem_t *semaphoreNotEmpty;
    sem_t *semaphoreNotFull;

    pthread_mutex_t mutex;
    pthread_cond_t notFull;
    pthread_cond_t notEmpty;

};

extern struct Fifo fifoCreate(int bufferSize);
extern void fifoDestroy(struct Fifo *f);
extern void fifoAdd(char c, struct Fifo *f);
extern void fifoRemove(struct Fifo *f);
extern void fifoPeek(struct Fifo *f);

#endif