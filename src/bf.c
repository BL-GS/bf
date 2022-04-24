#include "bf.h"

/******************************************************************************
 * SECTION: 宏定义
 *******************************************************************************/
#define OPTION(t, p)                             \
	{                                            \
		t, offsetof(struct custom_options, p), 1 \
	}

/******************************************************************************
 * SECTION: 全局变量
 *******************************************************************************/
static const struct fuse_opt option_spec[] = {/* 用于FUSE文件系统解析参数 */
											  OPTION("--device=%s", device),
											  FUSE_OPT_END};

struct custom_options bf_options; /* 全局选项 */
struct super super;
#define TEST 0
/******************************************************************************
 * SECTION: FUSE操作定义
 *******************************************************************************/
static struct fuse_operations operations = {
	.init = bf_init,	   /* mount文件系统 */
	.destroy = bf_destroy, /* umount文件系统 */
	.mkdir = bf_mkdir,	   /* 建目录，mkdir */
	.getattr = bf_getattr, /* 获取文件属性，类似stat，必须完成 */
	.readdir = bf_readdir, /* 填充dentrys */
	.mknod = bf_mknod,	   /* 创建文件，touch相关 */
	.write = bf_write,		   /* 写入文件 */
	.read = bf_read,		   /* 读文件 */
	.utimens = bf_utimens, /* 修改时间，忽略，避免touch报错 */
	.truncate = NULL,	   /* 改变文件大小 */
	.unlink = bf_unlink,		   /* 删除文件 */
	.rmdir = bf_rmdir,		   /* 删除目录， rm -r */
	.rename = bf_rename,		   /* 重命名，mv */

	.open = bf_open,
	.opendir = bf_opendir,
	.access = bf_access};
/******************************************************************************
 * SECTION: 必做函数实现
 *******************************************************************************/
/**
 * @brief 挂载（mount）文件系统
 *
 * @param conn_info 可忽略，一些建立连接相关的信息
 * @return void*
 */
void *bf_init(struct fuse_conn_info *conn_info)
{
	/* 下面是一个控制设备的示例 */
	super.fd = ddriver_open(bf_options.device);
	bf_mount();

	if (TEST)
	{
		mode_t mode = 0;
		bf_mkdir("/.xdg-volume-info", mode);
		bf_mkdir("/.xdg-volume-info/.git", mode);
		bf_mknod("/autorun.inf", mode, S_IFREG);
		bf_mkdir("/.git", mode);
		bf_mkdir("/.Trash", mode);
		bf_mkdir("/.Trash", mode);
		bf_mknod("/.Trash/1000", mode, S_IFREG);
		bf_mkdir("/.Trash-1000", mode);
		bf_mknod("/.Trash-1000/files", mode, S_IFREG);
		struct stat ss;
		bf_getattr("/.git", &ss);
	}

	return NULL;
}

/**
 * @brief 卸载（umount）文件系统
 *
 * @param p 可忽略
 * @return void
 */
void bf_destroy(void *p)
{
	bf_unmount();
	ddriver_close(super.fd);

	return;
}

/**
 * @brief 创建目录
 *
 * @param path 相对于挂载点的路径
 * @param mode 创建模式（只读？只写？），可忽略
 * @return int 0成功，否则失败
 */
int bf_mkdir(const char *path, mode_t mode)
{
	struct dentry *dentry;
	struct dentry *child_dentry;
	struct inode *inode;

	boolean find;
	boolean root;

	dentry = bf_lookup(path, &find, &root);

	if (find == TRUE)
	{
		return -BF_ERROR_EXIST;
	}
	if (dentry == NULL)
	{
		return -BF_ERROR_NOTFOUND;
	}
	if (dentry->type == DEG)
	{
		return -BF_ERROR_UNSUPPORTED;
	}

	if (dentry->inode == NULL)
	{
		bf_read_inode(dentry, dentry->ino);
	}
	
	inode = dentry->inode;
	child_dentry = bf_init_dentry(getFileName(path), DIR);
	bf_alloc_dentry(inode, child_dentry);
	bf_alloc_inode(child_dentry);

	return 0;
}

