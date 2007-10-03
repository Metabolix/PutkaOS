#include <filesys/ext2.h>
#include <filesys/filesystem.h>
#include <filesys/mount_err.h>
#include <malloc.h>
#include <screen.h>
#include <stddef.h>
#include <string.h>
#include <bit.h>
#include <debugprint.h>


struct ext2_fs ext2_op = {
	{
		.name = "ext2",
		.fs_mount = (fs_mount_t) ext2_mount,
		.fs_umount = (fs_umount_t) ext2_umount,
		.filefunc = {
			.fopen = (fopen_t) ext2_fopen,
			.fclose = (fclose_t) ext2_fclose,
			.fread = (fread_t) ext2_fread,
			.fwrite = (fwrite_t) ext2_fwrite,
			.fflush = (fflush_t) ext2_fflush,
			.fsetpos = (fsetpos_t) ext2_fsetpos,
			.ioctl = (ioctl_t) ext2_ioctl
		},
		.dirfunc = {
			.dmake = (dmake_t) ext2_dmake,
			.dopen = (dopen_t) ext2_dopen,
			.dclose = (dclose_t) ext2_dclose,
			.dread = (dread_t) ext2_dread
		}
	},
	.super_block = 0,
	.group_desc = 0,
	.group_desc_n = 0,
	.inode_bitmap_read = 0,
	.block_bitmap_read = 0,
	.block_size = 0,
	.device = 0,
	.refs_open = 0
};

int ext2_ioctl(struct ext2_file *f, int request, uintptr_t param)
{
	// TODO: ext2_ioctl
	return -1;
}

inline static int ext2_block_offset(struct ext2_fs * ext2, int block)
{
	return ext2->block_size*block;
}

static void ext2_set_bg(struct ext2_fs * ext2, unsigned int desc_n)
{
	fpos_t pos;

	if(desc_n != ext2->group_desc_n) {
		pos = ((fpos_t)ext2->super_block->s_blocks_per_group) *  ext2->block_size * desc_n;
		fsetpos(ext2->device, &pos);
		fread(ext2->group_desc, sizeof(struct ext2_group_desc), 1, ext2->device);

		ext2->group_desc_n = desc_n;
		ext2->inode_bitmap_read = 0;
		ext2->block_bitmap_read = 0;
	}
	
}

/* get_ext2_inode gets inode number `num' from filesystem */

static struct ext2_inode get_ext2_inode(struct ext2_fs * ext2, unsigned int num) {
	static struct ext2_inode inode;
	unsigned short desc_n = (num  - 1)/ ext2->super_block->s_inodes_per_group;
	fpos_t pos;

	if(num < 1) {
		DEBUGF("Bad inode number (%d)!\n", num);
		return inode;
	}

	ext2_set_bg(ext2, desc_n);

	pos = ((fpos_t)ext2->block_size) * ext2->group_desc->bg_inode_table + (num - 1) % ext2->super_block->s_inodes_per_group * ext2->super_block->s_inode_size;
	fsetpos(ext2->device, &pos);
	fread(&inode, ext2->super_block->s_inode_size, 1, ext2->device);

	return inode;
}


/* Get_block: Get a block from device*/
static char * get_block(unsigned long long int num, size_t size, struct ext2_fs * ext2, void * buffer) {
	fpos_t pos = size * num;
	fsetpos(ext2->device, &pos);
	if(fread(buffer, size, 1, ext2->device) < 1) {
		return (char*)0;
	}
	return buffer;
}

static int ext2_write_block(unsigned int num, size_t size, struct ext2_fs * ext2, void * buffer) {
	fpos_t pos = size*num;
	fsetpos(ext2->device, &pos);
	return fwrite(buffer, size, 1, ext2->device);
}

/* Get_part_block: Get a part of a block */
static char * get_part_block(unsigned int num, size_t size, struct ext2_fs * ext2, void * buffer, size_t read, unsigned int offset) {
	fpos_t pos = size*num + offset;
	fsetpos(ext2->device, &pos);
	if(fread(buffer, read, 1, ext2->device) < 1) {
		return (char*)0;
	}
	return buffer;
}

static int ext2_write_part_block(unsigned int num, size_t size, struct ext2_fs * ext2, void * buffer, size_t write, unsigned int offset) {
	fpos_t pos = size*num + offset;
	fsetpos(ext2->device, &pos);
	return fwrite(buffer, write, 1, ext2->device);
}

static void ext2_read_inode_bitmap(struct ext2_fs *ext2)
{
	fpos_t pos = (ext2->group_desc->bg_inode_bitmap) * ext2->block_size;
	if(ext2->inode_bitmap_read)
		return;
	if(!ext2->ext2_inode_bitmap) {
		ext2->ext2_inode_bitmap = kmalloc(ext2->super_block->s_inodes_per_group/8);
	}
	fsetpos(ext2->device, &pos);
	if(fread(ext2->ext2_inode_bitmap, ext2->super_block->s_inodes_per_group/8, 1, ext2->device) < 1) {
		memset(ext2->ext2_inode_bitmap, 0xFF, ext2->block_size);
	}
	ext2->inode_bitmap_read = 1;
}

