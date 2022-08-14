#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "tcsb_sboot.h"

#define DEBUG

#if defined(DEBUG)
#define DBG 	printf
#else
#define DBG
#endif

void ussage(){
	printf(" USSAGE : mkseimg INPUT_FILE OUTPUT_FILE \n");
}

static void *load_file(const char* filename, unsigned *size){
	char *data;
	int sz;
	int fd;

	data = 0;
	fd = open(filename, O_RDONLY);
	if(fd<0) goto oops;

	sz = lseek(fd, 0, SEEK_END);
	if(sz <0) goto oops;

	if(lseek(fd, 0, SEEK_SET) != 0) goto oops;

	data = (char*)malloc(sz);
	if(data == 0) goto oops;

	if(read(fd, data, sz) != sz) goto oops;
	close(fd);

	if(size) *size = sz;

	return data;

oops:
	close(fd);
	if(data !=0) free(data);
	return 0;

}

static unsigned char padding[16384] = {0,};

int write_padding(int fd, unsigned pagesize, unsigned itemsize)
{
		
	unsigned pagemask = pagesize -1;
	int count;

	count = pagesize - (itemsize & pagemask);

	if(write(fd, padding, count) != count){
		return -1;
	}else{
		return 0;
	}
}

int main(int argc, char *argv[])
{
	sboot_img_hdr shdr;
	int bfd, sfd;
	unsigned int f_len;
	char *bootimg_data = 0;
	unsigned int kernel_page_size=0;

	char *infile, *outfile;

	if(argc < 2){
		DBG(" INPUT or OUTPUT File is missing !!\n");
		ussage();
		goto exit;
	}

	infile = argv[1]; outfile = argv[2];



	if(argv[3] != NULL)
		kernel_page_size = strtoul(argv[3], 0, 10);

	DBG("kernel page size : %d\n" , kernel_page_size);

	memset(&shdr, 0, sizeof(shdr));

	memcpy(shdr.magic , BOOT_MAGIC, BOOT_MAGIC_SIZE);
	memcpy(shdr.ktag , KERNEL_TAG, KERNEL_TAG_SIZE);

	bfd = open(infile, O_RDONLY);
	if(bfd<0) goto exit;

	bootimg_data = load_file(infile, &shdr.image_size);

	DBG("BOOT MAIGC : %s , KERNEL_TAGS : %s , BOOT SIZE : %x\n" , shdr.magic, shdr.ktag, shdr.image_size);

	sfd = open(outfile, O_RDWR | O_CREAT | O_TRUNC, 0644);
	if(sfd <0) goto exit;

	if(write(sfd, &shdr, sizeof(shdr)) != sizeof(struct sboot_img_hdr)){
		DBG("SBOOT HEADER WRITE FAIL\n");
		goto exit;
	}
	if( kernel_page_size != 0){
		if(write_padding(sfd, kernel_page_size, sizeof(struct sboot_img_hdr)) != 0){
			DBG("SBOOT HEADER PADDING WRITE FAIL\n");
			goto exit;
		}
	}

	if(write(sfd, bootimg_data, shdr.image_size) != shdr.image_size) {
		DBG("BOOTIMG WRITE FAIL!!\n");
		goto exit;
	}

	if(kernel_page_size !=0){
		if(write_padding(sfd, kernel_page_size, shdr.image_size) != 0){
			DBG("SBOOT HEADER PADDING WRITE FAIL\n");
			goto exit;
		}
	}


	close(bfd);
	close(sfd);

	

exit:
	return 0;
}


