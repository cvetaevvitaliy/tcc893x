/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system. 
 * yaffs_mtdif.c  NAND mtd wrapper functions.
 *
 * Copyright (C) 2002 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

/* mtd interface for YAFFS2 */

const char *yaffs_mtdif2_c_version =
    "$Id: yaffs_mtdif2.c,v 1.14 2006/10/03 10:13:03 charles Exp $";

#include "yportenv.h"


#include "yaffs_mtdif2.h"

#include "linux/mtd/mtd.h"
#include "linux/types.h"
#include "linux/time.h"

#include "yaffs_packedtags2.h"


void nandmtd2_pt2buf(yaffs_Device *dev, yaffs_PackedTags2 *pt, int is_raw)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	__u8 *ptab = (__u8 *)pt; /* packed tags as bytes */
	
	int	i, j = 0, k, n;
#ifdef CONFIG_YAFFS_DOES_ECC
	size_t packed_size = sizeof(yaffs_PackedTags2);
#else
	size_t packed_size = sizeof(yaffs_PackedTags2TagsPart);
#endif

	/* Pack buffer with 0xff */
	for (i = 0; i < mtd->oobsize; i++)
		dev->spareBuffer[i] = 0xff;
		
	if(!is_raw){
		memcpy(dev->spareBuffer,pt,packed_size);
	} else {
		j = 0;
		k = mtd->ecclayout->oobfree[j].offset;
		n = mtd->ecclayout->oobfree[j].length;

		//printk("nandmtd2_pt2buf: writing %d bytes of extra data into %d\n", packed_size, mtd->oobsize);

		if (n == 0) {
			T(YAFFS_TRACE_ERROR, (TSTR("No OOB space for tags" TENDSTR)));
			YBUG();
		}

		for (i = 0; i < packed_size; i++) {
			if (n == 0) {
				j++;
				k = mtd->ecclayout->oobfree[j].offset;
				n = mtd->ecclayout->oobfree[j].length;
				if (n == 0 || j >= (sizeof(mtd->ecclayout->oobfree) / sizeof(mtd->ecclayout->oobfree[0]))) {
					T(YAFFS_TRACE_ERROR, (TSTR("No OOB space for tags" TENDSTR)));
					YBUG();
				}
			}
			dev->spareBuffer[k] = ptab[i];
			k++;
			n--;
		}
	}

}

void nandmtd2_buf2pt(yaffs_Device *dev, yaffs_PackedTags2 *pt, int is_raw)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	int	i, j = 0, k, n;
	__u8 *ptab = (__u8 *)pt; /* packed tags as bytes */
	size_t packed_size = dev->useNANDECC ? sizeof(yaffs_PackedTags2TagsPart) : sizeof(yaffs_PackedTags2);

	if (!is_raw) {
	
		memcpy(pt,dev->spareBuffer,packed_size);
	} else {
		j = 0;
		k = mtd->ecclayout->oobfree[j].offset;
		n = mtd->ecclayout->oobfree[j].length;

		if (n == 0) {
			T(YAFFS_TRACE_ERROR, (TSTR("No space in OOB for tags" TENDSTR)));
			YBUG();
		}

		for (i = 0; i < packed_size; i++) {
			if (n == 0) {
				j++;
				k = mtd->ecclayout->oobfree[j].offset;
				n = mtd->ecclayout->oobfree[j].length;
				if (n == 0 || j >= (sizeof(mtd->ecclayout->oobfree) / sizeof(mtd->ecclayout->oobfree[0]))) {
					T(YAFFS_TRACE_ERROR, (TSTR("No space in OOB for tags" TENDSTR)));
					YBUG();
				}
			}
			ptab[i] = dev->spareBuffer[k];
			k++;
			n--;
		}
	}
		
}

int nandmtd2_WriteChunkWithTagsToNAND(yaffs_Device * dev, int chunkInNAND,
				      const __u8 * data,
				      const yaffs_ExtendedTags * tags)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17))
	struct mtd_oob_ops ops;
#else
	size_t dummy;
#endif
	int retval = 0;

	loff_t addr = ((loff_t) chunkInNAND) * dev->nDataBytesPerChunk;

	yaffs_PackedTags2 pt;

	T(YAFFS_TRACE_MTD,
	  (TSTR
	   ("nandmtd2_WriteChunkWithTagsToNAND chunk %d data %p tags %p"
	    TENDSTR), chunkInNAND, data, tags));

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17))
	if (tags)
		yaffs_PackTags2(&pt, tags);
	else
		BUG(); /* both tags and data should always be present */

	nandmtd2_pt2buf(dev, &pt, 1);
	if (data) {
		ops.mode = MTD_OOB_PLACE;
		ops.ooblen = mtd->oobsize;
		ops.len = dev->nDataBytesPerChunk;
		ops.ooboffs = 0;
		ops.datbuf = (__u8 *)data;
		ops.oobbuf = dev->spareBuffer;
		retval = mtd->write_oob(mtd, addr, &ops);
	} else
		BUG(); /* both tags and data should always be present */