static void ext2_read_block_bitmap(struct ext2_fs *ext2)
{
	fpos_t pos = (ext2->group_desc->bg_block_bitmap) * ext2->block_size;
	if(ext2->block_bitmap_read)
		return;
	if(!ext2->ext2_block_bitmap) {
		ext2->ext2_block_bitmap = kmalloc(ext2->super_block->s_blocks_per_group/8);
	}
	fsetpos(ext2->device, &pos);
	if(fread(ext2->ext2_block_bitmap, ext2->super_block->s_blocks_per_group/8, 1, ext2->device) < 1) {
		memset(ext2->ext2_block_bitmap, 0xFF, ext2->block_size);
	}
	ext2->block_bitmap_read = 1;
}

static void ext2_reserve_inode(struct ext2_fs *ext2, int num)
{
	int byte = num / 8;
	int bit = num - byte*8;
	fpos_t pos = ext2->group_desc_n * (fpos_t)ext2->super_block->s_blocks_per_group *  ext2->block_size + 2 * ext2->block_size;
	fsetpos(ext2->device, &pos);
	
	ext2->group_desc->bg_free_inodes_count--;
	
	fwrite(ext2->group_desc, sizeof(struct ext2_group_desc), 1, ext2->device);
	fflush(ext2->device);
	
	ext2->ext2_inode_bitmap[byte] = set_bit(ext2->ext2_inode_bitmap[byte], bit, 1); 
	
	pos = ext2->group_desc->bg_inode_bitmap * ext2->block_size;
	fsetpos(ext2->device, &pos);
	fwrite(ext2->ext2_inode_bitmap, ext2->super_block->s_inodes_per_group/8, 1, ext2->device);
	fflush(ext2->device);
}

static void ext2_release_inode(struct ext2_fs *ext2, int num)
{
	int byte = num / 8;
	int bit = num - byte*8;
	fpos_t pos = ext2->group_desc_n * (fpos_t)ext2->super_block->s_blocks_per_group *  ext2->block_size;
	fsetpos(ext2->device, &pos);
	
	ext2->group_desc->bg_free_inodes_count++;
	
	fwrite(ext2->group_desc, ext2->block_size, 1, ext2->device);
	
	ext2->ext2_inode_bitmap[byte] = set_bit(ext2->ext2_inode_bitmap[byte], bit, 0); 
	
	pos = ext2->group_desc->bg_inode_bitmap * ext2->block_size;
	fsetpos(ext2->device, &pos);
	fwrite(&ext2->ext2_inode_bitmap[byte], 1, 1, ext2->device);
	fflush(ext2->device);
}

static void ext2_reserve_block(struct ext2_fs *ext2, int num)
{
	int byte = num / 8;
	int bit = num - byte*8;
	fpos_t pos = ext2->group_desc_n * (fpos_t)ext2->super_block->s_blocks_per_group *  ext2->block_size + 2 * ext2->block_size;
	fsetpos(ext2->device, &pos);
	
	ext2->group_desc->bg_free_blocks_count--;
	
	fwrite(ext2->group_desc, sizeof(struct ext2_group_desc), 1, ext2->device);
	fflush(ext2->device);
	
	ext2->ext2_block_bitmap[byte] = set_bit(ext2->ext2_block_bitmap[byte], bit, 1); 
	
	pos = ext2->group_desc->bg_block_bitmap * ext2->block_size;
	fsetpos(ext2->device, &pos);
	fwrite(ext2->ext2_block_bitmap, ext2->super_block->s_blocks_per_group/8, 1, ext2->device);
	fflush(ext2->device);
}

static void ext2_release_block(struct ext2_fs *ext2, int num)
{
	int byte = num / 8;
	int bit = num - byte*8;
	fpos_t pos = ext2->group_desc_n * (fpos_t)ext2->super_block->s_blocks_per_group *  ext2->block_size;
	fsetpos(ext2->device, &pos);
	
	ext2->group_desc->bg_free_blocks_count++;
	
	fwrite(ext2->group_desc, ext2->block_size, 1, ext2->device);
	
	ext2->ext2_block_bitmap[byte] = set_bit(ext2->ext2_block_bitmap[byte], bit, 0); 
	
	pos = ext2->group_desc->bg_block_bitmap * ext2->block_size;
	fsetpos(ext2->device, &pos);
	fwrite(&ext2->ext2_block_bitmap[byte], 1, 1, ext2->device);
	fflush(ext2->device);
}

