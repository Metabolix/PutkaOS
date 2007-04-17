#include <filesys/ext2.h>
#include <filesys/filesystem.h>
#include <malloc.h>
#include <screen.h>
#include <stddef.h>
#include <string.h>
#include <debugprint.h>


struct ext2_fs ext2_op = {
        {
                (fs_mount_t)  ext2_mount,
                (fs_umount_t) ext2_umount,
                {
                        (fopen_t)     ext2_fopen,
                        (fclose_t)    ext2_fclose,
                        (fread_t)     ext2_fread,
                        (fwrite_t)    ext2_fwrite,
                        (fflush_t)    ext2_fflush,
                        (fgetpos_t)   ext2_fgetpos,
                        (fsetpos_t)   ext2_fsetpos
                },
                {
                        (dmake_t)     ext2_dmake,
                        (dopen_t)     ext2_dopen,
                        (dclose_t)    ext2_dclose,
                        (dread_t)     ext2_dread
                }
        },
	0,
	0,
	0,
	0,
	0
};

/* get_ext2_inode gets inode number `num' from filesystem */

struct ext2_inode get_ext2_inode(struct ext2_fs * ext2, unsigned int num) {
	static struct ext2_inode inode;
	unsigned short desc_n = (num  - 1)/ ext2->super_block->s_inodes_per_group;

	if(num < 1 || num > ext2->super_block->s_inodes_count) {
		DEBUGP("Bad inode number!\n");
		return inode;
	}

	if(desc_n != ext2->group_desc_n) {
		fseek(ext2->device, ext2->super_block->s_blocks_per_group *  ext2->block_size * desc_n, SEEK_SET);
		fread(ext2->group_desc, sizeof(struct ext2_group_desc), 1, ext2->device);

		ext2->group_desc_n = desc_n;
	}

	fseek(ext2->device, ext2->block_size * ext2->group_desc->bg_inode_table + (num - 1) % ext2->super_block->s_inodes_per_group * ext2->super_block->s_inode_size, SEEK_SET);
	fread(&inode, ext2->super_block->s_inode_size, 1, ext2->device);

	return inode;
}
/* Get_block: Get a block from device*/
char * get_block(unsigned int num, size_t size, struct ext2_fs * ext2, void * buffer) {
	fseek(ext2->device, size * num, SEEK_SET);
	if(fread(buffer, size, 1, ext2->device) < 1) {
		return (char*)0;
	}
	return buffer;
}

/* Get_part_block: Get a part of a block */
char * get_part_block(unsigned int num, size_t size, struct ext2_fs * ext2, void * buffer, size_t read, unsigned int offset) {
	fseek(ext2->device, size*num + offset, SEEK_SET);
	if(fread(buffer, read, 1, ext2->device) < 1) {
		return (char*)0;
	}
	return buffer;
}

/* Get_iblock returns inode's block number `block'
   We can hope that it works ;) */
int get_iblock(struct ext2_inode * inode, unsigned int block, struct ext2_fs * ext2) {
	unsigned int block_n = 0;
	unsigned int * buffer = kmalloc(ext2->block_size);
	unsigned short p_per_block = ext2->block_size / sizeof(long);
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

struct fs *ext2_mount(FILE *device, uint_t mode) {
	struct ext2_fs * ext2_fs = kmalloc(sizeof(struct ext2_fs));

	memcpy(&ext2_fs->std, &ext2_op, sizeof(struct fs));
	ext2_fs->super_block = kmalloc(sizeof(struct ext2_super_block));
	ext2_fs->device = device;
	
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
		DEBUGP("S_log_block_size is something wierd\n");
		return 0;
	}

	//kprintf("inodes_count is %d\n", ext2_fs->super_block->s_inodes_count);

	ext2_fs->group_desc = kmalloc(sizeof(struct ext2_group_desc));
	ext2_fs->group_desc_n = 0;

	fseek(device, 2048, SEEK_SET);
	if(fread(ext2_fs->group_desc, sizeof(struct ext2_group_desc), 1, device) < 1) {
		DEBUGP("Read error\n");
		return 0;
	}
	return (struct fs*)ext2_fs;
}

int ext2_umount(struct ext2_fs *this) {
	kfree(this->group_desc);
	kfree(this->super_block);
	kfree(this);

	return 0;
}
/* looks for entry at end of the filename, returns inode number */
int ext2_search_entry(struct ext2_fs * ext2, const char * filename) 
{
	char entry[EXT2_NAME_LEN + 1];
	unsigned int chars;
	unsigned int inode_n = EXT2_ROOT_INO;
	unsigned int last_inode = -1;
	unsigned int entry_len;
	struct ext2_inode inode;
	struct ext2_dir_entry_2 * dir;
	unsigned int cur_block = 0;
	char * block = kmalloc(ext2->block_size);
	int first = 1;
	
	if(filename[0] == 0)
		return EXT2_ROOT_INO;
	
	while(filename[0] == '/' || first || filename[0]) { /* while something to parse */
		roll:
		first = 0;
		if(last_inode == inode_n) {
			if(filename[0] == '/')
				break;
			goto error;
		}

		last_inode = inode_n;
		chars = 0;
		while(filename[0] && filename[0] != '/') {
			entry[chars++] = filename[0];
			filename++;
		}
		filename++;
		entry[chars] = 0;

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
	}
	kfree(block);
	return inode_n;
error:
	kfree(block);
	return 0;
}

