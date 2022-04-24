#include "../include/bf.h"

/**
 *  @brief 获取文件名
 *  @param path 文件路径
 *  @return char*
 */
char*				
getFileName(const char *path)
{
    char *p = strrchr(path, '/') + 1;
    return p;
}

/**
 *  @brief 驱动读
 *  @param output 输出流
 *  @param offset 读取偏移量
 *  @param size 读取大小
 *  @return int 0 成功，否则失败
 */
int					
bf_driver_read(uint8_t *output, off_t offset, int size)
{
    int offset_aligned;
    int bias;
    int size_aligned;
    uint8_t* output_cursor;
    uint8_t* output_temp;
    
    if (output == NULL) 
    {
        return BF_ERROR_IS_NULL;
    }

    offset_aligned = ROUND_DOWN(offset, BF_SIZE_IO);
    bias           = offset - offset_aligned;
    size_aligned   = ROUND_UP(size, BF_SIZE_IO);

    output_temp    = (uint8_t *) malloc(size_aligned);
    output_cursor  = output_temp;

    ddriver_seek(super.fd, offset_aligned, SEEK_SET);
    while (size_aligned > 0)
    {
        ddriver_read(super.fd, output_cursor, BF_SIZE_IO);
        output_cursor += BF_SIZE_IO;
        size_aligned -= BF_SIZE_IO;
    }

    memcpy(output, output_temp + bias, size);
    free(output_temp);

    return 0;
}

/**
 *  @brief 驱动写
 *  @param input 输入流
 *  @param offset 写入偏移量
 *  @param size 写入大小
 *  @return int 0 成功，否则失败
 */
int					
bf_driver_write(uint8_t *input, off_t offset, int size)
{
    int offset_aligned;
    int bias;
    int size_aligned;
    uint8_t* input_temp;
    uint8_t* input_cursor;

    if (input == NULL)
    {
        return BF_ERROR_IS_NULL;
    }

    offset_aligned = ROUND_DOWN(offset, BF_SIZE_IO);
    bias           = offset - offset_aligned;
    size_aligned   = ROUND_UP(size, BF_SIZE_IO);
    input_temp     = (uint8_t *)malloc(size_aligned);
    input_cursor   = input_temp;

    bf_driver_read(input_temp, offset_aligned, size_aligned);
    memcpy(input_temp + bias, input, size);

    ddriver_seek(super.fd, offset_aligned, SEEK_SET);
    while (size_aligned > 0)
    {
        ddriver_write(super.fd, input_cursor, BF_SIZE_IO);
        input_cursor += BF_SIZE_IO;
        size_aligned -= BF_SIZE_IO;
    }

    free(input_temp);
    return 0;
}

/**
 *  @brief 初始化目录项
 *  @param name 文件名
 *  @param type 文件类型
 *  @return struct dentry*
 */
struct dentry* 		
bf_init_dentry(const char *name, FILE_TYPE type)
{
    struct dentry* dentry = (struct dentry *)malloc(sizeof(struct dentry));

    if (dentry != NULL)
    {
        strcpy(dentry->name, name);
        dentry->brother = NULL;
        dentry->parent  = NULL;
        dentry->ino     = -1;
        dentry->inode   = NULL;
        dentry->type    = type;       
    }

    return dentry;
}

/**
 *  @brief 分配目录项，目录项需要提前分配好 Inode
 *  @param inode dentry的上级 Inode
 *  @param dentry 待分配目录项
 *  @return int 0 成功，否则失败
 */
int					
bf_alloc_dentry(struct inode *inode, struct dentry *dentry)
{
    if (inode == NULL || dentry == NULL)
    {
        return BF_ERROR_IS_NULL;
    }

    dentry->brother = inode->dentrys;
    dentry->parent  = inode->dentry;

    inode->dentrys  = dentry;
    inode->dir_cnt++;
    return 0;
}

/**
 *  @brief 获取目录项
 *  @param inode dentry的上级Inode
 *  @param offset 第 offset 个目录，从 0 开始
 *  @return struct dentry*
 */
struct dentry*
bf_get_dentry(struct inode *inode, off_t offset)
{
    struct dentry* dentry_cursor;

    if (offset >= inode->dir_cnt)
    {
        return NULL;
    }

    dentry_cursor = inode->dentrys;
    while (dentry_cursor != NULL)
    {
        if (offset == 0)
        {
            break;
        }
        dentry_cursor = dentry_cursor->brother;
        offset--;
    }

    return dentry_cursor;
}