static int ext2_alloc_inode(struct ext2_fs * ext2)
{
	int byte;
	int bit;

	ext2_read_inode_bitmap(ext2);
	if(ext2->group_desc->bg_free_inodes_count == 0) {
		DEBUGF("No free inodes in current group"); /* search next group */
		return 0;
	} else {
		int bytes = ext2->super_block->s_inodes_per_group / 8;
		for(byte = 0; byte < bytes; byte++) {
			for(bit = 0; bit < 8; bit++) {
				if(!get_bit(ext2->ext2_inode_bitmap[byte], bit)) {
					goto found;
				}
			}
		}
		return 0; /* didn't find any, TODO: search next group */
	}
found:
	ext2_reserve_inode(ext2, byte * 8 + bit);
	ext2->super_block->s_free_inodes_count--;
	fseek(ext2->device, 1024, SEEK_SET);
	if(fwrite(ext2->super_block, 512, 2, ext2->device) < 2) {
		return 0;
	}

	return byte * 8 + bit + 1;
}

static int ext2_alloc_block(struct ext2_fs * ext2)
{
	int byte;
	int bit;

	ext2_read_block_bitmap(ext2);
	if(ext2->group_desc->bg_free_blocks_count == 0) {
		DEBUGF("No free blocks in current group"); /* search next group */
		return 0;
	} else {
		int bytes = ext2->super_block->s_blocks_per_group / 8;
		for(byte = 0; byte < bytes; byte++) {
			for(bit = 0; bit < 8; bit++) {
				if(!get_bit(ext2->ext2_block_bitmap[byte], bit)) {
					goto found;
				}
			}
		}
		return 0; /* didn't find any, TODO: search next group */
	}
found:
	ext2_reserve_block(ext2, byte * 8 + bit);
	ext2->super_block->s_free_blocks_count--;
	fseek(ext2->device, 1024, SEEK_SET);
	if(fwrite(ext2->super_block, 512, 2, ext2->device) < 2) {
		return 0;
	}

	return byte * 8 + bit + 1;
}

static void ext2_write_inode(struct ext2_fs * ext2, unsigned int inode, void * buffer)
{
	unsigned short desc_n = (inode  - 1)/ ext2->super_block->s_inodes_per_group;
	fpos_t pos;

	if(inode < 1) {
		DEBUGF("Bad inode number (%d)!\n", inode);
		return;
	}

	ext2_set_bg(ext2, desc_n);

	pos = ((fpos_t)ext2->block_size) * ext2->group_desc->bg_inode_table + ((inode - 1) % ext2->super_block->s_inodes_per_group * ext2->super_block->s_inode_size);
	fsetpos(ext2->device, &pos);
	fwrite(buffer, ext2->super_block->s_inode_size, 1, ext2->device);
	fflush(ext2->device);
}

/* Get_iblock returns inode's block number `block'
   We can hope that it works ;) */
static int get_iblock(struct ext2_inode * inode, unsigned int block, struct ext2_fs * ext2) {
	unsigned int block_n = 0;
	unsigned int * buffer = kmalloc(ext2->block_size);
	unsigned short p_per_block = ext2->block_size / 4;
	if(block < EXT2_NDIR_BLOCKS) {
		block_n = inode->i_block[block];
	} else if(block >= EXT2_IND_BLOCK && block < (p_per_block + EXT2_NDIR_BLOCKS)) {
		get_block(inode->i_block[EXT2_IND_BLOCK], ext2->block_size, ext2, buffer);
		block_n = buffer[block - EXT2_NDIR_BLOCKS];
	} else if(block >= (p_per_block + EXT2_NDIR_BLOCKS) && block < (p_per_block * p_per_block + EXT2_IND_BLOCK)) {
		buffer = (unsigned int*)get_block(inode->i_block[EXT2_DIND_BLOCK], ext2->block_size, ext2, buffer);
		block_n = buffer[(block - EXT2_IND_BLOCK - p_per_block) / p_per_block];
		if(!block_n)
			goto error;
		buffer = (unsigned int*)get_block(block_n, ext2->block_size, ext2, buffer);
		block_n = buffer[block - EXT2_IND_BLOCK - p_per_block * 2 ];
	} else if(block >= (p_per_block * p_per_block + EXT2_IND_BLOCK) && block < (p_per_block * p_per_block * p_per_block + EXT2_DIND_BLOCK)) {
		buffer = (unsigned int*)get_block(inode->i_block[EXT2_TIND_BLOCK], ext2->block_size, ext2, buffer);
		block_n = buffer[(block - EXT2_IND_BLOCK - p_per_block - p_per_block * p_per_block) / (p_per_block*p_per_block)];
		if(!block_n)
			goto error;
		buffer = (unsigned int*)get_block(block_n, ext2->block_size, ext2, buffer);
		block_n = buffer[(block - EXT2_IND_BLOCK - p_per_block - p_per_block * p_per_block) / p_per_block];
		if(!block_n)
			goto error;
		buffer = (unsigned int*)get_block(block_n, ext2->block_size, ext2, buffer);
		block_n = buffer[(block - EXT2_IND_BLOCK - p_per_block - p_per_block * p_per_block)];
	}
	if(!block_n)
		goto error;

	kfree(buffer);
	return block_n;

	error:
		kfree(buffer);
		return -1;
}

