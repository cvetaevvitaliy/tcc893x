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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tcc_dxb_queue.h"

/** Initialize a queue descriptor
 * 
 * @param queue The queue descriptor to initialize. 
 * The user needs to allocate the queue
 */
void tcc_dxb_queue_init (tcc_dxb_queue_t * queue)
{
	int       i;
	tcc_qelem_t  *newelem;
	tcc_qelem_t  *current;
	queue->first = malloc (sizeof (tcc_qelem_t));
	memset (queue->first, 0, sizeof (tcc_qelem_t));
	current = queue->last = queue->first;
	queue->nelem = 0;
	queue->nmax_elem = MAX_QUEUE_ELEMENTS;
	for (i = 0; i < queue->nmax_elem - 2; i++)
	{
		newelem = malloc (sizeof (tcc_qelem_t));
		memset (newelem, 0, sizeof (tcc_qelem_t));
		current->q_forw = newelem;
		current = newelem;
	}
	current->q_forw = queue->first;

	pthread_mutex_init (&queue->mutex, NULL);
}

void tcc_dxb_queue_init_ex (tcc_dxb_queue_t * queue, int nmax_elem)
{
	int       i;
	tcc_qelem_t  *newelem;
	tcc_qelem_t  *current;
	queue->first = malloc (sizeof (tcc_qelem_t));
	memset (queue->first, 0, sizeof (tcc_qelem_t));
	current = queue->last = queue->first;
	queue->nelem = 0;
	queue->nmax_elem = nmax_elem+2;
	for (i = 0; i < queue->nmax_elem - 2; i++)
	{
		newelem = malloc (sizeof (tcc_qelem_t));
		memset (newelem, 0, sizeof (tcc_qelem_t));
		current->q_forw = newelem;
		current = newelem;
	}
	current->q_forw = queue->first;

	pthread_mutex_init (&queue->mutex, NULL);
}

/** Deinitialize a queue descriptor
 * flushing all of its internal data
 * 
 * @param queue the queue descriptor to dump
 */
void tcc_dxb_queue_deinit (tcc_dxb_queue_t * queue)
{
	int       i;
	tcc_qelem_t  *current;
	current = queue->first;
	for (i = 0; i < queue->nmax_elem - 2; i++)
	{
		if (current != NULL)
		{
			current = current->q_forw;
			free (queue->first);
			queue->first = current;
		}
	}
	if (queue->first)
	{
		free (queue->first);
		queue->first = NULL;
	}
	pthread_mutex_destroy (&queue->mutex);
}


/** Enqueue an element to the given queue descriptor
 * 
 * @param queue the queue descritpor where to queue data
 * 
 * @param data the data to be enqueued
 */
void tcc_dxb_queue (tcc_dxb_queue_t * queue, void *data)
{
	if (queue->last->data != NULL)
	{
		return;
	}
	if (queue->nelem + 1 > queue->nmax_elem - 2)
	{
		printf ("\n---------- queue full!!! ----------\n");
	}
	pthread_mutex_lock (&queue->mutex);
	queue->last->data = data;
	queue->last = queue->last->q_forw;
	queue->nelem++;
	pthread_mutex_unlock (&queue->mutex);
}

int tcc_dxb_queue_ex (tcc_dxb_queue_t * queue, void *data)
{
	pthread_mutex_lock (&queue->mutex);
	if (queue->last->data != NULL)
	{
		pthread_mutex_unlock (&queue->mutex);
		return 0;
	}
	if (queue->nelem + 1 > queue->nmax_elem - 2)
	{
		printf ("\n---------- queue full!!! ----------\n");
		pthread_mutex_unlock (&queue->mutex);
		return 0;
	}
	queue->last->data = data;
	queue->last = queue->last->q_forw;
	queue->nelem++;
	pthread_mutex_unlock (&queue->mutex);
	
	return 1;
}

int tcc_dxb_queuefirst (tcc_dxb_queue_t * queue, void *data)
{
	void *data_c, *data_n;
	tcc_qelem_t  *current;
	if (queue->last->data != NULL)
	{
		return 0;
	}
	if (queue->nelem + 1 > queue->nmax_elem - 2)
	{
		printf ("\n---------- queue full!!! ----------\n");
		return 0;
	}
	pthread_mutex_lock (&queue->mutex);
	if (queue->first->data == NULL)
	{
		queue->last->data = data;	
	}
	else
	{
		current =  queue->first->q_forw;
		data_c = current->data;
		current->data = data;	
		while(1)
		{
			if( data_c == NULL )
				break;		
			data_n = current->q_forw->data;
			current->q_forw->data = data_c;
			current = current->q_forw;
			data_c = data_n;
		}	
	}	
	queue->last = queue->last->q_forw;
	queue->nelem++;
	pthread_mutex_unlock (&queue->mutex);
	
	return 1;
}

/** Dequeue an element from the given queue descriptor
 * 
 * @param queue the queue descriptor from which to dequeue the element
 * 
 * @return the element that has bee dequeued. If the queue is empty
 *  a NULL value is returned
 */
void     *tcc_dxb_dequeue (tcc_dxb_queue_t * queue)
{
	void     *data;
	if (queue->first->data == NULL)
	{
		return NULL;
	}
	pthread_mutex_lock (&queue->mutex);
	data = queue->first->data;
	queue->first->data = NULL;
	queue->first = queue->first->q_forw;
	queue->nelem--;
	pthread_mutex_unlock (&queue->mutex);

	return data;
}

void     *tcc_dxb_queue_getdata (tcc_dxb_queue_t * queue)
{
	if (queue->first->data == NULL)
	{
		return NULL;
	}
	return queue->first->data;
}

/** Returns the number of elements hold in the queue
 * 
 * @param queue the requested queue
 * 
 * @return the number of elements in the queue
 */
int tcc_dxb_getquenelem (tcc_dxb_queue_t * queue)
{
	int       qelem;
	pthread_mutex_lock (&queue->mutex);
	qelem = queue->nelem;
	pthread_mutex_unlock (&queue->mutex);
	return qelem;
}

/** Returns the number of max elements hold in the queue
* 
* @param queue the requested queue
* 
* @return the number of max elements in the queue
*/
int tcc_dxb_get_maxqueuelem (tcc_dxb_queue_t * queue)
{
    int       qelem;
    pthread_mutex_lock (&queue->mutex);
    qelem = queue->nmax_elem - 2;
    pthread_mutex_unlock (&queue->mutex);
    return qelem;
}
    
/*
nth : start form zero
*/
void *tcc_dxb_peek_nth(tcc_dxb_queue_t *queue, int nth)
{
	int i;
	tcc_qelem_t  *current, *tmp;
	
	if(nth >= queue->nelem)
	{
		return NULL;
	}

	pthread_mutex_lock (&queue->mutex);
	current = queue->first;
	for(i=0 ; i<nth ; i++)
	{
		tmp = current->q_forw;
		current = tmp;
	}
	pthread_mutex_unlock (&queue->mutex);

	return (void*)current->data;
}
