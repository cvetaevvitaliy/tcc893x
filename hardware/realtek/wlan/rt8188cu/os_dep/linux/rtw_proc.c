/******************************************************************************
 *
 * Copyright(c) 2007 - 2013 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

#include <drv_types.h>
#include "rtw_proc.h"

#ifdef CONFIG_PROC_DEBUG

static struct proc_dir_entry *rtw_proc = NULL;

inline struct proc_dir_entry *get_rtw_drv_proc(void)
{
	return rtw_proc;
}

#define RTW_PROC_NAME DRV_NAME

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0))
#define file_inode(file) ((file)->f_dentry->d_inode)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
#define PDE_DATA(inode) PDE((inode))->data
#define proc_get_parent_data(inode) PDE((inode))->parent->data
#endif

#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
#define get_proc_net proc_net
#else
#define get_proc_net init_net.proc_net
#endif

inline struct proc_dir_entry *rtw_proc_create_dir(const char *name, struct proc_dir_entry *parent, void *data)
{
	struct proc_dir_entry *entry;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	entry = proc_mkdir_data(name, S_IRUGO|S_IXUGO, parent, data);
#else
	//entry = proc_mkdir_mode(name, S_IRUGO|S_IXUGO, parent);
	entry = proc_mkdir(name, parent);
	if (entry)
		entry->data = data;
#endif

	return entry;
}

inline struct proc_dir_entry *rtw_proc_create_entry(const char *name, struct proc_dir_entry *parent, 
	const struct file_operations *fops, void * data)
{
	struct proc_dir_entry *entry;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	entry = proc_create_data(name,  S_IFREG|S_IRUGO, parent, fops, data);
#else
	entry = create_proc_entry(name, S_IFREG|S_IRUGO, parent);
	if (entry) {
		entry->data = data;
		entry->proc_fops = fops;
	}
#endif

	return entry;
}

static int proc_get_dummy(struct seq_file *m, void *v)
{
	return 0;
}

static int proc_get_drv_version(struct seq_file *m, void *v)
{
	dump_drv_version(m);
	return 0;
}

static int proc_get_log_level(struct seq_file *m, void *v)
{
	dump_log_level(m);
	return 0;
}

static ssize_t proc_set_log_level(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	char tmp[32];
	int log_level;

	if (count < 1)
		return -EINVAL;

	if (buffer && !copy_from_user(tmp, buffer, sizeof(tmp))) {		

		int num = sscanf(tmp, "%d ", &log_level);

		if( log_level >= 0 && log_level < 10 )
		{
			GlobalDebugLevel= log_level;
			printk("%d\n", GlobalDebugLevel);
		}
	} else {
		return -EFAULT;
	}
	
	return count;
}

#ifdef DBG_MEM_ALLOC
static int proc_get_mstat(struct seq_file *m, void *v)
{	
	rtw_mstat_dump(m);
	return 0;
}
#endif /* DBG_MEM_ALLOC */


/*
* rtw_drv_proc:
* init/deinit when register/unregister driver
*/
const struct rtw_proc_hdl drv_proc_hdls [] = {
	{"ver_info", proc_get_drv_version, NULL},
	{"log_level", proc_get_log_level, proc_set_log_level},
#ifdef DBG_MEM_ALLOC
	{"mstat", proc_get_mstat, NULL},
#endif /* DBG_MEM_ALLOC */
};

const int drv_proc_hdls_num = sizeof(drv_proc_hdls) / sizeof(struct rtw_proc_hdl);

static int rtw_drv_proc_open(struct inode *inode, struct file *file)
{
	//struct net_device *dev = proc_get_parent_data(inode);
	ssize_t index = (ssize_t)PDE_DATA(inode);
	const struct rtw_proc_hdl *hdl = drv_proc_hdls+index;
       return single_open(file, hdl->show, NULL);
}

static ssize_t rtw_drv_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
	ssize_t index = (ssize_t)PDE_DATA(file_inode(file));
	const struct rtw_proc_hdl *hdl = drv_proc_hdls+index;
	ssize_t (*write)(struct file *, const char __user *, size_t, loff_t *, void *) = hdl->write;
	
	if (write)
		return write(file, buffer, count, pos, NULL);

	return -EROFS;
}

static const struct file_operations rtw_drv_proc_fops = {
	.owner = THIS_MODULE,
	.open = rtw_drv_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = rtw_drv_proc_write,
};

