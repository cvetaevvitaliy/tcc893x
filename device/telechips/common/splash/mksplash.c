#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <string.h>

#include <sys/stat.h>

#include "mksplash.h"


static void ussage(){
}

static int get_file_size (int fd){

	return lseek(fd, 0 , SEEK_END);
}

static unsigned char padding[16384] = {0,};

static int write_padding(int fd,unsigned int pagesize, unsigned int itemsize){
	unsigned pagemask = pagesize -1;
	int count;

	if((itemsize & pagemask) == 0) return 0;

	count = pagesize - (itemsize & pagemask);

	if(write(fd, padding , count) != count){
		return -1;
	}else{
		return count;
	}
	return 0;
}


static void *load_file(char *file_name , int *size){

	int sz , fd;
	char *data = 0;

	fd = open(file_name , O_RDONLY, 0644);
	if(fd < 0){
		printf(" File [%s] Open Failed !!\n", file_name);
		goto opps;
	}

	sz = get_file_size(fd);

	printf("image read size = %d\n" ,sz);

	if(sz < 0){
		printf(" Get File [%d]  Size Failed !!\n", sz);
		goto opps;
	}

	lseek(fd, 0 , SEEK_SET);
	data = (char*)malloc(sz);
	if(data == 0) goto opps;
	memset(data, 0x0, sz);

	if(read(fd, data, sz) != sz) goto opps;

	close(fd);

	if(size) *size = sz;

	return data;

opps:

	close(fd);
	if(data !=0) free(data);
	return 0;
	
}

int main(int argc, char *argv[])
{

	if(argc < 5){
		ussage();
	}
		
	int ImageCnt = atoi(argv[2]);	
	
	SPLASH_IMAGE_Header_info_t	splash_image;
	SPLASH_BUFFER_t *splash_buffer = malloc(sizeof(SPLASH_BUFFER_t)*ImageCnt);
	unsigned int pAddr=0, pSize=0, pPad=0, page_size = DEFAULT_PAGE_SIZE;
	unsigned int pagemask = page_size -1;
	int fin , fout;	
	int ret, idx, size;
	char *PartitionName;
	char **image_name;
	char **image_resolution;
	char *part_name;

	page_size = atoi(argv[1]);

	printf("pagesize : %d\n" ,page_size);

	PartitionName = SPLASH_TAG;

	memset(&splash_image,0x0, sizeof(splash_image));
	memcpy(splash_image.ucPartition , PartitionName, SPLASH_TAG_SIZE);

	image_name = argv+3;
	image_resolution = image_name+ImageCnt;
	splash_image.ulNumber = ImageCnt;

	printf(" ### %s image number = %d \n",splash_image.ucPartition,splash_image.ulNumber );
		
	for( idx = 0; idx < ImageCnt ; idx++)
	{

		if((splash_buffer[idx].data = load_file(image_name[idx] , &size))){

			if(strrchr(image_name[idx], '/') != NULL) part_name = strtok((strrchr(image_name[idx] , '/')+1) ,".");
			else part_name = strtok(image_name[idx],".");
			memcpy(splash_image.SPLASH_IMAGE[idx].ucImageName, part_name, strlen(part_name));//save 16 ch

			if(idx ==0) splash_image.SPLASH_IMAGE[idx].ulImageAddr = page_size + (pAddr + pSize);
			else splash_image.SPLASH_IMAGE[idx].ulImageAddr = (pAddr + pSize + pPad);

			splash_image.SPLASH_IMAGE[idx].ulImageSize = size;		
			splash_buffer[idx].size = size;
			splash_image.SPLASH_IMAGE[idx].ulImageWidth = atoi(strtok(image_resolution[idx], "x"));
			splash_image.SPLASH_IMAGE[idx].ulImageHeight = atoi(strtok(NULL, " "));
			if((splash_image.SPLASH_IMAGE[idx].ulImageSize & pagemask) !=0) splash_image.SPLASH_IMAGE[idx].Padding = page_size - (size & pagemask);
			else splash_image.SPLASH_IMAGE[idx].Padding = 0;

			pAddr = splash_image.SPLASH_IMAGE[idx].ulImageAddr; 
			pSize = splash_image.SPLASH_IMAGE[idx].ulImageSize;
			pPad = splash_image.SPLASH_IMAGE[idx].Padding;

			printf(" ### %d %d \n",splash_image.SPLASH_IMAGE[idx].ulImageSize,size);
			printf(" ### %s  \n",splash_image.SPLASH_IMAGE[idx].ucImageName);
			printf(" ### %d  \n",splash_image.SPLASH_IMAGE[idx].ulImageAddr);
			printf(" ### %d  \n",splash_image.SPLASH_IMAGE[idx].Padding);
			printf(" ### %d  \n",splash_image.SPLASH_IMAGE[idx].Padding);
			printf(" ### %d  \n",splash_image.SPLASH_IMAGE[idx].ulImageWidth);
			printf(" ### %d  \n",splash_image.SPLASH_IMAGE[idx].ulImageHeight);
		}
		
	}	

	
	fout = open( argv[argc-1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	
	if( fout<0 ) {
		printf("[error] Can not open header for writing\n");
		return -1;
	}
	
	ret = write( fout, &splash_image, sizeof(splash_image));
	if(ret ==0)
		printf("[error] Can not make file !!!\n");

	ret = write_padding(fout, page_size, sizeof(splash_image));
	if(ret < 0) return -1;

	for(idx =0 ; idx < ImageCnt; idx++){

		ret = write( fout, splash_buffer[idx].data, splash_buffer[idx].size);
		if(ret == 0)
			printf("[error] Can not make file !!!\n");

		ret = write_padding(fout, page_size, splash_buffer[idx].size);
		if(ret < 0) return -1;

		free(splash_buffer[idx].data);
	}

	printf("sizeof(struct display_header) : %d\n", (unsigned int)sizeof(splash_image));

	free(splash_buffer);

	return 0;
}
