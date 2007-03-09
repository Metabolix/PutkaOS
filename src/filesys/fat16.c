#include <filesys/fat.h>
#include <filesys/fat16.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <screen.h>

struct fs fat16_fs = {
	(fs_mount_t)  fat_mount,
	(fs_umount_t) fat16_umount,
	{
		(fopen_t)     fat16_fopen,
		(fclose_t)    fat16_fclose,
		(fread_t)     fat16_fread,
		(fwrite_t)    fat16_fwrite,
		(fflush_t)    fat16_fflush,
		(fgetpos_t)   fat16_fgetpos,
		(fsetpos_t)   fat16_fsetpos
	},
	{
		(dmake_t)     fat16_dmake,
		(dopen_t)     fat16_dopen,
		(dclose_t)    fat16_dclose,
		(dread_t)     fat16_dread
	}
};

#define fat16_intceil(num, bound) (((num + bound - 1) / bound) * bound)

struct fat16_fs *fat16_mount(FILE *device, uint_t mode, const struct fat_header *fat_header, int fat12)
{
	struct fat16_fs ret;

	memset(&ret, 0, sizeof(ret));
	ret.header = *fat_header;

	ret.bytes_per_cluster = ret.header.sectors_per_cluster * ret.header.bytes_per_sector;
	ret.fat_start = ret.header.reserved_sectors * (size_t)ret.header.bytes_per_sector;
	ret.rootdir_start =
		+ ret.fat_start
		+ (size_t)ret.header.bytes_per_sector * ret.header.number_of_fats * ret.header.sectors_per_fat;
	ret.data_start =
		+ ret.rootdir_start
		+ (size_t)ret.header.max_rootdir_entries * sizeof(struct fat_direntry);
	ret.data_start = fat16_intceil(ret.data_start, ret.header.bytes_per_sector);
	ret.data_end = ret.header.total_sectors * (size_t)ret.header.bytes_per_sector;
	ret.cluster_count = 2 + (ret.data_end - ret.data_start) / ret.bytes_per_cluster;

	ret.std = fat16_fs;
	ret.device = device;
	struct fat16_fs *retval;
	if (fat12 != 12) {
		ret.get_fat = fat16_get_fat;
		ret.set_next_cluster = fat16_set_next_cluster;
		retval = kmalloc(sizeof(struct fat16_fs));
		if (!retval) return 0;
		*retval = ret;
		return retval;
	}
	ret.get_fat = fat12_get_fat;
	ret.set_next_cluster = fat12_set_next_cluster;
	ret.write_fat = fat12_write_fat;
	retval = kmalloc(sizeof(struct fat16_fs) + ret.cluster_count * sizeof(short));
	if (!retval) return 0;
	*retval = ret;

	if (fat12_read_fat(retval)) {
		kfree(retval);
		return 0;
	}
	return retval;
}