/**
 * @brief 获取文件或目录的属性，该函数非常重要
 *
 * @param path 相对于挂载点的路径
 * @param bf_stat 返回状态
 * @return int 0成功，否则失败
 */
int bf_getattr(const char *path, struct stat *bf_stat)
{
	struct dentry *dentry;
	struct inode *inode;
	boolean find;
	boolean root;

	dentry = bf_lookup(path, &find, &root);
	if (find == FALSE)
	{
		return -BF_ERROR_NOTFOUND;
	}
	inode = dentry->inode;

	if (IS_DIR((*inode)))
	{
		bf_stat->st_mode = S_IFDIR | BF_DEFAULT_PERM;
		bf_stat->st_size = inode->dir_cnt * sizeof(struct bf_dentry_d);
	}
	else if (IS_DEG((*inode)))
	{
		bf_stat->st_mode = S_IFREG | BF_DEFAULT_PERM;
		bf_stat->st_size = inode->size;
	}

	bf_stat->st_nlink = 1;
	bf_stat->st_uid = getuid();
	bf_stat->st_gid = getgid();
	bf_stat->st_atime = time(NULL);
	bf_stat->st_mtime = time(NULL);
	bf_stat->st_blksize = BF_SIZE_IO;

	if (root)
	{
		bf_stat->st_size = super.sz_usage;
		bf_stat->st_blocks = BF_SIZE_DISK / BF_SIZE_IO;
		bf_stat->st_nlink = 2; /* !特殊，根目录link数为2 */
	}

	return 0;
}

/**
 * @brief 遍历目录项，填充至buf，并交给FUSE输出
 *
 * @param path 相对于挂载点的路径
 * @param buf 输出buffer
 * @param filler 参数讲解:
 *
 * typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
 *				const struct stat *stbuf, off_t off)
 * buf: name会被复制到buf中
 * name: dentry名字
 * stbuf: 文件状态，可忽略
 * off: 下一次offset从哪里开始，这里可以理解为第几个dentry
 *
 * @param offset 第几个目录项？
 * @param fi 可忽略
 * @return int 0成功，否则失败
 */
int bf_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
			   struct fuse_file_info *fi)
{
	struct dentry *dentry;
	struct dentry *sub_dentry;
	struct inode *inode;
	struct stat *stbuf = (struct stat*)malloc(sizeof(struct stat));
	boolean find;
	boolean root;
	char name[MAX_NAME_LEN * 2];

	dentry = bf_lookup(path, &find, &root);
	if (find == FALSE)
	{
		return -BF_ERROR_NOTFOUND;
	}
	inode = dentry->inode;

	sub_dentry = bf_get_dentry(inode, offset);
	if (sub_dentry == NULL)
	{
		return 0;
	}

	strcpy(name, path);
	strcpy(name + strlen(path), sub_dentry->name);
	bf_getattr(name, stbuf);

	filler(buf, sub_dentry->name, stbuf, ++offset);

	free(stbuf);

	return 0;
}

/**
 * @brief 创建文件
 *
 * @param path 相对于挂载点的路径
 * @param mode 创建文件的模式，可忽略
 * @param dev 设备类型，可忽略
 * @return int 0成功，否则失败
 */
int bf_mknod(const char *path, mode_t mode, dev_t dev)
{
	struct dentry *dentry;
	struct dentry *sub_dentry;
	struct inode *inode;
	boolean find;
	boolean root;

	dentry = bf_lookup(path, &find, &root);
	if (find == TRUE)
	{
		return -BF_ERROR_EXIST;
	}
	inode = dentry->inode;

	if (S_ISDIR(mode))
	{
		sub_dentry = bf_init_dentry(getFileName(path), DIR);
	}
	else
	{
		sub_dentry = bf_init_dentry(getFileName(path), DEG);
	}

	bf_alloc_dentry(inode, sub_dentry);
	bf_alloc_inode(sub_dentry);

	return 0;
}

