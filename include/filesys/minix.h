#ifndef _MINIX_FS_H
#define _MINIX_FS_H 1

#include <filesys/filesystem.h>
#include <list.h>
#include <stdint.h>

struct minix_inode {
	uint16_t flags;
	/*
		rights : 9,
		_todo00 : 3,
		is_pipe  : 1,
		is_dev   : 1,
		is_dir   : 1,
		is_file  : 1;
	*/
	uint16_t uid;
	uint32_t size;
	uint32_t modified;
	uint8_t gid, num_refs;
	union {
		struct {
			uint16_t std[7], indir, dbl_indir;
		} zones;
		struct {
			uint8_t minor, major;
		} dev;
	} u;
} __attribute__((packed));

struct minix_direntry {
	uint16_t inode;
	char name[30];
};
struct minix_direntry_own {
	struct minix_direntry real;
	char nullchar;
};

struct minix_superblock {
	uint16_t num_inodes;
	uint16_t num_blocks;
	uint16_t num_inode_map_zones;
	uint16_t num_zone_map_zones;
	uint16_t first_data_zone;
	uint16_t _todo2;
	uint32_t max_size;
	uint32_t magic; // TODO: Onkohan oikein?
	uint32_t _todo_4567_jne;
};

typedef enum _1x2_t {
	_1x2__1,
	_1x2__X,
	_1x2__2
} _1x2_t;

#define MINIX_FS_MAGIC_A (0x0001137F)
#define MINIX_FS_MAGIC_B (0x0001138F)

#define MINIX_FS_NUM_ZONES 7
#define MINIX_FS_FLAG_RIGHTS (0x01ff) /*(0777)*/
#define MINIX_FS_RIGHTS(x) ((x) & MINIX_FS_FLAG_RIGHTS)
//#define MINIX_FS_FLAG_  (0x0200)
//#define MINIX_FS_FLAG_  (0x0400)
//#define MINIX_FS_FLAG_  (0x0800)
#define MINIX_FS_FLAG_PIPE      (0x1)
#define MINIX_FS_FLAG_CHARDEV   (0x2)
#define MINIX_FS_FLAG_DIR       (0x4)
#define MINIX_FS_FLAG_BLOCKDEV  (0x6)
#define MINIX_FS_FLAG_FILE      (0x8)
#define MINIX_FS_FLAG_SYMLINK   (0xa) /* FILE | CHARDEV */

#define MINIX_FS_IS(x, what) (((x) >> 12) == (MINIX_FS_FLAG_ ## what))

#define MINIX_FS_ZONE_SIZE 1024

#define MINIS_FS_INODE_SIZE (sizeof(struct minix_inode))
#define MINIX_FS_DIRENTRY_SIZE (sizeof(struct minix_direntry))

struct minix_list_inode {
	uint16_t inode_n;
	struct minix_inode inode;
	uint32_t num_refs;
};

LIST_TYPE(minix_list_inode, struct minix_list_inode);

struct minix_fs {
	struct fs std;

	FILE *dev;
	struct minix_superblock super;
	fpos_t pos_inode_map;
	fpos_t pos_zone_map;
	fpos_t pos_inodes;
	fpos_t pos_data;

	int filename_maxlen;

	uint32_t open_inodes_refcount;
	list_of_minix_list_inode open_inodes;

	uint8_t *inode_map;
	uint8_t *zone_map;
	uint8_t *end_map;
};

struct minix_file {
	FILE std;

	FILE *dev;
	void (*freefunc)(void*);
	uint32_t pos, size;
	uint32_t dev_zones_pos, dev_zone_map_pos;
	struct minix_fs *fs;
	uint16_t inode_n;
	struct minix_inode *inode;
	list_iter_of_minix_list_inode inode_iter;
};

struct minix_dir {
	DIR std;

	struct minix_file *file;
	struct minix_direntry_own direntry;
	struct minix_inode inode;
};

extern struct fs *minix_mount(FILE *device, uint_t mode);

int minix_umount(struct minix_fs *this);

struct minix_file *minix_fopen(struct minix_fs *this, const char * filename, uint_t mode);
int minix_fclose(struct minix_file *stream);

size_t minix_fread(void *buf, size_t size, size_t count, struct minix_file *stream);
size_t minix_fwrite(const void *buf, size_t size, size_t count, struct minix_file *stream);

int minix_fflush(struct minix_file *stream);

//int minix_fsetpos(struct minix_file *stream, const fpos_t *pos);

int minix_dmake(struct minix_fs *this, const char * dirname, uint_t owner, uint_t rights);
struct minix_dir *minix_dopen(struct minix_fs *this, const char * dirname);
int minix_dread(struct minix_dir *listing);
int minix_dclose(struct minix_dir *listing);

#endif
