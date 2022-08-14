/* tcsb_api.c
 *
 * Description: Telechips Secure Boot API.
 *
 * Created by hjbae@telechips.com
 * Date : 2012/05/08
 *
 ************************************************************************/

#if defined(TSBM_ENABLE)
extern void	tcsb_cipher_swap(uchar *raw, uchar *swap, uint size);
extern void	tcsb_cipher_AES(uint encode, uint opmode, uint size, uint sbase, uint dbase, uint *key, uint *iv);

extern uint tcsb_api_check_secureboot(uint nMHz, const void *pPubKey, const void *pWrapKey);

extern void *tcsb_api_decrypt(const void *pImage, void *pImageTarget, uint nImageSize);
#endif
/* tc_sb_api.c */