fat16_fat_t fat16_get_fat(struct fat16_fs *fs, fat16_fat_t real_cluster)
{
	unsigned short retval;
	fseek(fs->device, fs->fat_start + 2*real_cluster, SEEK_SET);
	if (fread(&retval, 1, 2, fs->device) != 1) {
		return 0xFFF7;
	}
	return retval;
}
fat16_fat_t fat12_get_fat(struct fat16_fs *fs, fat16_fat_t real_cluster)
{
	return fs->fat12_fat[real_cluster];
}
fat16_fat_t fat16_set_next_cluster(struct fat16_fs *fs, fat16_fat_t cluster)
{
	const long int FUNC_BUF_SIZE = 256;
	fat16_fat_t i, j, fat[FUNC_BUF_SIZE];
	if (fseek(fs->device, fs->fat_start, SEEK_SET)) {
		return 0xffff;
	}
	for (i = j = 2; j < fs->cluster_count; j += i, i = 0) {
		if (fread(fat, sizeof(fat16_fat_t), FUNC_BUF_SIZE, fs->device) != FUNC_BUF_SIZE) {
			return 0xffff;
		}
		for (; i < FUNC_BUF_SIZE && i+j < fs->cluster_count; ++i) {
			if (FAT16_CLUSTER_FREE(fat[i])) {
				if (fseek(fs->device, (i - FUNC_BUF_SIZE) * sizeof(fat16_fat_t), SEEK_CUR)) {
					return 0xffff;
				}
				if (fwrite(fat+i, sizeof(fat16_fat_t), 1, fs->device) != 1) {
					return 0xffff;
				}
				fat[i] = 0xffff;
				if (FAT16_CLUSTER_USED(cluster)) {
					if (fseek(fs->device, fs->fat_start + cluster * sizeof(fat16_fat_t), SEEK_SET)) {
						return 0xffff;
					}
					fat[0] = i + j;
					if (fwrite(fat, sizeof(fat16_fat_t), 1, fs->device) != 1) {
						return 0xffff;
					}
				}
				return i+j;
			}
		}
	}
	return 0xffff;
}
int fat12_read_fat(struct fat16_fs *fs)
{
	fat16_fat_t i, *fat;
	if (fseek(fs->device, fs->fat_start, SEEK_SET)) {
		return -1;
	}
	struct fat12_fat_duo buf;
	fat = fs->fat12_fat;
	for (i = 0; i < fs->cluster_count; i += 2, fat += 2) {
		if (fread(&buf, 3, 1, fs->device) != 1) {
			return -1;
		}
		fat[0] = buf.e1; if (fat[0] >= 0xFF0) fat[0] += 0xF000;
		fat[1] = buf.e2; if (fat[1] >= 0xFF0) fat[1] += 0xF000;
	}
	return 0;
}
int fat12_write_fat(struct fat16_fs *fs)
{
	fat16_fat_t i, *fat;
	if (fseek(fs->device, fs->fat_start, SEEK_SET)) {
		return -1;
	}
	struct fat12_fat_duo buf;
	fat = fs->fat12_fat;
	for (i = 0; i < fs->cluster_count; i += 2, fat += 2) {
		buf.e1 = fat[0];// = buf.e1; if (fat[0] >= 0xFF0) fat[0] += 0xF000;
		buf.e2 = fat[1];// = buf.e2; if (fat[1] >= 0xFF0) fat[1] += 0xF000;
		if (fwrite(&buf, 3, 1, fs->device) != 1) {
			return -1;
		}
	}
	return 0;
}
fat16_fat_t fat12_set_next_cluster(struct fat16_fs *fs, fat16_fat_t cluster)
{
	fat16_fat_t i;
	for (i = 2; i < fs->cluster_count; ++i) {
		if (FAT16_CLUSTER_FREE(fs->fat12_fat[i])) {
			fs->fat12_fat[i] = 0xffff;
			if (FAT16_CLUSTER_USED(cluster)) {
				fs->fat12_fat[cluster] = i;
			}
			return i;
		}
	}
	return 0xffff;
}

int fat16_umount(struct fat16_fs *this)
{
	if (!this) {
		return -1;
	}
	if (this->write_fat) {
		this->write_fat(this);
	}
	fflush(this->device);
	kfree(this);
	return 0;
}

int fat16_invalid_char(char merkki)
{
	if (merkki >= 'A' && merkki <= 'Z') return 0;
	if (merkki >= '0' && merkki <= '9') return 0;
	if (merkki == ' ') return 0;
	if (merkki < 0) return 0;
	const char *lista = "!#$%&'()-@^_`{}~";
	while (*lista) {
		if (*lista == merkki) {
			return 0;
		}
		++lista;
	}
	return -1;
}
int fat16_invalid_filename(const char *filename)
{
	int i, j;
	for (i = 0, j = 8; *filename; ++filename) {
		if (*filename == '/') {
			j = 8;
			i = 0;
		} else if (*filename == '.') {
			if (j == 3) {
				return -1;
			}
			j = 3;
			i = 0;
		} else if (fat16_invalid_char(*filename)) {
			return -1;
		} else {
			++i;
			if (i > j) {
				return -1;
			}
		}
	}
	return 0;
}

