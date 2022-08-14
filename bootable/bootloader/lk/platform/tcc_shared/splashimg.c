
#include <debug.h>
#include <string.h>
#include <dev/flash.h>
#include <lib/ptable.h>
#include <dev/fbcon.h>
#include <splash/splashimg.h>

#ifdef BOOTSD_INCLUDE
#include <fwdn/Disk.h>
#include <sdmmc/sd_memory.h>
#include <sdmmc/emmc.h>
#endif

#include <partition_parser.h>
#include<sfl.h>

#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))
#define BYTE_TO_SECTOR(x) (x)/512

unsigned char *splash[16384];

static int get_splash_index(SPLASH_IMAGE_Header_info_t *splash_hdr, char *part){

	unsigned int idx = 0;

	for(idx = 0; idx < splash_hdr->ulNumber ; idx ++){     

		dprintf(SPEW, "part name : %s idx : %d\n" , splash_hdr->SPLASH_IMAGE[idx].ucImageName , idx);

		if(!strcmp((splash_hdr->SPLASH_IMAGE[idx].ucImageName) , part)){
			return idx;
		}
	}

	return -1;
}

static int splash_image_nand_v8(char *partition){

	SPLASH_IMAGE_Header_info_t *splash_hdr = (void*)splash;
	struct fbcon_config *fb_display = NULL;
	int img_idx = 0;
	unsigned int page_size = flash_page_size();
	unsigned int page_mask = page_size -1;

	unsigned long long ptn = 0;

    memset(&splash_hdr, 0x00, sizeof(SPLASH_IMAGE_Header_info_t));

	dprintf(SPEW, "partition : %s\n", partition);


	ptn = flash_ptn_offset("splash");
	if(ptn == 0){
		dprintf(CRITICAL, "ERROR : No splash partition found !\n");
		return -1;
	}else{

		fb_display = fbcon_display();

		if(fb_display){

			if(!flash_read_tnftl_v8(ptn, page_size, splash_hdr)){
				if(strcmp(splash_hdr->ucPartition, SPLASH_TAG)){
					dprintf(CRITICAL, "Splash TAG Is Mismatched\n");
					return -1;
				}

			}

			img_idx = get_splash_index(splash_hdr, partition);
			dprintf(SPEW, "image idx = %d \n" ,img_idx);


			if(img_idx < 0){
				dprintf(CRITICAL, " there is no image [%s]\n", partition);
				return -1;
			}else{

				if((fb_display->width != splash_hdr->SPLASH_IMAGE[img_idx].ulImageWidth) 
						&& (fb_display->height != splash_hdr->SPLASH_IMAGE[img_idx].ulImageHeight)){

					fb_display->width = splash_hdr->SPLASH_IMAGE[img_idx].ulImageWidth;
					fb_display->height = splash_hdr->SPLASH_IMAGE[img_idx].ulImageHeight;
					display_splash_logo(fb_display);
				}

				if(flash_read_tnftl_v8(ptn+BYTE_TO_SECTOR(splash_hdr->SPLASH_IMAGE[img_idx].ulImageAddr),
							splash_hdr->SPLASH_IMAGE[img_idx].ulImageSize, fb_display->base)){
					fbcon_clear();
				}
			}
		}
	}

	return 0;

}

static int splash_image_sdmmc(char *partition){

	SPLASH_IMAGE_Header_info_t *splash_hdr = (void*)splash;
	struct fbcon_config *fb_display = NULL;
	int img_idx = 0;
	unsigned int page_size = flash_page_size();
	unsigned int page_mask = page_size -1;

	unsigned long long ptn = 0;

	dprintf(SPEW, "partition : %s\n", partition);

	ptn = emmc_ptn_offset("splash");
	if(ptn == 0){
		dprintf(CRITICAL, "ERROR : No splash partition found !\n");
		return -1;
	}else{

		fb_display = fbcon_display();

		if(fb_display){

			if(!emmc_read(ptn, page_size, splash_hdr)){
				if(strcmp(splash_hdr->ucPartition, SPLASH_TAG)){
					dprintf(CRITICAL, "Splash TAG Is Mismatched\n");
					return -1;
				}
			}
			img_idx = get_splash_index(splash_hdr, partition);
			dprintf(SPEW, "image idx = %d \n" ,img_idx);


			if(img_idx < 0){
				dprintf(CRITICAL, " there is no image [%s]\n", partition);
			}else{

				if((fb_display->width != splash_hdr->SPLASH_IMAGE[img_idx].ulImageWidth) 
						&& (fb_display->height != splash_hdr->SPLASH_IMAGE[img_idx].ulImageHeight)){

					fb_display->width = splash_hdr->SPLASH_IMAGE[img_idx].ulImageWidth;
					fb_display->height = splash_hdr->SPLASH_IMAGE[img_idx].ulImageHeight;
					display_splash_logo(fb_display);
				}

				if(emmc_read(ptn + BYTE_TO_SECTOR(splash_hdr->SPLASH_IMAGE[img_idx].ulImageAddr),
							splash_hdr->SPLASH_IMAGE[img_idx].ulImageSize, fb_display->base)){

					fbcon_clear();
				}
			}
		}
	}

	return 0;

}