/**
 *  @brief 删除目录项，不释放
 *  @param dentry 
 *  @return int 0 成功，否则失败
 */
int					
bf_drop_dentry(struct dentry *dentry)
{
    if (dentry == NULL) {
        return BF_ERROR_IS_NULL;
    }
    if (dentry == super.root_dentry) {
        return BF_ERROR_INVAL;
    }
    struct inode* inode = dentry->parent->inode;
    struct dentry* brother;

    if (inode->dentrys == dentry) 
    {
        inode->dentrys = dentry->brother;
    }
    else 
    {
        for (brother = inode->dentrys; brother->brother != dentry; brother = brother->brother);
        brother->brother = dentry->brother;
    }
    inode->dir_cnt--;

    return 0;
}

/**
 *  @brief 为 dentry 分配 Inode
 *  @param dentry
 *  @return struct inode*
 */
struct inode*		
bf_alloc_inode(struct dentry *dentry)
{
    struct inode* inode = (struct inode*)malloc(sizeof(struct inode));
    int byte_cursor;
    int bit_cursor;
    int ino_cursor = 0;
    boolean find = FALSE;
    
    ino_cursor = 0;
    for (byte_cursor = 0; byte_cursor < BF_BLK_SIZE(BF_INOMAP_BLKS); byte_cursor++)
    {
        for (bit_cursor = 0; bit_cursor < 8; bit_cursor++)
        {
            if ( (super.inomap[byte_cursor] & (1 << bit_cursor)) == 0 )
            {
                find = TRUE;
                super.inomap[byte_cursor] |= (1 << bit_cursor);
                super.datmap[byte_cursor] |= (1 << bit_cursor);
                break;
            }
            ino_cursor++;
        }

        if (find == TRUE)
        {
            break;
        }
    }

    inode->ino     = ino_cursor;
    inode->data    = NULL;
    inode->dentry  = dentry;
    inode->dentrys = NULL;
    inode->dir_cnt = 0;
    inode->type    = dentry->type;
    inode->size    = 0;

    inode->data = (uint8_t *)malloc(sizeof(BF_BLK_SIZE(MAX_DATA_PER_INODE)));

    dentry->inode = inode;
    dentry->ino = ino_cursor;
    
    return inode;
}

/**
 *  @brief 删除 Inode
 *  @param inode
 *  @return int 0 成功，否则失败
 */
int					
bf_drop_inode(struct inode* inode)
{
    struct dentry* child_dentry;
    struct dentry* temp_child;
    int ino;
    int byte_cursor;
    int bit_cursor;

    if (inode == NULL) 
    {
        return -BF_ERROR_IS_NULL;
    }

    for (child_dentry = inode->dentrys; child_dentry; child_dentry = temp_child)
    {
        if (child_dentry->inode == NULL)
        {
            bf_read_inode(child_dentry, child_dentry->ino);
        }
        bf_drop_inode(child_dentry->inode);
        temp_child = child_dentry->brother;
        bf_drop_dentry(child_dentry);
        free(child_dentry);
    }

    if (inode->data)
    {
        free(inode->data);
    }

    ino = inode->ino;
    byte_cursor = ino / 8;
    bit_cursor = ino % 8;

    bf_drop_dentry(inode->dentry);
    free(inode);

    super.datmap[byte_cursor] ^= 1 << bit_cursor;
    super.inomap[byte_cursor] ^= 1 << bit_cursor;

    return 0;
}

/**
 *  @brief 从磁盘读出 Inode 和相应 Data
 *  @param dentry 上级 dentry
 *  @param ino 待读出 Inode 编号
 *  @return struct inode*
 */