static int ext2_inode_add_block(struct ext2_inode * inode, int inodenum, struct ext2_fs * ext2)
{
	unsigned int block_n = 0;
	unsigned int new_block = ext2_alloc_block(ext2);
	unsigned int buffer[2] = {0,0};
	const unsigned int p_per_block = ext2->block_size / 4;
	unsigned block = ++inode->i_blocks;

	char path[4];

	if(new_block == 0)
		return 0;

	if(block < EXT2_NDIR_BLOCKS) {
		path[0] = block;
	} else if((block -= EXT2_NDIR_BLOCKS) < p_per_block) {
		path[0] = EXT2_IND_BLOCK;
		path[1] = block;
	} else if((block -= p_per_block) < (p_per_block * p_per_block)) {
		path[0] = EXT2_DIND_BLOCK;
		path[1] = block / p_per_block;
		path[2] = block - path[1] * p_per_block;
	} else if(block >= (p_per_block * p_per_block + EXT2_IND_BLOCK) && block < (p_per_block * p_per_block * p_per_block + EXT2_DIND_BLOCK)) {
		path[0] = EXT2_TIND_BLOCK;
		path[1] = block / (p_per_block*p_per_block);
		path[2] = block / p_per_block;
		path[3] = block - path[3] * p_per_block;
	}

	if(path[0] < EXT2_NDIR_BLOCKS) {
		inode->i_block[(int)path[0]] = new_block;
	} else {
		if(inode->i_block[(int)path[0]] == 0) {
			inode->i_block[(int)path[0]] = ext2_alloc_block(ext2);
		}
		block_n = inode->i_block[(int)path[0]];
		if((int)path[0] == EXT2_IND_BLOCK) {
			buffer[0] = new_block;
			ext2_write_part_block(block_n, ext2->block_size, ext2, buffer, 8, (int)path[1] * sizeof(int));
		} else {
			if(buffer[0] == 0) {
				buffer[0] = ext2_alloc_block(ext2);
				ext2_write_part_block(block_n, ext2->block_size, ext2, buffer, 8, (int)path[1] * sizeof(int));
			}
			block_n = buffer[0];
			if((int)path[0] == EXT2_DIND_BLOCK) {
				buffer[0] = new_block;
				ext2_write_part_block(block_n, ext2->block_size, ext2, buffer, 8, (int)path[2] * sizeof(int));
			} else {
				if(buffer[0] == 0) {
					buffer[0] = ext2_alloc_block(ext2);
					ext2_write_part_block(block_n, ext2->block_size, ext2, buffer, 8, (int)path[2] * sizeof(int));
				}
				block_n = buffer[0];
				ext2_write_part_block(block_n, ext2->block_size, ext2, buffer, 8, (int)path[2] * sizeof(int));
				buffer[0] = new_block;
				ext2_write_part_block(block_n, ext2->block_size, ext2, buffer, 8, (int)path[3] * sizeof(int));
			}
		}
	}
	ext2_write_inode(ext2, inodenum, inode); // add block count and possibly modify i_block

	return new_block;
}

	

struct fs *ext2_mount(FILE *device, uint_t mode) {
	struct ext2_fs * ext2_fs = kmalloc(sizeof(struct ext2_fs));

	memcpy(&ext2_fs->std, &ext2_op, sizeof(struct fs));
	ext2_fs->super_block = kmalloc(sizeof(struct ext2_super_block));
	ext2_fs->device = device;

	ext2_fs->mode = mode;

	fseek(device, 1024, SEEK_SET);
	if(fread(ext2_fs->super_block, 512, 2, device) < 2) {
		kprintf("Couldn't read superblock!\n");
		return 0;
	}
	//kprintf("S_inode_size is %x and s_magic is %x\n", ext2_fs->super_block->s_inode_size, ext2_fs->super_block->s_magic);

	if(ext2_fs->super_block->s_magic != EXT2_SUPER_MAGIC) {
		/* Bad magic number, there is no ext2fs here */
		return 0;
	}
	if(ext2_fs->super_block->s_log_block_size < 3)
		ext2_fs->block_size = 1024 << ext2_fs->super_block->s_log_block_size;
	else {
		DEBUGP("S_log_block_size is something weird\n");
		return 0;
	}

	//kprintf("inodes_count is %d\n", ext2_fs->super_block->s_inodes_count);

	ext2_fs->group_desc = kmalloc(sizeof(struct ext2_group_desc));
	ext2_fs->group_desc_n = 0;