void *fat16_fopen(struct fat16_fs *this, const char * filename, uint_t mode)
{
	return fat16_fopen_all(this, filename, mode, 0);
}

void *fat16_fopen_all(struct fat16_fs *this, const char * filename, uint_t mode, int accept_dir)
{
	if (!filename) {
		return 0;
	}
	/*
	if (!(mode & FILE_MODE_READ)) {
		kprintf("No write support in FAT12, file not opened...\n");
		return 0;
	}
	if (mode != FILE_MODE_READ) {
		kprintf("Attention! No write support in FAT12\n");
	}
	*/

	struct fat16_file *retval;
	int i;
	int len = strlen(filename);
	char *buffer = kmalloc(len + 1);
	char *buf;
	char nimi[8+3];
	unsigned short real_cluster;
	char rootdir_menossa;
	struct fat_direntry direntry;

	if (len == 0) {
		if (!accept_dir) {
			return 0;
		}
		retval = kcalloc(1, sizeof(struct fat16_file) + sizeof(struct filefunc));
		if (!retval) {
			return 0;
		}
		retval->fs = this;
		retval->std.func = (struct filefunc *)(retval + 1);
		memcpy(retval->std.func, &this->std.filefunc, sizeof(struct filefunc));
		retval->std.func->fread = (fread_t)fat16_fread_rootdir;
		retval->std.func->fwrite = (fwrite_t)fat16_fwrite_rootdir;
		retval->file_size = this->header.max_rootdir_entries * sizeof(struct fat_direntry);
		return retval;
	}
	strcpy(buffer, filename);

	for (i = 0; i < len; ++i) {
		if (buffer[i] >= 'a' && buffer[i] <= 'z') {
			buffer[i] = buffer[i] - 'a' + 'A';
		}
	}
	if (fat16_invalid_filename(buffer)) {
		kfree(buffer);
		return 0;
	}

	for (i = 0; i < len; ++i) {
		if (buffer[i] == '/') {
			buffer[i] = 0;
		}
	}

	rootdir_menossa = 1;
	real_cluster = 0; // Compiler wants it... Needless
	fseek(this->device, this->rootdir_start, SEEK_SET);
	for (buf = buffer; buf < buffer + len;) {
		memset(nimi, ' ', sizeof(nimi));
		for (i = 0; i < 8 && *buf && *buf != '.'; ++i, ++buf) {
			nimi[i] = *buf;
			if (*buf >= 'a' && *buf <= 'z') nimi[i] -= 'a' - 'A';
		}
		if (*buf == '.') {
			++buf;
			for (i = 8; i < 8+3 && *buf && *buf != '/'; ++i, ++buf) {
				nimi[i] = *buf;
				if (*buf >= 'a' && *buf <= 'z') nimi[i] -= 'a' - 'A';
			}
		}
	silmukka:
		for (i = 0; rootdir_menossa ? (++i <= this->header.max_rootdir_entries) : ((i += sizeof(direntry)) <= this->bytes_per_cluster);) {
			if (fread(&direntry, sizeof(direntry), 1, this->device) != 1) {
				kfree(buffer);
				return 0;
			}
			if (memcmp(direntry.basename, nimi, 8+3) == 0) {
				goto loytyi;
			}
		}
		if (rootdir_menossa) {
			kfree(buffer);
			return 0;
		}
		//kprintf("real_cluster %x = %x\n", (long)real_cluster, (long) this->get_fat(this, real_cluster));
		real_cluster = this->get_fat(this, real_cluster);
		if (!FAT16_CLUSTER_USED(real_cluster)) {
			kprintf("1 !FAT16_CLUSTER_USED(%04x)\n", real_cluster);
			kfree(buffer);
			return 0;
		}
		if (fseek(this->device, this->data_start + (real_cluster - 2) * this->bytes_per_cluster, SEEK_SET)) {
			kprintf("FAT16 (FAT12): fat16_fopen: fseek failed\n", real_cluster);
			kfree(buffer);
			return 0;
		}
		goto silmukka;
	loytyi:
		rootdir_menossa = 0;
		if ((direntry.attributes & FAT_ATTR_SUBDIR) == 0) {
			for(;*buf++;);
			break;
		}
		real_cluster = direntry.first_cluster;
		if (!FAT16_CLUSTER_USED(real_cluster)) {
			kprintf("2 !FAT16_CLUSTER_USED(%04x)\n", real_cluster);
			kfree(buffer);
			return 0;
		}
		if (fseek(this->device, this->data_start + (real_cluster - 2) * this->bytes_per_cluster, SEEK_SET)) {
			kprintf("FAT16 (FAT12): fat16_fopen: fseek failed\n", real_cluster);
			kfree(buffer);
			return 0;
		}
		for(;*buf++;);
	}
	kfree(buffer);
	if (buf < buffer + len) {
		kprintf("FAT16 (FAT12): '%s' not found\n", filename);
		return 0;
	}
	if (direntry.attributes & FAT_ATTR_SUBDIR)
	if (!accept_dir) {
		return 0;
	}

	real_cluster = direntry.first_cluster;
	if (!FAT16_CLUSTER_USED(real_cluster)) {
		kprintf("!FAT16_CLUSTER_USED(real_cluster) O_o\n");
		return 0;
	}

	if ((direntry.attributes & FAT_ATTR_READONLY) && (mode & FILE_MODE_WRITE)) {
		kprintf("FAT16 (FAT12): '%s' is read-only\n", filename);
		return 0;
	}
	retval = kcalloc(1, sizeof(struct fat16_file));
	retval->std.func = &this->std.filefunc;
	retval->fs = this;
	retval->real_cluster = retval->real_first_cluster = real_cluster;
	retval->file_size = direntry.file_size;
	retval->last_cluster = retval->file_size / retval->fs->bytes_per_cluster;
	retval->last_pos_in = retval->file_size % retval->fs->bytes_per_cluster;
	return retval;
}

