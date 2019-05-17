#ifndef __SHANNON_FILE_H
#define __SHANNON_FILE_H
#include "shannon_kcore.h"

struct __shannon_file_operations {
	RESERVE_MEM(248);
};

struct __shannon_seq_operations {
	RESERVE_MEM(64);
};


typedef void shannon_file_t;
typedef void shannon_inode_t;
typedef void shannon_dentry_t;
typedef void shannon_seq_file_t;
typedef struct __shannon_seq_operations shannon_seq_operations_t;
typedef struct __shannon_file_operations shannon_file_operations_t;


//  seq_file.h
extern int shannon_seq_open(shannon_file_t *file, shannon_seq_operations_t *op);
extern shannon_ssize_t shannon_seq_read(shannon_file_t *file, char __user *buf, shannon_size_t size, shannon_loff_t *ppos);
extern int shannon_seq_puts(shannon_seq_file_t *m, const char *s);
extern int shannon_seq_printf(shannon_seq_file_t *m, const char *f, ...);
extern int shannon_single_open(shannon_file_t *file, int (*show)(shannon_seq_file_t *, void *), void *data);
extern int shannon_single_release(shannon_inode_t *inode, shannon_file_t *file);
extern struct shannon_list_head *shannon_seq_list_start(struct shannon_list_head *head, shannon_loff_t pos);
extern struct shannon_list_head *shannon_seq_list_start_head(struct shannon_list_head *head, shannon_loff_t pos);
extern struct shannon_list_head *shannon_seq_list_next(void *v, struct shannon_list_head *head, shannon_loff_t *ppos);
extern shannon_loff_t shannon_seq_lseek(shannon_file_t *file, shannon_loff_t offset, int origin);
extern int shannon_seq_release(shannon_inode_t *inode, shannon_file_t *file);

extern void *shannon_file_private_data(shannon_file_t *file);
extern void shannon_set_file_private_data(shannon_file_t *file, void *pv);
extern void *shannon_seq_file_private(shannon_seq_file_t *file);
extern void shannon_set_seq_file_private(shannon_seq_file_t *sp, void *pv);
extern void *shannon_inode_i_private(shannon_inode_t *inode);

extern void shannon_set_file_ops_owner(shannon_file_operations_t *fops, void *owner);
extern void shannon_set_file_ops_llseek_handler(shannon_file_operations_t *fops, void *llseek);
extern void shannon_set_file_ops_read_handler(shannon_file_operations_t *fops, void *read);
extern void shannon_set_file_ops_write_handler(shannon_file_operations_t *fops, void *write);
extern void shannon_set_file_ops_open_handler(shannon_file_operations_t *fops, void *open);
extern void shannon_set_file_ops_release_handler(shannon_file_operations_t *fops, void *release);

extern void shannon_set_seq_ops_start_handler(shannon_seq_operations_t *sops, void *start);
extern void shannon_set_seq_ops_stop_handler(shannon_seq_operations_t *sops, void *stop);
extern void shannon_set_seq_ops_next_handler(shannon_seq_operations_t *sops, void *next);
extern void shannon_set_seq_ops_show_handler(shannon_seq_operations_t *sops, void *show);

//  debugfs.h
extern shannon_dentry_t *shannon_debugfs_create_dir(const char *name, shannon_dentry_t *parent);
extern shannon_dentry_t *shannon_debugfs_create_file(const char *name, shannon_mode_t mode, shannon_dentry_t *parent, void *data, const shannon_file_operations_t *fops);
extern void shannon_debugfs_remove(shannon_dentry_t *dentry);


#endif /* __SHANNON_FILE_H */
