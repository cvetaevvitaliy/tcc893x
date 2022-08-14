/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <fs_mgr.h>

#define LOG_TAG "Vold"

#include "cutils/klog.h"
#include "cutils/log.h"
#include "cutils/properties.h"

#include "VolumeManager.h"
#include "CommandListener.h"
#include "NetlinkManager.h"
#include "DirectVolume.h"
#include "NetworkVolume.h"
#include "cryptfs.h"

#include <cutils/properties.h>

#define USB_OTG_PATH  "/sys/class/drd_otg/drd0/drdmode"
#define USB_DEF_MODE_PATH  	"/sys/class/drd_otg/drd0/defmode"
static int process_config(VolumeManager *vm);
static void coldboot(const char *path);
pthread_t	uTh;

#define FSTAB_PREFIX "/fstab."
struct fstab *fstab;

void *usb_mode_thread(void *arg)
{
	int ret;
	char usb_mode[PROPERTY_VALUE_MAX];
	char old_mode[PROPERTY_VALUE_MAX];
	char pre_mode[PROPERTY_VALUE_MAX];
	char usb_conf[PROPERTY_VALUE_MAX];
	char boot_exit[PROPERTY_VALUE_MAX];
	char cmd[1000];

   FILE *fp;
   char drd_mode[256];
	bool loop = true;

	SLOGI("start USB mode thread");

	while (loop) {

					property_get("service.bootanim.exit", boot_exit, "");

					if (!strcmp(boot_exit, "1")) {
									loop = false;
					} else {
									usleep(2000 * 2000);
									continue;
					}
	}




   while (1) {
      // Old S/W mode
		strncpy(old_mode, usb_mode, PROPERTY_VALUE_MAX);
      // Old H/W mode
      strncpy(pre_mode, drd_mode, sizeof(pre_mode));
         
      // Get new S/W mode
		property_get("sys.usb.mode", usb_mode, "");
      property_get("persist.sys.usb.config", usb_conf, "");
#if 0
      // Get new H/W mode
      if (!(fp = fopen(USB_OTG_PATH, "r"))) {
         SLOGE("Error opening USB OTG (%s)", strerror(errno));
         usleep(1000 * 1000);
         continue;
      }

      fgets(drd_mode, sizeof(drd_mode), fp);

      if (fp)
         fclose(fp);
#endif
      // S/W and H/W mode match
		if (!strcmp(old_mode, usb_mode) && !strcmp(pre_mode, drd_mode)) {
			usleep(2000 * 1000);
			//SLOGI("-> drd satus: old_mode:%s usb_mode:%s pre_mode:%s drd_mode:%s", old_mode, usb_mode, pre_mode, drd_mode);
			continue;
		}

      usleep(1000 * 1000);

      property_set("sys.drd.mode", drd_mode);

      if (!strcmp(pre_mode, "usb_host") && !strcmp(drd_mode, "usb_device")) {
         ret = system("/system/bin/am broadcast -a android.intent.action.POPUP_CLOSE");
         if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
            SLOGE("USB Host Popup Close failed. %d.", ret);
            continue;
         }
			SLOGI("USB Host Popup window closed.");
			continue;
      }

      if (!strcmp(drd_mode, "usb_host") && !strcmp(usb_mode, "usb_device")) {
         usleep(1000 * 1000);
         SLOGI("Popup Window: old_mode:%s usb_mode:%s pre_mode:%s drd_mode:%s", old_mode, usb_mode, pre_mode, drd_mode);
#if 1
         if (!strcmp(old_mode, "") && !strcmp(pre_mode, "")) {
            /////////////////////////////////////////////////////////////////
            SLOGE("USB Host detection in System Booting.");
            /////////////////////////////////////////////////////////////////

				ret = system("insmod /lib/modules/udc-core.ko");
				if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB host to device switching fail(2) %d.", ret);
					continue;
				}

				ret = system("insmod /lib/modules/dwc_usb3.ko");
				if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB host to device switching fail(3) %d.", ret);
					continue;
				}

				ret = system("insmod /lib/modules/dwc3-tcc.ko");
				if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB host to device switching fail(4) %d.", ret);
					continue;
				}

				ret = system("insmod /lib/modules/g_android.ko");
				if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB host to device switching fail(5) %d.", ret);
					continue;
				}

				if (!strcmp(usb_conf, "mtp,adb")) {
					ret = system("setprop persist.sys.usb.config mtp,adb");
				} else if (!strcmp(usb_conf, "mtp")) {
					ret = system("setprop persist.sys.usb.config mtp");
				} else if (!strcmp(usb_conf, "mass_storage,adb")) {
					ret = system( "setprop persist.sys.usb.config mass_storage,adb");
				} else if (!strcmp(usb_conf, "mass_storage")) {
					ret = system("setprop persist.sys.usb.config mass_storage");
				} else if (!strcmp(usb_conf, "accessory,adb")) {
					ret = system("setprop persist.sys.usb.config accessory,adb");
				} else if (!strcmp(usb_conf, "accessory")) {
					ret = system("setprop persist.sys.usb.config accessory");
				}

				if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB device to host switching fail(6) %d.", ret);
					continue;
				}

				usleep(1000 * 1000);

				ret = system("start adbd");

				if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB device to host switching fail(5) %d.", ret);
					continue;
				}
            continue;
         }