#else
	if (tags) {
		yaffs_PackTags2(&pt, tags);
	}

	if (tags) {
		nandmtd2_pt2buf(dev, &pt, 1);
		retval =
		    mtd->write_ecc(mtd, addr, dev->nDataBytesPerChunk,
				   &dummy, data, dev->spareBuffer, NULL);
	} else if (data) {
			retval =
			    mtd->write(mtd, addr, dev->nDataBytesPerChunk, &dummy,
				       data);
	}
#endif

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int nandmtd2_ReadChunkWithTagsFromNAND(yaffs_Device * dev, int chunkInNAND,
				       __u8 * data, yaffs_ExtendedTags * tags)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17))
	struct mtd_oob_ops ops;
#endif
	size_t dummy;
	int retval = 0;

	loff_t addr = ((loff_t) chunkInNAND) * dev->nDataBytesPerChunk;

	yaffs_PackedTags2 pt;

	T(YAFFS_TRACE_MTD,
	  (TSTR
	   ("nandmtd2_ReadChunkWithTagsFromNAND chunk %d data %p tags %p"
	    TENDSTR), chunkInNAND, data, tags));

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17))
	if (data && !tags)
		retval = mtd->read(mtd, addr, dev->nDataBytesPerChunk,
				&dummy, data);
	else if (tags) {
		ops.mode = MTD_OOB_PLACE;
		ops.ooblen = mtd->oobsize;
		ops.len = data ? dev->nDataBytesPerChunk : mtd->oobsize;
		ops.ooboffs = 0;
		ops.datbuf = data;
		ops.oobbuf = dev->spareBuffer;
		retval = mtd->read_oob(mtd, addr, &ops);
		nandmtd2_buf2pt(dev, &pt, 1);
	}
#else
	if (data && tags) {
			retval =
			    mtd->read_ecc(mtd, addr, dev->nDataBytesPerChunk,
					  &dummy, data, dev->spareBuffer,
					  NULL);
		} else {
			retval =
			    mtd->read_ecc(mtd, addr, dev->nDataBytesPerChunk,
					  &dummy, data, dev->spareBuffer,
					  NULL);
		}
	} else {
		if (data)
			retval =
			    mtd->read(mtd, addr, dev->nDataBytesPerChunk, &dummy,
				      data);
		if (tags) {
			retval =
			    mtd->read_oob(mtd, addr, mtd->oobsize, &dummy,
					  dev->spareBuffer);
			nandmtd2_buf2pt(dev, &pt, 1);
		}
	}
#endif

	if (tags)
		yaffs_UnpackTags2(tags, &pt);
	
	if(tags && retval == -EBADMSG && tags->eccResult == YAFFS_ECC_RESULT_NO_ERROR)
		tags->eccResult = YAFFS_ECC_RESULT_UNFIXED;

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int nandmtd2_MarkNANDBlockBad(struct yaffs_DeviceStruct *dev, int blockNo)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	int retval;
	T(YAFFS_TRACE_MTD,
	  (TSTR("nandmtd2_MarkNANDBlockBad %d" TENDSTR), blockNo));

	retval =
	    mtd->block_markbad(mtd,
			       blockNo * dev->nChunksPerBlock *
			       dev->nDataBytesPerChunk);

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;

}

int nandmtd2_QueryNANDBlock(struct yaffs_DeviceStruct *dev, int blockNo,
			    yaffs_BlockState * state, int *sequenceNumber)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	int retval;

	T(YAFFS_TRACE_MTD,
	  (TSTR("nandmtd2_QueryNANDBlock %d" TENDSTR), blockNo));
	retval =
	    mtd->block_isbad(mtd,
			     blockNo * dev->nChunksPerBlock *
			     dev->nDataBytesPerChunk);

	if (retval) {
		T(YAFFS_TRACE_MTD, (TSTR("block is bad" TENDSTR)));

		*state = YAFFS_BLOCK_STATE_DEAD;
		*sequenceNumber = 0;
	} else {
		yaffs_ExtendedTags t;
		nandmtd2_ReadChunkWithTagsFromNAND(dev,
						   blockNo *
						   dev->nChunksPerBlock, NULL,
						   &t);

		if (t.chunkUsed) {
			*sequenceNumber = t.sequenceNumber;
			*state = YAFFS_BLOCK_STATE_NEEDS_SCANNING;
		} else {
			*sequenceNumber = 0;
			*state = YAFFS_BLOCK_STATE_EMPTY;
		}

		T(YAFFS_TRACE_MTD,
		  (TSTR("block is OK seq %d state %d" TENDSTR), *sequenceNumber,
		   *state));
	}

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

