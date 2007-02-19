#include <ext2.h>
#include <blockdev.h>
#include <malloc.h>
#include <screen.h>
#include <stddef.h>
#include <string.h>

struct ext2fs_mem* ext2;

/* get_ext2_inode gets inode number `num' from filesystem */

struct ext2_inode get_ext2_inode(struct ext2fs_mem * ext2, unsigned int num) {
	static struct ext2_inode inode;
	unsigned short desc_n = (num  - 1)/ ext2->super_block->s_inodes_per_group;

	if(num < 1 || num > ext2->super_block->s_inodes_count) {
		kprintf("Bad inode number!\n");
		return inode;
	}

	if(desc_n != ext2->group_desc_n) {
		dseek(ext2->dev, ext2->super_block->s_blocks_per_group *  ext2->block_size * desc_n, SEEK_SET);
		dread(ext2->group_desc, sizeof(struct ext2_group_desc), 1, ext2->dev);

		ext2->group_desc_n = desc_n;
	}

	dseek(ext2->dev, ext2->block_size * ext2->group_desc->bg_inode_table + (num - 1) % ext2->super_block->s_inodes_per_group * ext2->super_block->s_inode_size, SEEK_SET);
	dread(&inode, ext2->super_block->s_inode_size, 1, ext2->dev);

	return inode;
}
/* Get_block: Get a block from device*/
char * get_block(unsigned int num, size_t size, struct ext2fs_mem * ext2) {
	char * buffer = kmalloc(size);

	dseek(ext2->dev, size * num, SEEK_SET);
	if(dread(buffer, size, 1, ext2->dev) < 1) {
		kfree(buffer);
		return (char*)0;
	}
	return buffer;
}
/* Get_iblock returns inode's block number `block'
   We can hope that it works ;) */
char * get_iblock(struct ext2_inode * inode, unsigned int block, struct ext2fs_mem * ext2) {
	unsigned int block_n = 0;
	unsigned int * buffer;
	unsigned short p_per_block = ext2->block_size / sizeof(long);
	if(block < EXT2_NDIR_BLOCKS) {
		block_n = inode->i_block[block];
		if(inode->i_block[block] == 0)
			return (char*)0;
	} else if(block >= EXT2_IND_BLOCK && block < (p_per_block + EXT2_NDIR_BLOCKS)) {
		buffer = (unsigned int *)get_block(inode->i_block[EXT2_IND_BLOCK], ext2->block_size, ext2);
		block_n = buffer[block - EXT2_NDIR_BLOCKS];
		kfree(buffer);
	} else if(block >= (p_per_block + EXT2_NDIR_BLOCKS) && block < (p_per_block * p_per_block + EXT2_IND_BLOCK)) {
		buffer = (unsigned int*)get_block(inode->i_block[EXT2_DIND_BLOCK], ext2->block_size, ext2);
		block_n = buffer[(block - EXT2_IND_BLOCK - p_per_block) / p_per_block];
		kfree(buffer);
		buffer = (unsigned int*)get_block(block_n, ext2->block_size, ext2);
		block_n = buffer[block - EXT2_IND_BLOCK - p_per_block * 2 ];
		kfree(buffer);
	} else if(block >= (p_per_block * p_per_block + EXT2_IND_BLOCK) && block < (p_per_block * p_per_block * p_per_block + EXT2_DIND_BLOCK)) {
		buffer = (unsigned int*)get_block(inode->i_block[EXT2_TIND_BLOCK], ext2->block_size, ext2);
		block_n = buffer[(block - EXT2_IND_BLOCK - p_per_block - p_per_block * p_per_block) / (p_per_block*p_per_block)];
		kfree(buffer);
		buffer = (unsigned int*)get_block(block_n, ext2->block_size, ext2);
		block_n = buffer[(block - EXT2_IND_BLOCK - p_per_block - p_per_block * p_per_block) / p_per_block];
		kfree(buffer);
		buffer = (unsigned int*)get_block(block_n, ext2->block_size, ext2);
		block_n = buffer[(block - EXT2_IND_BLOCK - p_per_block - p_per_block * p_per_block)];
		kfree(buffer);
	}

	return get_block(block_n, ext2->block_size, ext2);
}

