#ifndef __ITCCXXXX_TSIF_P_H__
#define __ITCCXXXX_TSIF_P_H__

ITV_EXPORT(int, tcc_tsif_p_init, (unsigned char **, unsigned int *, unsigned int , unsigned int , unsigned int , 
			unsigned int , unsigned int , void (*callback)(void *), void *));
ITV_EXPORT(void, tcc_tsif_p_deinit, (void));
 #if defined(CONFIG_CHIP_TCC8930)
ITV_EXPORT(int, tcc_tsif_cm3_fw_init, (void));
ITV_EXPORT(void, tcc_tsif_cm3_fw_deinit, (void));
#endif
ITV_EXPORT(int, tcc_tsif_p_get_pos, (void));
ITV_EXPORT(int, tcc_tsif_p_start, (void));
ITV_EXPORT(int, tcc_tsif_p_stop, (void));
ITV_EXPORT(void, tcc_tsif_p_insert_pid, (int pid));
ITV_EXPORT(void, tcc_tsif_p_remove_pid, (int pid));
ITV_EXPORT(void, tcc_tsif_p_set_packetcnt, (int cnt));

#endif	/* __ITCC89XX_TSIF_P_H__ */