struct inode*
bf_read_inode(struct dentry* dentry, int ino)
{
    struct inode* inode;
    struct bf_inode_d inode_d;
    int i;

    struct bf_dentry_d sub_dentry_d;
    struct dentry* sub_dentry;

    if (ino < 0 || ino >= super.max_inode)
    {
        return NULL;
    }

    inode = (struct inode*) malloc(sizeof(struct inode));
    
    bf_driver_read((uint8_t *)&inode_d, INODE_OFS(ino), sizeof(inode_d));
    
    inode->ino = inode_d.ino;
    inode->dir_cnt = inode_d.dir_cnt;
    inode->type = inode_d.type;
    inode->size = inode_d.size;

    inode->dentry = dentry;
    inode->dentrys = NULL;
    inode->data = NULL;

    if (inode->type == DEG)
    {
        inode->data = (uint8_t *)malloc(BF_BLK_SIZE(MAX_DATA_PER_INODE));
        bf_driver_read((uint8_t *)inode->data, DATA_OFS(ino), BF_BLK_SIZE(MAX_DATA_PER_INODE));
    }
    
    // 创建目录项
    if (inode->type == DIR)
    {
        for (i = 0; i < inode->dir_cnt; ++i)
        {
            bf_driver_read((uint8_t *)&sub_dentry_d, DATA_OFS(ino) + i * sizeof(struct bf_dentry_d), sizeof(struct bf_dentry_d));
            sub_dentry = bf_init_dentry(sub_dentry_d.name, sub_dentry_d.type);
            sub_dentry->ino = sub_dentry_d.ino;
            sub_dentry->inode = NULL;
            sub_dentry->parent = inode->dentry;
            sub_dentry->brother = inode->dentrys;
            inode->dentrys = sub_dentry;
        }
    }
    
    return inode;
}

/**
 *  @brief 将 Inode 及其以下部分写入磁盘 
 *  @param inode
 *  @return int 0 成功，否则失败
 */
int					
bf_sync_inode(struct inode* inode)
{
    struct dentry* dentry;
    struct bf_inode_d inode_d;
    struct bf_dentry_d dentry_d;
    int i;

    inode_d.dir_cnt = inode->dir_cnt;
    inode_d.ino = inode->ino;
    inode_d.size = inode->size;
    inode_d.type = inode->type;

    bf_driver_write((uint8_t *)&inode_d, INODE_OFS(inode_d.ino), sizeof(struct bf_inode_d));

    if (inode_d.type == DEG)
    {
        bf_driver_write((uint8_t *)&inode->data, DATA_OFS(inode_d.ino), BF_BLK_SIZE(MAX_DATA_PER_INODE));
        return 0;
    }
    dentry = inode->dentrys;
    for (i = 0; i < inode->dir_cnt; i++)
    {
        dentry_d.ino = dentry->ino;
        dentry_d.type = dentry->type;
        strcpy(dentry_d.name, dentry->name);

        bf_driver_write((uint8_t *)&dentry_d, DATA_OFS(inode->ino) + i * sizeof(struct bf_dentry_d), sizeof(struct bf_dentry_d));
        if (dentry->inode)
        {
            bf_sync_inode(dentry->inode);
        }

        dentry = dentry->brother;
    }

    return 0;
}

/**
 *  @brief 遍历路径
 *  @param path 文件路径
 *  @param find 是否找到
 *  @param root 是否为根目录
 *  @return struct dentry* ,find 为 TRUE 时，返回当前目录项，否则返回最后目录项
 */
struct dentry*		
bf_lookup(const char *path, boolean *find, boolean *root)
{
    struct dentry* dentry = super.root_dentry;
    struct dentry* dentry_cursor;
    struct inode* inode;
    char *path_temp = (char *)malloc(sizeof(char) * strlen(path));
    char *name;
     
    *find = FALSE;
    *root = FALSE;

    strcpy(path_temp, path);
    name = strtok(path_temp, "/");

    if (name == NULL)
    {
        if (path[0] == '/') 
        {
            *find = TRUE;
            *root = TRUE;
            return dentry;
        }
        else
        {
            return NULL;
        }
    }

    while (name)
    {
        *find = FALSE;

        if (dentry->inode == NULL)
        {
            dentry->inode = bf_read_inode(dentry, dentry->ino);
        }
        inode = dentry->inode;

        for (dentry_cursor = inode->dentrys; dentry_cursor; dentry_cursor = dentry_cursor->brother)
        {
            if (strcmp(name, dentry_cursor->name) == 0)
            {
                *find = TRUE;
                dentry = dentry_cursor;
                break;
            }
        }

        if (*find == FALSE)
        {
            break;
        }

        name = strtok(NULL, "/");
    }

    if (*find == TRUE)
    {
        if (dentry->inode == NULL)
        {
            dentry->inode = bf_read_inode(dentry, dentry->ino);
        }
    }

    return dentry;
}

/**
 *  @brief 挂载
 *  @return int 0 成功，否则失败 
 */
