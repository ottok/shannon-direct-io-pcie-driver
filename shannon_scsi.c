#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_eh.h>
#include <linux/pci.h>
#include <linux/version.h>
#include <linux/scatterlist.h>
#include <linux/blkdev.h>
#include <linux/highmem.h>
#include <linux/module.h>
#include "shannon_port.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
/* Returns 0 if ok else (DID_ERROR << 16). Sets scp->resid . */
int shannon_fill_from_dev_buffer(void *scsi_cmnd, unsigned char *arr, int arr_len)
{
	struct scsi_cmnd *scp = (struct scsi_cmnd *)scsi_cmnd;
	int k, req_len, act_len, len, active;
	void * kaddr;
	void * kaddr_off;
	struct scatterlist * sgpnt;

	if (0 == scp->request_bufflen)
		return 0;
	if (NULL == scp->request_buffer)
		return (DID_ERROR << 16);
	if (! ((scp->sc_data_direction == DMA_BIDIRECTIONAL) ||
	      (scp->sc_data_direction == DMA_FROM_DEVICE)))
		return (DID_ERROR << 16);
	if (0 == scp->use_sg) {
		req_len = scp->request_bufflen;
		act_len = (req_len < arr_len) ? req_len : arr_len;
		memcpy(scp->request_buffer, arr, act_len);
		if (scp->resid)
			scp->resid -= act_len;
		else
			scp->resid = req_len - act_len;
		return 0;
	}
	sgpnt = (struct scatterlist *)scp->request_buffer;
	active = 1;
	for (k = 0, req_len = 0, act_len = 0; k < scp->use_sg; ++k, ++sgpnt) {
		if (active) {
			kaddr = (unsigned char *)
				kmap_atomic(sgpnt->page, KM_USER0);
			if (NULL == kaddr)
				return (DID_ERROR << 16);
			kaddr_off = (unsigned char *)kaddr + sgpnt->offset;
			len = sgpnt->length;
			if ((req_len + len) > arr_len) {
				active = 0;
				len = arr_len - req_len;
			}
			memcpy(kaddr_off, arr + req_len, len);
			kunmap_atomic(kaddr, KM_USER0);
			act_len += len;
		}
		req_len += sgpnt->length;
	}
	if (scp->resid)
		scp->resid -= act_len;
	else
		scp->resid = req_len - act_len;
	return 0;
}

#elif LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 25)
/* Returns 0 if ok else (DID_ERROR << 16). Sets scp->resid . */
int shannon_fill_from_dev_buffer(void *scsi_cmnd, unsigned char *arr, int arr_len)
{
	struct scsi_cmnd *scp = (struct scsi_cmnd *)scsi_cmnd;
	int k, req_len, act_len, len, active;
	void * kaddr;
	void * kaddr_off;
	struct scatterlist * sg;

	if (0 == scp->request_bufflen)
		return 0;
	if (NULL == scp->request_buffer)
		return (DID_ERROR << 16);
	if (! ((scp->sc_data_direction == DMA_BIDIRECTIONAL) ||
	      (scp->sc_data_direction == DMA_FROM_DEVICE)))
		return (DID_ERROR << 16);
	if (0 == scp->use_sg) {
		req_len = scp->request_bufflen;
		act_len = (req_len < arr_len) ? req_len : arr_len;
		memcpy(scp->request_buffer, arr, act_len);
		if (scp->resid)
			scp->resid -= act_len;
		else
			scp->resid = req_len - act_len;
		return 0;
	}
	active = 1;
	req_len = act_len = 0;
	scsi_for_each_sg(scp, sg, scp->use_sg, k) {
		if (active) {
			kaddr = (unsigned char *)
				kmap_atomic(sg_page(sg), KM_USER0);
			if (NULL == kaddr)
				return (DID_ERROR << 16);
			kaddr_off = (unsigned char *)kaddr + sg->offset;
			len = sg->length;
			if ((req_len + len) > arr_len) {
				active = 0;
				len = arr_len - req_len;
			}
			memcpy(kaddr_off, arr + req_len, len);
			kunmap_atomic(kaddr, KM_USER0);
			act_len += len;
		}
		req_len += sg->length;
	}
	if (scp->resid)
		scp->resid -= act_len;
	else
		scp->resid = req_len - act_len;
	return 0;
}

