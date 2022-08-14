#
# Copyright (C) 2013 Telechips Inc.
# Copyright (C) 2010-2013 ARM Limited. All rights reserved.
# 
# This program is free software and is provided to you under the terms of the GNU General Public License version 2
# as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
# 
# A copy of the licence is included with the program, and can also be obtained from Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

# This file is called by the Linux build system.

OSKOS=linux
FILES_PREFIX=

MALI_RELEASE_NAME = r4p0-00rel0

ARCH = arm
BUILD = release
ifeq ($(CONFIG_MALI_UMP),y)
USING_UMP = 1
else
USING_UMP = 0
endif
ifeq ($(CONFIG_MALI_OS_MEMORY),y)
USING_OS_MEMORY = 1
else
USING_OS_MEMORY = 0
endif
ifeq ($(CONFIG_GPU_BUS_SCALING),y)
USING_GPU_UTILIZATION = 1
else
USING_GPU_UTILIZATION = 0
endif

TARGET_PLATFORM = tcc-m400
CONFIG = $(TARGET_PLATFORM)

# set up defaults if not defined by the user
TIMESTAMP ?= default
OS_MEMORY_KERNEL_BUFFER_SIZE_IN_MB ?= 16
PROFILING_SKIP_PP_JOBS ?= 0
PROFILING_SKIP_PP_AND_GP_JOBS ?= 0
MALI_PP_SCHEDULER_FORCE_NO_JOB_OVERLAP ?= 0
MALI_PP_SCHEDULER_KEEP_SUB_JOB_STARTS_ALIGNED ?= 0
MALI_PP_SCHEDULER_FORCE_NO_JOB_OVERLAP_BETWEEN_APPS ?= 0
MALI_UPPER_HALF_SCHEDULING ?= 1
MALI_ENABLE_CPU_CYCLES ?= 0

EXTRA_CFLAGS += -I$(src)/arch-$(CONFIG)
# Get path to driver source from Linux build system
DRIVER_DIR=$(src)

# Check if a Mali Core sub module should be enabled, true or false returned
ifneq ($(CONFIG),)
submodule_enabled = $(shell gcc $(DEFINES) -E $(1)/arch-$(CONFIG)/config.h | grep type | grep -c $(2))
endif

# Validate selected config
ifneq ($(shell [ -d $(DRIVER_DIR)/arch-$(CONFIG) ] && [ -f $(DRIVER_DIR)/arch-$(CONFIG)/config.h ] && echo "OK"), OK)
$(warning Current directory is $(shell pwd))
$(error No configuration found for config $(CONFIG). Check that arch-$(CONFIG)/config.h exists)
else
# Link arch to the selected arch-config directory
$(shell [ -L $(DRIVER_DIR)/arch ] && rm $(DRIVER_DIR)/arch)
$(shell ln -sf arch-$(CONFIG) $(DRIVER_DIR)/arch)
$(shell touch $(DRIVER_DIR)/arch/config.h)
endif