void *ext2_fopen(struct ext2_fs *ext2, const char * filename, uint_t mode) {
	struct ext2_file * file; /*= kmalloc(sizeof(struct ext2_file));*/
	struct ext2_inode inode;
	int inode_n = ext2_search_entry(ext2, filename);
	if(!inode_n) {
		//kprintf("EXT2: no such entry\n");
		return 0;
	}
	inode = get_ext2_inode(ext2, inode_n);
	if((inode.i_mode >> 13) & EXT2_FT_DIR) /* directory? */
		return 0;

	file = kmalloc(sizeof(struct ext2_file));
	file->inode = kmalloc(sizeof(struct ext2_inode));
	memcpy(file->inode, &inode, sizeof(struct ext2_inode));
	file->std.func = &ext2_op.std.filefunc;
	file->fs = ext2;
	file->std.size = inode.i_size;

	return file;
}

int ext2_fclose(struct ext2_file *stream) {
	kfree(stream->inode);
	kfree(stream);

	return 0;
}

size_t ext2_fread(void *buf, size_t size, size_t count, struct ext2_file *stream) {
	unsigned int block, offset;
	size_t read = 0;
	int to_read = size * count;

	if(stream->std.pos >= stream->std.size)
		return 0;
	block = stream->std.pos / stream->fs->block_size;
	offset = block * stream->fs->block_size - stream->std.pos;
	while(to_read > 0) {
		if(!get_part_block(get_iblock(stream->inode, block, stream->fs), stream->fs->block_size, stream->fs, buf, ((stream->fs->block_size - offset) > to_read) ? to_read : (stream->fs->block_size - offset), offset))
			break;
		buf = (void*)((unsigned int)buf + stream->fs->block_size - offset);
		block++;
		read++;
		to_read -= stream->fs->block_size - offset; /* if we don't read whole block this still goes under 1 and we quit, so we don't have to care about it */
		offset = 0;
	}
	stream->std.pos += size * count;

	return read;
}

size_t ext2_fwrite(void *buf, size_t size, size_t count, struct ext2_file *stream) {
	return 0;
}

int ext2_fgetpos(struct ext2_file *stream, fpos_t *pos) {
	*pos = stream->std.pos;
	return 0;
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
	return EOF;
}

int ext2_dmake(struct ext2_fs * this, const char * dirname, uint_t owned, uint_t rights) {
	return -1;
}

DIR *ext2_dopen(struct ext2_fs * this, const char * dirname) 
{
	struct ext2_dir * dir;
	struct ext2_inode inode;
	
	int inode_n = ext2_search_entry(this, dirname);
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

	return (DIR*)dir;
}

int ext2_dread(struct ext2_dir * listing) {
	unsigned int found = 0;
	struct ext2_fs * ext2 = listing->fs;
	struct ext2_inode inode;
	struct ext2_dir_entry_2 * dir;
	unsigned int cur_block = 0;
	char * block = kmalloc(ext2->block_size);

	if(!listing->inode_num)
		found = 1;


	while(cur_block * ext2->block_size < (listing->inode->i_size)) {
		if(!get_block(get_iblock(listing->inode, cur_block, ext2), ext2->block_size, ext2, block)) {
			return 1;
		}
		dir = (struct ext2_dir_entry_2 *)block;
		while((unsigned int)dir < ((unsigned int)block + ext2->block_size)) {
			if(dir->inode && ((dir->inode == listing->inode_num && !found) || (dir->inode != listing->inode_num && found))) {
				if(found == 0) {
					found = 1;
				} else {
					inode = get_ext2_inode(ext2, dir->inode);

					listing->std.name = krealloc(listing->std.name, dir->name_len + 1);
					memcpy(listing->std.name, dir->name, dir->name_len);
					listing->std.name[dir->name_len] = 0;
					listing->std.size = inode.i_size;
					listing->std.owner = inode.i_uid;
					listing->std.rights = inode.i_mode & 0777;
					listing->std.created = inode.i_ctime;
					listing->std.accessed = inode.i_atime;
					listing->std.modified = inode.i_mtime;
					listing->std.references = inode.i_links_count;
					listing->inode_num = dir->inode;

					kfree(block);
					return 0;
				}
			}
			dir = (struct ext2_dir_entry_2*)((unsigned int) dir + dir->rec_len);
		}
		cur_block++;
	}
	kfree(block);
	return 1;
}

int ext2_dclose(struct ext2_dir *listing) {
	kfree(listing->inode);
	kfree(listing);

	return 0;
}
