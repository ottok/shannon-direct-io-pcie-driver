#include "shannon_file.h"
#include "shannon_list.h"
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include <linux/version.h>

typedef shannon_loff_t  (*file_ops_llseek_fn)  (struct file *, shannon_loff_t, int);
typedef shannon_ssize_t (*file_ops_read_fn)    (struct file *, char *, shannon_size_t, shannon_loff_t *);
typedef shannon_ssize_t (*file_ops_write_fn)   (struct file *, const char *, shannon_size_t, shannon_loff_t *);
typedef int     (*file_ops_open_fn)    (struct inode *, struct file *);
typedef int     (*file_ops_release_fn) (struct inode *, struct file *);

typedef void * (*seq_ops_start_fn) (struct seq_file *m, shannon_loff_t *pos);
typedef void   (*seq_ops_stop_fn)  (struct seq_file *m, void *v);
typedef void * (*seq_ops_next_fn)  (struct seq_file *m, void *v, shannon_loff_t *pos);
typedef int    (*seq_ops_show_fn)  (struct seq_file *m, void *v);

int shannon_seq_open(shannon_file_t *file, shannon_seq_operations_t *op)
{
	return seq_open((struct file *)file, (struct seq_operations *)op);
}

shannon_ssize_t shannon_seq_read(shannon_file_t *file, char __user *buf, shannon_size_t size, shannon_loff_t *ppos)
{
	return seq_read((struct file *)file, buf, size, ppos);
}

int shannon_seq_puts(shannon_seq_file_t *m, const char *s)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 3, 0)
	return seq_puts((struct seq_file *)m, s);
#else
	seq_puts((struct seq_file *)m, s);

	return 0;
#endif
}

int shannon_seq_printf(shannon_seq_file_t *m, const char *f, ...)
{
	va_list args;
	int len;
	struct seq_file *mm = (struct seq_file *)m;

	if (mm->count < mm->size) {
		va_start(args, f);
		len = vsnprintf(mm->buf + mm->count, mm->size - mm->count, f, args);
		va_end(args);
		if (mm->count + len < mm->size) {
			mm->count += len;
			return 0;
		}
	}
	mm->count = mm->size;
	return -1;
}

int shannon_single_open(shannon_file_t *file, int (*show)(shannon_seq_file_t *, void *), void *data)
{
	return single_open((struct file *)file, (seq_ops_show_fn)show, data);
}

int shannon_single_release(shannon_inode_t *inode, shannon_file_t *file)
{
	return single_release((struct inode *)inode, (struct file *)file);
}

shannon_loff_t shannon_seq_lseek(shannon_file_t *file, shannon_loff_t offset, int origin)
{
	return seq_lseek((struct file *)file, offset, origin);
}

int shannon_seq_release(shannon_inode_t *inode, shannon_file_t *file)
{
	return seq_release((struct inode *)inode, (struct file *)file);
}

struct shannon_list_head *shannon_seq_list_start(struct shannon_list_head *head, shannon_loff_t pos)
{
	struct shannon_list_head *lh;

	shannon_list_for_each(lh, head)
		if (pos-- == 0)
			return lh;

	return NULL;
}

struct shannon_list_head *shannon_seq_list_start_head(struct shannon_list_head *head, shannon_loff_t pos)
{
	if (!pos)
		return head;

	return shannon_seq_list_start(head, pos - 1);
}

struct shannon_list_head *shannon_seq_list_next(void *v, struct shannon_list_head *head, shannon_loff_t *ppos)
{
	struct shannon_list_head *lh;

	lh = ((struct shannon_list_head *)v)->next;
	++*ppos;
	return lh == head ? NULL : lh;
}

void * shannon_file_private_data(shannon_file_t *fp)
{
	return ((struct file *)fp)->private_data;
}

void shannon_set_file_private_data(shannon_file_t *file, void *pv)
{
	((struct file *)file)->private_data = pv;
}

void * shannon_seq_file_private(shannon_seq_file_t *fsp)
{
	return ((struct seq_file *)fsp)->private;
}

void shannon_set_seq_file_private(shannon_seq_file_t *sp, void *pv)
{
	((struct seq_file *)sp)->private = pv;
}

void *shannon_inode_i_private(shannon_inode_t *inode)
{
	return ((struct inode *)inode)->i_private;
}


void shannon_set_file_ops_owner(shannon_file_operations_t *fops, void *owner)
{
	((struct file_operations *)fops)->owner = (struct module *)owner;
}

void shannon_set_file_ops_llseek_handler(shannon_file_operations_t *fops, void *llseek)
{
	((struct file_operations *)fops)->llseek = (file_ops_llseek_fn)llseek;
}

void shannon_set_file_ops_read_handler(shannon_file_operations_t *fops, void *read)
{
	((struct file_operations *)fops)->read = (file_ops_read_fn)read;
}

void shannon_set_file_ops_write_handler(shannon_file_operations_t *fops, void *write)
{
	((struct file_operations *)fops)->write = (file_ops_write_fn)write;
}

void shannon_set_file_ops_open_handler(shannon_file_operations_t *fops, void *open)
{
	((struct file_operations *)fops)->open = (file_ops_open_fn)open;
}

void shannon_set_file_ops_release_handler(shannon_file_operations_t *fops, void *release)
{
	((struct file_operations *)fops)->release = (file_ops_release_fn)release;
}

void shannon_set_seq_ops_start_handler(shannon_seq_operations_t *sops, void *start)
{
	((struct seq_operations *)sops)->start = (seq_ops_start_fn)start;
}

void shannon_set_seq_ops_stop_handler(shannon_seq_operations_t *sops, void *stop)
{
	((struct seq_operations *)sops)->stop = (seq_ops_stop_fn)stop;
}

void shannon_set_seq_ops_next_handler(shannon_seq_operations_t *sops, void *next)
{
	((struct seq_operations *)sops)->next = (seq_ops_next_fn)next;
}

void shannon_set_seq_ops_show_handler(shannon_seq_operations_t *sops, void *show)
{
	((struct seq_operations *)sops)->show = (seq_ops_show_fn)show;
}


//  debugfs.h
shannon_dentry_t *shannon_debugfs_create_dir(const char *name, shannon_dentry_t *parent)
{
	return debugfs_create_dir(name, (struct dentry *)parent);
}

shannon_dentry_t *shannon_debugfs_create_file(const char *name, shannon_mode_t mode, shannon_dentry_t *parent, void *data, const shannon_file_operations_t *fops){
	return debugfs_create_file(name, mode, (struct dentry *)parent, data, (struct file_operations *)fops);
}

void shannon_debugfs_remove(shannon_dentry_t *dentry)
{
	debugfs_remove((struct dentry *)dentry);
}