int rtw_drv_proc_init(void)
{
	int ret = _FAIL;
	ssize_t i;
	struct proc_dir_entry *entry = NULL;

	if (rtw_proc != NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	rtw_proc = rtw_proc_create_dir(RTW_PROC_NAME, get_proc_net, NULL);

	if (rtw_proc == NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	for (i=0;i<drv_proc_hdls_num;i++) {
		entry = rtw_proc_create_entry(drv_proc_hdls[i].name, rtw_proc, &rtw_drv_proc_fops, (void *)i);
		if (!entry) {
			rtw_warn_on(1);
			goto exit;
		}
	}

	ret = _SUCCESS;

exit:
	return ret;
}

void rtw_drv_proc_deinit(void)
{
	int i;

	if (rtw_proc == NULL)
		return;

	for (i=0;i<drv_proc_hdls_num;i++)
		remove_proc_entry(drv_proc_hdls[i].name, rtw_proc);

	remove_proc_entry(RTW_PROC_NAME, get_proc_net);
	rtw_proc = NULL;
}

#ifdef CONFIG_SDIO_HCI
static int proc_get_sd_f0_reg_dump(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	sd_f0_reg_dump(m, adapter);

	return 0;
}
#endif /* CONFIG_SDIO_HCI */

static int proc_get_mac_reg_dump(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	mac_reg_dump(m, adapter);

	return 0;
}

static int proc_get_bb_reg_dump(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	bb_reg_dump(m, adapter);

	return 0;
}

static int proc_get_rf_reg_dump(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(dev);

	rf_reg_dump(m, adapter);

	return 0;
}

#ifdef CONFIG_DM_ADAPTIVITY
#include "../../hal/dm.h"
int proc_get_dm_adaptivity(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);

	dm_adaptivity_get_parm_msg(m, padapter);

	return 0;
}

ssize_t proc_set_dm_adaptivity(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	char tmp[32];
	u32 TH_L2H_ini;
	s8 TH_EDCCA_HL_diff;
	u32 IGI_Base;
	int ForceEDCCA;
	u8 AdapEn_RSSI;
	u8 IGI_LowerBound;

	if (count < 1)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, sizeof(tmp))) {

		int num = sscanf(tmp, "%x %hhd %x %d %hhu %hhu",
			&TH_L2H_ini, &TH_EDCCA_HL_diff, &IGI_Base, &ForceEDCCA, &AdapEn_RSSI, &IGI_LowerBound);

		if (num != 6)
			return count;

		dm_adaptivity_set_parm(padapter, (s8)TH_L2H_ini, TH_EDCCA_HL_diff, (s8)IGI_Base, (bool)ForceEDCCA, AdapEn_RSSI, IGI_LowerBound);
	}
	
	return count;
}
#endif /* CONFIG_DM_ADAPTIVITY */

/*
* rtw_adapter_proc:
* init/deinit when register/unregister net_device
*/
const struct rtw_proc_hdl adapter_proc_hdls [] = {
	{"write_reg", proc_get_dummy, proc_set_write_reg},
	{"read_reg", proc_get_read_reg, proc_set_read_reg},
	{"fwstate", proc_get_fwstate, NULL},
	{"sec_info", proc_get_sec_info, NULL},
	{"mlmext_state", proc_get_mlmext_state, NULL},
	{"qos_option", proc_get_qos_option, NULL},
	{"ht_option", proc_get_ht_option, NULL},
	{"rf_info", proc_get_rf_info, NULL},
	{"ap_info", proc_get_ap_info, NULL},
	{"adapter_state", proc_get_adapter_state, NULL},
	{"trx_info", proc_get_trx_info, NULL},

	{"mac_reg_dump", proc_get_mac_reg_dump, NULL},
	{"bb_reg_dump", proc_get_bb_reg_dump, NULL},
	{"rf_reg_dump", proc_get_rf_reg_dump, NULL},

#ifdef CONFIG_AP_MODE
	{"all_sta_info", proc_get_all_sta_info, NULL},
#endif

#ifdef DBG_MEMORY_LEAK
	{"_malloc_cnt", proc_get_malloc_cnt, NULL},
#endif

#ifdef CONFIG_FIND_BEST_CHANNEL

	{"best_channel", proc_get_best_channel, proc_set_best_channel},
#endif

	{"rx_signal", proc_get_rx_signal, proc_set_rx_signal},

	{"ht_enable", proc_get_ht_enable, proc_set_ht_enable},
	{"cbw40_enable", proc_get_cbw40_enable, proc_set_cbw40_enable},
	{"ampdu_enable", proc_get_ampdu_enable, proc_set_ampdu_enable},
	{"rx_stbc", proc_get_rx_stbc, proc_set_rx_stbc},

	{"path_rssi", proc_get_two_path_rssi, NULL},
	{"rssi_disp", proc_get_rssi_disp, proc_set_rssi_disp},

	{"vid", proc_get_vid, NULL},
	{"pid", proc_get_pid, NULL},

#if defined(DBG_CONFIG_ERROR_DETECT)
	{"sreset", proc_get_sreset, proc_set_sreset},
#endif /* DBG_CONFIG_ERROR_DETECT */

#ifdef CONFIG_DM_ADAPTIVITY
	{"dm_adaptivity", proc_get_dm_adaptivity, proc_set_dm_adaptivity},
#endif /* CONFIG_DM_ADAPTIVITY */
};

