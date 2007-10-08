#include <filesys/minix/minix.h>
#include <filesys/minix/zones.h>
#include <filesys/minix/maps.h>

#include <malloc.h>

#define MALLOC kmalloc
#define CALLOC kcalloc
#define FREE kfree
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/****
** Statics
****/

static int minix_get_std_zones(struct minix_zone_allocer_t * const zal);
static int minix_read_uint16s_from_zone(struct minix_zone_allocer_t * const zal, uint16_t zone, size_t begin, size_t end, int alloced);
static int minix_get_indir_zones(struct minix_zone_allocer_t * const zal);
static int minix_get_dbl_indir_zones_sub(struct minix_zone_allocer_t * const zal, const size_t begin, const size_t end);
static int minix_get_dbl_indir_zones(struct minix_zone_allocer_t * const zal);
static int minix_zones_flush(struct minix_zone_data_t *data, struct minix_file *f);

/************************
* MINIX ZONE FUNCTIONS **
************************/

static int minix_get_std_zones(struct minix_zone_allocer_t * const zal)
{
	uint16_t zone;
	const size_t end = MIN(zal->zone1, MINIX_STD_ZONES_END);
	if (!(zal->zone0 < zal->zone1)) {
		return 0;
	}
	while (zal->zone0 < end) {
		if (!(zone = zal->inode->u.zones.std[zal->zone0])) {
			if (!(zone = minix_alloc_zone(zal))) {
				return -1;
			}
			zal->inode->u.zones.std[zal->zone0] = zone;
		}
		*zal->zonelist = zone;
		++zal->zonelist;
		++zal->zone0;
	}
	return 0;
}

static int minix_read_uint16s_from_zone(struct minix_zone_allocer_t * const zal, uint16_t zone, size_t begin, size_t end, int alloced)
{
	if (!alloced) {
		const size_t count = end - begin;
		const fpos_t pos = zone * MINIX_ZONE_SIZE + begin * sizeof(uint16_t);
		if (fsetpos(zal->f->dev, &pos)) return -1;
		if (fread(zal->zonelist, sizeof(uint16_t), count, zal->f->dev) != count) return -1;
		while (*zal->zonelist) {
			++begin;
			++zal->zonelist;
			++zal->zone0;
			if (!(begin < end)) {
				return 0;
			}
		}
	}

	zal->buf1.zone = zone;
	zal->buf1.begin = zal->zonelist;
	zal->buf1.data = zal->buf1.begin - begin; // Osoite vain ;)

	while (begin < end) {
		if (!(zone = minix_alloc_zone(zal))) {
			zal->buf1.end = zal->zonelist;
			return -1;
		}
		*zal->zonelist = zone;
		++begin;
		++zal->zonelist;
		++zal->zone0;
	}
	zal->buf1.end = zal->zonelist;
	return 0;
}

static int minix_get_indir_zones(struct minix_zone_allocer_t * const zal)
{
	uint16_t zone;
	const size_t zone1 = MIN(zal->zone1, MINIX_INDIR_ZONES_END);

	const size_t begin = zal->zone0 - MINIX_STD_ZONES_END;
	const size_t end = zone1 - MINIX_STD_ZONES_END;

	if (!(zal->zone0 < zone1)) {
		return 0;
	}

	int alloced = 0;
	if (!(zone = zal->inode->u.zones.indir)) {
		if (!(zone = minix_alloc_zone(zal))) {
			return -1;
		}
		zal->inode->u.zones.indir = zone;
		alloced = 1;
	}
	return minix_read_uint16s_from_zone(zal, zone, begin, end, alloced);
}

static int minix_get_dbl_indir_zones_sub(struct minix_zone_allocer_t * const zal, const size_t begin, const size_t end)
{
	int alloced = 0;
	if (!*zal->dbl_indir_list) {
		alloced = 1;
		if (!(*zal->dbl_indir_list = minix_alloc_zone(zal))) {
			goto virhe_etc;
		}
		zal->buf2.end = zal->dbl_indir_list + 1;
	}
	uint16_t zone = *zal->dbl_indir_list;
	if (minix_read_uint16s_from_zone(zal, zone, begin, end, alloced)) {
		goto virhe_etc;
	}
	++zal->dbl_indir_list;

	goto return_etc;

return_etc:
	return 0;
virhe_etc:
	return -1;
}