#elif LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26)
/* Returns 0 if ok else (DID_ERROR << 16). Sets scp->resid . */
int shannon_fill_from_dev_buffer(void *scsi_cmnd, unsigned char *arr, int arr_len)
{
	struct scsi_cmnd *scp = (struct scsi_cmnd *)scsi_cmnd;
	int k, req_len, act_len, len, active;
	void * kaddr;
	void * kaddr_off;
	struct scatterlist *sg;
	struct scsi_data_buffer *sdb = scsi_in(scp);

	if (!sdb->length)
		return 0;
	if (!sdb->table.sgl)
		return (DID_ERROR << 16);
	if (!(scsi_bidi_cmnd(scp) || scp->sc_data_direction == DMA_FROM_DEVICE))
		return (DID_ERROR << 16);
	active = 1;
	req_len = act_len = 0;
	for_each_sg(sdb->table.sgl, sg, sdb->table.nents, k) {
		if (active) {
			kaddr = (unsigned char *)
				kmap_atomic(sg_page(sg), KM_USER0);
			if (NULL == kaddr)
				return (DID_ERROR << 16);
			kaddr_off = (unsigned char *)kaddr + sg->offset;
			len = sg->length;
			if ((req_len + len) > arr_len) {
				active = 0;
				len = arr_len - req_len;
			}
			memcpy(kaddr_off, arr + req_len, len);
			kunmap_atomic(kaddr, KM_USER0);
			act_len += len;
		}
		req_len += sg->length;
	}
	if (sdb->resid)
		sdb->resid -= act_len;
	else
		sdb->resid = req_len - act_len;
	return 0;
}

#elif LINUX_VERSION_CODE < KERNEL_VERSION(5, 1, 0)
/* Returns 0 if ok else (DID_ERROR << 16). Sets scp->resid . */
int shannon_fill_from_dev_buffer(void *scsi_cmnd, unsigned char *arr, int arr_len)
{
	struct scsi_cmnd *scp = (struct scsi_cmnd *)scsi_cmnd;
	int act_len;
	struct scsi_data_buffer *sdb = scsi_in(scp);

	if (!sdb->length)
		return 0;
	if (!(scsi_bidi_cmnd(scp) || scp->sc_data_direction == DMA_FROM_DEVICE))
		return DID_ERROR << 16;

	act_len = sg_copy_from_buffer(sdb->table.sgl, sdb->table.nents,
				      arr, arr_len);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
	sdb->resid = scsi_bufflen(scp) - act_len;
#else
	if (sdb->resid)
		sdb->resid -= act_len;
	else
		sdb->resid = scsi_bufflen(scp) - act_len;
#endif

	return 0;
}

#else // LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
int shannon_fill_from_dev_buffer(void *scsi_cmnd, unsigned char *arr, int arr_len)
{
	struct scsi_cmnd *scp = (struct scsi_cmnd *)scsi_cmnd;
	int act_len;
	struct scsi_data_buffer *sdb = &scp->sdb;

	if (!sdb->length)
		return 0;
	if (scp->sc_data_direction != DMA_FROM_DEVICE)
		return DID_ERROR << 16;

	act_len = sg_copy_from_buffer(sdb->table.sgl, sdb->table.nents,
					arr, arr_len);
	// scsi_set_resid(scp, scsi_bufflen(scp) - act_len);
	scsi_set_resid(scp, scsi_bufflen(scp) - act_len);

	return 0;
}
#endif /* end of LINUX_VERSION_CODE for shannon_fill_from_dev_buffer() */

void shannon_build_sense_buffer(char *sbuff, int key, int asc, int asq)
{
	memset(sbuff, 0, SHANNON_SCSI_SENSE_LEN);
	scsi_build_sense_buffer(0, sbuff, key, asc, asq);
}

