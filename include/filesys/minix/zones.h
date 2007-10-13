#ifndef _MINIX_ZONES_H
#define _MINIX_ZONES_H 1

#include <filesys/minix/minix.h>
#include <stdint.h>

struct minix_zone_data_t {
	uint16_t zone, *data, *begin, *end;
};
struct minix_zone_allocer_t {
	struct minix_file *f;
	struct minix_fs *fs;
	struct minix_inode *inode;
	size_t zone0, zone1;
	uint16_t *zonelist, *dbl_indir_list;
	const int write;
	uint8_t *map, *map_end;
	uint16_t map_pos, map_bit;

	struct minix_zone_data_t buf1, buf2;

	int eof;
};

extern size_t minix_get_zones(struct minix_file *f, const size_t zone0, const size_t zone1, uint16_t *zonelist, const int write);
extern int minix_free_all_zones_from_inode(struct minix_fs * const fs, struct minix_inode * const inode);

#endif

