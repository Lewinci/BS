#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include "fifo.h"

#if !defined(SYNCVARIANT)
#define SYNCVARIANT 1
    
#endif // SYNCVARIANT

void cleanEnterAdd(struct Fifo *f) {

    if(SYNCVARIANT == 1) {
        pthread_mutex_lock(&f->mutex);
        while(f->bufferSize == f->currentSize) {
            pthread_cond_wait(&f->notFull, &f->mutex);
        }
    }else{
        sem_wait(f->semaphoreNotFull);
        pthread_mutex_lock(&f->mutex);
    }
}

void cleanExitAdd(struct Fifo *f) {
    if(SYNCVARIANT == 1) {
        pthread_cond_signal(&f->notEmpty);
    }else{
        sem_post(f->semaphoreNotEmpty);
    }

    pthread_mutex_unlock(&f->mutex);
}

void cleanEnterRemove(struct Fifo *f) {
    if(SYNCVARIANT == 1) {
        pthread_mutex_lock(&f->mutex);
        while(f->currentSize == 0) {
            pthread_cond_wait(&f->notEmpty, &f->mutex);
        }
    }else{
        sem_wait(f->semaphoreNotEmpty);
        pthread_mutex_lock(&f->mutex);
    }
}

void cleanExitRemove(struct Fifo *f) {
    if(SYNCVARIANT == 1) {
        pthread_cond_signal(&f->notFull);
    }else{
        sem_post(f->semaphoreNotFull);
    }

    pthread_mutex_unlock(&f->mutex);
}

void mutexCleanupHandler(void *data) {
    pthread_mutex_t* mutex = (pthread_mutex_t*)data;
    pthread_mutex_unlock(mutex);
}

struct Fifo fifoCreate(int bufferSize) {

    struct Fifo newFifo;

    newFifo.head = 0;
    newFifo.tail = 0;
    newFifo.buffer = (char*)malloc(bufferSize * sizeof(char));
    newFifo.bufferSize = bufferSize;
    newFifo.currentSize = 0;

    pthread_mutex_init(&newFifo.mutex, NULL);
    pthread_cond_init(&newFifo.notEmpty, NULL);
    pthread_cond_init(&newFifo.notFull, NULL);

    sem_unlink("/NOT_EMPTY_SEM");
    sem_unlink("/NOT_FULL_SEM");
    newFifo.semaphoreNotEmpty = sem_open("/NOT_EMPTY_SEM", O_CREAT, S_IRUSR | S_IWUSR, 0);
    newFifo.semaphoreNotFull = sem_open("/NOT_FULL_SEM", O_CREAT, S_IRUSR | S_IWUSR, bufferSize);

    return newFifo;
}

void fifoDestroy(struct Fifo *f) {
    free(f->buffer);
    pthread_cond_destroy(&f->notEmpty);
    pthread_cond_destroy(&f->notFull);
    pthread_mutex_destroy(&f->mutex);
}

void fifoAdd(char c, struct Fifo *f) {
    pthread_cleanup_push(mutexCleanupHandler, &f->mutex);
    cleanEnterAdd(f);

    memcpy(f->buffer + f->tail, &c, sizeof(char));
    f->tail = (f->tail + 1) % f->bufferSize;
    f->currentSize++;

    printf("PUSHED -> %c \n", c);

    cleanExitAdd(f);
    pthread_cleanup_pop(0);
}

void fifoRemove(struct Fifo *f) {

    pthread_cleanup_push(mutexCleanupHandler, &f->mutex);
    cleanEnterRemove(f);
    
    char charToRemove = *(f->buffer + f->head);
    f->head = (f->head + 1) % f->bufferSize;
    f->currentSize--;

    printf("REMOVED -> %c \n", charToRemove);

    cleanExitRemove(f);
    pthread_cleanup_pop(0);   
}

void fifoPeek(struct Fifo *f) {
    pthread_mutex_lock(&f->mutex);
    
    for(int i = 0; i < f->currentSize; i++) {
        printf("%d:[%c] ", i, *(f->buffer + ((i + f->head) % f->bufferSize)));
    }
    printf("\n \n");

    pthread_mutex_unlock(&f->mutex);
}

