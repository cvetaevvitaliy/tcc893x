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

package com.android.videoplayer.util;

import android.net.Uri;

public class Image {
    private static final String TAG = "Image";

    private final int mIndex;
    private final Uri mUri;
    private final String mDataPath;
    private final String mTitle;

    public Image(ImageList container, int index, Uri uri, String dataPath, String title) {
        mIndex = index;
        mUri = uri;
        mDataPath = dataPath;
        mTitle = title;
    }

    public int getIndex() {
        return mIndex;
    }

    public Uri fullSizeImageUri() {
        return mUri;
    }

    public String getTitle() {
        return mTitle;
    }
}