	fseek(device, 2048, SEEK_SET);
	if(fread(ext2_fs->group_desc, sizeof(struct ext2_group_desc), 1, device) < 1) {
		DEBUGP("Couldn't read group descriptor!\n");
		return 0;
	}
	ext2_fs->ext2_inode_bitmap = kmalloc(ext2_fs->super_block->s_inodes_per_group / 8);

	return (struct fs*)ext2_fs;
}

int ext2_umount(struct ext2_fs *this) {
	if(this->refs_open > 0)
		return MOUNT_ERR_BUSY;

	if(this->ext2_inode_bitmap)
		kfree(this->ext2_inode_bitmap);
	if(this->ext2_block_bitmap)
		kfree(this->ext2-inode_bitmap);
	kfree(this->group_desc);
	kfree(this->super_block);
	kfree(this);

	return 0;
}
/* looks for entry at end of the filename, returns inode number */
static int ext2_search_entry(struct ext2_fs * ext2, const char * filename, unsigned int inode_n)
{
	char entry[EXT2_NAME_LEN + 1];
	unsigned int chars;
	unsigned int last_inode = -1;
	unsigned int entry_len;
	struct ext2_inode inode;
	struct ext2_dir_entry_2 * dir;
	unsigned int cur_block = 0;
	char * block;
	
	if(filename[0] == 0)
		return EXT2_ROOT_INO;
	block = kmalloc(ext2->block_size);

	while(filename[0]) { /* while something to parse */
		roll:
		last_inode = inode_n;
		chars = 0;

		while(filename[0] && filename[0] != '/') {
			entry[chars++] = filename[0];
			filename++;
		}
		if(filename[0] == '/')
			filename++;
		entry[chars] = 0;

		if(chars == 0)
			break;

		entry_len = strlen(entry);
		inode = get_ext2_inode(ext2, inode_n);

		while(cur_block * ext2->block_size < inode.i_size) {
			if((inode.i_mode >> 13) != EXT2_FT_DIR) {
				if(filename[0] == '/') {
					goto error;
				}
			}
			if(!get_block(get_iblock(&inode, cur_block, ext2), ext2->block_size, ext2, block)) {
				goto error;
			}
			dir = (struct ext2_dir_entry_2 *)block;
			while((unsigned int)dir < ((unsigned int)block + ext2->block_size)) {
				if(entry_len == dir->name_len) {
					char entry_name[EXT2_NAME_LEN + 1];
					strncpy(entry_name, dir->name, dir->name_len);
					entry_name[dir->name_len] = '\0';
					//kprintf("Entry_name == \"%s\", entry==\"%s\"\n", entry_name, entry);
					if(!strcmp(entry_name, entry)) {
						inode_n = dir->inode;
						//kprintf("Inode_n == %d\n", inode_n);
						//cur_block =  (inode.i_size  + ext2->block_size - 1)* ext2->block_size;
						goto roll;
					}
				}
				dir = (struct ext2_dir_entry_2*)((unsigned int) dir + dir->rec_len);
			}
			cur_block++;
		}
		if(last_inode == inode_n)
			goto error;
	}
	kfree(block);
	return inode_n;
error:
	kfree(block);
	return 0;
}

static void ext2_make_direntry(struct ext2_fs *ext2, const char * name, unsigned int dirinode, unsigned int targetinode, unsigned char type)
{
	unsigned int name_len = strlen(name);
	unsigned int entry_len = EXT2_DIR_REC_LEN(name_len);
	struct ext2_inode inode = get_ext2_inode(ext2, dirinode);
	struct ext2_dir_entry_2 * dir;
	unsigned int cur_block = 0;
	char * block = kmalloc(ext2->block_size);
	unsigned int real_block;
	fpos_t pos;

	while(cur_block * ext2->block_size < inode.i_size) {
		real_block = get_iblock(&inode, cur_block, ext2);
		if(!get_block(real_block, ext2->block_size, ext2, block)) {
			goto out;
		}
		dir = (struct ext2_dir_entry_2 *)block;
		while((unsigned int)dir < ((unsigned int)block + ext2->block_size)) {
			if(dir->rec_len - EXT2_DIR_REC_LEN(dir->name_len) >= entry_len) {
				/* Found a place */
				int len;
				len = dir->rec_len - EXT2_DIR_REC_LEN(dir->name_len);
				dir->rec_len = EXT2_DIR_REC_LEN(dir->name_len);
				dir = (struct ext2_dir_entry_2*)((unsigned int) dir + dir->rec_len);
				dir->rec_len = len;
				dir->name_len = name_len;
				dir->inode = targetinode;
				dir->file_type = type;
				strcpy(dir->name, name);

				/* Ok, let's write it */
				pos = real_block * ext2->block_size;
				fsetpos(ext2->device, &pos);
				fwrite(block, ext2->block_size, 1, ext2->device);
				goto out;
			}
			dir = (struct ext2_dir_entry_2*)((unsigned int) dir + dir->rec_len);
		}
		cur_block++;
	}
out:
	kfree(block);
}

