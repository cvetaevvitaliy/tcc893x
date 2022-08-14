/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/

/****************************************************************************

Revision History

****************************************************************************

****************************************************************************/
#define LOG_NDEBUG 0
#define LOG_TAG	"dxbserver"
#include <utils/Log.h>
#include <cutils/properties.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#if 0
extern int isdb_main(int argc, char **argv);
extern int dvb_main(int argc, char **argv);
#else
extern int dmb_main(int argc, char **argv);
#endif

using namespace android;

int getDxbType()
{
    char value[PROPERTY_VALUE_MAX];
    property_get("tcc.dxb.standard", value, "");
    return atoi(value);
}

int main(int argc, char **argv)
{
    sp < ProcessState > proc(ProcessState::self());
    sp < IServiceManager > sm = defaultServiceManager();

    system("am broadcast -a com.google.android.tv.mediadevices.START_SERVICE");
#if 0
    switch (getDxbType()) {
    case 1:
        isdb_main(argc, argv);
        break;
    case 2:
        dmb_main(argc, argv);
        break;
    case 3:
        dvb_main(argc, argv);
        break;
    }
#else
    dmb_main(argc, argv);
#endif
    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();

    return 0;
}
