
#define	IMAGE_SIZE_MAX		10
#define SPLASH_TAG		"splash"
#define SPLASH_TAG_SIZE		6

#define	SPLASH_IMAGE_WIDTH	800
#define	SPLASH_IMAGE_HEIGHT	480

#define DEFAULT_HEADER_SIZE	512
#define DEFAULT_PAGE_SIZE	512


typedef struct SPLASH_IMAGE_INFO{
	 char		ucImageName[16];
	unsigned int 		ulImageAddr;
	unsigned int		ulImageSize;
	unsigned int		ulImageWidth;
	unsigned int		ulImageHeight;
	unsigned int 		Padding;
	unsigned char		ucRev[12];
	
}SPLASH_IMAGE_INFO_t;

typedef struct SPLASH_IMAGE_Header_info{
	unsigned char 	ucPartition[8];
	int		                  ulNumber;	
	unsigned char 		ucRev[4];
	SPLASH_IMAGE_INFO_t	SPLASH_IMAGE[IMAGE_SIZE_MAX];
	
}SPLASH_IMAGE_Header_info_t;

typedef struct SPLASH_BUFFER {
	unsigned char *data;
	unsigned int size;
}SPLASH_BUFFER_t;


#if defined(BOOTSD_INCLUDE)
int splash_image_load_sdmmc(char *partition, struct fbcon_config *fb_cfg);
#endif

int splash_image(char* partition);
int splash_image_load(char *partition, struct fbcon_config *fb_cfg);

