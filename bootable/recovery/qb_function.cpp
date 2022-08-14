#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

#include <fs_mgr.h>
#include "bootloader.h"
#include "common.h"
#include "cutils/properties.h"
#include "cutils/android_reboot.h"
#include "install.h"
#include "minui/minui.h"
#include "minzip/DirUtil.h"
#include "roots.h"
#include "ui.h"
#include "screen_ui.h"
#include "device.h"
#include "adb_install.h"
extern "C" {
#include <sparse/sparse.h>
}

extern bool boot_mode_is_sdmmc;
extern RecoveryUI* ui;
extern int
get_menu_selection(const char* const * headers, const char* const * items,
                   int menu_only, int initial_selection, Device* device);
extern const char**
prepend_title(const char* const* headers);

extern const char *CACHE_ROOT;
extern const char *SDCARD_ROOT;
extern const char *INTERNAL_ROOT;
extern const char *USB_ROOT;
extern const char *QB_INTERNAL_ROOT;
extern const char *SYSTEM_ROOT;

const char* file_list[3][2] = {
  {"userdata.sparse.img", "userdata.sparse.img.gz"}, 
  {"cache.sparse.img", "cache.sparse.img.gz"}, 
  {"snapshot.sparse.img", "snapshot.sparse.img.gz"} 
};

enum PART_NAME {
  USERDATA,
  CACHE,
  SNAPSHOT
};

struct volume_list {
  char blk_label[512];
  Volume* list; 
};
struct volume_list v_head[3];

static void __do_convert_img2simg(const char *dev_mnt_point, int which)
{
  int in, out, ret;
  struct sparse_file *s;
  unsigned int block_size = 4096;
  char if_root[512], of_root[512];
  off64_t len;

  sprintf(if_root, "%s/%s", dev_mnt_point, v_head[which].blk_label);
  sprintf(of_root, "%s/%s", dev_mnt_point, file_list[which][1]);

  in = open(if_root, O_RDONLY);
  if (in < 0) {
    ui->Print("Cannot open input file %s\n", if_root);
    exit(-1);
  }

  out = open(of_root, O_WRONLY  | O_CREAT | O_TRUNC, 0664);
  if (out < 0) {
    ui->Print("Cannot open output file %s\n", of_root);
    exit(-1);
  }

  len = lseek64(in, 0, SEEK_END);
  lseek64(in, 0, SEEK_SET);

  s = sparse_file_new(block_size, len);
  if (!s) {
    ui->Print("Failed to create sparse file\n");
    exit(-1);
  }

  sparse_file_verbose(s);
  ret = sparse_file_read(s, in, false, false);
  if (ret) {
    ui->Print("Failed to read file\n");
    exit(-1);
  }

  ret = sparse_file_write(s, out, true, true, false);
  if (ret) {
    ui->Print("Failed to write sparse file\n");
    exit(-1);
  }

  close(in);
  close(out);
  
  exit(EXIT_SUCCESS);
}

static void __do_convert_ext2simg(const char *dev_mnt_point, int which) 
{
  int ret = 0;
  char if_root[512], of_root[512];

  sprintf(if_root, "%s/%s", dev_mnt_point, v_head[which].blk_label);
  sprintf(of_root, "%s/%s", dev_mnt_point, file_list[which][1]);

  
  ret = execl("/system/bin/ext2simg", 
      "ext2simg", "-z", if_root, of_root, NULL);
  exit(EXIT_SUCCESS);
}

static void __copy_sparse_raw_img(const char *dev_mnt_point, int which)
{
  int ret = 0;
  char src[512];

  sprintf(src, "%s/%s", dev_mnt_point, file_list[which][1]);

  
  ret = execl("/system/bin/cp", 
      "cp", src, QB_INTERNAL_ROOT, NULL);
  exit(EXIT_SUCCESS);
}

static void __copy_sparse_ext4_img(const char *dev_mnt_point, int which)
{
  int ret = 0;
  char src[512];

  sprintf(src, "%s/%s", dev_mnt_point, file_list[which][1]);

  ret = execl("/system/bin/cp", 
      "cp", src, QB_INTERNAL_ROOT, NULL);
  exit(EXIT_SUCCESS);
}

static void wait_child_ps(pid_t waiting_pid)
{
  int status;
  pid_t terminated_pid;

  terminated_pid = wait(&status);
}

