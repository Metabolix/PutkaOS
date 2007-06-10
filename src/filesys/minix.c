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

const struct fs minix_fs = {
	"minix",
	(fs_mount_t)  minix_mount,
	(fs_umount_t) minix_umount,
	{
		(fopen_t)     minix_fopen,
		(fclose_t)    minix_fclose,
		(fread_t)     minix_fread,
		(fwrite_t)    minix_fwrite,
		(fflush_t)    minix_fflush,
		(fsetpos_t)   fsetpos_copypos
	},
	{
		(dmake_t)     minix_dmake,
		(dopen_t)     minix_dopen,
		(dclose_t)    minix_dclose,
		(dread_t)     minix_dread
	}
};

const struct filefunc minix_filefunc_ro = {
	.fopen = (fopen_t)     minix_fopen,
	.fclose = (fclose_t)   minix_fclose,
	.fread = (fread_t)     minix_fread,
	.fwrite = 0,
	.fflush = (fflush_t)   fflush_none,
	.fsetpos = (fsetpos_t) fsetpos_copypos
};

const struct filefunc minix_filefunc_wo = {
	.fopen = (fopen_t)     minix_fopen,
	.fclose = (fclose_t)   minix_fclose,
	.fread = 0,
	.fwrite = (fwrite_t)   minix_fwrite,
	.fflush = (fflush_t)   minix_fflush,
	.fsetpos = (fsetpos_t) fsetpos_copypos
};

static int minix_check_filename(const char *filename, int maxlen, int *parts)
{
	if (!filename) {
		return -1;
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

	if ((this = kmalloc(sizeof(struct minix_fs)))) {
		memcpy(this, &fs, sizeof(struct minix_fs));
		list_init(this->open_inodes);
	}
	return (struct fs *)this;
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
	f->pos = 0;
	f->dev = f->fs->dev;
	f->dev_zone_map_pos = f->fs->pos_zone_map;
	f->dev_zones_pos = f->fs->pos_data;

	f->std.pos = 0;
	f->std.size = f->inode->size;
	f->std.errno = f->std.eof = 0;
	f->std.func = &this->std.filefunc;
	return f;
}

static uint16_t minix_alloc_inode(struct minix_fs *this, uint16_t type)
{
	// TODO: Rights, uid, gid, modified, num_refs
	struct minix_inode inode = {
		.flags = 0777 | (type << 12),
		.uid = 0,
		.size = 0,
		.modified = 0,
		.gid = 0,
		.num_refs = 1,
		.u.zones.std = {0},
		.u.zones.indir = 0,
		.u.zones.dbl_indir = 0
	};
	// TODO: Find space for this, mark inode_map, add to the list (with zero refs)
	return 0;
}

static int minix_free_inode(struct minix_fs *this, uint16_t inode)
{
	// TODO
	return -1;
}

static struct minix_file *minix_fopen_all(struct minix_fs *this, const char * filename, uint_t mode, _1x2_t file_or_dir)
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

	if (!(mode & (FILE_MODE_READ | FILE_MODE_WRITE))) {
		return 0;
	}
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

static int minix_get_zones(struct minix_file *stream, size_t numzones, uint16_t *zonelist, int write)
{
	// TODO
	return 0;
}

size_t minix_freadwrite(char *buf, size_t size, size_t count, struct minix_file *stream, fread_t ffunction, int write)
{
	const size_t pos0 = stream->pos;
	const fpos_t pos_1a = pos0 + (uint64_t)size * count;
	const size_t pos_1b = write ? stream->fs->super.max_size : stream->size;
	const size_t pos1 = pos_1a < pos_1b ? pos_1a : pos_1b;
	const size_t byte_count = pos1 - pos0;
	const size_t zone0 = pos0 / MINIX_FS_ZONE_SIZE;
	const size_t zone1 = (pos1 - 1) / MINIX_FS_ZONE_SIZE;
	const size_t numzones = 1 + zone1 - zone0;
	fpos_t pos;
	size_t zone;
	uint16_t zonelist[numzones];
	size_t done_size, pos_in_zone;

	minix_get_zones(stream, numzones, zonelist, write);

	if (numzones == 1) {
		pos = zonelist[0] * MINIX_FS_ZONE_SIZE;
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
	return (stream->pos - pos0) / size;
}

size_t minix_fread(void *buf, size_t size, size_t count, struct minix_file *stream)
{
	return minix_freadwrite(buf, size, count, stream, (fread_t) fread, 0);
}

size_t minix_fwrite(void *buf, size_t size, size_t count, struct minix_file *stream)
{
	return minix_freadwrite(buf, size, count, stream, (fread_t) fwrite, 1);
}

int minix_fflush(struct minix_file *stream)
{
	// TODO: kirjoita inode
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
	}
	return retval;
}

int minix_dread(struct minix_dir *listing)
{
alku:
	if (minix_fread(&listing->direntry, MINIX_FS_DIRENTRY_SIZE, 1, listing->file) != 1) {
		return EOF;
	}
	if (!listing->direntry.real.inode) {
		goto alku;
	}

	fpos_t pos = listing->file->fs->pos_inodes + (listing->direntry.real.inode - 1) * MINIS_FS_INODE_SIZE;
	if (fsetpos(listing->file->fs->dev, &pos)) return 0;
	if (fread(&listing->inode, MINIS_FS_INODE_SIZE, 1, listing->file->fs->dev) != 1) return 0;

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