#endif
         ret = system("/system/bin/am start -a android.intent.action.MAIN -n android/com.android.server.HostPopup");
         if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
            SLOGE("USB Host Popup failed. %d.", ret);
            continue;
         }
			continue;
      }

      SLOGI("Check usb mode..");

      if (!strcmp(usb_mode, "usb_host")) {
			SLOGI("Current usb mode is host.");

			if (1) { //(!strcmp(drd_mode, "usb_host")) {

				ret = system("stop adbd");
				if (WIFSIGNALED(ret)
						&& (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB device to host switching fail(1) %d.", ret);
					continue;
				}

				usleep(1000 * 1000);

				ret = system("rmmod g_android");
				if (WIFSIGNALED(ret)
						&& (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB device to host switching fail(2) %d.", ret);
					continue;
				}

				ret = system("rmmod dwc3_tcc");
				if (WIFSIGNALED(ret)
						&& (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB device to host switching fail(3) %d.", ret);
					continue;
				}

				ret = system("rmmod dwc_usb3");
				if (WIFSIGNALED(ret)
						&& (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB device to host switching fail(4) %d.", ret);
					continue;
				}

				ret = system("rmmod udc_core");
				if (WIFSIGNALED(ret)
						&& (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB device to host switching fail(5) %d.", ret);
					continue;
				}

				ret = system("insmod /lib/modules/xhci-hcd.ko");
				if (WIFSIGNALED(ret)
						&& (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB device to host switching fail(6) %d.", ret);
					continue;
				}

         } 

		} else if (!strcmp(usb_mode, "usb_device")) {
			SLOGI("Current usb mode is device.");

			if (1) { //(!strcmp(drd_mode, "usb_device")) {
						
            // waiting detatch device
            usleep(2000 * 1000);

            // check port status
		      //property_get("sys.drd.mode", drd_mode, "");

			   //if (!strcmp(drd_mode, "usb_host")) 
            //   continue;

				ret = system("rmmod xhci-hcd");
				if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB host to device switching fail(1) %d.", ret);
					continue;
				}

				ret = system("insmod /lib/modules/udc-core.ko");
				if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB host to device switching fail(2) %d.", ret);
					continue;
				}

				ret = system("insmod /lib/modules/dwc_usb3.ko");
				if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB host to device switching fail(3) %d.", ret);
					continue;
				}

				ret = system("insmod /lib/modules/dwc3-tcc.ko");
				if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB host to device switching fail(4) %d.", ret);
					continue;
				}

				ret = system("insmod /lib/modules/g_android.ko");
				if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB host to device switching fail(5) %d.", ret);
					continue;
				}

				//sscanf(cmd, "setprop persist.sys.usb.config %s\n", usb_conf);
				//ret = system(cmd); 			

				if (!strcmp(usb_conf, "mtp,adb")) {
					ret = system("setprop persist.sys.usb.config mtp,adb");
				} else if (!strcmp(usb_conf, "mtp")) {
					ret = system("setprop persist.sys.usb.config mtp");
				} else if (!strcmp(usb_conf, "mass_storage,adb")) {
					ret = system( "setprop persist.sys.usb.config mass_storage,adb");
				} else if (!strcmp(usb_conf, "mass_storage")) {
					ret = system("setprop persist.sys.usb.config mass_storage");
				} else if (!strcmp(usb_conf, "accessory,adb")) {
					ret = system("setprop persist.sys.usb.config accessory,adb");
				} else if (!strcmp(usb_conf, "accessory")) {
					ret = system("setprop persist.sys.usb.config accessory");
				}

				if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB host to device switching fail(6) %d.", ret);
					continue;
				}
				usleep(1000 * 1000);

				ret = system("start adbd");
				if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
					SLOGE("USB host to device switching fail(5) %d.", ret);
					continue;
				}

			}
		}

		usleep(1000 * 1000);
	}

   if (fp)
      fclose(fp);
	pthread_exit(NULL);
	SLOGI("USB mode thread exit.");
	return NULL;
}