char * readfile(const char * name) { /* TODO: Clean up when we get a new API */
	char entry[EXT2_NAME_LEN + 1];
	unsigned int chars;
	unsigned int inode_n = EXT2_ROOT_INO;
	unsigned int last_inode = EXT2_ROOT_INO;
	unsigned int entry_len;
	struct ext2_inode inode;
	struct ext2_dir_entry_2 * dir;
	unsigned int cur_block;
	char * ret;
	char * block;

	if(name[0] != '/')
		return 0;
	while(name[0]) { /* while something to parse */
		if(last_inode == inode_n) {
			kprintf("EXT2: Could not find entry!\n");
		}

		last_inode = inode_n;
		chars = 0;
		name++;
		while(name[0] && name[0] != '/') {
			entry[chars++] = name[0];
			name++;
		}

		entry_len = strlen(entry);
		inode = get_ext2_inode(ext2, inode_n);
		for(cur_block = 0; cur_block < (inode.i_size  + ext2->block_size - 1)/ ext2->block_size; cur_block++) {
			kprintf("inode.i_mode is %d, %d\n", inode.i_mode, inode.i_mode >> 13);
			if(inode.i_mode == (EXT2_FT_REG_FILE << 13)) {
				goto out;
			} else if(inode.i_mode == (EXT2_FT_DIR << 13)) { /* directory? */
				if(name[1] != '/') {
					kprintf("Trying to use directory as file!\n");
					return 0;
				}
			} else if(inode.i_mode & ~(EXT2_FT_DIR << 13)) {
				if(name[1] == '/') {
					kprintf("Trying to use file as directory!\n");
					return 0;
				}
			} 
			block = get_iblock(&inode, cur_block, ext2);
			if(!block) {
				return 0;
			}
			dir = (struct ext2_dir_entry_2 *)block;
			while((unsigned int)dir < ((unsigned int)block + ext2->block_size)) {
				kprintf("entry_len == %d, dir->name_len==%d, entry==%s\n", entry_len, dir->name_len, entry);
				if(entry_len == dir->name_len) {
					char entry_name[EXT2_NAME_LEN + 1];
					strncpy(entry_name, dir->name, dir->name_len);
					entry_name[dir->name_len] = '\0';
					kprintf("Entry_name == \"%s\", entry==\"%s\"\n", entry_name, entry);
					if(!strcmp(entry_name, entry)) {
						inode_n = dir->inode;
						kprintf("Inode_n == %d\n", inode_n);
						cur_block =  (inode.i_size  + ext2->block_size - 1)* ext2->block_size;
						break;
					}
				}
				dir = (struct ext2_dir_entry_2*)((unsigned int) dir + dir->rec_len);
			}
			
			kfree(block);
		}
	}
out:
	inode = get_ext2_inode(ext2, inode_n); 
	ret = kmalloc(inode.i_size);
	block = kmalloc(ext2->block_size);
	for(cur_block = 0; cur_block < (inode.i_size  + ext2->block_size - 1)/ ext2->block_size; cur_block++) {
		block = get_iblock(&inode, cur_block, ext2);
		memcpy(&ret[cur_block * ext2->block_size], block, ext2->block_size);
	}
	kfree(block);

	return ret;
}


void read_super_block(BD_DESC * dev) { /* TODO: Clean up when we get a new API */
	struct ext2_inode temp_inode;
	int cur_block;
	char * block;

	ext2 = kmalloc(sizeof(struct ext2fs_mem));
	ext2->super_block = kmalloc(1024);

	ext2->dev = dev;
	
	dseek(dev, 1024, SEEK_SET);
	if(dread(ext2->super_block, 512, 2, dev) < 2) {
		kprintf("Couldn't read superblock!\n");
		return;
	}
	kprintf("S_inode_size is %x\n", ext2->super_block->s_inode_size);
	if(ext2->super_block->s_magic != EXT2_SUPER_MAGIC) {
		kprintf("Invalid magic number in superblock! (no ext2)\n");
		return;
	}
	if(ext2->super_block->s_log_block_size < 3)
		ext2->block_size = 1024 << ext2->super_block->s_log_block_size;
	else {
		kprintf("S_log_block_size is something wierd\n");
		return;
	}

	kprintf("inodes_count is %d\n", ext2->super_block->s_inodes_count);

	ext2->group_desc = kmalloc(sizeof(struct ext2_group_desc));
	ext2->group_desc_n = 0;

	dseek(dev, 2048, SEEK_SET);
	if(dread(ext2->group_desc, sizeof(struct ext2_group_desc), 1, dev) < 1) {
		kprintf("Read error\n");
		return;
	}

	temp_inode = get_ext2_inode(ext2, EXT2_ROOT_INO);
	kprintf("It has %d blocks, i_links_count %d\n", (temp_inode.i_size  + ext2->block_size - 1)/ ext2->block_size, temp_inode.i_links_count);
	struct ext2_dir_entry_2 * dir;
	for(cur_block = 0; cur_block < (temp_inode.i_size  + ext2->block_size - 1)/ ext2->block_size; cur_block++) {
		block = get_iblock(&temp_inode, cur_block, ext2);
		if(!block) {
			kprintf("Got a problem, exiting\n");
			return;
		}
		dir = (struct ext2_dir_entry_2 *)block;
		while((unsigned int)dir < ((unsigned int)block + ext2->block_size)) {
			char name[EXT2_NAME_LEN + 1];
			strncpy(name, dir->name, dir->name_len);
			name[dir->name_len] = '\0';
			kprintf("/%s permissions: %o, %d\n", name, get_ext2_inode(ext2, dir->inode).i_mode & 0777, get_ext2_inode(ext2, dir->inode).i_mode >> 13);

			dir = (struct ext2_dir_entry_2*)((unsigned int) dir + dir->rec_len);
		}
		kfree(block);
	}
}
