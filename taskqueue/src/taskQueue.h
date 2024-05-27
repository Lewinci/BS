/********************************************************************
 * @file    taskqueue.h
 * @author  Franz Korf 
 *          HAW-Hamburg
 *          Berliner Tor  7
 *          D-20099 Hamburg
 *
 * @date    Apr 2019
 * @brief	Based on a  LINUX message queue (s. mq_overview) this module implements a task
 * 			queue of a thread pool. Each queue element stores one task. Two items define a
 * 			task
 * 			(1)	A pointer to the function that implements the behavior of the task
 * 			(2)	A structure that stores the arguments used by this function
 *
 * 			To support argument structures of different size, a queue element is divided into
 * 			two parts. The first part is a header that stores the function pointer and the size
 * 			of the structure that contains the arguments of the function. The argument structure
 * 			itself is stored as a sequence of chars, which is located in memory behind the header.
 *
 * 			Remarks:
 * 			- The maximum size of a queue and the maximum size of a queue element is given
 * 			  in /proc file system (e.g. /proc/sys/fs/mqueue/msg_max).
 * 			- This is the second release of the task queue module. In opposite to the first one,
 * 			  the argument of a task is stored in the message queue element. The first release
 * 			  stored such an argument on the heap.
 *                         
 ******************************************************************
 */

#ifndef _TASKQUEUE_H
#define _TASKQUEUE_H
#include <mqueue.h>
#include <stdbool.h>

/*
 * @brief Header of a task queue element.
 */
struct TaskHeader{
   void (*routineForTask)(void *arg); 	//! Pointer the the function that implements the task
   unsigned int argSize;				//! Size of the structure containing the arguments
};

/**
 * @brief This function creates a task queues using a LINUX message queue.
 *
 * @param name	The system-wide known name of the task queue. It must be an identifier
 * 				that begins with a "/".
 * @param queueSize	Maximum number of items that can be stored in the queue.
 * @param maxArgSize Maximum size of an argument structure (in bytes).
 *
 * @return      The descriptor of the generated queue. In case of an error
 *              an error message will be generated and the program will be
 *              terminated.
 */
extern mqd_t createTaskQueue(const char * name, const unsigned int queueSize,
		const unsigned int maxArgSize);

/**
 * @brief This function closes a task queue. Remaining queue elements are retained.
 *
 * @param mqdes The descriptor of the task queue.
 */
extern void closeTaskQueue(const mqd_t mqdes);


/**
 * @brief This function destroys a task queue and releases all resources.
 * 		  The destruction will be delayed until the task queue has been closed
 * 		  within all processes.
 *
 * @param name The system-wide known name of the task queue.
 */
extern void destroyTaskQueue(const char * name);

/**
 * @brief This function adds a task in a task queue.
 *
 * @param mqdes 	The descriptor of the task queue.
 * @param t 		The header of the task that should be added.
 * @param arg		Pointer to the argument structure of the task.
 * @param blocking 	This parameter determines whether sendToTaskQueue should
 * 				   	block in case of a full task queue.
 *
 * @return 		On success the return value is 0. If the queue is full and
 * 			blocking == false, then the task is not inserted in the queue
 * 			and the return value is 1.
 * 			In case of an error, an error message will be given an the program
 * 			terminates.
 */
extern int sendToTaskQueue(const mqd_t mqdes, const struct TaskHeader t, const char *arg, const bool blocking);

/**
 * @brief This function takes a task out of a task queue. If the task
 *        queue is empty, this function blocks until an element is available.
 *
 * @param mqdes        The descriptor of the task queue.
 * @param t            The header of the task, that has been taken out.
 * @param argBuf       A buffer for the argument structure of the task that has been taken out.
 * @param sizeOfArgBuf Size of argBuf
 * @return	       In case of an error, the program terminates with an error message.
 *                     On success 0 will be returned. In case of an non blocking access
 *                     to a full queue, 1 will be returned.
 *
 */
extern void receiveFromTaskQueue(const mqd_t mqdes, struct TaskHeader *t, char *argBuf,
		const unsigned int sizeOfArgBuf);

#endif

//EOF