const int adapter_proc_hdls_num = sizeof(adapter_proc_hdls) / sizeof(struct rtw_proc_hdl);

static int rtw_adapter_proc_open(struct inode *inode, struct file *file)
{
	ssize_t index = (ssize_t)PDE_DATA(inode);
	const struct rtw_proc_hdl *hdl = adapter_proc_hdls+index;

	return single_open(file, hdl->show, proc_get_parent_data(inode));
}

static ssize_t rtw_adapter_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
	ssize_t index = (ssize_t)PDE_DATA(file_inode(file));
	const struct rtw_proc_hdl *hdl = adapter_proc_hdls+index;
	ssize_t (*write)(struct file *, const char __user *, size_t, loff_t *, void *) = hdl->write;

	if (write)
		return write(file, buffer, count, pos, ((struct seq_file *)file->private_data)->private);

	return -EROFS;
}

static const struct file_operations rtw_adapter_proc_fops = {
	.owner = THIS_MODULE,
	.open = rtw_adapter_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = rtw_adapter_proc_write,
};

struct proc_dir_entry *rtw_adapter_proc_init(struct net_device *dev)
{
	struct proc_dir_entry *drv_proc = get_rtw_drv_proc();
	struct proc_dir_entry *dir_dev = NULL;
	struct proc_dir_entry *entry = NULL;
	_adapter *adapter = rtw_netdev_priv(dev);
	u8 rf_type;
	ssize_t i;

	if (drv_proc == NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	if (adapter->dir_dev != NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	dir_dev = rtw_proc_create_dir(dev->name, drv_proc, dev);
	if (dir_dev == NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	adapter->dir_dev = dir_dev;

	for (i=0;i<adapter_proc_hdls_num;i++) {
		entry = rtw_proc_create_entry(adapter_proc_hdls[i].name, dir_dev, &rtw_adapter_proc_fops, (void *)i);
		if (!entry) {
			rtw_warn_on(1);
			goto exit;
		}
	}

exit:
	return dir_dev;
}

void rtw_adapter_proc_deinit(struct net_device *dev)
{
	struct proc_dir_entry *drv_proc = get_rtw_drv_proc();
	struct proc_dir_entry *dir_dev = NULL;
	_adapter *adapter = rtw_netdev_priv(dev);
	int i;

	dir_dev = adapter->dir_dev;

	if (dir_dev == NULL) {
		rtw_warn_on(1);
		return;
	}

	for (i=0;i<adapter_proc_hdls_num;i++)
		remove_proc_entry(adapter_proc_hdls[i].name, dir_dev);

	remove_proc_entry(dev->name, drv_proc);

	adapter->dir_dev = NULL;
}

void rtw_adapter_proc_replace(struct net_device *dev)
{
	struct proc_dir_entry *drv_proc = get_rtw_drv_proc();
	struct proc_dir_entry *dir_dev = NULL;
	_adapter *adapter = rtw_netdev_priv(dev);
	int i;

	dir_dev = adapter->dir_dev;

	if (dir_dev == NULL) {
		rtw_warn_on(1);
		return;
	}

	for (i=0;i<adapter_proc_hdls_num;i++)
		remove_proc_entry(adapter_proc_hdls[i].name, dir_dev);

	remove_proc_entry(adapter->old_ifname, drv_proc);

	adapter->dir_dev = NULL;

	rtw_adapter_proc_init(dev);

}

#endif /* CONFIG_PROC_DEBUG */