/**
 * @brief 修改时间，为了不让touch报错
 *
 * @param path 相对于挂载点的路径
 * @param tv 实践
 * @return int 0成功，否则失败
 */
int bf_utimens(const char *path, const struct timespec tv[2])
{
	(void)path;
	return 0;
}
/******************************************************************************
 * SECTION: 选做函数实现
 *******************************************************************************/
/**
 * @brief 写入文件
 *
 * @param path 相对于挂载点的路径
 * @param buf 写入的内容
 * @param size 写入的字节数
 * @param offset 相对文件的偏移
 * @param fi 可忽略
 * @return int 写入大小
 */
int bf_write(const char *path, const char *buf, size_t size, off_t offset,
			 struct fuse_file_info *fi)
{
	/* 选做 */
	struct dentry* dentry;
	struct inode* inode;
	boolean root;
	boolean find;

	int size_actually;

	dentry = bf_lookup(path, &find, &root);
	if (find == FALSE) 
	{
		return -BF_ERROR_NOTFOUND;
	}
	inode = dentry->inode;
	if (IS_DEG((*inode)) == FALSE)
	{
		return -BF_ERROR_UNSUPPORTED;
	}

	if (inode->size < offset)
	{
		return -BF_ERROR_SEEK;
	}
	
	size_actually = (offset + size > BF_BLK_SIZE(MAX_DATA_PER_INODE)) ? BF_BLK_SIZE(MAX_DATA_PER_INODE) - offset : size;
	memcpy(inode->data + offset, buf, size_actually);
	inode->size = offset + size > inode->size ? offset + size : inode->size;

	return size_actually;
}

/**
 * @brief 读取文件
 *
 * @param path 相对于挂载点的路径
 * @param buf 读取的内容
 * @param size 读取的字节数
 * @param offset 相对文件的偏移
 * @param fi 可忽略
 * @return int 读取大小
 */
int bf_read(const char *path, char *buf, size_t size, off_t offset,
			struct fuse_file_info *fi)
{
	struct dentry* dentry;
	struct inode* inode;
	boolean root;
	boolean find;

	int size_actually;

	dentry = bf_lookup(path, &find, &root);
	if (find == FALSE) 
	{
		return -BF_ERROR_NOTFOUND;
	}
	inode = dentry->inode;
	if (IS_DEG((*inode)) == FALSE)
	{
		return -BF_ERROR_UNSUPPORTED;
	}
	
	size_actually = (offset + size > BF_BLK_SIZE(MAX_DATA_PER_INODE)) ? BF_BLK_SIZE(MAX_DATA_PER_INODE) - offset : size;
	memcpy(buf, inode->data + offset, size_actually);

	return size_actually;
}

/**
 * @brief 删除文件
 *
 * @param path 相对于挂载点的路径
 * @return int 0成功，否则失败
 */
int bf_unlink(const char *path)
{
	struct dentry* dentry;
	struct inode* inode;

	boolean root;
	boolean find;

	dentry = bf_lookup(path, &find, &root);
	if (find == FALSE)
	{
		return -BF_ERROR_NOTFOUND;
	}
	if (root == TRUE)
	{
		return -BF_ERROR_INVAL;
	}
	inode = dentry->inode;

	bf_drop_inode(inode);

	return 0;
}

/**
 * @brief 删除目录
 *
 * 一个可能的删除目录操作如下：
 * rm ./tests/mnt/j/ -r
 *  1) Step 1. rm ./tests/mnt/j/j
 *  2) Step 2. rm ./tests/mnt/j
 * 即，先删除最深层的文件，再删除目录文件本身
 *
 * @param path 相对于挂载点的路径
 * @return int 0成功，否则失败
 */
int bf_rmdir(const char *path)
{
	bf_unlink(path);
	return 0;
}

/**
 * @brief 重命名文件
 *
 * @param from 源文件路径
 * @param to 目标文件路径
 * @return int 0成功，否则失败
 */
