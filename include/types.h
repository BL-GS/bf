#ifndef _TYPES_H_
#define _TYPES_H_

typedef enum boolean {
	FALSE = 0,
	TRUE = 1
} boolean;

typedef enum FILE_TYPE {
	DEG,
	DIR
} FILE_TYPE;

#define     BF_MAGIC                0x12345678  
#define     BF_DEFAULT_PERM         0777

#define     MAX_NAME_LEN            128
#define     MAX_INODE_PER_FILE      1
#define     MAX_DATA_PER_INODE      4
#define     INOMAP_LEN_PER_BLKS     8
#define     DATAMAP_LEN_PER_BLKS    8
/******************************************************************************
* SECTION: 全局变量
******************************************************************************/
int				size_io;
int				size_disk;
struct super	super;

#define		ROUND_UP(value, size)		((value % size == 0) ? value : (value / size + 1) * size)
#define		ROUND_DOWN(value, size)		((value % size == 0) ? value : (value / size) * size)
/******************************************************************************
* SECTION: 系统定义
* /------------/-------------/--------------/-------------/------------/
* |   Super    |   InodeMap  |    DataMap   |    Inode    |    Data    |
* /------------/-------------/--------------/-------------/------------/
******************************************************************************/
#define		BF_SIZE_IO					size_io
#define		BF_SIZE_DISK				size_disk
#define		BF_DEVICE					super.fd
#define		BF_BLK_SIZE(blks)			( BF_SIZE_IO * blks )

#define		BF_UPPER_BLKS(size)			( ROUND_UP(size) / BF_SIZE_IO )
#define		BF_SUPER_BLKS				( BF_UPPER_BLKS(sizeof(struct super)) )
#define		BF_INOMAP_BLKS				( super.inomap_blks )
#define		BF_DATMAP_BLKS				( super.datmap_blks )
#define		BF_INODE_BLKS				( BF_BLK_SIZE(super.max_ino) )
#define		BF_DATA_BLKS				( BF_BLK_SIZE(super.max_data) )

#define		BF_SUPER_OFS				0
#define		BF_INOMAP_OFS				( super.inomap_offset )
#define		BF_DATMAP_OFS				( super.datmap_offset )
#define		BF_INODE_OFS				( super.inode_offset )
#define		BF_DATA_OFS					( super.data_offset )

#define     INODE_OFS(ino)              ( BF_INODE_OFS + BF_BLK_SIZE(MAX_INODE_PER_FILE) * ino )
#define     DATA_OFS(ino)               ( BF_DATA_OFS + BF_BLK_SIZE(MAX_DATA_PER_INODE) * ino )

#define 	IS_DIR(inode)				(inode.type == DIR)
#define		IS_DEG(inode)				(inode.type == DEG)

/******************************************************************************
* SECTION: 文件系统结构
******************************************************************************/

struct custom_options {
	const char*        device;
};

struct bf_super_d {
	uint32_t magic;
	int             max_inode;
	int             max_data;

	int             inomap_offset;
	int             datmap_offset;
	int             inode_offset;
	int             data_offset;

	int             inomap_blks;
	int             datmap_blks;
	int             inode_blks;
	int             data_blks;

	int             sz_usage;
};

struct bf_inode_d {
	int             ino;
	int             dir_cnt;
	int             size;

	FILE_TYPE       type;
};

struct bf_dentry_d {
	char     name[MAX_NAME_LEN];

	int             ino;
	
	FILE_TYPE       type;
};

struct super {
	int             fd;

	int             max_inode;
	int             max_data;

	int             inomap_offset;
	int             datmap_offset;
	int             inode_offset;
	int             data_offset;

	int             inomap_blks;
	int             datmap_blks;
	int             inode_blks;
	int             data_blks;

	uint8_t*        inomap;
	uint8_t*        datmap;

	int             sz_usage;
	struct dentry*  root_dentry;
};

struct inode {
	int             ino;
	int             dir_cnt;
	int             size;
				 
	struct dentry*  dentry;
	struct dentry*  dentrys;
	uint8_t*        data;

	FILE_TYPE       type;
};

struct dentry {
	char           name[MAX_NAME_LEN];

	int             ino;
	struct inode*   inode;

	struct dentry*  parent;
	struct dentry*  brother;
	
	FILE_TYPE       type;
};

#endif /* _TYPES_H_ */