static int ext2_make_entry(struct ext2_fs * this, const char * dirname, unsigned char type, uint_t owned, uint_t rights)
{
	char * dname;
	char * entry;
	char * cur = (char*)dirname;
	char * last = cur;
	unsigned int dir_inode;
	int block;
	struct ext2_dir_entry_2 direntry[2];
	unsigned int inode;
	unsigned int retval = 0;
	struct ext2_inode ino;
	fpos_t pos;

	while(*cur) {
		if(*cur == '/')
			last = cur;
		cur++;
	}
	dname = kmalloc(last - (char*) dirname + 2);
	memcpy(dname + 1, dirname, last - (char*)dirname);
	dname[0] = '/';
	dname[last - (char*)dirname + 1] = 0;

	entry = kmalloc(cur - last + 1);
	memcpy(entry, last, cur - last + 1);

	*(last) = 0;
	dir_inode = ext2_search_entry(this, dname, EXT2_ROOT_INO);
	if(!dir_inode) {
		retval = DIR_ERR_CANT_MAKE;
		goto out;
	}
	if(ext2_search_entry(this, entry, dir_inode)) {
		retval = DIR_ERR_EXISTS;
		goto out;
	}
	inode = ext2_alloc_inode(this);
	if(!inode) {
		retval = DIR_ERR_CANT_MAKE;
		goto out;
	}
	block = ext2_alloc_block(this);
	if(!block) {
		retval = DIR_ERR_CANT_MAKE;
		goto out;
	}

	ext2_make_direntry(this, entry, dir_inode, inode, type);

	direntry[0].inode = inode;
	strcpy(direntry[0].name, ".");
	direntry[0].name_len = 1;
	direntry[0].file_type = EXT2_FT_DIR;
	direntry[0].rec_len = EXT2_DIR_REC_LEN(direntry[0].name_len);
	direntry[1].inode = dir_inode;
	strcpy(direntry[1].name, "..");
	direntry[1].name_len = 2;
	direntry[1].file_type = EXT2_FT_DIR;
	direntry[1].rec_len = this->block_size - direntry[0].rec_len;

	pos = ext2_block_offset(this, block);
	fsetpos(this->device, &pos);
	fwrite(&direntry[0], sizeof(struct ext2_dir_entry_2), 1, this->device);
	pos += direntry[0].rec_len;
	fsetpos(this->device, &pos);
	fwrite(&direntry[1], sizeof(struct ext2_dir_entry_2), 1, this->device);

	memset(&ino, 0, sizeof(ino));
	ino.i_mode = rights | type << 13;
	ino.i_uid = owned;
	ino.i_links_count = 1;
	ino.i_blocks = 1;
	ino.i_size = direntry[0].rec_len + direntry[1].rec_len;
	ino.i_block[0] = block;
	ext2_write_inode(this, inode, &ino);

out:
	kfree(dname);
	kfree(entry);
	return retval;
}


static void ext2_release_all_blocks(struct ext2_fs *ext2, int inode_n, struct ext2_inode * inode)
{
	const unsigned int p_per_block = ext2->block_size / 4;
	char * buffer[3];
	int a;
	for(a = 0; a < EXT2_NDIR_BLOCKS && inode->i_blocks; a++, inode->i_blocks--) {
		ext2_release_block(ext2, inode->i_block[a]);
	}
	if(inode->i_blocks) {
		buffer[0] = kmalloc(ext2->block_size);
		if(get_block(inode->i_block[EXT2_IND_BLOCK], ext2->block_size, ext2, buffer[0])) {
			for(a = 0; a < p_per_block && inode->i_blocks; a++, inode->i_blocks--) {
				ext2_release_block(ext2, (int)buffer[0][a]);
			}
		}
		if(inode->i_blocks)
		if(get_block(inode->i_block[EXT2_DIND_BLOCK], ext2->block_size, ext2, buffer[0])) {
			buffer[1] = kmalloc(ext2->block_size);
			for(a = 0; a < p_per_block && inode->i_blocks; a++) {
				int b;
				if(get_block(buffer[0][a], ext2->block_size, ext2, buffer[1])) {
					for(b = 0; b < p_per_block && inode->i_blocks; b++, inode->i_blocks--) {
						ext2_release_block(ext2, (int)buffer[1][b]);
					}
				}
			}
			kfree(buffer[1]);
		}
		if(inode->i_blocks)
		if(get_block(inode->i_block[EXT2_TIND_BLOCK], ext2->block_size, ext2, buffer[0])) {
			buffer[1] = kmalloc(ext2->block_size);
			buffer[2] = kmalloc(ext2->block_size);
			for(a = 0; a < p_per_block && inode->i_blocks; a++) {
				int b;
				if(get_block(buffer[0][a], ext2->block_size, ext2, buffer[1])) {
					for(b = 0; b < p_per_block && inode->i_blocks; b++) {
						if(get_block(buffer[0][b], ext2->block_size, ext2, buffer[2])) {
							int c;
							for(c = 0; c < p_per_block && inode->i_blocks; c++) {
								ext2_release_block(ext2, (int)buffer[2][c]);

							}

						}
					}
				}
			}
			kfree(buffer[1]);
			kfree(buffer[2]);
		}
		kfree(buffer[0]);
	}
	ext2_write_inode(ext2, inode_n, inode); // add block count and possibly modify i_block
}

