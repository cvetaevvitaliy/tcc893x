/*
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/


package com.telechips.android.dtvdump;

public class DtvDump {

    static {
        System.loadLibrary("dtvdump");
    }

    static public native int init(int iDxbType, int iFreq, int iBW, int iVoltage, int iMOD) throws IllegalStateException;
    static public native int deinit() throws IllegalStateException;
    static public native int start(int[] piPidTable, String pcFile, int iDumpSize) throws IllegalStateException;
    static public native int stop() throws IllegalStateException;
    static public native int sockstart(String iIp, int iPort, String pcFile, int iDumpSize) throws IllegalStateException;
    static public native int getSize() throws IllegalStateException;
    static public native int getFreeSpace(String pcPath) throws IllegalStateException;
}