void end_scsi_cmnd(struct shannon_bio *sbio, enum shannon_scsi_cmd_status scsi_status, unsigned char *sense_buffer)
{
	struct scsi_cmnd *scsi_cmnd = (struct scsi_cmnd *)sbio->scsi_cmnd;

	if ((scsi_status & 0xff) == STATUS_CODE_CHECK_CONDITION) {
		if (scsi_cmnd && sense_buffer)
			memcpy(scsi_cmnd->sense_buffer, sense_buffer, SHANNON_SCSI_SENSE_LEN);
	}
	scsi_cmnd->result = scsi_status;
	scsi_cmnd->host_scribble = NULL;
	scsi_cmnd->scsi_done(scsi_cmnd);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)

#define sg_next(sg)            ((sg) + 1)
#define sg_last(sg, nents)     (&(sg[(nents) - 1]))

/*
 * Loop over each sg element, following the pointer to a new list if necessary
 */
#define for_each_sg(sglist, sg, nr, __i)       \
       for (__i = 0, sg = (sglist); __i < (nr); __i++, sg = sg_next(sg))

#define sg_page(sg)		((sg)->page)

#endif

int shannon_convert_scsi_scmd(struct shannon_bio *sbio, int logicb_size)
{
	struct scsi_cmnd *scsi_cmnd = (struct scsi_cmnd *)sbio->scsi_cmnd;
	struct page *page;
	shannon_sg_list_t *sg = NULL;
	int i, offset, remained_size, map_size = 0, undone = 0;
	int space_in_page;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
	struct scsi_data_buffer *sdb = &scsi_cmnd->sdb;
	struct scatterlist *sgl = sdb->table.sgl, *sge;
	sbio->bio_size = sdb->length;
#else
	struct scatterlist *sgl = (struct scatterlist *)scsi_cmnd->request_buffer, *sge;
	sbio->bio_size = scsi_cmnd->request_bufflen;
#endif

	if (scsi_cmnd->sc_data_direction == DMA_FROM_DEVICE)
		sbio->dma_dir = SHANNON_DMA_FROMDEVICE;
	else if (scsi_cmnd->sc_data_direction == DMA_TO_DEVICE)
		sbio->dma_dir = SHANNON_DMA_TODEVICE;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
	sbio->segments = sdb->table.nents;
#else
	sbio->segments = scsi_cmnd->use_sg;
	BUG_ON(0 == scsi_cmnd->use_sg);
#endif
	sbio->sg_count = 2 * ((sbio->bio_size + logicb_size - 1)/logicb_size) + sbio->segments;
	sbio->sg = shannon_sg_alloc(sbio->sg_count, GFP_ATOMIC);
	if (sbio->sg == NULL) {
		shannon_err("alloc sg failed.\n");
		return -ENOMEM;
	}
	shannon_sg_init_table(sbio->sg, sbio->sg_count);

	sbio->first_size = logicb_size - ((sbio->start_sector << 9) & (logicb_size - 1));
	sbio->first_size = sbio->first_size % logicb_size;
	sbio->first_size = min(sbio->bio_size, sbio->first_size);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
	debugs1("sdb->length=%d, bio_size=%d, start_sector=0x%lx, segments=%d, sg_count=%d, first_size=%d.\n",
			sdb->length, sbio->bio_size, sbio->start_sector, sbio->segments, sbio->sg_count, sbio->first_size);
#else
	debugs1("scsi_cmnd->request_bufflen=%d, bio_size=%d, start_sector=0x%lx, segments=%d, sg_count=%d, first_size=%d.\n",
			scsi_cmnd->request_bufflen, sbio->bio_size, sbio->start_sector, sbio->segments, sbio->sg_count, sbio->first_size);
#endif

	remained_size = sbio->first_size ? sbio->first_size : logicb_size;

	sbio->has_hole = 0;
	sbio->used_sg_count = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
	for_each_sg(sgl, sge, sdb->table.nents, i) {
		debugs1("i=%d, nents=%d, offset=%d, length=%d.\n", i, sdb->table.nents, sge->offset, sge->length);
#else
	for_each_sg(sgl, sge, scsi_cmnd->use_sg, i) {
		debugs1("i=%d, nents=%d, offset=%d, length=%d.\n", i, scsi_cmnd->use_sg, sge->offset, sge->length);
#endif
		/* all segments' bv_offset have to be 0, except the first one. */
		if ((i != 0) && (sge->offset != 0))
			sbio->has_hole = 1;
		/* all segments have to end at 4096, except the last one. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
		if ((i != sdb->table.nents-1) && ((sge->offset + sge->length)&0xfff))
#else
		if ((i != scsi_cmnd->use_sg-1) && ((sge->offset + sge->length)&0xfff))
#endif
			sbio->has_hole = 1;

		offset = 0; /* the offset start from bvec->bv_offset. */
		while (offset < sge->length && sbio->used_sg_count < sbio->sg_count) {
			space_in_page = PAGE_SIZE - ((sge->offset + offset) % PAGE_SIZE);
			if (space_in_page < remained_size) {
				if ((sge->length - offset) < space_in_page) {
					map_size = sge->length - offset;
					undone = 1;
				} else {
					map_size = space_in_page;
					undone = 1;
				}
			} else {
				if ((sge->length - offset) < remained_size) {
					map_size = sge->length - offset;
					undone = 1;
				} else {
					map_size = remained_size;
					undone = 0;
				}
			}
			remained_size -= map_size;
			if (remained_size == 0)
				remained_size = logicb_size;
			sg = sg ? shannon_sg_next(sg) : sbio->sg;
			debugs1("offset=%d, map_size=%d.\n", offset, map_size);
			page = sg_page(sge);
			shannon_sg_set_page(sg,  page + (sge->offset + offset)/PAGE_SIZE, map_size, (sge->offset + offset) % PAGE_SIZE);
			sbio->used_sg_count++;
			offset += map_size;
		}
		BUG_ON(offset < sge->length);
	}
#if 0
	sg = sbio->sg;
	for (i = 0; i < sbio->sg_count; i++) {
		shannon_info("i=%d, sg=0x%lx, shannon_sg_page(sg)=0x%lx.\n", i, sg, shannon_sg_page(sg));
		sg = shannon_sg_next(sg);
	}
#endif

	return 0;
}

extern int shannon_receive_scsi_cmd(struct shannon_bio *sbio, unsigned char *sense_buffer);
static int shannon_scsi_queuecommand_lck(struct scsi_cmnd *scsi_cmnd, void (*done_fn)(struct scsi_cmnd *))
{
	struct Scsi_Host *shost = scsi_cmnd->device->host;
	struct scsi_device *sdevice = scsi_cmnd->device;
	struct scsi_target *starget = scsi_target(sdevice);
	// int target = scsi_cmnd->device->id;
	struct shannon_bio *sbio;
	unsigned char sense_buffer[SHANNON_SCSI_SENSE_LEN];
	int ret;

	debugs1("host=%d, channel=%d, target=%d, lun=%d, cmd=0x%x.\n",
			shost->this_id, starget->channel, starget->id, sdevice->lun, *scsi_cmnd->cmnd);
	scsi_cmnd->scsi_done = done_fn;
	sbio = alloc_sbio(GFP_NOWAIT);
	sbio->hostdata = (struct shannon_scsi_private *)shost->hostdata;
	sbio->scsi_cmnd = scsi_cmnd;
	sbio->cmd = scsi_cmnd->cmnd;

	scsi_cmnd->host_scribble = (unsigned char *)sbio;

	if (starget->id != 0) {
		debugs1("target is absent! target=%d, cmd=0x%x.\n", starget->id, *sbio->cmd);
		shannon_build_sense_buffer(sense_buffer, SENSE_KEY_ILLEGAL_REQUEST, ASC_LOGICAL_UNIT_NOT_READY, 0);
		end_scsi_cmnd(sbio, CHECK_CONDITION_STATUS, sense_buffer);
		free_sbio(sbio);
		return 0;
	}

	ret = shannon_receive_scsi_cmd(sbio, sense_buffer);
	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
DEF_SCSI_QCMD(shannon_scsi_queuecommand)
#endif

static const char *shannon_scsi_info(struct Scsi_Host *host)
{
	return "Shannon Direct IO  SCSI Adapter.\n";
}

static int shannon_scsi_biosparam(struct scsi_device *sdev, struct block_device *bdev, sector_t capacity, int geom[])
{
	int heads, sectors, cylinders;

	heads = 64;
	sectors = 32;
	cylinders = (unsigned long)capacity / (heads * sectors);
	if (cylinders > 1024) {
		heads = 255;
		sectors = 63;
		cylinders = (unsigned long)capacity / (heads * sectors);
	}
	geom[0] = heads;
	geom[1] = sectors;
	geom[2] = cylinders;

	return 0;
}

static int shannon_scsi_slave_configure(struct scsi_device *sdp)
{
	shannon_info("slave_configure <%u %u %u %u>\n", sdp->host->host_no, sdp->channel, sdp->id, sdp->lun);
	if (sdp->host->max_cmd_len != 32)
		sdp->host->max_cmd_len = 32;
	if (sdp->host->cmd_per_lun)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0)
		scsi_change_queue_depth(sdp, sdp->host->cmd_per_lun);
#else
		scsi_adjust_queue_depth(sdp, 0, sdp->host->cmd_per_lun);
#endif
	blk_queue_max_segment_size(sdp->request_queue, 512 * 1024);
	return 0;
}


static struct scsi_host_template shannon_scsi_template = {
	.module         = THIS_MODULE,
	.name           = "Shannon Direct IO",
	.info           = shannon_scsi_info,
	.slave_configure        = shannon_scsi_slave_configure,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
	.queuecommand           = shannon_scsi_queuecommand_lck,
#else
	.queuecommand           = shannon_scsi_queuecommand,
#endif
	.bios_param             = shannon_scsi_biosparam,
	.can_queue              = 0xfffff,
	.this_id                = -1,
	.sg_tablesize           = SG_ALL,
	.cmd_per_lun            = 0x3fff,
	.max_sectors            = 0xffff, /* The max size of a bio */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
	.use_clustering         = DISABLE_CLUSTERING,
#else
	.dma_boundary		= PAGE_SIZE - 1,
#endif
};

extern void shannon_remove(void *data, shannon_pci_dev_t *pdev);
extern int shannon_probe(shannon_pci_dev_t *pdev, const shannon_pci_device_id_t *id, struct shannon_scsi_private *hostdata);
int shannon_scsi_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct Scsi_Host *shost;
	struct shannon_scsi_private *hostdata;

	shost = scsi_host_alloc(&shannon_scsi_template, sizeof(struct shannon_scsi_private));
	hostdata = (struct shannon_scsi_private *)shost->hostdata;
	shannon_memset(hostdata, 0, sizeof(struct shannon_scsi_private));
	hostdata->scsi_host = shost;
	hostdata->scsi_host_no = shost->host_no;
	shannon_pci_set_drvdata(pdev, hostdata);
	shannon_spin_lock_init(&hostdata->stats_lock);
	hostdata->sectors[0] = hostdata->sectors[1] = 0;
	hostdata->ios[0] = hostdata->ios[1] = 0;
	hostdata->ticks[0] = hostdata->ticks[1] = 0;
	hostdata->in_flight[0] = hostdata->in_flight[1] = 0;
	return shannon_probe(pdev, id, hostdata);
}

void shannon_scsi_remove(struct pci_dev *pdev)
{
	struct shannon_scsi_private *hostdata = shannon_pci_get_drvdata(pdev);
	shannon_remove(hostdata->sdev, pdev);
	scsi_host_put(hostdata->scsi_host);
}

int shannon_attach_scsi(struct shannon_scsi_private *private, void *data)
{
	struct pci_dev *pdev = (struct pci_dev *)data;
	debugs1("enter.\n");
	if (scsi_add_host(private->scsi_host, &pdev->dev))
		return -1;
	scsi_scan_host(private->scsi_host);
	debugs1("exit.\n");
	return 0;
}

void shannon_detach_scsi(struct shannon_scsi_private *private)
{
	debugs1("enter.\n");
	scsi_remove_host(private->scsi_host);
	debugs1("exit.\n");
}
