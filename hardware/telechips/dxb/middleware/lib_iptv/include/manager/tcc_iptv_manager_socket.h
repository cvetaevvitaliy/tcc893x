/*
 * Copyright (C) 2013 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef	_TCC_DVB_MANAGER_SOCKET_H_
#define	_TCC_DVB_MANAGER_SOCKET_H_

#include "tcc_dxb_interface_type.h"

#ifdef __cplusplus
extern "C" {
#endif

extern DxB_ERR_CODE tcc_manager_socket_init(void);
extern DxB_ERR_CODE tcc_manager_socket_ipsetting(unsigned char* ip, unsigned int portnum, int protocol);
extern DxB_ERR_CODE tcc_manager_socket_start(void);
extern DxB_ERR_CODE tcc_manager_socket_stop(void);
extern void tcc_manager_socket_senddata(unsigned char *data_ptr, unsigned int data_size);
extern DxB_ERR_CODE tcc_manager_socket_command(int cmd);

#ifdef __cplusplus
}
#endif

#endif


