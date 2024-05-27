#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include "fifo.h"

typedef enum {false = 0, true = 1} bool;

struct ProducerData{
    bool isUpper;
    struct Fifo *f;
};


void *produce(bool isUpper, struct Fifo *f) {
    char stringToPush[] = "abcdefghijklmnopqrstuvwxyz";

    int idx = 0;
    int arrayLength = sizeof(stringToPush)/sizeof(char);

    while(idx < arrayLength - 1) {
        if(isUpper == true) {
            fifoAdd(toupper(stringToPush[idx]), f);
        }else {
            fifoAdd(stringToPush[idx], f);
        }

        if(idx == arrayLength - 2) {
            idx = 0;
        }else {
            idx++;
        }
        sleep(3);
    }
    return 0;
} 


void *consume(struct Fifo *f) {
    while(1){
        fifoRemove(f);
        sleep(2);
    }
    return 0;
}

void *producer1Thread(void *data) {
    struct Fifo *f = (struct Fifo*)data;
    produce(false, f);
    return 0;
}

void *producer2Thread(void *data) {
    struct Fifo *f = (struct Fifo*)data;
    produce(true, f);
    return 0;
}

void *consumerThread(void *data) {
    struct Fifo *f = (struct Fifo *)data;
    consume(f);
    return 0;
}

void *controlThreadRoutine(void *data){

    struct Fifo *f = (struct Fifo *)data;
    pthread_t producer1;
    pthread_t producer2;
    pthread_t consumer;

    bool isRunningP1 = false;
    bool isRunningP2 = false;
    bool isRunningC = false;

    bool quitSystem = false;

    while(quitSystem == false) {
        switch (getchar())
        {
        case 'c':
        case 'C':
            if(isRunningC == false) {
                pthread_create(&consumer, NULL, consumerThread, f);
                isRunningC = true;
            }else {
                pthread_cancel(consumer);
                isRunningC = false;
            }
            break;

        case '1':
            if(isRunningP1 == false) {
                pthread_create(&producer1, NULL, producer1Thread, f);
                isRunningP1 = true;
            }else {
                pthread_cancel(producer1);
                isRunningP1 = false;
            }
            break;

        case '2':
            if(isRunningP2 == false) {
                pthread_create(&producer2, NULL, producer2Thread, f);
                isRunningP2 = true;
            }else {
                pthread_cancel(producer2);
                isRunningP2 = false;
            }
            break;

        case 'h':
            printf("– Tastatureingabe 1: Starte / Stoppe Producer_1 \n– Tastatureingabe 2: Starte / Stoppe Producer_2\n– Tastatureingabe c oder C: Starte / Stoppe Consumer \n– Tastatureingabe q oder Q: Termination des Systems \n– Tastatureingabe h: Liefert diese Liste von möglichen Eingaben \n– Alle anderen Tastatureingaben werden ignoriert.\n");
            break;
        case 'q':
        case 'Q':
            quitSystem = true;
            break;
        
        default: 
            break;
        }
    }

    return 0;
}


int main(int argc, char const *argv[])
{
    struct Fifo f = fifoCreate(10);

    pthread_t controlThread;
    pthread_create(&controlThread, NULL, controlThreadRoutine, &f);
    pthread_join(controlThread, NULL);
    fifoDestroy(&f);
}
