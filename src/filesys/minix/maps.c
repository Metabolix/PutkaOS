#include <filesys/minix/minix.h>
#include <filesys/minix/zones.h>
#include <filesys/minix/maps.h>

#include <time.h>
#include <memory/kmalloc.h>

#define MALLOC kmalloc
#define CALLOC kcalloc
#define FREE kfree
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static const char minix_reserved_zeros[MINIX_ZONE_SIZE] = {0};

static uint16_t minix_alloc_unclean_zone(struct minix_zone_allocer_t * const zal);

uint16_t minix_alloc_inode(struct minix_fs *fs, uint16_t type)
{
	// TODO: minix_alloc_inode: Rights, uid, gid
	struct minix_list_inode inoli = {
		.inode = {
			.flags = (type << 12) |
				(type == MINIX_FLAG_SYMLINK ? 0777 :
				(type == MINIX_FLAG_DIR ? 0755 : 0644)),
			.uid = 0,
			.size = 0,
			.modified = time(0),
			.gid = 0,
			.num_refs = 1,
		},
		.num_refs = 0,
	};
	uint8_t *ptr;
	int i;
	for (ptr = fs->inode_map; ptr != fs->zone_map; ++ptr) {
		if (*ptr != 0xff) {
			break;
		}
	}
	if (ptr == fs->zone_map) {
		return 0;
	}
	for (i = 0; i < 8; ++i) {
		if ((*ptr & (1 << i)) == 0) {
			break;
		}
	}
	*ptr |= 1 << i;
	fs->inode_map_changed = 1;

	inoli.inode_n = 8 * (ptr - fs->inode_map) + i;
	list_insert(list_end(fs->open_inodes), inoli);
	return inoli.inode_n;
}

#define MAP_RM_BIT(map, n) (*((uint8_t*)(map) + ((n) / 8)) &= ~(1 << ((n) % 8)))

int minix_free_inode(struct minix_fs *fs, uint16_t n)
{
	MAP_RM_BIT(fs->inode_map, n);
	fs->inode_map_changed = 1;
	return 0;
}

int minix_free_zone(struct minix_fs *fs, uint16_t n)
{
	n -= fs->super.first_data_zone - 1;
	MAP_RM_BIT(fs->zone_map, n);
	fs->zone_map_changed = 1;
	return 0;
}

static uint16_t minix_alloc_unclean_zone(struct minix_zone_allocer_t * const zal)
{
	if (!zal->write) {
		zal->eof = 1;
		return 0;
	}
	while (zal->map != zal->map_end) {
		if (*zal->map != 0xff) while (zal->map_bit < 8) {
			if ((*zal->map & (1 << zal->map_bit)) == 0) {
				*zal->map |= 1 << zal->map_bit;
				zal->fs->zone_map_changed = 1;
				return ((zal->map_pos << 3) + zal->map_bit)
					+ (zal->fs->super.first_data_zone - 1);
			}
			++zal->map_bit;
		}
		++zal->map;
		++zal->map_pos;
		zal->map_bit = 0;
	}
	zal->eof = 1;

	return 0;
}

uint16_t minix_alloc_zone(struct minix_zone_allocer_t * const zal)
{
	uint16_t z = minix_alloc_unclean_zone(zal);
	if (!z) {
		return 0;
	}
	fpos_t pos = z * MINIX_ZONE_SIZE;
	if (fsetpos(zal->f->dev, &pos)
	|| fwrite(minix_reserved_zeros, MINIX_ZONE_SIZE, 1, zal->f->dev) != 1) {
		goto error_etc;
	}
	return z;
error_etc:
	minix_free_zone(zal->fs, z);
	return 0;
}
