#ifndef _SBOOT_IMAGE_H_
#define _SBOOT_IMAGE_H_

typedef struct sboot_img_hdr sboot_img_hdr;

#define BOOT_MAGIC	"ANDROID@"
#define KERNEL_TAG	"KERN"
#define BOOT_MAGIC_SIZE	8
#define KERNEL_TAG_SIZE	4

struct sboot_img_hdr{

	unsigned char magic[BOOT_MAGIC_SIZE];
	unsigned char ktag[KERNEL_TAG_SIZE];
	unsigned int  image_size;
	unsigned char rev[496];
};

#endif