void *ext2_fopen(struct ext2_fs *ext2, const char * filename, uint_t mode)
{
	struct ext2_file * file;
	struct ext2_inode inode;
	int inode_n;
	char * dname;
	char * entry;
	char * cur = (char*)filename;
	char * last = cur;
	unsigned int dir_inode;

	if(ext2->mode == FILE_MODE_READ && mode & FILE_MODE_WRITE)
		return 0;

	inode_n = ext2_search_entry(ext2, filename, EXT2_ROOT_INO);
	if(!inode_n) {
		if(mode & FILE_MODE_WRITE) {
			inode_n = ext2_alloc_inode(ext2);
			while(*cur) {
				if(*cur == '/')
					last = cur;
				cur++;
			}
			dname = kmalloc(last - (char*)filename + 2);
			memcpy(dname + 1, filename, last - (char*)filename);
			dname[0] = '/';
			dname[last - (char*)filename + 1] = 0;

			entry = kmalloc(cur - last + 1);
			memcpy(entry, last, cur - last + 1);

			*(last) = 0;
			dir_inode = ext2_search_entry(ext2, dname, EXT2_ROOT_INO);
			ext2_make_direntry(ext2, entry, dir_inode, inode_n, EXT2_FT_REG_FILE);
			kfree(dname);
			kfree(entry);
		} else {
			return 0;
		}
	}
	inode = get_ext2_inode(ext2, inode_n);
	if((inode.i_mode >> 13) == EXT2_FT_DIR) {
		return 0;
	}

	if(ext2->mode == FILE_MODE_CLEAR) {
		ext2_release_all_blocks(ext2, inode_n, &inode);
	}

	file = kmalloc(sizeof(struct ext2_file));
	file->inode = kmalloc(sizeof(struct ext2_inode));
	memcpy(file->inode, &inode, sizeof(struct ext2_inode));
	file->std.func = &ext2_op.std.filefunc;
	file->fs = ext2;
	file->inode_num = inode_n;
	file->std.size = inode.i_size;
	file->mode = mode;

	if(mode & FILE_MODE_CLEAR)
		ext2_inode_add_block(file->inode, inode_n, ext2);

	ext2->refs_open++;

	return file;
}

int ext2_fclose(struct ext2_file *stream) {
	if(!stream)
		return 1;
	if(stream->mode & FILE_MODE_WRITE)
		ext2_write_inode(stream->fs, stream->inode_num, stream->inode);
	stream->fs->refs_open--;
	kfree(stream->inode);
	kfree(stream);
	

	return 0;
}

size_t ext2_fread(void *buf, size_t size, size_t count, struct ext2_file *stream)
{
	unsigned int block, offset;
	size_t read = 0;
	int to_read = size * count;
	size_t howmuch;

	if(stream->std.pos >= stream->std.size)
		return 0;
	block = stream->std.pos / stream->fs->block_size;
	offset = stream->std.pos - block * stream->fs->block_size;
	while(to_read > 0) {
		howmuch = ((stream->fs->block_size - offset) > to_read) ? to_read : (stream->fs->block_size - offset);
		if(!get_part_block(get_iblock(stream->inode, block, stream->fs), stream->fs->block_size, stream->fs, buf, size, offset)) {
			return read/size;
		}
		buf = (void*)((unsigned int)buf + stream->fs->block_size - offset);
		block++;
		read += howmuch;
		to_read -= stream->fs->block_size - offset; /* if we don't read whole block this still goes under 1 and we quit, so we don't have to care about it */
		offset = 0;
		stream->std.pos += howmuch;
	}

	return read/size;
}

size_t ext2_fwrite(void *buf, size_t size, size_t count, struct ext2_file *stream)
{
	unsigned int block, offset;
	size_t write = 0;
	int to_write = size * count;
	size_t howmuch;

	block = stream->std.pos / stream->fs->block_size;
	offset = stream->std.pos - block * stream->fs->block_size;
	while(to_write > 0) {
		howmuch = ((stream->fs->block_size - offset) > to_write) ? to_write : (stream->fs->block_size - offset);
		if(!ext2_write_part_block(get_iblock(stream->inode, block, stream->fs), stream->fs->block_size, stream->fs, buf, size, offset)) {
			return write/size;
		}
		buf = (void*)((unsigned int)buf + stream->fs->block_size - offset);
		block++;
		write += howmuch;
		to_write -= stream->fs->block_size - offset; /* if we don't write whole block this still goes under 1 and we quit, so we don't have to care about it */
		offset = 0;
		stream->std.pos += howmuch;
		stream->inode->i_size += howmuch;
		if((stream->std.pos / stream->fs->block_size) >= stream->inode->i_blocks) {
			ext2_inode_add_block(stream->inode, stream->inode_num, stream->fs);
		}
	}

	return write/size;
}

