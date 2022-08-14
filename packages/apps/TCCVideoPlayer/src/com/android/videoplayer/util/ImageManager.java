/*
 * Copyright (C) 2007 The Android Open Source Project
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
import android.os.Parcel;
import android.os.Parcelable;
import android.media.MediaFile;

import java.io.File;
import java.util.HashMap;

/**
 * ImageManager is used to retrieve and store images
 * in the media content provider.
 */
public class ImageManager {
    private static final String TAG = "ImageManager";

    // Inclusion
    public static final int INCLUDE_VIDEO = (1 << 0);
	public static final int INCLUDE_CAPTION = (1 << 1);
	public static final int INCLUDE_IMAGE = (1 << 2);

    // Sort
    public static final int SORT_ASCENDING = 1;
    public static final int SORT_DESCENDING = 2;

    // Caption
    public static final int CAPTION_SAMI_TYPE      = 1;
    public static final int CAPTION_SUBRIP_TYPE    = 2;
    public static final int CAPTION_SUBVIEWER_TYPE = 3;
    public static final int CAPTION_SSA_TYPE       = 4;

    private static HashMap<String, Integer> sCaptionMap 
            = new HashMap<String, Integer>();
    static {
        sCaptionMap.put("SMI", Integer.valueOf(CAPTION_SAMI_TYPE));
        sCaptionMap.put("SAMI",Integer.valueOf(CAPTION_SAMI_TYPE));
        sCaptionMap.put("SRT",Integer.valueOf(CAPTION_SUBRIP_TYPE));
        sCaptionMap.put("SUB",Integer.valueOf(CAPTION_SUBVIEWER_TYPE));
        sCaptionMap.put("SSA",Integer.valueOf(CAPTION_SSA_TYPE));
    }

    public static class ImageListParam implements Parcelable {
        public int mInclusion;
        public int mSort;
        public String mBucketId;

        // This is only used if we are creating a single image list.
        public Uri mSingleImageUri;

        // This is only used if we are creating an empty image list.
        public boolean mIsEmptyImageList;

        public ImageListParam() {}

        public void writeToParcel(Parcel out, int flags) {
            out.writeInt(mInclusion);
            out.writeInt(mSort);
            out.writeString(mBucketId);
            out.writeParcelable(mSingleImageUri, flags);
            out.writeInt(mIsEmptyImageList ? 1 : 0);
        }

        private ImageListParam(Parcel in) {
            mInclusion = in.readInt();
            mSort = in.readInt();
            mBucketId = in.readString();
            mSingleImageUri = in.readParcelable(null);
            mIsEmptyImageList = (in.readInt() != 0);
        }

        public String toString() {
            return String.format("ImageListParam{inc=%d,sort=%d," +
                "bucket=%s,empty=%b,single=%s}", mInclusion,
                mSort, mBucketId, mIsEmptyImageList, mSingleImageUri);
        }

        public static final Parcelable.Creator CREATOR
                = new Parcelable.Creator() {
            public ImageListParam createFromParcel(Parcel in) {
                return new ImageListParam(in);
            }

            public ImageListParam[] newArray(int size) {
                return new ImageListParam[size];
            }
        };

        public int describeContents() {
            return 0;
        }
    }

	// This is the factory function to create an image list.
    public static ImageList makeImageList(ImageListParam param, String path) {
        int inclusion = param.mInclusion;
        int sort = param.mSort;
        String bucketId = param.mBucketId;

        // use this code to merge videos and stills into the same list
        ImageList l = new ImageList();

        File[] files = new File(path).listFiles();

        if (files == null) return l;

        for(int i = 0; i < files.length; i++) {
            File f = files[i];
            if(!f.isDirectory()) {
            	if (isVideoFile(f.getName())) {
            		if ((inclusion & INCLUDE_VIDEO) != 0) l.addImage(f);
                } else if (isCaptionFile(f.getName())) {
                    if ((inclusion & INCLUDE_CAPTION) != 0) l.addImage(f);
            	} else if (isImageFile(f.getName())) {
            		if ((inclusion & INCLUDE_IMAGE) != 0) l.addImage(f);
            	}
            }
		}
        return l;
    }

    public static boolean isVideoFile(String path) {
        boolean ret = false;
        MediaFile.MediaFileType mediaFileType = MediaFile.getFileType(path);
        if (mediaFileType == null) return false;

        int fileType = mediaFileType.fileType;

        if (MediaFile.isVideoFileType(fileType)) {
            ret = true;
        }
        return ret;
    }

    public static boolean isImageFile(String path) {
        boolean ret = false;
        MediaFile.MediaFileType mediaFileType = MediaFile.getFileType(path);
        if (mediaFileType == null) return false;

        int fileType = mediaFileType.fileType;

        if (MediaFile.isImageFileType(fileType)) {
            ret = true;
        }
        return ret;
    }

    public static boolean isCaptionFile(String path) {
        int fileType=0;
        int lastDot = path.lastIndexOf(".");
        if (lastDot < 0) return false;
        Integer value = sCaptionMap.get(path.substring(lastDot + 1).toUpperCase());
        fileType = (value == null ? 0 : value.intValue());
        return (fileType >= CAPTION_SAMI_TYPE &&fileType <= CAPTION_SSA_TYPE);
    }

    public static boolean isApkFile(String path) {
        int lastDot = path.lastIndexOf(".");
        if (lastDot < 0) return false;
        return "APK".equals(path.substring(lastDot + 1).toUpperCase());
    }

    public static ImageListParam getImageListParam(int inclusion, int sort, String bucketId) {
         return getImageListParam(inclusion, sort, bucketId, null);
    }

    public static ImageListParam getImageListParam(int inclusion, int sort, String bucketId, String limit) {
         ImageListParam param = new ImageListParam();
         param.mInclusion = inclusion;
         param.mSort = sort;
         param.mBucketId = bucketId;
         return param;
    }
}
