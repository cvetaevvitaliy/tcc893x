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

#include <media/stagefright/MediaSource.h>

namespace android {

MediaSource::MediaSource() {}

MediaSource::~MediaSource() {}

////////////////////////////////////////////////////////////////////////////////

MediaSource::ReadOptions::ReadOptions() {
    reset();
}

void MediaSource::ReadOptions::reset() {
    mOptions = 0;
    mSeekTimeUs = 0;
    mLatenessUs = 0;
    mReset = false;
}

void MediaSource::ReadOptions::setSeekTo(int64_t time_us, SeekMode mode) {
    mOptions |= kSeekTo_Option;
    mSeekTimeUs = time_us;
    mSeekMode = mode;
}

void MediaSource::ReadOptions::clearSeekTo() {
    mOptions &= ~kSeekTo_Option;
    mSeekTimeUs = 0;
    mSeekMode = SEEK_CLOSEST_SYNC;
}

bool MediaSource::ReadOptions::getSeekTo(
        int64_t *time_us, SeekMode *mode) const {
    *time_us = mSeekTimeUs;
    *mode = mSeekMode;
    return (mOptions & kSeekTo_Option) != 0;
}

void MediaSource::ReadOptions::setLateBy(int64_t lateness_us) {
    mLatenessUs = lateness_us;
}

int64_t MediaSource::ReadOptions::getLateBy() const {
    return mLatenessUs;
}

void MediaSource::ReadOptions::setReset(bool flag) {
	mOptions |= kReset_Option;
}

void MediaSource::ReadOptions::setCurPos(int64_t cur_pos_us) {
    mCurrentPos = cur_pos_us;
}

int64_t MediaSource::ReadOptions::getCurPos() const {
    return mCurrentPos;
}

bool MediaSource::ReadOptions::getReset(bool *flag) const {
	*flag = mReset;
	return (mOptions & kReset_Option) != 0;
}

void MediaSource::ReadOptions::clearReset() {
	mOptions &= ~kReset_Option;
	mReset = false;
}

void MediaSource::ReadOptions::setPlayRate(int32_t rate) {
	mOptions |= kPlayRate_Option;
	mPlayRate = rate;
}

bool MediaSource::ReadOptions::getPlayRate(int32_t *rate) const {
	*rate = mPlayRate;
	return (mOptions & kPlayRate_Option) != 0;
}

void MediaSource::ReadOptions::clearPlayRate() {
	mOptions &= ~kPlayRate_Option;
	mPlayRate = 0;
}

void MediaSource::ReadOptions::enablePreventSeekBeyondLastKey() {
    mPreventSeekBeyondLastKey = true;

}

void MediaSource::ReadOptions::disablePreventSeekBeyondLastKey() {
    mPreventSeekBeyondLastKey = false;
}

bool MediaSource::ReadOptions::getParamPreventSeekBeyondLastKey() const {
    return mPreventSeekBeyondLastKey;
}

}  // namespace android
