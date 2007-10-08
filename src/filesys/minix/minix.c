#include <filesys/minix/minix.h>
#include <filesys/minix/zones.h>
#include <filesys/minix/fileutils.h>
#include <filesys/minix/maps.h>

#include <time.h>
#include <string.h>
#include <malloc.h>
#include <debugprint.h>
#include <filesys/mount_err.h>

#define MALLOC kmalloc
#define CALLOC kcalloc
#define FREE kfree
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static struct minix_file *minix_fopen_inodenum(struct minix_fs *fs, uint16_t inode, struct minix_file *f);
static uint16_t minix_locate_inode(struct minix_fs *fs, const char *name, const char *name_end, uint16_t inode);
static int minix_insert_direntry(struct minix_fs *fs, uint16_t dir, uint16_t file, const char *name);
static struct minix_file *minix_fopen_all(struct minix_fs * restrict const fs, const char * restrict filename, uint_t mode, uint_t type, struct minix_file *f);
static struct minix_dir *minix_dopen_inodenum(struct minix_fs *fs, uint16_t inode_n, struct minix_dir *d);
static int minix_dread_only_dir(struct minix_dir *listing);

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
	},
	.fileutils = {
		.link = (link_t) minix_link,
		.symlink = (link_t) minix_symlink,
		.unlink = (unlink_t) minix_unlink,
		.getprops = (getprops_t) minix_getprops,
		.setprops = (setprops_t) minix_setprops
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

static int minix_fsetpos(struct minix_file *f, const fpos_t *pos)
{
	return fsetpos_copypos(&f->std, pos);
}

int minix_ioctl(struct minix_file *f, int request, uintptr_t param)
{
	// TODO: minix_ioctl
	return -1;
}

struct fs *minix_mount(FILE *device, uint_t mode)
{
	struct minix_fs _fs = {
		.std = minix_fs,
		.dev = device,
		.filename_maxlen = 30,
	};
	struct minix_fs *fs = &_fs;
	fpos_t pos;

	if ((mode & FILE_MODE_WRITE) && !fs->dev->func->fwrite) {
		return 0;
	}

	pos = MINIX_ZONE_SIZE;
	if (fsetpos(fs->dev, &pos)) {
		return 0;
	}
	if (fread(&fs->super, sizeof(fs->super), 1, fs->dev) != 1) {
		return 0;
	}

	// TODO: Tunnista kunnolla, onko ja mikä, v1/v2, muita tarkistuksia?
	if (fs->super.magic == MINIX_MAGIC_A) {
		fs->filename_maxlen = 14;
	} else if (fs->super.magic == MINIX_MAGIC_B) {
		fs->filename_maxlen = 30;
	} else {
		return 0;
	}

#if 0
	// Kas, UINT16_MAX < MINIX_MAX_ZONES
	if (fs->super.num_zones > MINIX_MAX_ZONES) {
		DEBUGF("fs->super.num_zones = %d > %d (MINIX_MAX_ZONES)\n",
			fs->super.num_zones, MINIX_MAX_ZONES);
		return 0;
	}
#endif
	fs->inode_map_size = fs->super.num_inode_map_zones * MINIX_ZONE_SIZE;
	fs->zone_map_size = fs->super.num_zone_map_zones * MINIX_ZONE_SIZE;
	const size_t maps_size = fs->inode_map_size + fs->zone_map_size;

	fs->pos_inode_map = 2 * MINIX_ZONE_SIZE;
	fs->pos_zone_map = fs->pos_inode_map + fs->inode_map_size;
	fs->pos_inodes = fs->pos_zone_map + fs->zone_map_size;
	fs->pos_data = fs->super.first_data_zone * MINIX_ZONE_SIZE;

	if (!(fs = MALLOC(sizeof(struct minix_fs) + maps_size))) goto failure;

	memcpy(fs, &_fs, sizeof(struct minix_fs));
	fs->inode_map = (uint8_t*)(fs + 1);
	fs->zone_map = fs->inode_map + fs->inode_map_size;
	fs->end_map = fs->zone_map + fs->zone_map_size;

	if (fsetpos(fs->dev, &fs->pos_inode_map)) goto failure;
	if (fread(fs->inode_map, fs->inode_map_size, 1, fs->dev) != 1) goto failure;

	if (fsetpos(fs->dev, &fs->pos_zone_map)) goto failure;
	if (fread(fs->zone_map, fs->zone_map_size, 1, fs->dev) != 1) goto failure;
	list_init(fs->open_inodes);

	return (struct fs *)fs;
failure:
	if (!fs || fs == &_fs) return 0;
	FREE(fs);
	return 0;
}

int minix_umount(struct minix_fs *fs)
{
	if (!fs) {
		return -1;
	}
	if (fs->open_inodes_refcount) {
		return MOUNT_ERR_BUSY;
	}
	if (fs->std.filefunc.fwrite) {
		if (fs->inode_map_changed) {
			if (fsetpos(fs->dev, &fs->pos_inode_map)
			|| fwrite(fs->inode_map, fs->inode_map_size, 1, fs->dev) != 1) {
				// TODO: umount: fwrite inode_map failed.
			}
		}

		if (fs->inode_map_changed) {
			if (fsetpos(fs->dev, &fs->pos_zone_map)
			|| fwrite(fs->zone_map, fs->zone_map_size, 1, fs->dev) != 1) {
				// TODO: umount: fwrite zone_map failed.
			}
		}
		fflush(fs->dev);
	}
	list_destroy(fs->open_inodes);
	FREE(fs);
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
	if (stream->alloced) {
		FREE(stream);
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

static struct minix_file *minix_fopen_inodenum(struct minix_fs *fs, uint16_t inode, struct minix_file *f)
{
	if (!f || !inode) return 0;

	list_iter_of_minix_list_inode iter;
	list_loop(iter, fs->open_inodes) {
		if (list_item(iter).inode_n == inode) {
			goto loytyi;
		}
	}
	struct minix_list_inode inoli = {
		.inode_n = inode,
		.num_refs = 0,
	};
	fpos_t pos = fs->pos_inodes + (inode - 1) * MINIX_FS_INODE_SIZE;
	if (fsetpos(fs->dev, &pos)) return 0;
	if (fread(&inoli.inode, MINIX_FS_INODE_SIZE, 1, fs->dev) != 1) return 0;
	list_insert(list_end(fs->open_inodes), inoli);
	iter = list_prev(list_end(fs->open_inodes));

loytyi:
	fs->open_inodes_refcount++;
	list_item(iter).num_refs++;

	f->fs = fs;
	f->inode_n = list_item(iter).inode_n;
	f->inode = &list_item(iter).inode;
	f->inode_iter = iter;
	f->dev = f->fs->dev;
	f->dev_zone_map_pos = f->fs->pos_zone_map;
	f->dev_zones_pos = f->fs->pos_data;

	f->pos = f->std.pos = 0;
	f->size = f->std.size = f->inode->size;
	f->std.errno = f->std.eof = 0;
	f->std.func = &fs->std.filefunc;
	return f;
}

static uint16_t minix_locate_inode(struct minix_fs *fs, const char *name, const char *name_end, uint16_t inode)
{
	if (!name) return 0;
	if (*name == '/') {
		++name;
	}
	if (!name_end) {
		name_end = name + strlen(name);
		if (name_end[-1] == '/') {
			--name_end;
		}
	}
	if (name_end < name) {
		return 0;
	}
	if (name_end == name) {
		return inode;
	}

	struct minix_dir d = {
		.alloced = 0,
		.file.alloced = 0
	};
	const size_t maxlen = fs->filename_maxlen;
	while (name < name_end) {
		const char *end = name;
		while (end < name_end && *end != '/') {
			// TODO: Jotain kiellettyjä merkkejä?
			if (end - name == maxlen) {
				return 0;
			}
			++end;
		}

		if (!minix_dopen_inodenum(fs, inode, &d)) {
			return 0;
		}
		while (1) {
			if (minix_dread_only_dir(&d) != 0) {
				minix_dclose(&d);
				goto return_etc;
			}
			if (0 == memcmp(name, d.direntry.name, end - name)) {
				inode = d.direntry.inode;
				break;
			}
		}
		minix_dclose(&d);

		name = end + 1;
	}
	return inode;

return_etc:
	return 0;
}

static int minix_insert_direntry(struct minix_fs *fs, uint16_t dir, uint16_t file, const char *name)
{
	struct minix_file f = {.alloced = 0};
	struct minix_direntry entry;
	size_t len = strlen(name);

	if (len > fs->filename_maxlen) {
		return -1;
	}

	if (!minix_fopen_inodenum(fs, dir, &f)) {
		return -1;
	}
	while (1 == minix_fread(&entry, sizeof(entry), 1, &f)) {
		if (!entry.inode) {
			fpos_t pos = f.std.pos - sizeof(entry);
			if (!minix_fsetpos(&f, &pos)) {
				goto error_etc;
			}
			break;
		}
	}

	memcpy(entry.name, name, len);
	memset(entry.name + len, 0, sizeof(entry.name) - len);
	entry.inode = file;
	if (1 != minix_fwrite(&entry, sizeof(entry), 1, &f)) {
		goto error_etc;
	}

	minix_fclose(&f);
	return 0;

error_etc:
	minix_fclose(&f);
	return -1;
}

static struct minix_file *minix_fopen_all(struct minix_fs * restrict const fs, const char * restrict filename, uint_t mode, uint_t type, struct minix_file *f)
{
	if (!f || !fs || !filename) {
		return 0;
	}
//	print(""); // TODO: minix_fopen_all: Miksi tarvitaan? :D
	if ((mode & FILE_MODE_WRITE) && !fs->std.filefunc.fwrite) {
		return 0;
	}
	if ((mode & FILE_MODE_READ) && !fs->std.filefunc.fread) {
		return 0;
	}

	const char *fn_filepart;
	uint16_t pinode, finode;

	fn_filepart = filename + strlen(filename);
	if (fn_filepart[-1] == '/') --fn_filepart;
	while (fn_filepart[-1] != '/' && fn_filepart > filename) --fn_filepart;

	pinode = MINIX_ROOT_INODE;
	if (fn_filepart != filename) {
		if (!(pinode = minix_locate_inode(fs, filename, fn_filepart - 1, pinode))) {
			return 0;
		}
	}
	if (!(finode = minix_locate_inode(fs, fn_filepart, 0, pinode))) {
		if (!(mode & FILE_MODE_WRITE)) {
			return 0;
		}
		if (!(finode = minix_alloc_inode(fs, type))) {
			return 0;
		}
		if (minix_insert_direntry(fs, pinode, finode, fn_filepart) != 0) {
			minix_free_inode(fs, finode);
			return 0;
		}
	}

	// TODO: minix_insert_direntry
	// TODO: tarkistukset, ks. vanha koodi.
	if (!minix_fopen_inodenum(fs, finode, f)) {
		return 0;
	}
	if ((type == MINIX_FLAG_DIR && !MINIX_IS(f->inode->flags, DIR))
	|| (type == MINIX_FLAG_FILE && !MINIX_IS(f->inode->flags, FILE)) ){
		goto close_ret_error;
	}

	if (!(mode & FILE_MODE_WRITE)) {
		f->std.func = &minix_filefunc_ro;
	} else if (!(mode & FILE_MODE_READ)) {
		f->std.func = &minix_filefunc_wo;
	}
	if ((mode & FILE_MODE_APPEND)) {
		f->std.pos = f->std.size;
	}
	if ((mode & FILE_MODE_CLEAR)) {
		// TODO: minix_fopen_all: FILE_MODE_CLEAR
	}
	return f;

close_ret_error:
	minix_fclose(f);
	return 0;
}

struct minix_file *minix_fopen(struct minix_fs *fs, const char * filename, uint_t mode)
{
	struct minix_file *fp, f = {.alloced = 0};
	if (!minix_fopen_all(fs, filename, mode, MINIX_FLAG_FILE, &f)) {
		return 0;
	}
	if ((fp = MALLOC(sizeof(f)))) {
		memcpy(fp, &f, sizeof(f));
		return fp;
	}
	minix_fclose(&f);
	return 0;
}

size_t minix_freadwrite(char *buf, size_t size, size_t count, struct 
minix_file *stream, fread_t ffunction, int write) {
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
	if (!stream || !buf) return 0;
	return minix_freadwrite((char *)buf, size, count, stream, (fread_t) fread, 0);
}

size_t minix_fwrite(const void *buf, size_t size, size_t count, struct minix_file *stream)
{
	if (!stream || !buf) return 0;
	fpos_t pos0 = stream->pos;
	size_t ret;
	ret = minix_freadwrite((char *)buf, size, count, stream, (fread_t) fwrite, 1);
	if (stream->pos != pos0) {
		stream->written = 1;
	}
	return ret;
}

int minix_dmake(struct minix_fs *fs, const char * dirname)
{
	// TODO: minix_dmake: virheiden hoitelu...
        struct minix_file f = {.alloced = 0};

	if (minix_fopen_all(fs, dirname, FILE_MODE_READ, 0, &f)) {
		minix_fclose(&f);
		return DIR_ERR_EXISTS;
	}
	if (!minix_fopen_all(fs, dirname, FILE_MODE_RW, MINIX_FLAG_DIR, &f)) {
		return DIR_ERR_CANT_MAKE;
	}

	struct minix_direntry direntry = {
		.inode = f.inode_n
	};
	direntry.name[0] = '.';
	if (minix_fwrite(&direntry, sizeof(direntry), 1, &f) != 1) {
		minix_fclose(&f);
		return DIR_ERR_CANT_WRITE;
	}
	++f.inode->num_refs;
	direntry.name[1] = '.';
	if (minix_fwrite(&direntry, sizeof(direntry), 1, &f) != 1) {
		minix_fclose(&f);
		return DIR_ERR_CANT_WRITE;
	}
	++f.inode->num_refs;

	minix_fclose(&f);
	return 0;
}

static struct minix_dir *minix_dopen_inodenum(struct minix_fs *fs, uint16_t inode_n, struct minix_dir *d)
{
	if (!d || !inode_n) return 0;

	if (!minix_fopen_inodenum(fs, inode_n, &d->file)) {
		return 0;
	}
	if (!MINIX_IS(d->file.inode->flags, DIR)) {
		minix_fclose(&d->file);
		return 0;
	}
	return d;
}

struct minix_dir *minix_dopen(struct minix_fs *fs, const char * dirname)
{
	struct minix_dir *dp, d = {
		.std.func = &fs->std.dirfunc
	};

	uint16_t inode_n = minix_locate_inode(fs, dirname, 0, MINIX_ROOT_INODE);
	if (!minix_dopen_inodenum(fs, inode_n, &d)) {
		return 0;
	}

	if (!(dp = CALLOC(sizeof(struct minix_dir), 1))) {
		minix_fclose(&d.file);
		return 0;
	}
	dp->alloced = 1;
	memcpy(dp, &d, sizeof(d));
	return dp;
}

static int minix_dread_only_dir(struct minix_dir *listing)
{
	do if (minix_fread(&listing->direntry, MINIX_DIRENTRY_SIZE, 1, &listing->file) != 1) {
		return EOF;
	} while (!listing->direntry.inode);
	return 0;
}

int minix_dread(struct minix_dir *listing)
{
	if (minix_dread_only_dir(listing)) {
		return EOF;
	}

	fpos_t pos = listing->file.fs->pos_inodes + (listing->direntry.inode - 1) * MINIX_FS_INODE_SIZE;
	if (fsetpos(listing->file.fs->dev, &pos)) return EOF;
	if (fread(&listing->inode, MINIX_FS_INODE_SIZE, 1, listing->file.fs->dev) != 1) return EOF;

	listing->std.entry = (DIRENTRY) {
		.name = listing->direntry.name,
		.size = listing->inode.size,
		.uid = listing->inode.uid,
		.gid = listing->inode.gid,
		.rights = MINIX_RIGHTS(listing->inode.flags),
		.created = listing->inode.modified,
		.accessed = listing->inode.modified,
		.modified = listing->inode.modified,
		.references = listing->inode.num_refs,
		.type = MINIX_DIRENTRY_TYPE(listing->inode.flags),
	};
	if (MINIX_IS(listing->inode.flags, CHARDEV)
	|| MINIX_IS(listing->inode.flags, BLOCKDEV)) {
		listing->std.entry.dev_major = listing->inode.u.dev.major;
		listing->std.entry.dev_minor = listing->inode.u.dev.minor;
	}
	return 0;
}

int minix_dclose(struct minix_dir *listing)
{
	int i;
	i = minix_fclose(&listing->file);
	if (listing->alloced) {
		FREE(listing);
	}
	return i;
}