int fat16_devseek(struct fat16_file *stream)
{
	return fseek(stream->fs->device, stream->fs->data_start + (stream->real_cluster - 2) * stream->fs->bytes_per_cluster + stream->pos_in, SEEK_SET);
}

int fat16_fclose(struct fat16_file *stream)
{
	kfree(stream);
	return 0;
}

size_t fat16_fread_rootdir(void *buf, size_t size, size_t count, struct fat16_file *stream)
{
	fseek(stream->fs->device, stream->fs->rootdir_start + stream->pos_in, SEEK_SET);
	if (count * size > stream->file_size - stream->pos_in) {
		count = (stream->file_size - stream->pos_in) / size;
	}
	count = fread(buf, size, count, stream->fs->device);
	stream->pos_in += count * size;
	return count;
}

size_t fat16_fwrite_rootdir(void *buf, size_t size, size_t count, struct fat16_file *stream)
{
	fseek(stream->fs->device, stream->fs->rootdir_start + stream->pos_in, SEEK_SET);
	if (count * size > stream->file_size - stream->pos_in) {
		count = (stream->file_size - stream->pos_in) / size;
	}
	count = fwrite(buf, size, count, stream->fs->device);
	stream->pos_in += count * size;
	return count;
}

int fat16_fwrite_next_cluster(struct fat16_file *stream)
{
	size_t new_cluster = stream->fs->get_fat(stream->fs, stream->real_cluster);
	if (FAT16_CLUSTER_EOF(new_cluster)) {
		new_cluster = stream->fs->set_next_cluster(stream->fs, stream->real_cluster);
		if (FAT16_CLUSTER_EOF(new_cluster)) {
			stream->std.eof = EOF;
			return -1;
		}
	}
	stream->real_cluster = new_cluster;
	return 0;
}