int get_default_usb_drd_mode()
{
	FILE *fp;
	char def_mode[256];
	
	// Get default drd mode
	if (!(fp = fopen(USB_DEF_MODE_PATH, "r"))) {
		SLOGE("Error opening USB_DEF_MODE_PATH (%s)", strerror(errno));
		return 0; // device mode
	}else{
		fgets(def_mode, sizeof(def_mode), fp);
		fclose(fp);
	}

	/* 0: device, 1: host */
	return (strcmp(def_mode, "usb_host")==0)?1:0;
}

int main() {

    VolumeManager *vm;
    CommandListener *cl;
    NetlinkManager *nm;
    int ret;
	int host_mode = 0;
    char mode[PROPERTY_VALUE_MAX];	
    char chip[PROPERTY_VALUE_MAX];	

    SLOGI("Vold 2.1 (the revenge) firing up");

    mkdir("/dev/block/vold", 0755);

    /* For when cryptfs checks and mounts an encrypted filesystem */
    klog_set_level(6);

    /* Create our singleton managers */
    if (!(vm = VolumeManager::Instance())) {
        SLOGE("Unable to create VolumeManager");
        exit(1);
    };

    if (!(nm = NetlinkManager::Instance())) {
        SLOGE("Unable to create NetlinkManager");
        exit(1);
    };


    cl = new CommandListener();
    vm->setBroadcaster((SocketListener *) cl);
    nm->setBroadcaster((SocketListener *) cl);

    if (vm->start()) {
        SLOGE("Unable to start VolumeManager (%s)", strerror(errno));
        exit(1);
    }

    if (process_config(vm)) {
        SLOGE("Error reading configuration (%s)... continuing anyways", strerror(errno));
    }

    if (nm->start()) {
        SLOGE("Unable to start NetlinkManager (%s)", strerror(errno));
        exit(1);
    }

    coldboot("/sys/block");
//    coldboot("/sys/class/switch");

    /*
     * Now that we're up, we can respond to commands
     */
    if (cl->startListener()) {
        SLOGE("Unable to start CommandListener (%s)", strerror(errno));
        exit(1);
    }

    property_get("ro.usb.switch", mode, "");	
    property_get("ro.chip.product", chip, "");	

    if (!strcmp(mode, "otg") && strcmp(chip, "tcc8935s")) {		
       SLOGI("USB OTG Switching Enabled.");

		host_mode = get_default_usb_drd_mode();
		SLOGI("drd %s mode detecting...", (host_mode)?"host":"device");
		
      if(host_mode){
         ret = system("setprop sys.usb.mode usb_host");
      }else{
         ret = system("setprop sys.usb.mode usb_device"); 			
      }
		
       if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
          SLOGE("USB device mode set fail (%d).", ret);						
       }

       if (pthread_create(&uTh, NULL, usb_mode_thread, NULL)) {
          SLOGE("pthread_create (%s)", strerror(errno));
          return -1;
	    }
	}
    // Eventually we'll become the monitoring thread
    while(1) {
        sleep(1000);
    }

    SLOGI("Vold exiting");
    exit(0);
}

