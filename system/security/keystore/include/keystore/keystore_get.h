/*
 * Copyright (C) 2009 The Android Open Source Project
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

#ifndef __KEYSTORE_GET_H__
#define __KEYSTORE_GET_H__

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <cutils/sockets.h>

#define KEYSTORE_MESSAGE_SIZE 65535

#ifdef __cplusplus
extern "C" {
#endif

/* This function is provided for native components to get values from keystore.
 * Users are required to link against libkeystore_binder.
 *
 * Keys and values are 8-bit safe. The first two arguments are the key and its
 * length. The third argument is a pointer to an array that will be malloc()
 * and the caller is responsible for calling free() on the buffer.
 */
ssize_t keystore_get(const char *key, size_t length, uint8_t** value);

#ifdef KEY_GET_TEMP
static int keystore_get_temp(const char *key, int length, char *value)
{
    uint8_t bytes[2] = {(uint8_t)(length >> 8), (uint8_t)length};
    uint8_t code = 'g';
    int sock;

    if (length < 0 || length > KEYSTORE_MESSAGE_SIZE) {
        return -1;
    }
    sock = socket_local_client("keystore", ANDROID_SOCKET_NAMESPACE_RESERVED,
                               SOCK_STREAM);
    if (sock == -1) {
        return -1;
    }
    if (send(sock, &code, 1, 0) == 1 && send(sock, bytes, 2, 0) == 2 &&
        send(sock, key, length, 0) == length && shutdown(sock, SHUT_WR) == 0 &&
        recv(sock, &code, 1, 0) == 1 && code == /* NO_ERROR */ 1 &&
        recv(sock, &bytes[0], 1, 0) == 1 && recv(sock, &bytes[1], 1, 0) == 1) {
        int offset = 0;
        length = bytes[0] << 8 | bytes[1];
        while (offset < length) {
            int n = recv(sock, &value[offset], length - offset, 0);
            if (n <= 0) {
                length = -1;
                break;
            }
            offset += n;
        }
    } else {
        length = -1;
    }

    close(sock);
    return length;
}
#endif

#ifdef __cplusplus
}
#endif

#endif
