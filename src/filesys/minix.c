#include <filesys/minix.h>

#include <time.h>
#include <string.h>
#include <malloc.h>
#include <debugprint.h>
#include <filesys/mount_err.h>

#define def_ceil(name, type) static type name (type x, type bound) { return (x % bound) ? (x + bound - x % bound) : x; }
def_ceil(ceil_uint32, uint32_t)

static int minix_check_filename(const char *filename, int maxlen, int *parts);
static struct minix_file *minix_fopen_all(struct minix_fs *this, const char * filename, uint_t mode, _1x2_t file_or_dir);
static size_t minix_get_zones(struct minix_file *this, const size_t zone0, const size_t zone1, uint16_t *zonelist, const int write);
static size_t minix_alloc_zones(struct minix_file *this, const size_t zone0, const size_t zone2, uint16_t *zonelist);

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
	int i, c;
	c = 1;
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
	struct minix_fs *this;
	fpos_t pos;

	if ((mode & FILE_MODE_WRITE) && !fs.dev->func->fwrite) {
		return 0;
	}

	pos = MINIX_FS_ZONE_SIZE;
	if (fsetpos(fs.dev, &pos)) {
		return 0;
	}
	if (fread(&fs.super, sizeof(fs.super), 1, fs.dev) != 1) {
		return 0;
	}

	// TODO: Tunnista se kunnolla!
	if (fs.super.magic == MINIX_FS_MAGIC_A) {
		fs.filename_maxlen = 14;
	} else if (fs.super.magic == MINIX_FS_MAGIC_B) {
		fs.filename_maxlen = 30;
	} else {
		return 0;
	}

	fs.pos_inode_map = 2 * MINIX_FS_ZONE_SIZE;
	fs.pos_zone_map = fs.pos_inode_map + fs.super.num_inode_map_zones * MINIX_FS_ZONE_SIZE;
	fs.pos_inodes = fs.pos_zone_map + fs.super.num_zone_map_zones * MINIX_FS_ZONE_SIZE;
	fs.pos_data = fs.pos_inodes + ceil_uint32((uint32_t)fs.super.num_inodes * sizeof(struct minix_inode), MINIX_FS_ZONE_SIZE);

	const size_t inode_map_size = fs.super.num_inodes / 8 + 1;
	const size_t zone_map_size = fs.super.num_blocks / 8 + 1;
	const size_t maps_size = inode_map_size + zone_map_size;
	//fs.pos_inodes - fs.pos_inode_map;
	if (!(this = kmalloc(sizeof(struct minix_fs) + maps_size))) goto failure;

	memcpy(this, &fs, sizeof(struct minix_fs));
	this->inode_map = (uint8_t*)(this + 1);
	this->zone_map = this->inode_map + inode_map_size;
	this->end_map = this->zone_map + zone_map_size;

	if (fsetpos(fs.dev, &fs.pos_inode_map)) goto failure;
	if (fread(this->inode_map, inode_map_size, 1, fs.dev) != 1) goto failure;

	if (fsetpos(fs.dev, &fs.pos_zone_map)) goto failure;
	if (fread(this->zone_map, zone_map_size, 1, fs.dev) != 1) goto failure;
	list_init(this->open_inodes);

	return (struct fs *)this;
