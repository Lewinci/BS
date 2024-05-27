/*
 ********************************************************************
 * @file    taskqueue.c
 * @author  Franz Korf 
 *          HAW-Hamburg
 *          Berliner Tor  7
 *          D-20099 Hamburg
 *
 * @date    Apr 2019
 * @brief   This C Module implements a task queue for a thread pool.
 *          More Details: taskQueue.h
 *
 *******************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "myError.h"
#include "taskQueue.h" 

#define PROC_FS_MSG_MAX     "/proc/sys/fs/mqueue/msg_max"
#define PROC_FS_MSGSIZE_MAX "/proc/sys/fs/mqueue/msgsize_max"

#define DEFAULT_PRIORITY	5	// Priority used within the queue

/**
 * @brief @brief This function reads an int value from a file.
 *
 * @param file Name of the file to be read.
 *
 * @return  The value that had been read out of the file.
 * 		    In case of an error the program will terminate
 * 		    with an error message.
 */

static int readIntFromFile(char *file) {
   FILE *fd = fopen(file, "r");
   int val;
   if (fd == NULL) { HANDLE_ERR(-1) }

   if (EOF == fscanf(fd,"%d",&val)){ HANDLE_ERR(-1) }

   if (EOF == fclose(fd)) { HANDLE_ERR(-1) }
   return val;
}

int sendToTaskQueue(const mqd_t mqdes, const struct TaskHeader t, const char *arg, const bool blocking){
   const struct timespec timeout = {0,0};
   int erg;

   char block[sizeof(struct TaskHeader) + t.argSize];
   memcpy(block,&t, sizeof(struct TaskHeader)); // write header to buffer
   memcpy(block+sizeof(struct TaskHeader), arg, t.argSize); // write argument to buffer

   if (blocking) {
      erg = mq_send(mqdes, block, sizeof(struct TaskHeader) + t.argSize, DEFAULT_PRIORITY);
   } else {
      erg = mq_timedsend(mqdes, block, sizeof(struct TaskHeader) + t.argSize, DEFAULT_PRIORITY, &timeout);
   }
   if ((erg == -1) && (!blocking) && (errno == ETIMEDOUT)) {
      return 1;
   }
   HANDLE_ERR(erg);
   return 0;
}

mqd_t createTaskQueue(const char * name, const unsigned int queueSize, const unsigned int maxArgSize){
   struct mq_attr attr;

   // Set up the attribute structure
   attr.mq_maxmsg = queueSize;
   attr.mq_msgsize = maxArgSize + sizeof(struct TaskHeader);
   attr.mq_flags = 0;

   // check parameter of message queue
   if (readIntFromFile(PROC_FS_MSG_MAX) < attr.mq_maxmsg) {
      printf("File %s Line %d: Maximal number of msg queue elements (%ld) out of range (s. %s) \n",
    		  __FILE__, __LINE__, attr.mq_maxmsg, PROC_FS_MSG_MAX);
      exit(EXIT_FAILURE);
   }

   if (readIntFromFile(PROC_FS_MSGSIZE_MAX) < attr.mq_msgsize) {
      printf("File %s Line %d: Maximal size of msg queue element (%d) out of range (s. %s) \n",
    		  __FILE__, __LINE__, (int) attr.mq_msgsize, PROC_FS_MSGSIZE_MAX);
      exit(EXIT_FAILURE);
   }

   // Set access mode to read & write for owner & group; to read for others
   mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

   mqd_t mqdes = mq_open (name, O_RDWR | O_CREAT, mode, &attr);
   HANDLE_ERR((mqdes != -1)?0:-1);
   return mqdes;
}

void receiveFromTaskQueue(const mqd_t mqdes, struct TaskHeader *t, char *argBuf, const unsigned int sizeOfArgBuf){
   unsigned int prio;
   struct mq_attr attr;

   HANDLE_ERR(mq_getattr(mqdes, &attr));
   char *buf = malloc(attr.mq_msgsize);
   HANDLE_ERR((buf == NULL)?-1:0);

   int msgSize = mq_receive(mqdes, buf, attr.mq_msgsize, &prio);

   HANDLE_ERR((msgSize != -1) ? 0 : -1)

   if (msgSize < (int) sizeof(struct TaskHeader)){
	   printf("File %s Line %d: size of message (%d) smaller than sizeof(struct Task) (%ld)\n",
			   __FILE__, __LINE__, msgSize, sizeof(struct TaskHeader));
	   exit(EXIT_FAILURE);
   }

   t->argSize = ((struct TaskHeader*)buf)->argSize;
   t->routineForTask = ((struct TaskHeader*)buf)->routineForTask;

   if ((msgSize - sizeof(struct TaskHeader)) != t->argSize){
	   printf("File %s Line %d: work argument size (%ld) - expected argument size = %d\n",
			   __FILE__, __LINE__, msgSize-sizeof(struct TaskHeader), t->argSize);
	   exit(EXIT_FAILURE);
   }

   if (t->argSize > sizeOfArgBuf){
	   printf("File %s Line %d: argument buffer to small (%d) - expected buffersize = %d\n",
			   __FILE__, __LINE__, sizeOfArgBuf, t->argSize);
	   exit(EXIT_FAILURE);
   }

   memcpy(argBuf,buf + sizeof(struct TaskHeader), t->argSize);
   free(buf);
}

void closeTaskQueue(const mqd_t mqdes) {
   HANDLE_ERR(mq_close(mqdes));
}

void destroyTaskQueue(const char * name){
   int e = mq_unlink(name);
   if ((e == -1) && (errno != ENOENT)) HANDLE_ERR(e);
}

// EOF
