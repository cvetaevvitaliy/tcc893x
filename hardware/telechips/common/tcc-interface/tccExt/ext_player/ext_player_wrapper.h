/**
 * Copyright (C) 2013, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef _W_PLAYER_H_
#define _W_PLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

int createExt(void *surf, int codec_info, size_t decodedWidth, size_t decodedHeight);
void destoryExt();
int startExt();
int stopExt();
int ExtProcess(const uint8_t *data, uint32_t size, int64_t timestamp_ms);

#ifdef __cplusplus
}
#endif
#endif