int splash_image(char *partition){

#if defined(BOOTSD_INCLUDE)
	return splash_image_sdmmc(partition);
#elif defined(TNFTL_V8_INCLUDE)
	return splash_image_nand_v8(partition);
#else
	//return splash_image_nand_v7(partition);
#endif

}

#if defined(BOOTSD_INCLUDE)
int splash_image_load_sdmmc(char *partition, struct fbcon_config *fb_cfg)
{

	SPLASH_IMAGE_Header_info_t *splash_hdr = (void*)splash;
	struct fbcon_config *fb_display = NULL;
	int img_idx = 0;
	unsigned int page_size = flash_page_size();
	unsigned int page_mask = page_size -1;

	unsigned long long ptn = 0;
	unsigned int ptn_index;

	dprintf(INFO, "partition : %s\n", partition);

	ptn_index = partition_get_index(partition);
	ptn = partition_get_offset(ptn_index);

	if(ptn == 0){
		dprintf(CRITICAL, "ERROR : No splash partition found !\n");
		return -1;
	}else{

		if(tcc_read(ptn, page_size, splash_hdr)){
			if(strcmp(splash_hdr->ucPartition, SPLASH_TAG)){
				dprintf(CRITICAL, "Splash TAG Is Mismatched\n");
				return -1;
			}
		}

		img_idx = get_splash_index(splash_hdr, partition);
		dprintf(SPEW, "image idx = %d \n" ,img_idx);

		if(img_idx < 0){
			dprintf(CRITICAL, " there is no image from emmc [%s]\n", partition);
		}else{

			fb_cfg->width = splash_hdr->SPLASH_IMAGE[img_idx].ulImageWidth;
			fb_cfg->height = splash_hdr->SPLASH_IMAGE[img_idx].ulImageHeight;
			//fb_display->bpp = splash_hdr->SPLASH_IMAGE[img_idx].fmt

			tcc_read(ptn + BYTE_TO_SECTOR(splash_hdr->SPLASH_IMAGE[img_idx].ulImageAddr),
					splash_hdr->SPLASH_IMAGE[img_idx].ulImageSize, fb_cfg->base);    			    			
		}

	}

	return 0;

}
#endif


int splash_image_load(char *partition, struct fbcon_config *fb_cfg)
{

	SPLASH_IMAGE_Header_info_t *splash_hdr = (void*)splash;
	struct fbcon_config *fb_display = NULL;
	int img_idx = 0;

	unsigned int page_size = flash_page_size();
	unsigned int page_mask = page_size -1;

	unsigned long long ptn = 0;
	unsigned int ptn_index;

	const char* ptn_name = "splash";

	ptn_index = partition_get_index(ptn_name);
	ptn = partition_get_offset(ptn_index);

	if(ptn == 0){
		dprintf(CRITICAL, "ERROR : No splash partition found !\n");
		return -1;
	}else{

		if(tcc_read(ptn, splash_hdr, page_size))
		{
			dprintf(CRITICAL, "Splash TAG Is Mismatched\n");
			return -1;
		}

              if(splash_hdr->ulNumber < 0)
              {
                     dprintf(CRITICAL, " there is no data in splash partition \n");
                        return -1;
               }

		img_idx = get_splash_index(splash_hdr, partition);
		dprintf(SPEW, "image idx = %d \n" ,img_idx);


		if(img_idx < 0){
			dprintf(CRITICAL, " there is no image from [%s]\n", partition);
		}else{

			fb_cfg->width = splash_hdr->SPLASH_IMAGE[img_idx].ulImageWidth;
			fb_cfg->height = splash_hdr->SPLASH_IMAGE[img_idx].ulImageHeight;
            
			tcc_read(ptn+(splash_hdr->SPLASH_IMAGE[img_idx].ulImageAddr),
						fb_cfg->base, splash_hdr->SPLASH_IMAGE[img_idx].ulImageSize );
		}   
	}
	return 0;

}