int fat16_fread_next_cluster(struct fat16_file *stream)
{
	if (stream->cluster > stream->last_cluster) {
		stream->std.eof = EOF;
		return -1;
	}
	if (stream->cluster == stream->last_cluster)
	if (stream->pos_in >= stream->last_pos_in) {
		stream->std.eof = EOF;
		return -1;
	}
	stream->real_cluster = stream->fs->get_fat(stream->fs, stream->real_cluster);
	return 0;
}

size_t fat16_fread(void *buf, size_t size, size_t count, struct fat16_file *stream)
{
	return fat16_freadwrite((char*)buf, size, count, stream, (fat16_freadwrite_t) fread, fat16_fread_next_cluster);
}

size_t fat16_fwrite(const void *buf, size_t size, size_t count, struct fat16_file *stream)
{
	size_t retval = fat16_freadwrite((char*)buf, size, count, stream, (fat16_freadwrite_t) fwrite, fat16_fwrite_next_cluster);
	if (stream->std.pos > stream->file_size) {
		stream->file_size = (size_t) stream->std.pos;
		// TODO: file_size to FAT
	}
	return retval;
}

size_t fat16_freadwrite(char *buf, const size_t size, const size_t count, struct fat16_file *stream, const fat16_freadwrite_t freadwrite, const fat16_next_cluster_t fat16_next_cluster)
{
	size_t try_read, bytes_read, size_rem, count_rem;
	if (!count || !size) {
		return 0;
	}

	if (!FAT16_CLUSTER_USED(stream->real_cluster) || fat16_devseek(stream)) {
		return 0;
	}
	for (count_rem = count; count_rem; --count_rem) {
		size_rem = size;
		while (size_rem) {
			if (stream->pos_in >= stream->fs->bytes_per_cluster) {
				stream->pos_in = 0;
				stream->cluster++;
				if (fat16_next_cluster(stream)) {
					return count - count_rem;
				}

				if (!FAT16_CLUSTER_USED(stream->real_cluster) || fat16_devseek(stream)) {
					return count - count_rem;
				}
			}
			if ((try_read = size_rem) > stream->fs->bytes_per_cluster - stream->pos_in) {
				try_read = stream->fs->bytes_per_cluster - stream->pos_in;
			}
			bytes_read = freadwrite(buf, 1, try_read, stream->fs->device);
			stream->std.pos += bytes_read;
			if (try_read != bytes_read) {
				return count - count_rem;
			}
			buf += bytes_read;
			size_rem -= bytes_read;
			stream->pos_in += bytes_read;
		}
	}
	return count;
}

int fat16_fflush(struct fat16_file *stream)
{
	return fflush(stream->fs->device);
}

int fat16_fgetpos(struct fat16_file *stream, fpos_t *pos)
{
	*pos = stream->std.pos;
	return 0;
}

int fat16_fsetpos(struct fat16_file *stream, const fpos_t *pos)
{
	uint64_t pos_in_64;
	size_t pos_in, cluster, clus, real_cluster;
	cluster = uint64_div_rem(*pos, stream->fs->bytes_per_cluster, &pos_in_64);
	pos_in = pos_in_64;

	if (cluster > stream->last_cluster) {
		return EOF;
	}

	if (cluster == stream->last_cluster) {
		if (pos_in > stream->last_pos_in) {
			return EOF;
		} else {
			stream->std.eof = EOF;
		}
	}
	if (cluster > stream->cluster) {
		real_cluster = stream->real_first_cluster;
		clus = 0;
	} else {
		real_cluster = stream->real_cluster;
		clus = stream->cluster;
	}
	for (; clus < cluster; ++clus) {
		real_cluster = stream->fs->get_fat(stream->fs, real_cluster);
		if (!FAT16_CLUSTER_USED(real_cluster)) {
			return EOF;
		}
	}
	stream->real_cluster = real_cluster;
	stream->cluster = cluster;
	stream->pos_in = pos_in;
	stream->std.pos = *pos;
	return 0;
}

