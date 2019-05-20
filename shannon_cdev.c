#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>

#include "shannon_device.h"
#include "shannon_memblock.h"

int debug_cdev_open(struct inode *inode, struct file *file)
{
	struct debug_cdev *dev;

	dev = container_of((shannon_cdev_t *)inode->i_cdev, struct debug_cdev, cdev);
	file->private_data = dev;

	return 0;
}

int debug_cdev_release(struct inode *inode, struct file *file)
{
	return 0;
}

ssize_t debug_cdev_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos)
{
	struct debug_cdev *dev = file->private_data;
	struct scatter_memblock *smb;
	size_t remain;
	size_t data_len;
	size_t buf_offset = 0;
	ssize_t ret = 0;

	if (*f_pos >= dev->size)
		goto out;
	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	if (dev->type == NORMAL_TYPE) {
		if(copy_to_user(buf, dev->buf + *f_pos, count)) {
			ret = -EFAULT;
			goto out;
		}
		*f_pos += count;
	} else if (dev->type == MAPTABLE_MEMBLOCK_TYPE) {
		smb = (struct scatter_memblock *)dev->buf;
		remain = count;
		while (remain > 0) {
			data_len = (remain < (MAPTABLE_MEMBLOCK_SIZE - ((*f_pos) % MAPTABLE_MEMBLOCK_SIZE)) ? remain : (MAPTABLE_MEMBLOCK_SIZE - ((*f_pos) % MAPTABLE_MEMBLOCK_SIZE)));
			if (((*f_pos) % MAPTABLE_MEMBLOCK_SIZE) + data_len > MAPTABLE_MEMBLOCK_SIZE)
				shannon_err("f_pos + data_len > memblock_size, offset=%ld, f_pos%mb->memblock_size=%ld, data_len=%ld\n", *f_pos, (*f_pos) % MAPTABLE_MEMBLOCK_SIZE, data_len);
			if (unlikely(check_and_alloc_memblock(smb, *f_pos))) {
				ret = -EFAULT;
				goto out;
			}
			if (copy_to_user(buf + buf_offset, ((u8 *)smb->memblock_list[(*f_pos) / MAPTABLE_MEMBLOCK_SIZE]) + ((*f_pos) % MAPTABLE_MEMBLOCK_SIZE),  data_len)) {
				ret = -EFAULT;
				goto out;
			}
			*f_pos += data_len;
			remain -= data_len;
			buf_offset += data_len;
		}
	} else if (dev->type == TEMPTABLE_MEMBLOCK_TYPE) {
		smb = (struct scatter_memblock *)dev->buf;
		remain = count;
		while (remain > 0) {
			data_len = (remain < (TEMPTABLE_MEMBLOCK_SIZE - ((*f_pos) % TEMPTABLE_MEMBLOCK_SIZE)) ? remain : (TEMPTABLE_MEMBLOCK_SIZE - ((*f_pos) % TEMPTABLE_MEMBLOCK_SIZE)));
			if (((*f_pos) % TEMPTABLE_MEMBLOCK_SIZE) + data_len > TEMPTABLE_MEMBLOCK_SIZE)
				shannon_err("f_pos + data_len > memblock_size, offset=%ld, f_pos%mb->memblock_size=%ld, data_len=%ld\n", *f_pos, (*f_pos) % TEMPTABLE_MEMBLOCK_SIZE, data_len);
			if (unlikely(check_and_alloc_memblock(smb, *f_pos))) {
				ret = -EFAULT;
				goto out;
			}
			if (copy_to_user(buf + buf_offset, ((u8 *)(&smb->memblock_list[(*f_pos) / TEMPTABLE_MEMBLOCK_SIZE]->temptable_slot[0])) + ((*f_pos) % TEMPTABLE_MEMBLOCK_SIZE),  data_len)) {
				ret = -EFAULT;
				goto out;
			}
			*f_pos += data_len;
			remain -= data_len;
			buf_offset += data_len;
		}
	}

	ret = count;
out:
	return ret;
}

