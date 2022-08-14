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

    private native int init(int iDxbType, int iFreq, int iBW, int iVoltage, int iMOD) throws IllegalStateException;
    private native int deinit() throws IllegalStateException;
    private native int start(int[] piPidTable, String pcFile, int iDumpSize) throws IllegalStateException;
    private native int stop() throws IllegalStateException;
    private native int sockstart(String iIp, int iPort, String pcFile, int iDumpSize) throws IllegalStateException;
    private native int getSize() throws IllegalStateException;
    private native int getFreeSpace(String pcPath) throws IllegalStateException;

    public static void main(String[] args) {
        try {
            (new DtvDump()).run(args);
        } catch (IllegalArgumentException e) {
            showUsage();
            System.err.println("Error: " + e.getMessage());
        } catch (Exception e) {
            e.printStackTrace(System.err);
            System.exit(1);
        }
    }

    private void run(String[] args) throws Exception {
        if (args.length < 5) {
            showUsage();
            return;
        }
        int j = 0;
        int dxbtype = Integer.parseInt(args[j++]);
        int voltage = 0;
        if (dxbtype == 2) {
            voltage = Integer.parseInt(args[j++]);
        }
        int freq = Integer.parseInt(args[j++]);
        int bw = Integer.parseInt(args[j++]);
        int dump_size = Integer.parseInt(args[j++]);
        int modualtion = Integer.parseInt(args[j++]);		
        String file_name = args[j++];
        int i;
        int[] PID;
        if (args.length == j) {
            // all pid dump
            PID = new int[1];
            PID[0] = 0x2000;
        } else {
            PID = new int[args.length - j];
            for (i = j; i < args.length; i++) {
                PID[i-j] = Integer.parseInt(args[i]);
            }
        }

        if (init(dxbtype, freq, bw, voltage, modualtion) != 0) {
            System.err.println("Failed to initialize tuner");
            return;
        }

        if (start(PID, file_name, dump_size) == 0) {
            while(true) {
                int currSize = getSize();
                System.err.println("W : " + currSize + " , " + dump_size + "\n");
                if (currSize >= dump_size)
                    break;
                Thread.sleep(1000);
            }
            stop();
        }
        deinit();
    }

    private static void showUsage() {
        System.err.println(
            "Usage: dtvdump dxbtype frequency bandwidth dumpsize file [pid] [pid] ...\n" +
            "       dxbtype(0: dvbt, 1: isdbt, 2: dvbs)\n" +
            "       [voltage(0: 13v, 1: 18v, 2: 0v)]\n" +
            "       frequency, bandwidth is Khz unit.\n" +
            "       dumpsize is Kbyte unit.\n" +
            "       if there are no pids as an input arguments, save whole ts stream.\n"
                );
    }
}
