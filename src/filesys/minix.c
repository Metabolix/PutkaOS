#include <filesys/minix.h>

#include <time.h>
#include <string.h>
#include <malloc.h>
#include <debugprint.h>
#include <filesys/mount_err.h>

#define MALLOC kmalloc
#define CALLOC kcalloc
#define FREE kfree
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static int minix_check_filename(const char *filename, int maxlen, int *parts);
static struct minix_file *minix_fopen_all(struct minix_fs *this, const char * filename, uint_t mode, int file_or_dir);
static size_t minix_get_zones(struct minix_file *this, const size_t zone0, const size_t zone1, uint16_t *zonelist, const int write);

const struct fs minix_fs = {
	.name = "minix",
	.fs_mount = (fs_mount_t)  minix_mount,
	.fs_umount = (fs_umount_t) minix_umount,
	.filefunc = {
		.fopen = (fopen_t) minix_fopen,
		.fclose = (fclose_t) minix_fclose,
		.fread = (fread_t) minix_fread,
		.fwrite = (fwrite_t) minix_fwrite,
		.fflush = (fflush_t) minix_fflush,
		.fsetpos = (fsetpos_t) fsetpos_copypos,
		.ioctl = (ioctl_t) minix_ioctl
	},
	.dirfunc = {
		.dmake = (dmake_t) minix_dmake,
		.dopen = (dopen_t) minix_dopen,
		.dclose = (dclose_t) minix_dclose,
		.dread = (dread_t) minix_dread
	}
};

const struct filefunc minix_filefunc_ro = {
	.fopen = (fopen_t) minix_fopen,
	.fclose = (fclose_t) minix_fclose,
	.fread = (fread_t) minix_fread,
	.fwrite = 0,
	.fflush = (fflush_t) fflush_none,
	.fsetpos = (fsetpos_t) fsetpos_copypos,
	.ioctl = (ioctl_t) minix_ioctl
};

const struct filefunc minix_filefunc_wo = {
	.fopen = (fopen_t) minix_fopen,
	.fclose = (fclose_t) minix_fclose,
	.fread = 0,
	.fwrite = (fwrite_t) minix_fwrite,
	.fflush = (fflush_t) minix_fflush,
	.fsetpos = (fsetpos_t) fsetpos_copypos,
	.ioctl = (ioctl_t) minix_ioctl
};

int minix_ioctl(struct minix_file *f, int request, uintptr_t param)
{
	// TODO: minix_ioctl
	return -1;
}

static int minix_check_filename(const char *filename, int maxlen, int *parts)
{
	if (!filename) {
		return -1;
	}
	if (!*filename) {
		if (parts) *parts = 0;
		return 0;
	}
	int i = 0, c = 1;
	while (*filename) {
		if (*filename == '/') {
			++c;
			if (i == 0) {
				return -1;
			}
			i = 0;
		} else {
			++i;
			if (i > maxlen) {
				return -1;
			}
		}
		++filename;
	}
	if (filename[-1] == '/') --c;
	if (parts) *parts = c;
	return 0;
}

struct fs *minix_mount(FILE *device, uint_t mode)
{
	struct minix_fs fs = {
		.std = minix_fs,
		.dev = device,
		.filename_maxlen = 30,
	};
	struct minix_fs *this = &fs;
	fpos_t pos;

	if ((mode & FILE_MODE_WRITE) && !this->dev->func->fwrite) {
		return 0;
	}

	pos = MINIX_ZONE_SIZE;
	if (fsetpos(this->dev, &pos)) {
		return 0;
	}
	if (fread(&this->super, sizeof(this->super), 1, this->dev) != 1) {
		return 0;
	}

	// TODO: Tunnista kunnolla, onko varmasti minix
	if (this->super.magic == MINIX_MAGIC_A) {
		this->filename_maxlen = 14;
	} else if (this->super.magic == MINIX_MAGIC_B) {
		this->filename_maxlen = 30;
	} else {
		return 0;
	}

	// TODO: Jotain muuta tarkistusta
#if 0
	// Kas, UINT16_MAX < MINIX_MAX_ZONES
	if (this->super.num_zones > MINIX_MAX_ZONES) {
		DEBUGF("this->super.num_zones = %d > %d (MINIX_MAX_ZONES)\n",
			this->super.num_zones, MINIX_MAX_ZONES);
		return 0;
	}
#endif
	this->inode_map_size = this->super.num_inode_map_zones * MINIX_ZONE_SIZE;
	this->zone_map_size = this->super.num_zone_map_zones * MINIX_ZONE_SIZE;
	const size_t maps_size = this->inode_map_size + this->zone_map_size;