int fat16_dmake(struct fat16_fs *this, const char * dirname, uint_t owner, uint_t rights)
{
	void *f = fat16_fopen_all(this, dirname, FILE_MODE_READ, 1);
	if (f) {
		fat16_fclose(f);
		return -1;
	}
	f = fat16_fopen_all(this, dirname, FILE_MODE_WRITE, 1);
	if (f) {
		fat16_fclose(f);
		return 0;
	}
	return -1;
}

struct fat16_dir *fat16_dopen(struct fat16_fs *this, const char * dirname)
{
	struct fat16_dir *retval = kcalloc(1, sizeof(struct fat16_dir));
	if (!retval) return 0;
	retval->file = fat16_fopen_all(this, dirname, FILE_MODE_READ, 1);
	if (!retval->file) {
		kfree(retval);
		return 0;
	}
	retval->std.func = &fat16_fs.dirfunc;
	retval->std.name = retval->name;
	return retval;
}

int fat16_dclose(struct fat16_dir *listing)
{
	int retval = fat16_fclose(listing->file);
	kfree(listing);
	return retval;
}

int fat16_dread(struct fat16_dir *listing)
{
fat16_dread_alku:
	if (listing->file->std.func->fread(&listing->direntry, sizeof(struct fat_direntry), 1, (FILE*)listing->file) != 1) {
		return -1;
	}
	switch (listing->direntry.basename[0]) {
		case 0:
			return -1;
		case (char)0xe5:
			goto fat16_dread_alku;
		case 0x05:
			listing->direntry.basename[0] = 0xe5;
	}
	int i, j;
	for (i = 8; i;) { --i;
		if (listing->direntry.basename[i] != ' ') {
			break;
		}
	}
	listing->name[i] = listing->direntry.basename[i];
	for (j = i; i;) { --i;
		listing->name[i] = listing->direntry.basename[i];
	}
	if (memcmp(listing->direntry.extension, "   ", 3) == 0) {
		listing->name[++j] = 0;
	} else {
		listing->name[++j] = '.';
		for (++j, i = 0; i < 3; ++i) {
			listing->name[j+i] = listing->direntry.extension[i];
		}
		for (i = 3; i--;) {
			if (listing->name[j+i] != ' ') {
				break;
			}
			listing->name[j+i] = 0;
		}
	}
	if (strlen(listing->name) == 0) {
		goto fat16_dread_alku;
	}
/*
struct fat_direntry {
	char basename[8];
	char extension[3];
	unsigned char attributes;
	unsigned char reserved;
	unsigned char create_time_10ms;
	unsigned short create_time;
	unsigned short create_date;
	unsigned short access_date;
	union {
		unsigned short ea_index;
		unsigned short first_cluster_hiword;
	};
	unsigned short modify_time;
	unsigned short modify_date;
	unsigned short first_cluster;
	unsigned long file_size;
} __attribute__((packed));

typedef struct _DIR {
	char *name;
	fpos_t size;
	uint_t owner;
	uint_t rights;
	time_t created, accessed, modified;
	uint_t references;
} DIR;
*/
	listing->std.size = listing->direntry.file_size;
	struct tm tm;
	tm.tm_usec = 0;
	MK_STRUCT_TM(listing->direntry.create_date, listing->direntry.create_time, tm);
	tm.tm_sec += (listing->direntry.create_time_10ms + 50) / 100;
	listing->std.created = mktime(&tm);
	MK_STRUCT_TM(listing->direntry.modify_date, listing->direntry.modify_time, tm);
	listing->std.modified = mktime(&tm);
	MK_STRUCT_TM(listing->direntry.access_date, listing->direntry.modify_time, tm);
	tm.tm_sec = tm.tm_min = 0; tm.tm_hour = 12;
	listing->std.accessed = mktime(&tm);
	// TODO: owner, rights, references
	return 0;
}