# For customer releases the Linux Device Drivers will be provided as ARM proprietary and GPL releases:
# The ARM proprietary product will only include the license/proprietary directory
# The GPL product will only include the license/gpl directory
ifeq ($(wildcard $(DRIVER_DIR)/linux/license/gpl/*),)
EXTRA_CFLAGS += -I$(DRIVER_DIR)/linux/license/proprietary
# Disable profiling for proprietary
$(error "Profiling is not supported without GPL.")
else
EXTRA_CFLAGS += -I$(DRIVER_DIR)/linux/license/gpl
endif

OSKFILES = \
	$(FILES_PREFIX)$(OSKOS)/mali_osk_atomics.c \
	$(FILES_PREFIX)$(OSKOS)/mali_osk_irq.c \
	$(FILES_PREFIX)$(OSKOS)/mali_osk_wq.c \
	$(FILES_PREFIX)$(OSKOS)/mali_osk_locks.c \
	$(FILES_PREFIX)$(OSKOS)/mali_osk_wait_queue.c \
	$(FILES_PREFIX)$(OSKOS)/mali_osk_low_level_mem.c \
	$(FILES_PREFIX)$(OSKOS)/mali_osk_math.c \
	$(FILES_PREFIX)$(OSKOS)/mali_osk_memory.c \
	$(FILES_PREFIX)$(OSKOS)/mali_osk_misc.c \
	$(FILES_PREFIX)$(OSKOS)/mali_osk_mali.c \
	$(FILES_PREFIX)$(OSKOS)/mali_osk_notification.c \
	$(FILES_PREFIX)$(OSKOS)/mali_osk_time.c \
	$(FILES_PREFIX)$(OSKOS)/mali_osk_timers.c \
	$(FILES_PREFIX)$(OSKOS)/mali_memory.c \
	$(FILES_PREFIX)$(OSKOS)/mali_memory_os_alloc.c \
	$(FILES_PREFIX)$(OSKOS)/mali_memory_external.c \
	$(FILES_PREFIX)$(OSKOS)/mali_memory_block_alloc.c
UKKFILES = \
	$(FILES_PREFIX)$(OSKOS)/mali_ukk_mem.c \
	$(FILES_PREFIX)$(OSKOS)/mali_ukk_gp.c \
	$(FILES_PREFIX)$(OSKOS)/mali_ukk_pp.c \
	$(FILES_PREFIX)$(OSKOS)/mali_ukk_core.c \
	$(FILES_PREFIX)$(OSKOS)/mali_ukk_soft_job.c \
	$(FILES_PREFIX)$(OSKOS)/mali_ukk_timeline.c

MALI_PLATFORM_FILE = platform/$(TARGET_PLATFORM)/mali_platform.c

# Source files which always are included in a build
SRC = \
	common/mali_kernel_core.c \
	linux/mali_kernel_linux.c \
	common/mali_kernel_descriptor_mapping.c \
	common/mali_session.c \
	linux/mali_device_pause_resume.c \
	common/mali_kernel_vsync.c \
	linux/mali_ukk_vsync.c \
	linux/mali_kernel_sysfs.c \
	common/mali_mmu.c \
	common/mali_mmu_page_directory.c \
	common/mali_mem_validation.c \
	common/mali_hw_core.c \
	common/mali_gp.c \
	common/mali_pp.c \
	common/mali_pp_job.c \
	common/mali_gp_job.c \
	common/mali_soft_job.c \
	common/mali_scheduler.c \
	common/mali_gp_scheduler.c \
	common/mali_pp_scheduler.c \
	common/mali_group.c \
	common/mali_dlbu.c \
	common/mali_broadcast.c \
	common/mali_pm.c \
	common/mali_pmu.c \
	common/mali_user_settings_db.c \
	common/mali_kernel_utilization.c \
	common/mali_l2_cache.c \
	common/mali_dma.c \
	common/mali_timeline.c \
	common/mali_timeline_fence_wait.c \
	common/mali_timeline_sync_fence.c \
	common/mali_spinlock_reentrant.c \
	common/mali_pm_domain.c \
	$(OSKOS)/mali_osk_pm.c \
	linux/mali_pmu_power_up_down.c \
	$(MALI_PLATFORM_FILE) \
	$(OSKFILES) \
	$(UKKFILES)

# Selecting files to compile by parsing the config file
ifeq ($(CONFIG_MALI400_PROFILING),y)
SRC += \
	linux/mali_ukk_profiling.c \
	linux/mali_osk_profiling.c

ifeq ($(CONFIG_MALI400_INTERNAL_PROFILING),y)
SRC += \
    linux/mali_profiling_internal.c \
    timestamp-$(TIMESTAMP)/mali_timestamp.c
EXTRA_CFLAGS += -I$(DRIVER_DIR)/timestamp-$(TIMESTAMP)
endif
endif

ifeq ($(CONFIG_DMA_SHARED_BUFFER),y)
SRC += \
	linux/mali_memory_dma_buf.o
endif
ifeq ($(CONFIG_SYNC),y)
SRC += \
	linux//mali_sync.o
endif

ifeq ($(CONFIG_MALI_UMP),y)
SRC += \
	linux/mali_memory_ump.o
endif

ifeq ($(CONFIG_GPU_BUS_SCALING),y)
SRC += \
	platform/$(TARGET_PLATFORM)/mali_platform_dvfs.c
endif

# Tell the Linux build system to enable building of our .c files
# Tell the Linux build system from which .o file to create the kernel module
obj-$(CONFIG_MALI_GPU) := mali.o
mali-y := $(SRC:.c=.o)


DEFINES := $(EXTRA_DEFINES)

# Set up our defines, which will be passed to gcc
DEFINES += -DUSING_OS_MEMORY=$(USING_OS_MEMORY)
DEFINES += -DCONFIG_MALI400_UMP=$(USING_UMP)
DEFINES += -DCONFIG_MALI_DMA_BUF_MAP_ON_ATTACH=$(CONFIG_DMA_SHARED_BUFFER)
DEFINES += -DPROFILING_SKIP_PP_JOBS=$(PROFILING_SKIP_PP_JOBS)
DEFINES += -DPROFILING_SKIP_PP_AND_GP_JOBS=$(PROFILING_SKIP_PP_AND_GP_JOBS)

DEFINES += -DMALI_PP_SCHEDULER_FORCE_NO_JOB_OVERLAP=$(MALI_PP_SCHEDULER_FORCE_NO_JOB_OVERLAP)
DEFINES += -DMALI_PP_SCHEDULER_KEEP_SUB_JOB_STARTS_ALIGNED=$(MALI_PP_SCHEDULER_KEEP_SUB_JOB_STARTS_ALIGNED)
DEFINES += -DMALI_PP_SCHEDULER_FORCE_NO_JOB_OVERLAP_BETWEEN_APPS=$(MALI_PP_SCHEDULER_FORCE_NO_JOB_OVERLAP_BETWEEN_APPS)
DEFINES += -DMALI_STATE_TRACKING=1
DEFINES += -DMALI_FAKE_PLATFORM_DEVICE=1
DEFINES += -DMALI_OS_MEMORY_KERNEL_BUFFER_SIZE_IN_MB=$(OS_MEMORY_KERNEL_BUFFER_SIZE_IN_MB)
DEFINES += -DUSING_GPU_UTILIZATION=$(USING_GPU_UTILIZATION)
DEFINES += -DMALI_ENABLE_CPU_CYCLES=$(MALI_ENABLE_CPU_CYCLES)

ifeq ($(MALI_UPPER_HALF_SCHEDULING),1)
	DEFINES += -DMALI_UPPER_HALF_SCHEDULING
endif


ifeq ($(USING_UMP),1)
	EXTRA_CFLAGS += -I$(src)/../ump/include/ump
endif

ifeq ($(BUILD), debug)
DEFINES += -DDEBUG
endif


DEFINES += -DSVN_REV=$(MALI_RELEASE_NAME)
DEFINES += -DSVN_REV_STRING=\"$(MALI_RELEASE_NAME)\"


# Use our defines when compiling
EXTRA_CFLAGS += $(DEFINES) -I$(src) -I$(src)/include -I$(src)/common -I$(src)/linux -I$(src)/platform