int					
bf_mount()
{
    struct bf_super_d super_d;
    struct dentry* root_dentry;
    struct inode* root_inode;

    int super_blks;
    int inode_num;
    int data_num;
    int map_inode_blks;
    int map_data_blks;

    boolean init = FALSE;

    ddriver_ioctl(super.fd, IOC_REQ_DEVICE_IO_SZ, &size_io);
    ddriver_ioctl(super.fd, IOC_REQ_DEVICE_SIZE, &size_disk);

    bf_driver_read((uint8_t *)&super_d, BF_SUPER_OFS, sizeof(super_d));

    if (super_d.magic != BF_MAGIC)
    {
        super_blks            = ROUND_UP(sizeof(struct bf_super_d), BF_SIZE_IO) / BF_SIZE_IO;
        inode_num             = BF_SIZE_DISK / (BF_SIZE_IO * (MAX_INODE_PER_FILE + MAX_DATA_PER_INODE));
        data_num              = MAX_DATA_PER_INODE * inode_num;
        map_inode_blks        = ROUND_UP(ROUND_UP(inode_num, 32), BF_SIZE_IO) / BF_SIZE_IO;
        map_data_blks         = ROUND_UP(ROUND_UP(data_num, 32), BF_SIZE_IO) / BF_SIZE_IO;

        super_d.sz_usage      = 0;
        
        super_d.max_inode     = (inode_num - super_blks - map_inode_blks - map_data_blks);
        super_d.max_data      = super_d.max_inode * MAX_DATA_PER_INODE / MAX_INODE_PER_FILE;

        super_d.inomap_blks   = map_inode_blks;
        super_d.datmap_blks   = map_data_blks;
        super_d.inode_blks    = super_d.max_inode;
        super_d.data_blks     = super_d.max_data * MAX_DATA_PER_INODE;

        super_d.inomap_offset = BF_SUPER_OFS + BF_BLK_SIZE(super_blks);
        super_d.datmap_offset = super_d.inomap_offset + BF_BLK_SIZE(map_inode_blks);
        super_d.inode_offset  = super_d.datmap_offset + BF_BLK_SIZE(map_data_blks);
        super_d.data_offset   = super_d.inode_offset + BF_BLK_SIZE(super_d.max_inode);

        init = TRUE;
    }
    
    super.max_inode     = super_d.max_inode;
    super.max_data      = super_d.max_data;
    super.inomap_blks   = super_d.inomap_blks;
    super.datmap_blks   = super_d.datmap_blks;
    super.inode_blks    = super_d.inode_blks;
    super.data_blks     = super_d.data_blks;
    super.inomap_offset = super_d.inomap_offset;
    super.datmap_offset = super_d.datmap_offset;
    super.inode_offset = super_d.inode_offset;
    super.data_offset = super_d.data_offset;
    
    super.sz_usage = super_d.sz_usage;
    
    super.inomap = (uint8_t *)malloc(BF_BLK_SIZE(super.inomap_blks));
    super.datmap = (uint8_t *)malloc(BF_BLK_SIZE(super.datmap_blks));

    bf_driver_read((uint8_t *)(super.inomap), super.inomap_offset, BF_BLK_SIZE(super.inomap_blks));
    bf_driver_read((uint8_t *)(super.datmap), super.datmap_offset, BF_BLK_SIZE(super.datmap_blks));    
    
    root_dentry = bf_init_dentry("/", DIR);

    if (init == TRUE)
    {
        root_inode = bf_alloc_inode(root_dentry);
        bf_sync_inode(root_inode);
    }
    
    root_inode  = bf_read_inode(root_dentry, 0);
    root_dentry->inode = root_inode;
    root_dentry->ino = root_inode->ino;
    super.root_dentry = root_dentry;

    return 0;    
}

/**
 *  @brief 卸载
 *  @return int 0 成功，否则失败
 */
int		
bf_unmount()
{
    struct bf_super_d super_d;
    
    super_d.max_data      = super.max_data;
    super_d.max_inode     = super.max_inode;

    super_d.inomap_offset = super.inomap_offset;
    super_d.datmap_offset = super.datmap_offset;
    super_d.inode_offset  = super.inode_offset;
    super_d.data_offset   = super.data_offset;

    super_d.inomap_blks   = super.inomap_blks;
    super_d.datmap_blks   = super.datmap_blks;
    super_d.inode_blks    = super.inode_blks;
    super_d.data_blks     = super.data_blks;

    super_d.magic         = BF_MAGIC;
    super_d.sz_usage      = super.sz_usage;

    bf_sync_inode(super.root_dentry->inode);
    bf_driver_write((uint8_t *)&super_d, BF_SUPER_OFS, sizeof(super_d));

    return 0;
}