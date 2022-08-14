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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

#include "tcc_dxb_semaphore.h"

/** Initializes the semaphore at a given value
 * 
 * @param tsem the semaphore to initialize
 * @param val the initial value of the semaphore
 * 
 */
void tcc_dxb_sem_init(tcc_dxb_sem_t* tsem, unsigned int val) {
  pthread_cond_init(&tsem->condition, NULL);
  pthread_mutex_init(&tsem->mutex, NULL);
  tsem->semval = val;
}

/** Destroy the semaphore
 *  
 * @param tsem the semaphore to destroy
 */
void tcc_dxb_sem_deinit(tcc_dxb_sem_t* tsem) {
  pthread_cond_destroy(&tsem->condition);
  pthread_mutex_destroy(&tsem->mutex);
}

/** Decreases the value of the semaphore. Blocks if the semaphore
 * value is zero.
 * 
 * @param tsem the semaphore to decrease
 */
void tcc_dxb_sem_down(tcc_dxb_sem_t* tsem) {
  pthread_mutex_lock(&tsem->mutex);
  while (tsem->semval == 0) {
	  pthread_cond_wait(&tsem->condition, &tsem->mutex);
  }
  tsem->semval--;
  pthread_mutex_unlock(&tsem->mutex);
}

int tcc_dxb_sem_down_timewait(tcc_dxb_sem_t* tsem,int expire_time)
{
	int err;
    struct timeval now;
    struct timespec ts;

    gettimeofday(&now, NULL);
    ts.tv_sec = now.tv_sec + expire_time; 	// sec 단위로 입력..
    ts.tv_nsec = now.tv_usec * 1000;

	pthread_mutex_lock(&tsem->mutex);
	while (tsem->semval == 0) {
		err = pthread_cond_timedwait(&tsem->condition, &tsem->mutex , &ts);

		if(err == ETIMEDOUT)
		{
			tsem->semval++;
		}
	}
	tsem->semval--;
	pthread_mutex_unlock(&tsem->mutex);

	if(err == ETIMEDOUT)
		return 0;
	else return 1;
}

int tcc_dxb_sem_down_timewait_msec(tcc_dxb_sem_t* tsem,int expire_time)
{
	int err;
    struct timeval now;
    struct timespec ts;

    gettimeofday(&now, NULL);
    ts.tv_sec = now.tv_sec; 	
    ts.tv_nsec = now.tv_usec * 1000 + expire_time*1000*1000; // nsec 단위로 입력..

	pthread_mutex_lock(&tsem->mutex);
	while (tsem->semval == 0) {
		err = pthread_cond_timedwait(&tsem->condition, &tsem->mutex , &ts);

		if(err == ETIMEDOUT)
		{
			tsem->semval++;
		}
	}
	tsem->semval--;
	pthread_mutex_unlock(&tsem->mutex);

	if(err == ETIMEDOUT)
		return 0;
	else return 1;
}


/** Increases the value of the semaphore
 * 
 * @param tsem the semaphore to increase
 */
void tcc_dxb_sem_up(tcc_dxb_sem_t* tsem) {
  pthread_mutex_lock(&tsem->mutex);
  tsem->semval++;
  pthread_cond_signal(&tsem->condition);
  pthread_mutex_unlock(&tsem->mutex);
}

/** Reset the value of the semaphore
 * 
 * @param tsem the semaphore to reset
 */
void tcc_dxb_sem_reset(tcc_dxb_sem_t* tsem) {
  pthread_mutex_lock(&tsem->mutex);
  tsem->semval=0;
  pthread_mutex_unlock(&tsem->mutex);
}

/** Wait on the condition.
 * 
 * @param tsem the semaphore to wait
 */
void tcc_dxb_sem_wait(tcc_dxb_sem_t* tsem) {
  pthread_mutex_lock(&tsem->mutex);
  pthread_cond_wait(&tsem->condition, &tsem->mutex);
  pthread_mutex_unlock(&tsem->mutex);
}

/** Signal the condition,if waiting
 * 
 * @param tsem the semaphore to signal
 */
void tcc_dxb_sem_signal(tcc_dxb_sem_t* tsem) {
  pthread_mutex_lock(&tsem->mutex);
  pthread_cond_signal(&tsem->condition);
  pthread_mutex_unlock(&tsem->mutex);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