int ext2_fsetpos(struct ext2_file *stream, const fpos_t *pos) {
	stream->std.pos = *pos;
	if(stream->std.pos > stream->std.size)
		stream->std.pos = stream->std.size;
	return 0;
}

int ext2_fflush(struct ext2_file *stream) {
	return fflush(stream->fs->device);
}

long ext2_ftell(struct ext2_file *stream) {
	return stream->std.pos;
}

int ext2_fseek(struct ext2_file * stream, long int offset, int origin) {
	if(origin == SEEK_SET) {
		if(stream->std.size >= offset)
			stream->std.pos = stream->std.size - 1;
		else
			stream->std.pos = offset;
	} else if(origin == SEEK_CUR) {
		stream->std.pos += offset;
		if(stream->std.pos >= stream->std.size)
			stream->std.pos = stream->std.size - 1;
	} else if(origin == SEEK_END) {
		stream->std.pos = stream->std.size - offset - 1;
		if(stream->std.pos < 0)
			stream->std.pos = 0;
	}

	return 0;
}

int ext2_dmake(struct ext2_fs * this, const char * dirname, uint_t owned, uint_t rights)
{
	return ext2_make_entry(this, dirname, EXT2_FT_DIR, owned, rights);
}

DIR *ext2_dopen(struct ext2_fs * this, const char * dirname)
{
	struct ext2_dir * dir;
	struct ext2_inode inode;

	int inode_n = ext2_search_entry(this, dirname, EXT2_ROOT_INO);
	if(!inode_n) {
		return 0;
	}

	inode = get_ext2_inode(this, inode_n);
	if((inode.i_mode >> 13) != (EXT2_FT_DIR)) {/* not directory? */
		return 0;
	}

	dir = kmalloc(sizeof(struct ext2_dir));
	if(!dir)
		return 0;

	dir->directory_inode = inode_n;
	dir->inode = kmalloc(sizeof(struct ext2_inode));
	if(!dir->inode)
		return 0;
	memcpy(dir->inode, &inode, sizeof(struct ext2_inode));

	dir->std.func = &ext2_op.std.dirfunc;
	dir->fs = this;
	dir->std.size = inode.i_size;
	dir->std.name = 0;
	dir->inode_num = 0;
	dir->buffer = kmalloc(this->block_size);
	dir->pos = 0;
	dir->block = -1;

	return (DIR*)dir;
}

int ext2_dread(struct ext2_dir * listing) {
	struct ext2_fs * ext2 = listing->fs;
	struct ext2_inode inode;
	struct ext2_dir_entry_2 * dir = (struct ext2_dir_entry_2 *)(listing->pos + listing->buffer);
	unsigned int cur_block = listing->block;

	if(listing->block == -1) {
		listing->block = 0;
		cur_block = 0;
		if(!get_block(get_iblock(listing->inode, cur_block, ext2), ext2->block_size, ext2, listing->buffer))
			return 1;
	}

	while(cur_block * ext2->block_size < (listing->inode->i_size)) {
		while((unsigned int)dir < ((unsigned int)listing->buffer + ext2->block_size)) {
			if(dir->inode) {
				inode = get_ext2_inode(ext2, dir->inode);

				listing->std.name = krealloc(listing->std.name, dir->name_len + 1);
				memcpy(listing->std.name, dir->name, dir->name_len);
				listing->std.name[dir->name_len] = 0;
				listing->std.size = inode.i_size;
				listing->std.uid = inode.i_uid;
				listing->std.gid = inode.i_gid;
				listing->std.rights = inode.i_mode & 0777;
				listing->std.created = inode.i_ctime;
				listing->std.accessed = inode.i_atime;
				listing->std.modified = inode.i_mtime;
				listing->std.references = inode.i_links_count;
				listing->inode_num = dir->inode;

				listing->pos = ((unsigned int)dir - (unsigned int)listing->buffer) + dir->rec_len;
				listing->block = cur_block;
				return 0;
			}
			dir = (struct ext2_dir_entry_2*)((unsigned int) dir + dir->rec_len);
		}
		cur_block++;
		if(!get_block(get_iblock(listing->inode, cur_block, ext2), ext2->block_size, ext2, listing->buffer))
			return 1;
	}
	return 1;
}

int ext2_dclose(struct ext2_dir *listing) {
	kfree(listing->inode);
	kfree(listing->buffer);
	kfree(listing);

	return 0;
}