static int minix_get_dbl_indir_zones(struct minix_zone_allocer_t * const zal)
{
	uint16_t zone;
	int virhe = 0;
	int alloced = 0;

	size_t begin_z = zal->zone0;
	const size_t end_z = MIN(zal->zone1, MINIX_DBL_INDIR_ZONES_END);

	size_t begin_indir = (begin_z - MINIX_INDIR_ZONES_END) / MINIX_ZONES_PER_ZONE;
	const size_t end_indir = 1 + (end_z - 1 - MINIX_INDIR_ZONES_END) / MINIX_ZONES_PER_ZONE;

	const size_t begin_in_1st_indir = (begin_z - MINIX_INDIR_ZONES_END) % MINIX_ZONES_PER_ZONE;
	const size_t end_in_last_indir = (end_z - MINIX_INDIR_ZONES_END) % MINIX_ZONES_PER_ZONE;

	if (!(begin_z < end_z)) {
		goto return_etc;
	}

	if (!(zone = zal->inode->u.zones.dbl_indir)) {
		alloced = 1;
		if (!(zone = minix_alloc_zone(zal))) {
			goto virhe_etc;
		}
		zal->inode->u.zones.dbl_indir = zone;
		zal->buf2.zone = zone;
		zal->buf2.begin = zal->buf2.data;
		zal->dbl_indir_list = zal->buf2.data + begin_indir;
	} else {
		zal->buf2.begin = zal->buf2.data + begin_indir;
		zal->dbl_indir_list = zal->buf2.data + begin_indir;

		const size_t count = end_indir - begin_indir;
		const fpos_t pos = zone * MINIX_ZONE_SIZE + begin_indir * sizeof(uint16_t);
		if (fsetpos(zal->f->dev, &pos)) {
			goto virhe_etc;
		}
		if (fread(zal->dbl_indir_list, sizeof(uint16_t), count, zal->f->dev) != count) {
			goto virhe_etc;
		}
	}

	// Vain yksi zone
	if (end_indir - begin_indir == 1) {
		if (minix_get_dbl_indir_zones_sub(zal, begin_in_1st_indir, end_in_last_indir) != 0) {
			goto virhe_etc;
		}
		goto return_etc;
	}

	// Ensimmäinen
	if (minix_get_dbl_indir_zones_sub(zal, begin_in_1st_indir, MINIX_ZONES_PER_ZONE) != 0) {
		goto virhe_etc;
	}
	++begin_indir;

	// Muut
	while (begin_indir < end_indir - 1) {
		if (minix_get_dbl_indir_zones_sub(zal, begin_in_1st_indir, end_in_last_indir) != 0) {
		}
		++begin_indir;
	}

	// Viimeinen
	if (minix_get_dbl_indir_zones_sub(zal, 0, end_in_last_indir) != 0) {
		goto virhe_etc;
	}
virhe_etc:
	virhe = -1;

return_etc:
	if (alloced && zal->buf2.begin) {
		zal->buf2.end = zal->buf2.begin + MINIX_ZONES_PER_ZONE;
	}
	return virhe;
}

static int minix_zones_flush(struct minix_zone_data_t *data, struct minix_file *f)
{
	if (!data || !f) return -1;
	if (!data->zone || !data->data || !data->begin || !data->end || data->begin == data->end) return 0;

	const size_t bbeg = (data->begin - data->data) * sizeof(uint16_t);
	const size_t count = data->end - data->begin;
	fpos_t pos;
	pos = data->zone * MINIX_ZONE_SIZE + bbeg;
	if (fsetpos(f->dev, &pos)) return -1;
	if (fwrite(data->begin, sizeof(uint16_t), count, f->dev) != count) return -1;
	data->begin = data->end;
	return 0;
}

size_t minix_get_zones(struct minix_file *f, const size_t zone0, const size_t zone1, uint16_t *zonelist, const int write)
{
	// TODO: minix_get_zones: virheenkäsittely, jos flush pieleen...
	struct minix_zone_allocer_t zal_ = {
		.f = f,
		.fs = f->fs,
		.inode = f->inode,
		.zone0 = zone0,
		.zone1 = zone1,
		.zonelist = zonelist,
		.write = write,
		.map = f->fs->zone_map,
		.map_end = f->fs->inode_map,
	}, * const zal = &zal_;

	if (zal->zone1 > zal->fs->super.num_zones) {
		zal->zone1 = zal->fs->super.num_zones;
	}
	if (zone0 >= zone1) return 0;

	// standard zones
	if (minix_get_std_zones(zal) != 0) goto return_etc;

	// indirect zones & flush
	if (minix_get_indir_zones(zal) != 0) goto return_etc;
	if (minix_zones_flush(&zal->buf1, zal->f) != 0) goto return_etc;

	// alloc for dbl_indir-table & double indirect zones
	if (zal->zone1 > MINIX_STD_ZONES_END) {
		zal->buf2.data = CALLOC(MINIX_ZONE_SIZE, 1);
		if (!zal->buf2.data) {
			goto return_etc;
		}
	}
	if (minix_get_dbl_indir_zones(zal) != 0) goto return_etc;

return_etc:
	minix_zones_flush(&zal->buf1, zal->f);
	minix_zones_flush(&zal->buf2, zal->f);
	if (zal->buf2.data) {
		FREE(zal->buf2.data);
	}
	return zal->zonelist - zonelist;
}
