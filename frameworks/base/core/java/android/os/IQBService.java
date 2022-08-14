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

package android.os;

import android.os.Binder;
import android.os.IBinder;
import android.os.IInterface;
import android.os.Parcel;
import android.os.RemoteException;

/**
 * WARNING! Update IQBService.h and IQBService.cpp if you change this
 * file. In particular, the ordering of the methods below must match the
 * _TRANSACTION enum in IQBService.cpp
 * 
 * @hide - Applications should use android.os to access
 *       storage functions.
 */
public interface IQBService extends IInterface {
    /** Local-side IPC implementation stub class. */
    public static abstract class Stub extends Binder implements IQBService {
        private static class Proxy implements IQBService {
            private IBinder mRemote;

            Proxy(IBinder remote) {
                mRemote = remote;
            }

            public IBinder asBinder() {
                return mRemote;
            }

            public int doHibernateReboot() throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    mRemote.transact(Stub.TRANSACTION_doHibernateReboot, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }
            
            public int doHibernateImageCreate() throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    mRemote.transact(Stub.TRANSACTION_doHibernateImageCreate, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }
            
            public int doHibernateImageErase() throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    mRemote.transact(Stub.TRANSACTION_doHibernateImageErase, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }
            
            public int doHibernateImageRestore() throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    mRemote.transact(Stub.TRANSACTION_doHibernateImageRestore, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }
            
            public int doHibernatePreprocess() throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    mRemote.transact(Stub.TRANSACTION_doHibernatePreprocess, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }
            
            public int SetJobTimeout(java.lang.String title, int timeout) throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                	_data.writeInterfaceToken(DESCRIPTOR);
                	_data.writeString(title);
                	_data.writeInt(timeout);
                    mRemote.transact(Stub.TRANSACTION_SetJobTimeout, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }
            
            public int SetJobFinish() throws RemoteException {
                Parcel _data = Parcel.obtain();
                Parcel _reply = Parcel.obtain();
                int _result;
                try {
                    mRemote.transact(Stub.TRANSACTION_SetJobFinish, _data, _reply, 0);
                    _reply.readException();
                    _result = _reply.readInt();
                } finally {
                    _reply.recycle();
                    _data.recycle();
                }
                return _result;
            }
        }

        private static final String DESCRIPTOR = "IQBService";

        static final int TRANSACTION_doHibernateReboot = IBinder.FIRST_CALL_TRANSACTION + 0;
        static final int TRANSACTION_doHibernateImageCreate = TRANSACTION_doHibernateReboot + 1;
        static final int TRANSACTION_doHibernateImageErase = TRANSACTION_doHibernateImageCreate + 1;
        static final int TRANSACTION_doHibernateImageRestore = TRANSACTION_doHibernateImageErase + 1;       
        static final int TRANSACTION_doHibernatePreprocess = TRANSACTION_doHibernateImageRestore + 1;
        static final int TRANSACTION_SetJobTimeout = TRANSACTION_doHibernatePreprocess + 1;
        static final int TRANSACTION_SetJobFinish = TRANSACTION_SetJobTimeout + 1;
        /**
         * Cast an IBinder object into an IQBService interface, generating a
         * proxy if needed.
         */
        public static IQBService asInterface(IBinder obj) {
            if (obj == null) {
                return null;
            }
            IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
            if (iin != null && iin instanceof IQBService) {
                return (IQBService) iin;
            }
            return new IQBService.Stub.Proxy(obj);
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
                case TRANSACTION_doHibernateReboot: {
                    int result =  doHibernateReboot();
                    reply.writeNoException();
                    reply.writeInt(result);
                    return true;
                }
                case TRANSACTION_doHibernateImageCreate: {
                    int result =  doHibernateImageCreate();
                    reply.writeNoException();
                    reply.writeInt(result);
                    return true;
                }
                case TRANSACTION_doHibernateImageErase: {
                    int result =  doHibernateImageErase();
                    reply.writeNoException();
                    reply.writeInt(result);
                    return true;
                }
                
                case TRANSACTION_doHibernateImageRestore: {
                    int result =  doHibernateImageRestore();
                    reply.writeNoException();
                    reply.writeInt(result);
                    return true;
                }
                
                case TRANSACTION_doHibernatePreprocess: {
                	 int result =  doHibernatePreprocess();
                     reply.writeNoException();
                     reply.writeInt(result);
                     return true;
                }
                
				case TRANSACTION_SetJobTimeout: {
					data.enforceInterface(DESCRIPTOR);
					java.lang.String _arg0;
					_arg0 = data.readString();
	
					int _arg1 = data.readInt();
	
					int result = SetJobTimeout(_arg0, _arg1);
					reply.writeNoException();
					reply.writeInt(result);
					return true;
				}
				
				case TRANSACTION_SetJobFinish: {
					int result = SetJobFinish();
					reply.writeNoException();
					reply.writeInt(result);
					return true;
				}
            
            }
            return super.onTransact(code, data, reply, flags);
        }
    }
   
    public int doHibernateReboot() throws RemoteException;
    public int doHibernateImageCreate() throws RemoteException;
    public int doHibernateImageErase() throws RemoteException;
    public int doHibernateImageRestore() throws RemoteException;
    public int doHibernatePreprocess() throws RemoteException;
    
    public int SetJobTimeout(String title, int timeout) throws RemoteException;
    public int SetJobFinish() throws RemoteException;
}