	this->pos_inode_map = 2 * MINIX_ZONE_SIZE;
	this->pos_zone_map = this->pos_inode_map + this->inode_map_size;
	this->pos_inodes = this->pos_zone_map + this->zone_map_size;
	this->pos_data = this->super.first_data_zone * MINIX_ZONE_SIZE;

	if (!(this = MALLOC(sizeof(struct minix_fs) + maps_size))) goto failure;

	memcpy(this, &fs, sizeof(struct minix_fs));
	this->inode_map = (uint8_t*)(this + 1);
	this->zone_map = this->inode_map + this->inode_map_size;
	this->end_map = this->zone_map + this->zone_map_size;

	if (fsetpos(this->dev, &this->pos_inode_map)) goto failure;
	if (fread(this->inode_map, this->inode_map_size, 1, this->dev) != 1) goto failure;

	if (fsetpos(this->dev, &this->pos_zone_map)) goto failure;
	if (fread(this->zone_map, this->zone_map_size, 1, this->dev) != 1) goto failure;
	list_init(this->open_inodes);

	return (struct fs *)this;
failure:
	if (!this || this == &fs) return 0;
	FREE(this);
	return 0;
}

int minix_umount(struct minix_fs *this)
{
	if (!this) {
		return -1;
	}
	if (this->open_inodes_refcount) {
		return MOUNT_ERR_BUSY;
	}
	if (this->std.filefunc.fwrite) {
		if (this->inode_map_changed) {
			if (fsetpos(this->dev, &this->pos_inode_map)
			|| fwrite(this->inode_map, this->inode_map_size, 1, this->dev) != 1) {
				// TODO: umount: fwrite inode_map failed.
			}
		}

		if (this->inode_map_changed) {
			if (fsetpos(this->dev, &this->pos_zone_map)
			|| fwrite(this->zone_map, this->zone_map_size, 1, this->dev) != 1) {
				// TODO: umount: fwrite zone_map failed.
			}
		}
		fflush(this->dev);
	}
	list_destroy(this->open_inodes);
	FREE(this);
	return 0;
}

int minix_fclose(struct minix_file *stream)
{
	minix_fflush(stream);
	list_item(stream->inode_iter).num_refs--;
	stream->fs->open_inodes_refcount--;
	if (!list_item(stream->inode_iter).num_refs) {
		list_erase(stream->inode_iter);
	}
	if (stream->freefunc) {
		stream->freefunc(stream);
	}
	return 0;
}

int minix_fflush(struct minix_file *stream)
{
	if (!stream->written) {
		return 0;
	}
	fpos_t pos;

	pos = stream->fs->pos_inodes + (stream->inode_n - 1) * MINIX_FS_INODE_SIZE;
	if (fsetpos(stream->dev, &pos)) return EOF;
	if (fwrite(stream->inode, MINIX_FS_INODE_SIZE, 1, stream->dev) != 1) return EOF;

	stream->written = 0;
	return fflush(stream->dev);
}

struct minix_file *minix_fopen_inodenum(struct minix_fs *this, uint16_t inode, struct minix_file *f)
{
	if (!f || !inode) return 0;

	list_iter_of_minix_list_inode iter;
	list_loop(iter, this->open_inodes) {
		if (list_item(iter).inode_n == inode) {
			goto loytyi;
		}
	}
	struct minix_list_inode inoli = {
		.inode_n = inode,
		.num_refs = 0,
	};
	fpos_t pos = this->pos_inodes + (inode - 1) * MINIX_FS_INODE_SIZE;
	if (fsetpos(this->dev, &pos)) return 0;
	if (fread(&inoli.inode, MINIX_FS_INODE_SIZE, 1, this->dev) != 1) return 0;
	list_insert(list_end(this->open_inodes), inoli);
	iter = list_prev(list_end(this->open_inodes));

loytyi:
	this->open_inodes_refcount++;
	list_item(iter).num_refs++;

	f->fs = this;
	f->inode_n = list_item(iter).inode_n;
	f->inode = &list_item(iter).inode;
	f->inode_iter = iter;
	f->dev = f->fs->dev;
	f->dev_zone_map_pos = f->fs->pos_zone_map;
	f->dev_zones_pos = f->fs->pos_data;

	f->pos = f->std.pos = 0;
	f->size = f->std.size = f->inode->size;
	f->std.errno = f->std.eof = 0;
	f->std.func = &this->std.filefunc;
	return f;
}

