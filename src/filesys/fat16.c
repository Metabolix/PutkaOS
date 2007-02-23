#include <filesys/fat.h>
#include <filesys/fat16.h>
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
		0,//(fwrite_t)    fat16_fwrite,
		(fflush_t)    fat16_fflush,
		(ftell_t)     fat16_ftell,
		(fseek_t)     fat16_fseek,
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

struct fat16_fs *fat12_mount(FILE *device, uint_t mode, const struct fat_header *fat_header)
{
	size_t i, fat_koko;
	struct fat16_fs *retval;
	unsigned short *fat;
	kprintf("fat12_mount...\n");

	fat_koko = sizeof(short) * fat_header->total_sectors / fat_header->sectors_per_cluster;
	retval = kmalloc(sizeof(struct fat16_fs) + fat_koko + sizeof(short));
	// - sizeof(short) oikeasti, mutta otetaan varmasti

	struct {unsigned eka : 12, toinen : 12;} buf;
	if (!retval) return 0;

	retval->std = fat16_fs;
	retval->std.fs_mount = (fs_mount_t) fat12_mount;
	retval->device = device;
	retval->get_fat = fat12_get_fat;
	retval->header = *fat_header;
	retval->fat_start = fat_header->reserved_sectors;
	retval->bytes_per_cluster = fat_header->sectors_per_cluster * fat_header->bytes_per_sector;
	retval->rootdir_start =
		+ fat_header->reserved_sectors
		+ fat_header->number_of_fats * fat_header->sectors_per_fat;
	if (!fseek(device, fat_header->bytes_per_sector * (fat_header->reserved_sectors + (size_t)fat_header->sectors_per_fat * fat_header->number_of_fats), SEEK_SET)) goto paha_loppu;
	fat = retval->fat12_fat;
	for (i = 0; i < fat_koko; i += 2, fat += 2) {
		if (fread(&buf, 3, 1, device) != 1) {
			goto paha_loppu;
		}
		fat[0] = buf.eka; if (fat[0] >= 0xFF0) fat[0] += 0xF000;
		fat[1] = buf.toinen; if (fat[1] >= 0xFF0) fat[1] += 0xF000;
	}
	return retval;
paha_loppu:
	kfree(retval);
	return 0;
}

unsigned short fat16_get_fat(struct fat16_fs *fs, int cluster_num)
{
	unsigned short retval;
	fseek(fs->device, fs->fat_start + 2*cluster_num, SEEK_SET);
	if (fread(&retval, 1, 2, fs->device) != 1) {
		return 0xFFF7;
	}
	return retval;
}
unsigned short fat12_get_fat(struct fat16_fs *fs, int cluster_num)
{
	return fs->fat12_fat[cluster_num];
}