int bf_rename(const char *from, const char *to)
{
	struct dentry* from_dentry;
	struct dentry* to_parent_dentry;
	struct inode* to_parent_inode;

	boolean find;
	boolean root;

	from_dentry = bf_lookup(from, &find, &root);
	if (find == FALSE)
	{
		return -BF_ERROR_NOTFOUND;
	}
	else if (root == TRUE)
	{
		return -BF_ERROR_UNSUPPORTED;
	}

	to_parent_dentry = bf_lookup(to, &find, &root);
	if (find == TRUE)
	{
		return -BF_ERROR_EXIST;
	}
	to_parent_inode = to_parent_dentry->inode;

	bf_drop_dentry(from_dentry);
	bf_alloc_dentry(to_parent_inode, from_dentry);
	
	return 0;
}

/**
 * @brief 打开文件，可以在这里维护fi的信息，例如，fi->fh可以理解为一个64位指针，可以把自己想保存的数据结构
 * 保存在fh中
 *
 * @param path 相对于挂载点的路径
 * @param fi 文件信息
 * @return int 0成功，否则失败
 */
int bf_open(const char *path, struct fuse_file_info *fi)
{
	struct dentry* dentry;
	struct inode* inode;
	boolean find;
	boolean root;

	dentry = bf_lookup(path, &find, &root);
	if (find == FALSE)
	{
		return -BF_ERROR_NOTFOUND;
	}
	inode = dentry->inode;

	if (IS_DEG((*inode)) == FALSE)
	{
		return -BF_ERROR_UNSUPPORTED;
	}
	fi->fh = (uint64_t)dentry;

	return 0;
}

/**
 * @brief 打开目录文件
 *
 * @param path 相对于挂载点的路径
 * @param fi 文件信息
 * @return int 0成功，否则失败
 */
int bf_opendir(const char *path, struct fuse_file_info *fi)
{
	struct dentry* dentry;
	struct inode* inode;
	boolean find;
	boolean root;

	dentry = bf_lookup(path, &find, &root);
	if (find == FALSE)
	{
		return -BF_ERROR_NOTFOUND;
	}
	inode = dentry->inode;

	if (IS_DIR((*inode)) == FALSE)
	{
		return -BF_ERROR_UNSUPPORTED;
	}
	fi->fh = (uint64_t)dentry;

	return 0;
}

/**
 * @brief 改变文件大小
 *
 * @param path 相对于挂载点的路径
 * @param offset 改变后文件大小
 * @return int 0成功，否则失败
 */
int bf_truncate(const char *path, off_t offset)
{
	/* 选做 */
	return 0;
}

/**
 * @brief 访问文件，因为读写文件时需要查看权限
 *
 * @param path 相对于挂载点的路径
 * @param type 访问类别
 * R_OK: Test for read permission.
 * W_OK: Test for write permission.
 * X_OK: Test for execute permission.
 * F_OK: Test for existence.
 *
 * @return int 0成功，否则失败
 */
int bf_access(const char *path, int type)
{
	/* 选做: 解析路径，判断是否存在 */
	struct dentry* dentry;

	boolean find;
	boolean root;
	boolean access = FALSE;

	dentry = bf_lookup(path, &find, &root);
	if (find == FALSE) {
		return -BF_ERROR_NOTFOUND;
	}

	switch (type)
	{
	case R_OK:
		access = TRUE;
		break;
	case W_OK:
		access = TRUE;
		break;
	case X_OK:
		access = TRUE;
		break;
	case F_OK:
		access = TRUE;
		break;
	default:
		break;
	}
	
	return access ? 0 : -BF_ERROR_ACCESS;
}
/******************************************************************************
 * SECTION: FUSE入口
 *******************************************************************************/
int main(int argc, char **argv)
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	bf_options.device = strdup("/home/blgs/ddriver");

	if (fuse_opt_parse(&args, &bf_options, option_spec, NULL) == -1)
		return -1;

	ret = fuse_main(args.argc, args.argv, &operations, NULL);
	fuse_opt_free_args(&args);
	return ret;
}