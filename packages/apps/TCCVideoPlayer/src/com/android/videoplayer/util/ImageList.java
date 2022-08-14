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

import java.io.File;

/**
 * Represents an ordered collection of Image objects. Provides an API to add
 * and remove an image.
 */
public class ImageList {
    private static final String TAG = "ImageList";

    private static final int CACHE_CAPACITY = 512;
    private final LruCache<Integer, Image> mCache =
            new LruCache<Integer, Image>(CACHE_CAPACITY);
    private static int mCount;

    public ImageList() {
        mCount = 0;
    }

    public int addImage(File f) {
        Image image = new Image(this, mCount, Uri.fromFile(f), f.getPath(), f.getName());
        mCache.put(mCount, image);
        return mCount++;
    }

    public Image getImageAt(int i) {
        Image result = mCache.get(i);
        return result;
    }

    public Image getImageForUri(Uri uri) {
        for (int i = 0; i < mCount; i++) {
            Image image = mCache.get(i);
            if (image != null) {
                if (image.fullSizeImageUri().equals(uri)) {
                    return image;
                }
            }
        }
        return null;
    }

    public int getImageIndex(Image image) {
        return image.getIndex();
    }

    public int getCount() {
        return mCount;
    }
}
