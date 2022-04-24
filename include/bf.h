#ifndef _BF_H_
#define _BF_H_

#define FUSE_USE_VERSION 26
#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>
#include "fcntl.h"
#include "string.h"
#include "fuse.h"
#include <stddef.h>
#include "ddriver.h"
#include "errno.h"
#include "types.h"

#define 		BF_ERROR_IS_NULL		0
#define 		BF_ERROR_NOTFOUND		ENOENT
#define			BF_ERROR_NOSPACE		ENOSPC
#define			BF_ERROR_EXIST			EEXIST
#define			BF_ERROR_UNSUPPORTED	ENXIO
#define			BF_ERROR_ACCESS			EACCES
#define			BF_ERROR_IO				EIO
#define			BF_ERROR_ISDIR			EISDIR
#define			BF_ERROR_INVAL			EINVAL
#define			BF_ERROR_SEEK			ESPIPE

/******************************************************************************
* SECTION: bf_utils.c
******************************************************************************/
char*				getFileName(const char *path);
int					bf_driver_read(uint8_t *output, off_t offset, int size);
int					bf_driver_write(uint8_t *input, off_t offset, int size);
struct dentry*		bf_lookup(const char *path, boolean *find, boolean *root);

struct dentry* 		bf_init_dentry(const char *name, FILE_TYPE type);
int					bf_alloc_dentry(struct inode *inode, struct dentry *dentry);
struct dentry*		bf_get_dentry(struct inode *inode, off_t offset);
int					bf_drop_dentry(struct dentry *dentry);

struct inode*		bf_alloc_inode(struct dentry *dentry);
int					bf_drop_inode(struct inode* inode);

struct inode*		bf_read_inode(struct dentry* dentry, int ino);
int					bf_sync_inode(struct inode* inode);

int					bf_mount();
int					bf_unmount();

/******************************************************************************
* SECTION: bf.c
*******************************************************************************/
void* 			   bf_init(struct fuse_conn_info *);
void  			   bf_destroy(void *);
int   			   bf_mkdir(const char *, mode_t);
int   			   bf_getattr(const char *, struct stat *);
int   			   bf_readdir(const char *, void *, fuse_fill_dir_t, off_t,
						                struct fuse_file_info *);
int   			   bf_mknod(const char *, mode_t, dev_t);
int   			   bf_write(const char *, const char *, size_t, off_t,
					                  struct fuse_file_info *);
int   			   bf_read(const char *, char *, size_t, off_t,
					                 struct fuse_file_info *);
int   			   bf_access(const char *, int);
int   			   bf_unlink(const char *);
int   			   bf_rmdir(const char *);
int   			   bf_rename(const char *, const char *);
int   			   bf_utimens(const char *, const struct timespec tv[2]);
int   			   bf_truncate(const char *, off_t);
			
int   			   bf_open(const char *, struct fuse_file_info *);
int   			   bf_opendir(const char *, struct fuse_file_info *);

#endif  /* _bf_H_ */