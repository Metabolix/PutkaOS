#ifndef _MINIX_H
#define _MINIX_H 1

#include <filesys/filesystem.h>
#include <list.h>
#include <stdint.h>

#define MINIX_STD_ZONES (7)
#define MINIX_ZONES_PER_ZONE (512)
#define MINIX_INDIR_ZONES (MINIX_ZONES_PER_ZONE)

#define MINIX_STD_ZONES_END \
	MINIX_STD_ZONES
#define MINIX_INDIR_ZONES_END \
	(MINIX_STD_ZONES_END + MINIX_INDIR_ZONES)
#define MINIX_DBL_INDIR_ZONES_END \
	(MINIX_INDIR_ZONES_END + (MINIX_INDIR_ZONES * MINIX_INDIR_ZONES))

#define MINIX_MAX_ZONES (MINIX_DBL_INDIR_ZONES_END)

#define MINIX_ROOT_INODE (1)

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
			uint16_t std[MINIX_STD_ZONES], indir, dbl_indir;
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

struct minix_superblock {
	uint16_t num_inodes;
	uint16_t num_zones;
	uint16_t num_inode_map_zones;
	uint16_t num_zone_map_zones;
	uint16_t first_data_zone;
	uint16_t _todo2;
	uint32_t max_size;
	uint32_t magic; // TODO: Onkohan oikein?
	uint32_t _todo_4567_jne;
};

#define MINIX_MAGIC_A (0x0001137F)
#define MINIX_MAGIC_B (0x0001138F)

#define MINIX_NUM_ZONES 7
#define MINIX_FLAG_RIGHTS (0x01ff) /*(0777)*/
#define MINIX_RIGHTS(x) ((x) & MINIX_FLAG_RIGHTS)
//#define MINIX_FLAG_  (0x0200)
//#define MINIX_FLAG_  (0x0400)
//#define MINIX_FLAG_  (0x0800)
#define MINIX_FLAG_PIPE      (0x1)
#define MINIX_FLAG_CHARDEV   (0x2)
#define MINIX_FLAG_DIR       (0x4)
#define MINIX_FLAG_BLOCKDEV  (0x6) /* DIR | CHARDEV */
#define MINIX_FLAG_FILE      (0x8)
#define MINIX_FLAG_SYMLINK   (0xa) /* FILE | CHARDEV */

#define MINIX_IS(x, what) (((x) >> 12) == (MINIX_FLAG_ ## what))
#if 0
#define MINIX_DIRENTRY_TYPES ( (const uint_t[]) { \
		/*0,1*/ DIRENTRY_ERROR, DIRENTRY_PIPE, \
		/*2,3*/ DIRENTRY_CHARDEV, DIRENTRY_ERROR, \
		/*4,5*/ DIRENTRY_DIR, DIRENTRY_ERROR, \
		/*6,7*/ DIRENTRY_BLOCKDEV, DIRENTRY_ERROR, \
		/*8,9*/ DIRENTRY_FILE, DIRENTRY_, \
		/*a,b*/ DIRENTRY_, DIRENTRY_, \
		/*c,d*/ DIRENTRY_, DIRENTRY_, \
		/*e,f*/ DIRENTRY_, DIRENTRY_, \
} )
#define MINIX_DIRENTRY_TYPE(x) (MINIX_DIRENTRY_TYPES[((x) >> 12)])
#else
#define MINIX_DIRENTRY_TYPEy(x) ( \
	x == MINIX_FLAG_FILE ? DIRENTRY_FILE : (\
	x == MINIX_FLAG_DIR ? DIRENTRY_DIR : (\
	x == MINIX_FLAG_SYMLINK ? DIRENTRY_SYMLINK : (\
	x == MINIX_FLAG_PIPE ? DIRENTRY_PIPE : (\
	x == MINIX_FLAG_CHARDEV ? DIRENTRY_CHARDEV : (\
	x == MINIX_FLAG_BLOCKDEV ? DIRENTRY_BLOCKDEV : (\
	DIRENTRY_ERROR )))))) \
)
#define MINIX_DIRENTRY_TYPE(x) ( MINIX_DIRENTRY_TYPEy(((x) >> 12)) )
#endif

#define MINIX_ZONE_SIZE 1024

#define MINIX_FS_INODE_SIZE (sizeof(struct minix_inode))
#define MINIX_DIRENTRY_SIZE (sizeof(struct minix_direntry))

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

	size_t inode_map_size, zone_map_size;

	int filename_maxlen;

	uint32_t open_inodes_refcount;
	list_of_minix_list_inode open_inodes;

	uint8_t *inode_map;
	uint8_t *zone_map;
	uint8_t *end_map;
	int zone_map_changed, inode_map_changed;
};

struct minix_file {
	FILE std;

	FILE *dev;
	uint32_t pos, size;
	uint32_t dev_zones_pos, dev_zone_map_pos;
	struct minix_fs *fs;
	uint16_t inode_n;
	struct minix_inode *inode;
	list_iter_of_minix_list_inode inode_iter;

	int written;
	int alloced;
};

struct minix_dir {
	DIR std;

	int alloced;
	struct minix_file file;
	struct minix_direntry direntry;
	int nullchar;
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
int minix_ioctl(struct minix_file *f, int request, uintptr_t param);

int minix_dmake(struct minix_fs *this, const char * dirname);
struct minix_dir *minix_dopen(struct minix_fs *this, const char * dirname);
int minix_dread(struct minix_dir *listing);
int minix_dclose(struct minix_dir *listing);

int minix_link (struct minix_fs *fs, const char *src, const char *dest);
int minix_symlink (struct minix_fs *fs, const char *src, const char *dest);
int minix_unlink (struct minix_fs *fs, const char *src);
int minix_getprops (struct minix_fs *fs, const char *src, struct file_props *val);
int minix_setprops (struct minix_fs *fs, const char *src, const struct file_props *val);

#endif