int fat16_umount(struct fat16_fs *this)
{
	if (!this) {
		return -1;
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

void *fat16_fopen(struct fat16_fs *this, const char * filename, unsigned int mode)
{
	if (*filename == 0) {
		return 0;
	}
	if (!(mode & FILE_MODE_READ)) {
		kprintf("No write support in FAT12, file not opened...\n");
		return 0;
	}
	if (mode != FILE_MODE_READ) {
		kprintf("Attention! No write support in FAT12\n");
	}

	int i;
	int len = strlen(filename);
	char *buffer = kmalloc(len + 1);
	char *buf;
	char nimi[8+3];
	unsigned short cluster_num;
	struct fat_direntry direntry;
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

	cluster_num = 2;
	for (buf = buffer; buf < buffer + len;) {
		memset(nimi, ' ', sizeof(nimi));
		for (i = 0; i < 8 && *buf && *buf != '.'; ++i, ++buf) {
			nimi[i] = *buf;
			if (*buf >= 'a' && *buf <= 'z') nimi[i] -= 'a' - 'A';
		}
		if (*buf == '.') {
			++buf;
			for (i = 8; i < 8+3 && *buf && *buf != '/'; ++i) {
				nimi[i] = *buf;
				if (*buf >= 'a' && *buf <= 'z') nimi[i] -= 'a' - 'A';
			}
		}
	silmukka:
		fseek(this->device, this->rootdir_start + (cluster_num - 2) * this->bytes_per_cluster, SEEK_SET);
		for (i = 0; (i += sizeof(direntry)) <= this->bytes_per_cluster;) {
			if (fread(&direntry, sizeof(direntry), 1, this->device) != 1) {
				kfree(buffer);
				return 0;
			}
			if (memcmp(direntry.basename, nimi, 8+3) == 0) {
				goto loytyi;
			}
		}
		cluster_num = this->get_fat(this, cluster_num);
		if (!FAT16_CLUSTER_USED(cluster_num)) {
			kfree(buffer);
			return 0;
		}
		goto silmukka;
	loytyi:
		if ((direntry.attributes & FAT_ATTR_SUBDIR) == 0) {
			break;
		}
		cluster_num = direntry.first_cluster;
		if (!FAT16_CLUSTER_USED(cluster_num)) {
			kfree(buffer);
			return 0;
		}
		for(;*buf++;);
	}
	/* Menikö yli, oliko haluttu tiedosto hakemisto? */
	if (buf >= buffer + len) {
		kfree(buffer);
		kprintf("FAT16 (FAT12): '%s' is a dir\n", filename);
		return 0;
	}

	/* Jäikö lyhyeksi, ei löydy? */
	for(;*buf++;);
	if (buf < buffer + len) {
		kfree(buffer);
		kprintf("FAT16 (FAT12): '%s' not found\n", filename);
		return 0;
	}
	kfree(buffer);

	if (!FAT16_CLUSTER_USED(cluster_num)) {
		kprintf("!FAT16_CLUSTER_USED(cluster_num) O_o\n");
		return 0;
	}

	cluster_num = direntry.first_cluster;
	if ((direntry.attributes & FAT_ATTR_READONLY) && (mode & FILE_MODE_WRITE)) {
		kprintf("FAT16 (FAT12): '%s' is read-only\n", filename);
		return 0;
	}
	struct fat16_file *retval = kcalloc(1, sizeof(struct fat16_file));
	retval->std.func = &this->std.filefunc;
	retval->fs = this;
	retval->cluster_num = retval->first_cluster_num = cluster_num;
	return retval;
}

int fat16_fclose(struct fat16_file *stream)
{
	kfree(stream);
	return 0;
}

size_t fat16_fread(void *buf, size_t size, size_t count, struct fat16_file *stream)
{
	return 0;
}

size_t fat16_fwrite(void *buf, size_t size, size_t count, struct fat16_file *stream)
{
	return 0;
}

int fat16_fflush(struct fat16_file *stream)
{
	return EOF;
}

long fat16_ftell(struct fat16_file *stream)
{
	return stream->pos_in + (stream->cluster * stream->fs->bytes_per_cluster);
	return 0;
}

int fat16_fseek(struct fat16_file *stream, long int offset, int origin)
{
	size_t cluster_num, pos_in, clus, i;
	switch (origin) {
		case SEEK_SET:
			cluster_num = offset / stream->fs->bytes_per_cluster;
			pos_in = offset % stream->fs->bytes_per_cluster;
			break;
		case SEEK_END:
			offset = (stream->file_size) - offset;
			cluster_num = offset / stream->fs->bytes_per_cluster;
			pos_in = offset % stream->fs->bytes_per_cluster;
			break;
		case SEEK_CUR:
			pos_in = stream->pos_in + offset % stream->fs->bytes_per_cluster;
			cluster_num = stream->cluster + (offset + pos_in) / stream->fs->bytes_per_cluster;
			pos_in %= stream->fs->bytes_per_cluster;
			break;
		default:
			return EOF;
	}
	if (cluster_num > stream->last_cluster) {
		return EOF;
	}
	if (cluster_num == stream->last_cluster)
	if (pos_in > stream->last_pos_in) {
		return EOF;
	}
	clus = stream->cluster_num;
	for (i = stream->cluster; i < cluster_num; ++i) {
		clus = stream->fs->get_fat(stream->fs, clus);
		if (!FAT16_CLUSTER_USED(clus)) {
			return EOF;
		}
	}

	stream->pos_in = pos_in;
	stream->cluster = cluster_num;
	stream->cluster_num = clus;

	stream->std.pos.lo_dword = pos_in;
	stream->std.pos.hi_dword = cluster_num;

	return 0;
}

int fat16_fgetpos(struct fat16_file *stream, fpos_t *pos)
{
	pos->lo_dword = stream->pos_in;
	pos->hi_dword = stream->cluster;
	return 0;
}

int fat16_fsetpos(struct fat16_file *stream, const fpos_t *pos)
{
	stream->pos_in = pos->lo_dword;
	stream->cluster = pos->hi_dword;
	return 0;
}

int fat16_dmake(struct fat16_fs *this, const char * dirname, uint_t owner, uint_t rights)
{
	void *f = fat16_fopen(this, dirname, FILE_MODE_READ);
	if (f) {
		fat16_fclose(f);
		return -1;
	}
	f = fat16_fopen(this, dirname, FILE_MODE_WRITE);
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
	retval->file = fat16_fopen(this, dirname, FILE_MODE_READ);
	if (!retval->file) {
		kfree(retval);
		return 0;
	}
	retval->std.func = &fat16_fs.dirfunc;
	retval->std.entry.name = retval->name;
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
	if (fat16_fread(&listing->direntry, 1, sizeof(struct fat_direntry), listing->file) != 1) {
		return -1;
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
	listing->std.entry.size.lo_dword = listing->direntry.file_size;
	// TODO: created, modified, accessed
	return 0;
}