static uint16_t minix_alloc_inode(struct minix_fs *this, uint16_t type)
{
	// TODO: minix_alloc_inode: Rights, uid, gid
	struct minix_list_inode inoli = {
		.inode = {
			.flags = 0777 | (type << 12),
			.uid = 0,
			.size = 0,
			.modified = time(0),
			.gid = 0,
			.num_refs = 1,
			.u.zones.std = {0},
			.u.zones.indir = 0,
			.u.zones.dbl_indir = 0
		},
		.num_refs = 0,
	};
	uint8_t *ptr;
	int i;
	for (ptr = this->inode_map; ptr != this->zone_map; ++ptr) {
		if (*ptr != 0xff) {
			break;
		}
	}
	if (ptr == this->zone_map) {
		return 0;
	}
	for (i = 0; i < 8; ++i) {
		if ((*ptr & (1 << i)) == 0) {
			break;
		}
	}
	*ptr |= 1 << i;
	this->inode_map_changed = 1;

	inoli.inode_n = 8 * (ptr - this->inode_map) + i;
	list_insert(list_end(this->open_inodes), inoli);
	return inoli.inode_n;
}

static int minix_free_inode(struct minix_fs *this, uint16_t inode)
{
	uint8_t *ptr;
	int i;

	ptr = this->inode_map + inode / 8;
	i = inode & 7;
	*ptr &= *ptr ^ (1 << i);
	return 0;
}

static struct minix_file *minix_fopen_all(struct minix_fs * restrict const this, const char * restrict filename, uint_t mode, int file_or_dir)
{
	struct minix_file f = {
		.freefunc = 0,
		.fs = this,
		.std.func = &this->std.filefunc,
		.inode_n = 0,
	};

	struct minix_direntry direntry;
	int parts, len;
	const char *str, *str_end;
	uint16_t next_inode;

	print(""); // TODO: minix_fopen_all: Miksi tarvitaan? :D
	if (minix_check_filename(filename, this->filename_maxlen, &parts)) {
		return 0;
	}
	if ((mode & FILE_MODE_WRITE) && !this->std.filefunc.fwrite) {
		return 0;
	}
	if ((mode & FILE_MODE_READ) && !this->std.filefunc.fread) {
		return 0;
	}

	if (!minix_fopen_inodenum(this, 1, &f)) return 0;

	str_end = filename - 1;
	while (parts) {
		--parts;
		str = str_end + 1;
		for (str_end = str; *str_end && *str_end != '/'; ++str_end);
		len = str_end - str;

		next_inode = 0;
		while (minix_fread(&direntry, MINIX_DIRENTRY_SIZE, 1, &f) == 1) {
			if (memcmp(direntry.name, str, len) == 0 && direntry.name[len] == 0) {
				next_inode = direntry.inode;
				break;
			}
		}
		if (!next_inode && !parts) {
			goto need_new_file;
		}
		minix_fclose(&f);
		if (!minix_fopen_inodenum(this, next_inode, &f)) return 0;
		continue;
	}
	goto found_it_already;

need_new_file:
	// TODO: minix_fopen_all: need_new_file: Tarkista, että fs on rw.
	if (!(mode & FILE_MODE_WRITE)) {
		minix_fclose(&f);
		return 0;
	}
	memset(&direntry, 0, MINIX_DIRENTRY_SIZE);

	// TODO: minix_fopen_all: need_new_file: Kivat liput...
	uint16_t type_flags;
	if (file_or_dir == MINIX_FLAG_DIR) {
		type_flags = MINIX_FLAG_DIR;
	} else {
		type_flags = MINIX_FLAG_FILE;
	}

	direntry.inode = minix_alloc_inode(this, type_flags);
	if (!direntry.inode) {
		minix_fclose(&f);
		return 0;
	}
	memcpy(direntry.name, str, str_end - str);
	if (minix_fwrite(&direntry, MINIX_DIRENTRY_SIZE, 1, &f) != 1) {
		minix_free_inode(this, direntry.inode);
		minix_fclose(&f);
		return 0;
	}
	minix_fclose(&f);

	if (!minix_fopen_inodenum(this, direntry.inode, &f)) {
		minix_free_inode(this, direntry.inode);
		return 0;
	}

found_it_already:
	switch (file_or_dir) {
		case MINIX_FLAG_FILE:
			if (MINIX_IS(f.inode->flags, DIR)) {
				minix_fclose(&f);
				return 0;
			}
			break;
		case MINIX_FLAG_DIR:
			if (!MINIX_IS(f.inode->flags, DIR)) {
				minix_fclose(&f);
				return 0;
			}
			break;
		default:
			break;
	}