static void do_coldboot(DIR *d, int lvl)
{
    struct dirent *de;
    int dfd, fd;

    dfd = dirfd(d);

    fd = openat(dfd, "uevent", O_WRONLY);
    if(fd >= 0) {
        write(fd, "add\n", 4);
        close(fd);
    }

    while((de = readdir(d))) {
        DIR *d2;

        if (de->d_name[0] == '.')
            continue;

        if (de->d_type != DT_DIR && lvl > 0)
            continue;

        fd = openat(dfd, de->d_name, O_RDONLY | O_DIRECTORY);
        if(fd < 0)
            continue;

        d2 = fdopendir(fd);
        if(d2 == 0)
            close(fd);
        else {
            do_coldboot(d2, lvl + 1);
            closedir(d2);
        }
    }
}

static void coldboot(const char *path)
{
    DIR *d = opendir(path);
    if(d) {
        do_coldboot(d, 0);
        closedir(d);
    }
}

static int process_config(VolumeManager *vm)
{
    char fstab_filename[PROPERTY_VALUE_MAX + sizeof(FSTAB_PREFIX)];
    char propbuf[PROPERTY_VALUE_MAX];
    int i, j;
    int ret = -1;
    int flags;

    property_get("ro.hardware", propbuf, "");
    snprintf(fstab_filename, sizeof(fstab_filename), FSTAB_PREFIX"%s", propbuf);

    property_get("ro.bootmode", propbuf, "");
    if (!strcmp(propbuf, "emmc"))
      sprintf(fstab_filename, "%s.%s", fstab_filename, propbuf);
    
    fstab = fs_mgr_read_fstab(fstab_filename);
    if (!fstab) {
        SLOGE("failed to open %s\n", fstab_filename);
        return -1;
    }

    /* Loop through entries looking for ones that vold manages */
    for (i = 0; i < fstab->num_entries; i++) {
        if (fs_mgr_is_voldmanaged(&fstab->recs[i])) {
            DirectVolume *dv = NULL;
            flags = 0;

            if (!strcmp(fstab->recs[i].fs_type, "nfs")) {
               NetworkVolume *dv = NULL;

               dv = new NetworkVolume(vm, &(fstab->recs[i]), flags);

               vm->addVolume(dv);

               continue;
            }
            /* Only use this flag in the box of telechips */
            if (fs_mgr_is_nofuse(&fstab->recs[i])) {
                flags |= VOL_NOFUSE;
            }
            /* Set any flags that might be set for this volume */
            if (fs_mgr_is_nonremovable(&fstab->recs[i])) {
                flags |= VOL_NONREMOVABLE;
            }
            if (fs_mgr_is_encryptable(&fstab->recs[i])) {
                flags |= VOL_ENCRYPTABLE;
            }
            /* Only set this flag if there is not an emulated sd card */
            if (fs_mgr_is_noemulatedsd(&fstab->recs[i]) &&
                !strcmp(fstab->recs[i].fs_type, "vfat")) {
                flags |= VOL_PROVIDES_ASEC;
            }
            dv = new DirectVolume(vm, &(fstab->recs[i]), flags);

            if (dv->addPath(fstab->recs[i].blk_device)) {
                SLOGE("Failed to add devpath %s to volume %s",
                      fstab->recs[i].blk_device, fstab->recs[i].label);
                goto out_fail;
            }

	    for (j=0; j<BLK_DEVICE2_NUM; j++) {
	      if (strcmp(fstab->recs[i].blk_device2[j], NO_BLK_DEVICE2)) {
		if (dv->addPath(fstab->recs[i].blk_device2[j])) {
		  SLOGE("Failed to add devpath %s", fstab->recs[i].blk_device2[j]);
		  goto out_fail;
		}
	      }
	    }

            vm->addVolume(dv);
        }
    }

    ret = 0;

out_fail:
    return ret;
}