static void __do_extract_binary_data(const char *dev_mnt_point, int which)
{
  int ret = 0;
  char if_root[512], of_root[512];

  sprintf(if_root, "if=%s", v_head[which].list->blk_device);
  sprintf(of_root, "of=%s/%s", dev_mnt_point, v_head[which].blk_label);

  ret = execl("/system/bin/dd", 
      "dd", if_root, of_root, NULL);
  exit(EXIT_SUCCESS);
}

void __do_gunzip(const char* path)
{
  execl("/system/bin/gzip", "gzip", "-d", path, NULL);
  exit(EXIT_SUCCESS);
}

void __do_gzip(const char* path)
{
  execl("/system/bin/gzip", "gzip", path, NULL);
  exit(EXIT_SUCCESS);
}

static void do_extract_qb_data(const char *root, int which)
{
  pid_t pid;

  pid = fork();
  if (pid==0) {
    __do_extract_binary_data(root, which);
  }
  wait_child_ps(pid);

  pid = fork();
  if (pid==0) {
    if (which<2)
      __do_convert_ext2simg(root, which);
    else 
      __do_convert_img2simg(root, which);
  }
  wait_child_ps(pid);

#if 1
  pid = fork();
  if (pid==0) {
    if (which<2)
      __copy_sparse_ext4_img(root, which);
    else 
      __copy_sparse_raw_img(root, which);
  }
  wait_child_ps(pid);
#endif
  exit(EXIT_SUCCESS);
}

static int extract_qb_data(const char *root)
{
  struct timeval tv1, tv2, tv3, tv4;
  unsigned long long usec = 0;

  gettimeofday(&tv1, NULL);
  ui->Print("[extract_qb_data] Start extracting qb data function...\n");
#if 0
  int i = 0;
  pid_t pid[3];

  for (i=0; i<3; i++) {
    pid[i] = fork();
    if (pid[i] == -1) {
      ui->Print("Fail to create dd ps.\n");
    } else if (pid[i] == 0) {
      do_extract_qb_data(root, i);
    } 
  }
  for (i=0; i<3; i++) {
    wait_child_ps(pid[i]);
  }
#else
  pid_t pid;

  ui->Print("Phase(1/3) Extracting...");
  pid = fork();
  if (pid==0) {
    __do_extract_binary_data(root, CACHE);
  }
  wait_child_ps(pid);
  pid = fork();
  if (pid==0) {
    __do_extract_binary_data(root, SNAPSHOT);
  }
  wait_child_ps(pid);
  pid = fork();
  if (pid==0) {
    __do_extract_binary_data(root, USERDATA);
  }
  wait_child_ps(pid);
  gettimeofday(&tv2, NULL);
  usec = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
  ui->Print("Done.(%lld usec)\n", usec);

  ui->Print("Phase(2/3) Converting...");
  pid = fork();
  if (pid==0) {
    __do_convert_ext2simg(root, CACHE);
  }
  wait_child_ps(pid);
  pid = fork();
  if (pid==0) {
    __do_convert_ext2simg(root, USERDATA);
  }
  wait_child_ps(pid);
  pid = fork();
  if (pid==0) {
    __do_convert_img2simg(root, SNAPSHOT);
  }
  wait_child_ps(pid);
  gettimeofday(&tv3, NULL);
  usec = (tv3.tv_sec - tv2.tv_sec) * 1000000 + tv3.tv_usec - tv2.tv_usec;
  ui->Print("Done.(%lld usec)\n", usec);

  ui->Print("Phase(2/3) Copying...");
  pid = fork();
  if (pid==0) {
    __copy_sparse_ext4_img(root, CACHE);
  }
  wait_child_ps(pid);
  pid = fork();
  if (pid==0) {
    __copy_sparse_ext4_img(root, USERDATA);
  }
  wait_child_ps(pid);
  pid = fork();
  if (pid==0) {
    __copy_sparse_raw_img(root, SNAPSHOT);
  }
  wait_child_ps(pid);
  gettimeofday(&tv4, NULL);
  usec = (tv4.tv_sec - tv3.tv_sec) * 1000000 + tv4.tv_usec - tv3.tv_usec;
  ui->Print("Done.(%lld usec)\n", usec);
#endif
  gettimeofday(&tv2, NULL);
  usec = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
  ui->Print("[extract_qb_data] Finished.(%lld usec)\n", usec);

  return 0;
}

