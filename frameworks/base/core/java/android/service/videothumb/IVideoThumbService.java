/*
 * Copyright (C) 2010 The Android Open Source Project
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

package android.service.videothumb;

import java.io.FileDescriptor;

import android.graphics.Bitmap;
import android.os.Binder;
import android.os.IBinder;
import android.os.IInterface;
import android.os.Parcel;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;

/**
 * WARNING! Update IQuickBootService.h and IQuickBootService.cpp if you change
 * this file. In particular, the ordering of the methods below must match the
 * _TRANSACTION enum in IQuickBootService.cpp
 * 
 * @hide - Applications should use android.os to access storage functions.
 */
public interface IVideoThumbService extends IInterface {
    /** Local-side IPC implementation stub class. */
    public static abstract class Stub extends Binder implements IVideoThumbService {
        private static class Proxy implements IVideoThumbService {
            private IBinder mRemote;

            Proxy(IBinder remote) {
                mRemote = remote;
            }

            public IBinder asBinder() {
                return mRemote;
            }

            public Bitmap createVideoThumbnail(FileDescriptor fd) throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                Bitmap _result = null;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeFileDescriptor(fd);
                    mRemote.transact(Stub.TRANSACTION_createVideoThumbnail, _data, _reply, 0);
                    _reply.readException();
                    if ((0 != _reply.readInt())) {
                        _result = android.graphics.Bitmap.CREATOR.createFromParcel(_reply);
                    }
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }

            public int pauseVideoThumbnail(int releaseMillis) throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result = 0;
                try {
                    _data.writeInterfaceToken(DESCRIPTOR);
                    _data.writeInt(releaseMillis);
                    mRemote.transact(Stub.TRANSACTION_pauseVideoThumbnail, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }
        }

        private static final String DESCRIPTOR = "IVideoThumbService";

        static final int TRANSACTION_createVideoThumbnail = IBinder.FIRST_CALL_TRANSACTION + 0;
        static final int TRANSACTION_pauseVideoThumbnail = TRANSACTION_createVideoThumbnail + 1;

        /**
         * Cast an IBinder object into an IQuickBootService interface,
         * generating a proxy if needed.
         */
        public static IVideoThumbService asInterface(IBinder obj) {
            if (obj == null) {
                return null;
            }
            IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
            if (iin != null && iin instanceof IVideoThumbService) {
                return (IVideoThumbService) iin;
            }
            return new IVideoThumbService.Stub.Proxy(obj);
        }

        /** Construct the stub at attach it to the interface. */
        public Stub() {
            attachInterface(this, DESCRIPTOR);
        }

        public IBinder asBinder() {
            return this;
        }

        @Override
        public boolean onTransact(int code, Parcel data, Parcel reply,
                int flags) throws RemoteException {
            switch (code) {
                case INTERFACE_TRANSACTION: {
                    reply.writeString(DESCRIPTOR);
                    return true;
                }
                case TRANSACTION_createVideoThumbnail: {
                	ParcelFileDescriptor fd;
                    
                    data.enforceInterface(DESCRIPTOR);
                    fd = data.readFileDescriptor();

                    Bitmap _result = createVideoThumbnail(fd.getFileDescriptor());

                    reply.writeNoException();
                    if ((_result != null)) {
                        reply.writeInt(1);
                        _result.writeToParcel(reply,
                                android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
                    }
                    else {
                        reply.writeInt(0);
                    }
                    return true;
                }
                case TRANSACTION_pauseVideoThumbnail: {
                    int _result = 0;
                    
                    data.enforceInterface(DESCRIPTOR);
                    int timeout = data.readInt();
                    
                    _result = pauseVideoThumbnail(timeout);
                    
                    reply.writeNoException();
                    reply.writeInt(_result);
                    return true;
                }
            }
            return super.onTransact(code, data, reply, flags);
        }
    }

    public Bitmap createVideoThumbnail(FileDescriptor fd) throws RemoteException;
    
    public int pauseVideoThumbnail(int releaseMillis) throws RemoteException;
}