failure:
	if (!this) return 0;
	kfree(this);
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
		fflush(this->dev);
	}
	list_destroy(this->open_inodes);
	kfree(this);
	return 0;
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
	fpos_t pos = this->pos_inodes + (inode - 1) * MINIS_FS_INODE_SIZE;
	if (fsetpos(this->dev, &pos)) return 0;
	if (fread(&inoli.inode, MINIS_FS_INODE_SIZE, 1, this->dev) != 1) return 0;
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
	// TODO: Rights, uid, gid, modified, num_refs
	struct minix_list_inode inoli = {
		.inode = {
			.flags = 0777 | (type << 12),
			.uid = 0,
			.size = 0,
			.modified = 0,
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

static struct minix_file *minix_fopen_all(struct minix_fs * restrict const this, const char * restrict filename, uint_t mode, _1x2_t file_or_dir)
{
	struct minix_file f = {
		.freefunc = 0,
		.fs = this,
		.std.func = &this->std.filefunc,
		.inode_n = 0,
	};

	struct minix_direntry direntry;
	int parts;
	const char *str, *str_end;
	uint16_t next_inode;

	print(""); // TODO: Miksi hemmetissä? :D
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

		next_inode = 0;
		while (minix_fread(&direntry, MINIX_FS_DIRENTRY_SIZE, 1, &f) == 1) {
			if (memcmp(direntry.name, str, str_end - str) == 0) {
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
	if (!(mode & FILE_MODE_WRITE)) {
		minix_fclose(&f);
		return 0;
	}
	memset(&direntry, 0, MINIX_FS_DIRENTRY_SIZE);

	// TODO: Nice flags...
	uint16_t type_flags;
	if (file_or_dir == _1x2__2) {type_flags = MINIX_FS_FLAG_DIR;}
	else {type_flags = MINIX_FS_FLAG_FILE;}

	direntry.inode = minix_alloc_inode(this, type_flags);
	if (!direntry.inode) {
		minix_fclose(&f);
		return 0;
	}
	memcpy(direntry.name, str, str_end - str);
	if (minix_fwrite(&direntry, MINIX_FS_DIRENTRY_SIZE, 1, &f) != 1) {
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
		case _1x2__1:
			if (MINIX_FS_IS(f.inode->flags, DIR)) {
				minix_fclose(&f);
				return 0;
			}
			break;
		case _1x2__2:
			if (!MINIX_FS_IS(f.inode->flags, DIR)) {
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
		// TODO
	}
	struct minix_file *retval;
	if ((retval = kmalloc(sizeof(struct minix_file)))) {
		memcpy(retval, &f, sizeof(struct minix_file));
		retval->freefunc = kfree;
	}
	return retval;
}

struct minix_file *minix_fopen(struct minix_fs *this, const char * filename, uint_t mode)
{
	return minix_fopen_all(this, filename, mode, 0);
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

static size_t minix_get_zones(struct minix_file *this, const size_t zone0, const size_t zone1, uint16_t *zonelist, const int write)
{
#define INSERT_ZONE(z, val) {if (!((z < zone0) || (z > zone1))) zonelist[z - zone0] = val;}
	size_t j, k;
	size_t zone;
	fpos_t pos;
	uint16_t indir[512], dbl_indir[512];
	size_t got_zones = 0;

	const size_t end_a = (zone1 < MINIX_STD_ZONES) ? zone1 : MINIX_STD_ZONES;
	const size_t end_b = (zone1 < MINIX_STD_ZONES + MINIX_INDIR_ZONES) ? zone1 : (MINIX_STD_ZONES + MINIX_INDIR_ZONES);
	const size_t end_c = zone1;

	zone = zone0;

	if (zone > end_b) goto read_dbl_indir;
	if (zone > end_a) goto read_indir;

	for (; zone < end_a; ++zone) {
		INSERT_ZONE(zone, this->inode->u.zones.std[zone]);
		if (!this->inode->u.zones.std[zone]) {
			goto alloc_rest;
		}
		++got_zones;
	}
	if (zone >= zone1) {
		goto return_etc;
	}

read_indir:
	{
		const size_t count_b = end_b - zone;
		if (!this->inode->u.zones.indir) {
			goto alloc_rest;
		}

		pos = this->inode->u.zones.indir * MINIX_FS_ZONE_SIZE;
		//pos -= MINIX_FS_ZONE_SIZE;
		pos += zone * sizeof(uint16_t);
		if (fsetpos(this->dev, &pos)) {
			goto read_error;
		}
		if (fread(indir, sizeof(uint16_t), count_b, this->dev) != count_b) {
			goto read_error;
		}

		for (j = 0; zone < end_b; ++zone, ++j) {
			INSERT_ZONE(zone, indir[j]);
			if (!indir[j]) {
				goto alloc_rest;
			}
			++got_zones;
		}
		if (zone >= zone1) {
			goto return_etc;
		}
	}

read_dbl_indir:
	{
		if (!this->inode->u.zones.dbl_indir) {
			goto alloc_rest;
		}
		pos = this->inode->u.zones.dbl_indir * MINIX_FS_ZONE_SIZE;
		//pos -= MINIX_FS_ZONE_SIZE;
		if (fsetpos(this->dev, &pos)) {
			goto read_error;
		}
		if (fread(dbl_indir, sizeof(uint16_t), 512, this->dev) != 512) {
			goto read_error;
		}

		while (zone < end_c) {
			k = zone - 6 - 512;

			pos = dbl_indir[k / 512] * MINIX_FS_ZONE_SIZE;
			//pos -= MINIX_FS_ZONE_SIZE;
			if (fsetpos(this->dev, &pos)) {
				goto read_error;
			}
			if (fread(indir, sizeof(uint16_t), 512, this->dev) != 512) {
				goto read_error;
			}

			for (j = k % 512; j < 512 && zone < end_c; ++zone, ++j) {
				INSERT_ZONE(zone, indir[j]);
				if (!indir[j]) {
					goto alloc_rest;
				}
				++got_zones;
			}
		}
		if (zone >= zone1) {
			goto return_etc;
		}
	}
alloc_rest:
	if (!write) {
		// TODO: Corrupted inode (size > zonelist)
		goto return_etc;
	}
	got_zones += minix_alloc_zones(this, zone, zone1, zonelist + zone);
read_error:
return_etc:
	return got_zones;
}

enum minix_alloc_state_t {
	MINIX_ALLOC_STD,
	MINIX_ALLOC_INDIR_LIST,
	MINIX_ALLOC_INDIR,
	MINIX_ALLOC_DBL_INDIR_LIST_LIST,
	MINIX_ALLOC_DBL_INDIR_LIST,
	MINIX_ALLOC_DBL_INDIR,
};
struct minix_alloc_zones_t {
	int state;
	size_t zone0, zone1;

	uint16_t *indir_list;
	uint16_t *dbl_indir_list, *dbl_indir_ptr;

	size_t indir_begin, indir_end;

	uint16_t *zonelist;
	struct minix_inode *inode;
	struct minix_file *f;
};

static int minix_handle_alloced_zone(struct minix_alloc_zones_t * const info, uint16_t zone)
{
	fpos_t pos;
	uint16_t write_zone;
	size_t write_begin, write_end, write_len = 0;
	void *write_buf;

	switch (info->state) {
	case MINIX_ALLOC_STD:
		info->inode->u.zones.std[info->zone0] = *info->zonelist = zone;
		++info->zonelist;
		++info->zone0;
		if (info->zone0 >= MINIX_STD_ZONES) {
			info->state = MINIX_ALLOC_INDIR_LIST;
		}
		return 0;

	case MINIX_ALLOC_INDIR_LIST:
		info->inode->u.zones.indir = zone;
		info->indir_list = info->zonelist;
		info->indir_begin =
		info->indir_end = info->zone0 - MINIX_STD_ZONES;
		info->state = MINIX_ALLOC_INDIR;

		write_zone = zone;
		write_begin = 0;
		write_len = MINIX_INDIR_ZONES * sizeof(uint16_t);
		write_buf = 0;
		goto flush_disk;

	case MINIX_ALLOC_INDIR:
		*info->zonelist = zone;
		++info->zonelist;
		++info->zone0;
		++info->indir_end;
		if (info->indir_end == MINIX_INDIR_ZONES || info->zone0 == info->zone1) {
			info->state = MINIX_ALLOC_DBL_INDIR_LIST_LIST;

			write_zone = info->inode->u.zones.indir;
			write_begin = info->indir_begin * sizeof(uint16_t);
			write_end = info->indir_end * sizeof(uint16_t);
			write_buf = info->indir_list;

			goto flush_disk;
		}
		return 0;

	case MINIX_ALLOC_DBL_INDIR_LIST_LIST:
		info->inode->u.zones.dbl_indir = zone;
		info->dbl_indir_ptr =
		info->dbl_indir_list = kcalloc(MINIX_INDIR_ZONES, sizeof(uint16_t));
		info->state = MINIX_ALLOC_DBL_INDIR_LIST;
		return 0;

	case MINIX_ALLOC_DBL_INDIR_LIST:
		if (info->dbl_indir_ptr - info->dbl_indir_list >= MINIX_INDIR_ZONES) {
			return -1;
		}
		*info->dbl_indir_ptr = zone;
		++info->dbl_indir_ptr;
		info->indir_list = info->zonelist;
		info->indir_begin =
		info->indir_end = (info->zone0 - MINIX_STD_ZONES - MINIX_INDIR_ZONES) % MINIX_INDIR_ZONES;
		info->state = MINIX_ALLOC_INDIR;
		return 0;

	case MINIX_ALLOC_DBL_INDIR:
		*info->zonelist = zone;
		++info->zonelist;
		++info->zone0;
		++info->indir_end;
		if (info->indir_end == MINIX_INDIR_ZONES || info->zone0 == info->zone1) {
			info->state = MINIX_ALLOC_DBL_INDIR_LIST;

			write_zone = info->inode->u.zones.indir;
			write_begin = info->indir_begin * sizeof(uint16_t);
			write_end = info->indir_end * sizeof(uint16_t);
			write_buf = info->indir_list;

			goto flush_disk;
		}
		return 0;
	}
flush_disk:
	if (!write_len) {
		write_len = write_end - write_begin;
	}
	pos = write_zone * MINIX_FS_ZONE_SIZE + write_begin;
	if (fsetpos(info->f->dev, &pos)) {
		return -1;
	}
	if (write_buf) {
		if (fwrite(write_buf, 1, write_len, info->f->dev) != write_len) {
			return -1;
		}
	} else {
		while (write_len--) if (fwrite(&write_buf, 1, 1, info->f->dev) != 1) {
			return -1;
		}
	}
	return 0;
}

static size_t minix_alloc_zones(struct minix_file *this, const size_t zone0, const size_t zone1, uint16_t *zonelist)
{
	struct minix_alloc_zones_t info = {
		//.state = 0,
		.zone0 = zone0,
		.zone1 = zone1,
		.zonelist = zonelist,
		.inode = this->inode,
		.f = this,
//		uint16_t *indir_list, *dbl_indir_list, *dbl_indir_ptr;
//		size_t indir_begin, indir_end;
	};

	if (zone0 < MINIX_STD_ZONES) {
		info.state = MINIX_ALLOC_STD;
	} else if (zone0 < MINIX_STD_ZONES + MINIX_INDIR_ZONES) {
		if (info.inode->u.zones.indir) {
			info.indir_list = info.zonelist;
			info.indir_begin =
			info.indir_end = zone0 - MINIX_STD_ZONES;
			info.state = MINIX_ALLOC_INDIR;
		} else {
			info.state = MINIX_ALLOC_INDIR_LIST;
		}
	} else {
		if (info.inode->u.zones.dbl_indir) {
			info.dbl_indir_list = kcalloc(MINIX_INDIR_ZONES, sizeof(uint16_t));
			// TODO: Lue se dbl_indir-sektori! (ja myöhemmin kirjoita)
			info.dbl_indir_ptr = info.dbl_indir_list + (zone0 - (MINIX_STD_ZONES + MINIX_INDIR_ZONES)) / MINIX_INDIR_ZONES;
			if (*info.dbl_indir_ptr) {
				info.indir_list = info.zonelist;
				info.indir_begin =
				info.indir_end = (zone0 - MINIX_STD_ZONES - MINIX_INDIR_ZONES) % MINIX_INDIR_ZONES;
				info.state = MINIX_ALLOC_DBL_INDIR;
			} else {
				info.state = MINIX_ALLOC_DBL_INDIR_LIST;
			}
		} else {
			info.state = MINIX_ALLOC_DBL_INDIR_LIST_LIST;
		}
	}

	size_t count = 0;
	uint8_t * const zone_map = this->fs->zone_map, * const end_map = this->fs->end_map;
	uint8_t *ptr = zone_map;
	int i = 0;
	while (info.zone0 != info.zone1 && ptr != end_map) {
		if (*ptr == 0xff) {
			++ptr;
			i = 0;
			continue;
		}
		for (; *ptr & (1 << i); ++i);
		*ptr |= 1 << i;
		if (minix_handle_alloced_zone(&info, 8 * (ptr - zone_map) + i)) {
			break;
		}
		++i;
		++count;
	}

	if (info.dbl_indir_list) {
		if (info.dbl_indir_list != info.dbl_indir_ptr) {
			// TODO: Kirjoita se lista! (Ja aiemmin lue)
		}
		kfree(info.dbl_indir_list);
	}
	return count;
}

size_t minix_freadwrite(char *buf, size_t size, size_t count, struct minix_file *stream, fread_t ffunction, int write)
{
	const size_t  pos0        = stream->pos;
	const fpos_t  pos_1a      = pos0 + (uint64_t)size * count;
	const size_t  pos_1b      = write ? stream->fs->super.max_size : stream->size;
	const size_t  pos1        = pos_1a < pos_1b ? pos_1a : pos_1b;
	const size_t  byte_count  = pos1 - pos0;
	const size_t  zone0       = pos0 / MINIX_FS_ZONE_SIZE;
	const size_t  zone1       = 1 + (pos1 - 1) / MINIX_FS_ZONE_SIZE;
	const size_t  numzones_c  = zone1 - zone0;
	fpos_t pos;
	size_t zone;
	uint16_t *zonelist = kcalloc(sizeof(uint16_t), numzones_c);
	size_t done_size, pos_in_zone;
	size_t numzones = minix_get_zones(stream, zone0, zone1, zonelist, write);

	pos_in_zone = stream->pos % MINIX_FS_ZONE_SIZE;
	if (numzones == 1) {
		pos = zonelist[0] * MINIX_FS_ZONE_SIZE + pos_in_zone;
		if (fsetpos(stream->dev, &pos)) {
			goto return_etc;
		}
		stream->pos += ffunction(buf, 1, byte_count, stream->dev);

		goto return_etc;
	}

	zone = 0;

	pos_in_zone = stream->pos % MINIX_FS_ZONE_SIZE;
	if (pos_in_zone) {
		pos = zonelist[zone] * MINIX_FS_ZONE_SIZE + pos_in_zone;
		if (fsetpos(stream->dev, &pos)) {
			goto return_etc;
		}
		done_size = ffunction(buf, 1, MINIX_FS_ZONE_SIZE - pos_in_zone, stream->dev);
		buf += done_size;
		stream->pos += done_size;
		pos_in_zone += done_size;
		if (pos_in_zone != MINIX_FS_ZONE_SIZE) {
			goto return_etc;
		}

		++zone;
	}

	for (; zone < numzones - 1; ++zone) {
		pos = zonelist[zone] * MINIX_FS_ZONE_SIZE;
		if (fsetpos(stream->dev, &pos)) {
			goto return_etc;
		}

		done_size = ffunction(buf, 1, MINIX_FS_ZONE_SIZE, stream->dev);
		buf += done_size;
		stream->pos += done_size;
		if (done_size != MINIX_FS_ZONE_SIZE) {
			goto return_etc;
		}
	}

	pos = zonelist[zone] * MINIX_FS_ZONE_SIZE;
	if (fsetpos(stream->dev, &pos)) {
		goto return_etc;
	}
	stream->pos += ffunction(buf, 1, pos1 - stream->pos, stream->dev);

return_etc:
	stream->std.pos = stream->pos;
	if (write && stream->pos > stream->size) {
		stream->size = stream->pos;
		stream->std.size = stream->size;
	}
	kfree(zonelist);
	return (stream->pos - pos0) / size;
}

size_t minix_fread(void *buf, size_t size, size_t count, struct minix_file *stream)
{
	return minix_freadwrite((char *)buf, size, count, stream, (fread_t) fread, 0);
}

size_t minix_fwrite(const void *buf, size_t size, size_t count, struct minix_file *stream)
{
	return minix_freadwrite((char *)buf, size, count, stream, (fread_t) fwrite, 1);
}

int minix_fflush(struct minix_file *stream)
{
	fpos_t pos;

	pos = stream->fs->pos_inodes + (stream->inode_n - 1) * MINIS_FS_INODE_SIZE;
	if (fsetpos(stream->dev, &pos)) return EOF;
	if (fwrite(stream->inode, MINIS_FS_INODE_SIZE, 1, stream->dev) != 1) return EOF;

	return fflush(stream->dev);
}

int minix_dmake(struct minix_fs *this, const char * dirname, uint_t owner, uint_t rights)
{
	// TODO
	return EOF;
}

struct minix_dir *minix_dopen(struct minix_fs *this, const char * dirname)
{
	struct minix_file *file;
	struct minix_dir *retval;

	file = minix_fopen_all(this, dirname, FILE_MODE_READ, _1x2__2);
	if (!file) return 0;

	if ((retval = kcalloc(sizeof(struct minix_dir), 1))) {
		retval->file = file;
		retval->std.func = &file->fs->std.dirfunc;
	}
	return retval;
}

int minix_dread(struct minix_dir *listing)
{
	do if (minix_fread(&listing->direntry, MINIX_FS_DIRENTRY_SIZE, 1, listing->file) != 1) {
		return EOF;
	} while (!listing->direntry.real.inode);

	fpos_t pos = listing->file->fs->pos_inodes + (listing->direntry.real.inode - 1) * MINIS_FS_INODE_SIZE;
	if (fsetpos(listing->file->fs->dev, &pos)) return EOF;
	if (fread(&listing->inode, MINIS_FS_INODE_SIZE, 1, listing->file->fs->dev) != 1) return EOF;

	listing->std = (DIR) {
		.name = listing->direntry.real.name,
		.size = listing->inode.size,
		.uid = listing->inode.uid,
		.gid = listing->inode.gid,
		.rights = MINIX_FS_RIGHTS(listing->inode.flags),
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
	kfree(listing);
	return 0;
}
