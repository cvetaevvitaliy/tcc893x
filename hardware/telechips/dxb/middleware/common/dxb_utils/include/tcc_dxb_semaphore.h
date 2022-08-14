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

#ifndef __TCC_DXB_SEMAPHORE_H__
#define __TCC_DXB_SEMAPHORE_H__

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/** The structure contains the semaphore value, mutex and green light flag
 */ 
typedef struct tcc_dxb_sem_t{
  pthread_cond_t condition;
  pthread_mutex_t mutex;
  unsigned int semval;
}tcc_dxb_sem_t;

/** Initializes the semaphore at a given value
 * 
 * @param tsem the semaphore to initialize
 * 
 * @param val the initial value of the semaphore
 */
void tcc_dxb_sem_init(tcc_dxb_sem_t* tsem, unsigned int val);

/** Destroy the semaphore
 *  
 * @param tsem the semaphore to destroy
 */
void tcc_dxb_sem_deinit(tcc_dxb_sem_t* tsem);

/** Decreases the value of the semaphore. Blocks if the semaphore
 * value is zero.
 * 
 * @param tsem the semaphore to decrease
 */
void tcc_dxb_sem_down(tcc_dxb_sem_t* tsem);

/** Increases the value of the semaphore
 * 
 * @param tsem the semaphore to increase
 */
void tcc_dxb_sem_up(tcc_dxb_sem_t* tsem);

/** Reset the value of the semaphore
 * 
 * @param tsem the semaphore to reset
 */
void tcc_dxb_sem_reset(tcc_dxb_sem_t* tsem);

/** Wait on the condition.
 * 
 * @param tsem the semaphore to wait
 */
void tcc_dxb_sem_wait(tcc_dxb_sem_t* tsem);

/** Signal the condition,if waiting
 * 
 * @param tsem the semaphore to signal
 */
void tcc_dxb_sem_signal(tcc_dxb_sem_t* tsem);

int tcc_dxb_sem_down_timewait(tcc_dxb_sem_t* tsem,int expire_time);

#ifdef __cplusplus
}
#endif

#endif