int update_qb_image(const char* dev_mnt_point, int which) 
{
  int in, out, ret;
  struct sparse_file *s;
  pid_t pid;
  char if_path[512], of_path[512];

  sprintf(if_path, "%s/%s", dev_mnt_point, file_list[which][1]);
  sprintf(of_path, "%s", v_head[which].list->blk_device);

  in = open(if_path,  O_RDONLY);
  if (in < 0) {
    ui->Print("[update_qb_image] Cannot open input file %s\n", if_path);
    return -1;
  }
  close(in);

  ui->Print("[update_qb_image] Start to decompress %s... ", if_path);
  pid = vfork();
  if (pid == -1) {
    ui->Print("Fail to create gunzip ps.\n");
  } else if (pid == 0) {
    __do_gunzip(if_path);
  } else { 
    wait_child_ps(pid);
  }
  ui->Print("Finished.\n");

  sprintf(if_path, "%s/%s", dev_mnt_point, file_list[which][0]);
  ui->Print("[update_qb_image] Open output file %s\n", of_path);
  out = open(of_path, O_WRONLY);
  if (out < 0) {
    ui->Print("[update_qb_image] Cannot open output file %s\n", of_path);
    exit(-1);
  }

  ui->Print("[update_qb_image] Open input file %s\n", if_path);
  in = open(if_path,  O_RDONLY);
  if (in < 0) {
    ui->Print("[update_qb_image] Cannot open input file %s\n", if_path);
    exit(-1);
  }

  ui->Print("[update_qb_image] Read sparse file\n");
  s = (struct sparse_file*)sparse_file_import(in, true, false);
  if (!s) {
    ui->Print("[update_qb_image] Failed to read sparse file\n");
    exit(-1);
  }

  lseek(out, 0, SEEK_SET);

  ui->Print("[update_qb_image] Write output file\n");
  ret = sparse_file_write(s, out, false, false, false);
  if (ret < 0) {
    ui->Print("[update_qb_image] Cannot write output file\n");
    exit(-1);
  }
  ui->Print("[update_qb_image] Destroy sparse file\n");
  sparse_file_destroy(s);
  close(in);
  close(out);

  pid = vfork();
  if (pid == -1) {
    ui->Print("Fail to create gzip ps.\n");
  } else if (pid == 0) {
    __do_gzip(if_path);
  } else { 
    wait_child_ps(pid);
  }

  return 0;
}

void init_volume_list(const char* volume, int num)
{
  char *token, *t_blk_label;
  char t_path[512];

  v_head[num].list = volume_for_path(volume);
  strcpy(t_path, v_head[num].list->blk_device);

  token = strtok(t_path, "/");
  do {
    t_blk_label = strdup(token);
  } while ((token=strtok(NULL, "/")) != NULL);
  strcpy(v_head[num].blk_label, t_blk_label);

#if 0
  ui->Print("v_head[%d].list->blk_device(%s)\n", num, v_head[num].list->blk_device);
  ui->Print("v_head[%d].list->mount_point(%s)\n", num, v_head[num].list->mount_point);
  ui->Print("v_head[%d].list->fs_type(%s)\n", num, v_head[num].list->fs_type);
  ui->Print("v_head[%d].list->blk_label(%s)\n", num, v_head[num].blk_label);
#endif
}

void quickboot_factory_reset(void)
{
  init_volume_list("/data", USERDATA);
  init_volume_list("/cache", CACHE);
  init_volume_list("/snapshot", SNAPSHOT);
  update_qb_image(QB_INTERNAL_ROOT, USERDATA);
  update_qb_image(QB_INTERNAL_ROOT, CACHE);
  update_qb_image(QB_INTERNAL_ROOT, SNAPSHOT);
}