	if (!(mode & FILE_MODE_WRITE)) {
		f.std.func = &minix_filefunc_ro;
	} else if (!(mode & FILE_MODE_READ)) {
		f.std.func = &minix_filefunc_wo;
	}
	if ((mode & FILE_MODE_APPEND)) {
		f.std.pos = f.std.size;
	}
	if ((mode & FILE_MODE_CLEAR)) {
		// TODO: minix_fopen_all: FILE_MODE_CLEAR
	}
	struct minix_file *retval;
	if ((retval = MALLOC(sizeof(struct minix_file)))) {
		memcpy(retval, &f, sizeof(struct minix_file));
		retval->freefunc = FREE;
	}
	return retval;
}

struct minix_file *minix_fopen(struct minix_fs *this, const char * filename, uint_t mode)
{
	return minix_fopen_all(this, filename, mode, 0);
}

size_t minix_freadwrite(char *buf, size_t size, size_t count, struct minix_file *stream, fread_t ffunction, int write)
{
	const size_t  pos0        = stream->pos = stream->std.pos;
	const fpos_t  pos_1a      = pos0 + (uint64_t)size * count;
	const size_t  pos_1b      = write ? stream->fs->super.max_size : stream->size;
	const size_t  pos1        = MIN(pos_1a, pos_1b);
	const size_t  byte_count  = pos1 - pos0;
	const size_t  zone0       = pos0 / MINIX_ZONE_SIZE;
	const size_t  zone1       = 1 + (pos1 - 1) / MINIX_ZONE_SIZE;
	const size_t  numzones_c  = zone1 - zone0;
	fpos_t pos;
	size_t zone;
	uint16_t *zonelist = CALLOC(sizeof(uint16_t), numzones_c);
	size_t done_size, pos_in_zone;
	size_t numzones = minix_get_zones(stream, zone0, zone1, zonelist, write);

	if (!numzones) {
		goto return_etc;
	}

	pos_in_zone = stream->pos % MINIX_ZONE_SIZE;
	if (numzones == 1) {
		pos = zonelist[0] * MINIX_ZONE_SIZE + pos_in_zone;
		if (fsetpos(stream->dev, &pos)) {
			goto return_etc;
		}
		stream->pos += ffunction(buf, 1, byte_count, stream->dev);

		goto return_etc;
	}

	zone = 0;

	pos_in_zone = stream->pos % MINIX_ZONE_SIZE;
	if (pos_in_zone) {
		pos = zonelist[zone] * MINIX_ZONE_SIZE + pos_in_zone;
		if (fsetpos(stream->dev, &pos)) {
			goto return_etc;
		}
		done_size = ffunction(buf, 1, MINIX_ZONE_SIZE - pos_in_zone, stream->dev);
		buf += done_size;
		stream->pos += done_size;
		pos_in_zone += done_size;
		if (pos_in_zone != MINIX_ZONE_SIZE) {
			goto return_etc;
		}

		++zone;
	}

	for (; zone < numzones - 1; ++zone) {
		pos = zonelist[zone] * MINIX_ZONE_SIZE;
		if (fsetpos(stream->dev, &pos)) {
			goto return_etc;
		}

		done_size = ffunction(buf, 1, MINIX_ZONE_SIZE, stream->dev);
		buf += done_size;
		stream->pos += done_size;
		if (done_size != MINIX_ZONE_SIZE) {
			goto return_etc;
		}
	}

	pos = zonelist[zone] * MINIX_ZONE_SIZE;
	if (fsetpos(stream->dev, &pos)) {
		goto return_etc;
	}
	stream->pos += ffunction(buf, 1, pos1 - stream->pos, stream->dev);

return_etc:
	stream->std.pos = stream->pos;
	if (write && stream->pos > stream->size) {
		stream->size = stream->pos;
		stream->std.size = stream->size;
		stream->inode->size = stream->size;
	}
	FREE(zonelist);
	return (stream->pos - pos0) / size;
}

size_t minix_fread(void *buf, size_t size, size_t count, struct minix_file *stream)
{
	return minix_freadwrite((char *)buf, size, count, stream, (fread_t) fread, 0);
}

size_t minix_fwrite(const void *buf, size_t size, size_t count, struct minix_file *stream)
{
	stream->written = 1;
	return minix_freadwrite((char *)buf, size, count, stream, (fread_t) fwrite, 1);
}

