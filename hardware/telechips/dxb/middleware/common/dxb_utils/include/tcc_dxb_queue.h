/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/

#ifndef __TCC_DXB_QUEUE_H__
#define __TCC_DXB_QUEUE_H__

#include <pthread.h>

#ifdef __cplusplus
extern    "C"
{
#endif

/** Maximum number of elements in a queue 
 */
#define MAX_QUEUE_ELEMENTS 160 //ZzaU:: increase count because of encode!!
#define TDMB_MAX_QUEUE_ELEMENTS 40

/** Output port queue element. Contains an OMX buffer header type
 */
	typedef struct tcc_qelem_t tcc_qelem_t;
	struct tcc_qelem_t
	{
		tcc_qelem_t  *q_forw;
		void     *data;
	};

/** This structure contains the queue
 */
	typedef struct tcc_dxb_queue_t
	{
		tcc_qelem_t  *first;
				  /**< Output buffer queue head */
		tcc_qelem_t  *last;
				 /**< Output buffer queue tail */
		int       nelem;
			 /**< Number of elements in the queue */
		int       nmax_elem;
				 /**maximum number of elements**/
		pthread_mutex_t mutex;
	} tcc_dxb_queue_t;

/** Initialize a queue descriptor
 * 
 * @param queue The queue descriptor to initialize. 
 * The user needs to allocate the queue
 */
	void      tcc_dxb_queue_init (tcc_dxb_queue_t * tcc_dxb_queue);
	void      tcc_dxb_queue_init_ex (tcc_dxb_queue_t * tcc_dxb_queue, int nmax_elem);

/** Deinitialize a queue descriptor
 * flushing all of its internal data
 * 
 * @param queue the queue descriptor to dump
 */
	void      tcc_dxb_queue_deinit (tcc_dxb_queue_t * queue);

/** Enqueue an element to the given queue descriptor
 * 
 * @param queue the queue descritpor where to queue data
 * 
 * @param data the data to be enqueued
 */
	void      tcc_dxb_queue (tcc_dxb_queue_t * queue, void *data);
	int 	  tcc_dxb_queue_ex (tcc_dxb_queue_t * queue, void *data);
	int		  tcc_dxb_queuefirst (tcc_dxb_queue_t * queue, void *data);


/** Dequeue an element from the given queue descriptor
 * 
 * @param queue the queue descriptor from which to dequeue the element
 * 
 * @return the element that has bee dequeued. If the queue is empty
 *  a NULL value is returned
 */
	void     *tcc_dxb_dequeue (tcc_dxb_queue_t * queue);
	void     *tcc_dxb_queue_getdata (tcc_dxb_queue_t * queue);

/** Returns the number of elements hold in the queue
 * 
 * @param queue the requested queue
 * 
 * @return the number of elements in the queue
 */
	int       tcc_dxb_getquenelem (tcc_dxb_queue_t * queue);


	int tcc_dxb_get_maxqueuelem (tcc_dxb_queue_t * queue);
	void *tcc_dxb_peek_nth(tcc_dxb_queue_t *queue, int nth);


#ifdef __cplusplus
}
#endif

#endif