int do_qb_prebuilt(Device* device) {
  static const char** title_headers = NULL;

  bool enable_qb_prebuilt_menu = false;
  char enable_qb_prebuilt[4096];
  property_get("ro.tcc.qb_prebuilt_mode", enable_qb_prebuilt, "");
  if (!strcmp(enable_qb_prebuilt, "1"))
    enable_qb_prebuilt_menu = true;

  if (title_headers == NULL) {
    const char* headers[] = { "Quickboot Pre-built related functions",
      //"  THIS CAN NOT BE UNDONE.",
      "",
      NULL };
    title_headers = prepend_title((const char**)headers);
  }

  init_volume_list("/data", USERDATA);
  init_volume_list("/cache", CACHE);
  init_volume_list("/snapshot", SNAPSHOT);

  const char* items_enable[] = { 
    " Extract QB data to external SD card",
    " Extract QB data to USB memory stick",
    " Update userdata image from SD card",
    " Update userdata image from USB memory stick",
    " Update userdata image from internal storage",
    " Update snapshot image from SD card",
    " Update snapshot image from USB memory stick",
    " Update snapshot image from internal storage",
    " Update cachedata image from SD card",
    " Update cachedata image from USB memory stick",
    " Update cachedata image from internal storage",
    " Update all(userdata, cache, snapshot) from SD card",
    " Update all(userdata, cache, snapshot) from USB memory stick",
    " Update all(userdata, cache, snapshot) from internal storage",
    " Back",
    NULL };
  const char* items_disable[] = { 
    " Back",
    NULL };

  ensure_path_mounted(SYSTEM_ROOT);
  ensure_path_unmounted(CACHE_ROOT);

  while (1) 
  {
    int chosen_item = 0;
    if (enable_qb_prebuilt_menu) 
      chosen_item = get_menu_selection(title_headers, items_enable, 1, 0, device);
    else 
      chosen_item = get_menu_selection(title_headers, items_disable, 1, 0, device);
    switch (chosen_item) 
    {
      case 0:
	if (!enable_qb_prebuilt_menu) {
	  umount(SYSTEM_ROOT);
	  return 0;
	}
	ensure_path_mounted(SDCARD_ROOT);
	ensure_path_mounted(QB_INTERNAL_ROOT);
	extract_qb_data(SDCARD_ROOT);
	break;
      case 1:
	ensure_path_mounted(USB_ROOT);
	ensure_path_mounted(QB_INTERNAL_ROOT);
	extract_qb_data(USB_ROOT);
	break;
      case 2:
	ensure_path_mounted(SDCARD_ROOT);
	update_qb_image(SDCARD_ROOT, USERDATA);
	break;
      case 3:
	ensure_path_mounted(USB_ROOT);
	update_qb_image(USB_ROOT, USERDATA);
	break;
      case 4:
	ensure_path_mounted(QB_INTERNAL_ROOT);
	update_qb_image(QB_INTERNAL_ROOT, USERDATA);
	break;
      case 5:
	ensure_path_mounted(SDCARD_ROOT);
	update_qb_image(SDCARD_ROOT, SNAPSHOT);
	break;
      case 6:
	ensure_path_mounted(USB_ROOT);
	update_qb_image(USB_ROOT, SNAPSHOT);
	break;
      case 7:
	ensure_path_mounted(QB_INTERNAL_ROOT);
	update_qb_image(QB_INTERNAL_ROOT, SNAPSHOT);
	break;
      case 8:
	ensure_path_mounted(SDCARD_ROOT);
	update_qb_image(SDCARD_ROOT, CACHE);
	break;
      case 9:
	ensure_path_mounted(USB_ROOT);
	update_qb_image(USB_ROOT, CACHE);
	break;
      case 10:
	ensure_path_mounted(QB_INTERNAL_ROOT);
	update_qb_image(QB_INTERNAL_ROOT, CACHE);
	break;
      case 11:
	ensure_path_mounted(SDCARD_ROOT);
	update_qb_image(SDCARD_ROOT, USERDATA);
	update_qb_image(SDCARD_ROOT, CACHE);
	update_qb_image(SDCARD_ROOT, SNAPSHOT);
	break;
      case 12:
	ensure_path_mounted(USB_ROOT);
	update_qb_image(USB_ROOT, USERDATA);
	update_qb_image(USB_ROOT, CACHE);
	update_qb_image(USB_ROOT, SNAPSHOT);
	break;
      case 13:
	ensure_path_mounted(QB_INTERNAL_ROOT);
	update_qb_image(QB_INTERNAL_ROOT, USERDATA);
	update_qb_image(QB_INTERNAL_ROOT, CACHE);
	update_qb_image(QB_INTERNAL_ROOT, SNAPSHOT);
	break;
      case 14:
	umount(SYSTEM_ROOT);
	return 0;
      default:
	ui->Print("This menu isn't implemented yet...\n");
    }
  }

  umount(SYSTEM_ROOT);
  
  return 0;
}