int minix_dmake(struct minix_fs *this, const char * dirname, uint_t owner, uint_t rights)
{
	// TODO: minix_dmake: virheiden hoitelu...
        struct minix_file *file;
        file = minix_fopen_all(this, dirname, FILE_MODE_READ, MINIX_FLAG_DIR);
        if (file) {
		minix_fclose(file);
		return DIR_ERR_EXISTS;
	}
        file = minix_fopen_all(this, dirname, FILE_MODE_RW, MINIX_FLAG_DIR);
	if (!file) {
		return DIR_ERR_CANT_MAKE;
	}

	struct minix_direntry direntry = {
		.inode = file->inode_n
	};
	direntry.name[0] = '.';
	if (minix_fwrite(&direntry, sizeof(direntry), 1, file) != 1) {
		minix_fclose(file);
		return DIR_ERR_CANT_WRITE;
	}
	++file->inode->num_refs;
	direntry.name[1] = '.';
	if (minix_fwrite(&direntry, sizeof(direntry), 1, file) != 1) {
		minix_fclose(file);
		return DIR_ERR_CANT_WRITE;
	}
	++file->inode->num_refs;

	minix_fclose(file);
	return 0;
}

struct minix_dir *minix_dopen(struct minix_fs *this, const char * dirname)
{
	struct minix_file *file;
	struct minix_dir *retval;

	file = minix_fopen_all(this, dirname, FILE_MODE_READ, MINIX_FLAG_DIR);
	if (!file) return 0;

	if ((retval = CALLOC(sizeof(struct minix_dir), 1))) {
		retval->file = file;
		retval->std.func = &file->fs->std.dirfunc;
	}
	return retval;
}

int minix_dread(struct minix_dir *listing)
{
	do if (minix_fread(&listing->direntry, MINIX_DIRENTRY_SIZE, 1, listing->file) != 1) {
		return EOF;
	} while (!listing->direntry.real.inode);

	fpos_t pos = listing->file->fs->pos_inodes + (listing->direntry.real.inode - 1) * MINIX_FS_INODE_SIZE;
	if (fsetpos(listing->file->fs->dev, &pos)) return EOF;
	if (fread(&listing->inode, MINIX_FS_INODE_SIZE, 1, listing->file->fs->dev) != 1) return EOF;

	listing->std = (DIR) {
		.name = listing->direntry.real.name,
		.size = listing->inode.size,
		.uid = listing->inode.uid,
		.gid = listing->inode.gid,
		.rights = MINIX_RIGHTS(listing->inode.flags),
		.created = listing->inode.modified,
		.accessed = listing->inode.modified,
		.modified = listing->inode.modified,
		.references = listing->inode.num_refs,
		.func = &minix_fs.dirfunc
	};
	return 0;
}

int minix_dclose(struct minix_dir *listing)
{
	int i;
	i = minix_fclose(listing->file);
	if (i) return i;
	FREE(listing);
	return 0;
}

/************************
* MINIX ZONE FUNCTIONS **
************************/

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

static uint16_t minix_alloc_zone(struct minix_zone_allocer_t * const zal)
{
	if (!zal->write) {
		zal->eof = 1;
		return 0;
	}
	while (zal->map != zal->map_end) {
		if (*zal->map != 0xff) for (; zal->map_bit < 8; ++zal->map_bit) {
			if ((*zal->map & (1 << zal->map_bit)) == 0) {
				*zal->map |= 1 << zal->map_bit;
				zal->fs->zone_map_changed = 1;
				return ((zal->map_pos << 3) + zal->map_bit) + zal->fs->super.first_data_zone - 1;
			}
		}
		++zal->map;
		++zal->map_pos;
		zal->map_bit = 0;
	}
	zal->eof = 1;
	return 0;
}

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

int minix_get_dbl_indir_zones_sub(struct minix_zone_allocer_t * const zal, const size_t begin, const size_t end)
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

		// TODO: Luetaan se zone
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

static size_t minix_get_zones(struct minix_file *this, const size_t zone0, const size_t zone1, uint16_t *zonelist, const int write)
{
	// TODO: Virheenkäsittely, jos flush pieleen...
	struct minix_zone_allocer_t zal_ = {
		.f = this,
		.fs = this->fs,
		.inode = this->inode,
		.zone0 = zone0,
		.zone1 = zone1,
		.zonelist = zonelist,
		.write = write,
		.map = this->fs->zone_map,
		.map_end = this->fs->inode_map,
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