ssize_t debug_cdev_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct debug_cdev *dev = file->private_data;
	struct scatter_memblock *smb;
	size_t remain;
	size_t data_len;
	size_t buf_offset = 0;
	ssize_t ret = 0;

	if (*f_pos >= dev->size)
		goto out;
	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	if (dev->type == NORMAL_TYPE) {
		if (copy_from_user(dev->buf + *f_pos, buf, count)) {
			shannon_err("copy_from_user failed.\n");
			ret = -EFAULT;
			goto out;
		}

		*f_pos += count;
	} else if (dev->type == MAPTABLE_MEMBLOCK_TYPE) {
		smb = (struct scatter_memblock *)dev->buf;
		remain = count;
		while (remain > 0) {
			data_len = (remain < (MAPTABLE_MEMBLOCK_SIZE - ((*f_pos) % MAPTABLE_MEMBLOCK_SIZE)) ? remain : (MAPTABLE_MEMBLOCK_SIZE - ((*f_pos) % MAPTABLE_MEMBLOCK_SIZE)));
			if (((*f_pos) % MAPTABLE_MEMBLOCK_SIZE) + data_len > MAPTABLE_MEMBLOCK_SIZE)
				shannon_err("f_pos + data_len > memblock_size, offset=%ld, f_pos%smb->memblock_size=%ld, data_len=%ld\n", *f_pos, (*f_pos) % MAPTABLE_MEMBLOCK_SIZE, data_len);
			if (unlikely(check_and_alloc_memblock(smb, *f_pos))) {
				ret = -EFAULT;
				goto out;
			}
			if (copy_from_user((u8 *)(smb->memblock_list[(*f_pos) / MAPTABLE_MEMBLOCK_SIZE]) + ((*f_pos) % MAPTABLE_MEMBLOCK_SIZE), buf + buf_offset, data_len)) {
				ret = -EFAULT;
				goto out;
			}
			*f_pos += data_len;
			remain -= data_len;
			buf_offset += data_len;
		}
	} else if (dev->type == TEMPTABLE_MEMBLOCK_TYPE) {
		smb = (struct scatter_memblock *)dev->buf;
		remain = count;
		while (remain > 0) {
			data_len = (remain < (TEMPTABLE_MEMBLOCK_SIZE - ((*f_pos) % TEMPTABLE_MEMBLOCK_SIZE)) ? remain : (TEMPTABLE_MEMBLOCK_SIZE - ((*f_pos) % TEMPTABLE_MEMBLOCK_SIZE)));
			if (((*f_pos) % TEMPTABLE_MEMBLOCK_SIZE) + data_len > TEMPTABLE_MEMBLOCK_SIZE)
				shannon_err("f_pos + data_len > memblock_size, offset=%ld, f_pos%smb->memblock_size=%ld, data_len=%ld\n", *f_pos, (*f_pos) % TEMPTABLE_MEMBLOCK_SIZE, data_len);
			if (unlikely(check_and_alloc_memblock(smb, *f_pos))) {
				ret = -EFAULT;
				goto out;
			}
			if (copy_from_user((u8 *)(&smb->memblock_list[(*f_pos) / TEMPTABLE_MEMBLOCK_SIZE]->temptable_slot[0]) + ((*f_pos) % TEMPTABLE_MEMBLOCK_SIZE), buf + buf_offset, data_len)) {
				ret = -EFAULT;
				goto out;
			}
			*f_pos += data_len;
			remain -= data_len;
			buf_offset += data_len;
		}
	}
	ret = count;
out:
	return ret;
}

loff_t debug_cdev_llseek(struct file *filp, loff_t off, int whence)
{
	struct debug_cdev *dev = filp->private_data;
	loff_t newpos;

	switch(whence) {
	case SEEK_SET:
		newpos = off;
		break;
	case SEEK_CUR:
		newpos = filp->f_pos + off;
		break;
	case SEEK_END:
		newpos = dev->size + off;
		break;
	default: /* can't happen */
		return -EINVAL;
	}

	if (newpos < 0) return -EINVAL;
	filp->f_pos = newpos;
	return newpos;
}

struct file_operations debug_cdev_fops = {
	.owner	= THIS_MODULE,
	.open	= debug_cdev_open,
	.release = debug_cdev_release,
	.read	= debug_cdev_read,
	.write	= debug_cdev_write,
	.llseek = debug_cdev_llseek,
};
EXPORT_SYMBOL(debug_cdev_fops);
