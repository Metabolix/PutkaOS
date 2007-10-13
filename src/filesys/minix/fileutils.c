#include <filesys/minix/minix.h>
#include <filesys/minix/fileutils.h>
#include <filesys/minix/zones.h>
#include <filesys/minix/maps.h>

int minix_link (struct minix_fs *fs, const char *name, const char *linkname)
{
	uint16_t inode, pinode;
	const char *dend;
	struct minix_file f = {.alloced = 0};

	dend = linkname + strlen(linkname);

	do {--dend;} while (dend > linkname && *dend != '/');

	if (!(pinode = minix_locate_inode(fs, linkname, dend, MINIX_ROOT_INODE))
	||  !(inode = minix_locate_inode(fs, name, 0, MINIX_ROOT_INODE))
	||  !minix_fopen_inodenum(fs, inode, &f)) {
		goto error_etc;
	}
	if (*dend == '/') ++dend;
	if (!minix_insert_direntry(fs, pinode, inode, dend)) {
		goto error_etc;
	}
	++f.inode->num_refs;
	f.written = 1;

	goto return_etc;

return_etc:
	if (!minix_fclose(&f)) {
		return -1;
	}
	return 0;

error_etc:
	if (f.fs) {
		minix_fclose(&f);
	}
	return -1;
}

int minix_symlink (struct minix_fs *fs, const char *name, const char *linkname)
{
	struct minix_file f = {.alloced = 0};

	if (!minix_fopen_all(fs, linkname, FILE_MODE_WRITE | FILE_MODE_MUST_BE_NEW, MINIX_FLAG_SYMLINK, &f)) {
		return -1;
	}
#if 1
	if (minix_fwrite(name, strlen(name), 1, &f) != 1) {
		minix_fclose(&f);
		return -1;
	}
#else
	size_t len = strlen(name);
	int p1 = (len % MINIX_ZONE_SIZE != 0) ? 1 : 0;
	if (minix_fwrite(name, len + p1, 1, &f) != 1) {
		minix_fclose(&f);
		return -1;
	}
	if (p1) {
		f.inode->size -= p1;
	}
	minix_fclose(&f);
#endif
	return 0;
}

int minix_unlink (struct minix_fs *fs, const char *name)
{
	uint16_t inode, pinode;
	const char *dend, *filepart;
	struct minix_file f = {.alloced = 0};
	struct minix_direntry dirent;
	int ret = 0;

	size_t len = strlen(name);
	dend = name + len;
	do {--dend;} while (dend > name && *dend != '/');
	if (dend > name) {
		filepart = dend + 1;
	} else {
		filepart = dend;
	}
	len = (name + len) - filepart;
	if (len > fs->filename_maxlen) {
		goto error_etc;
	}

	// Haetaan ja avataan hakemisto
	if (!(pinode = minix_locate_inode(fs, name, dend, MINIX_ROOT_INODE))) {
		goto error_etc;
	}
	if (!minix_fopen_inodenum(fs, pinode, &f)) {
		goto error_etc;
	}

	// Etsitään tämä tiedosto
	while (minix_fread(&dirent, sizeof(dirent), 1, &f) == 1) {
		if (dirent.inode && 0 == strncmp(dirent.name, filepart, fs->filename_maxlen)) {
			goto loytyi;
		}
	}
	goto error_etc;
loytyi:
	inode = dirent.inode;

	// Tyhjennetään direntry
	dirent.inode = 0;
	fpos_t pos = f.std.pos - sizeof(dirent);
	if (minix_fsetpos(&f, &pos)
	|| minix_fwrite(&dirent, sizeof(dirent), 1, &f) != 1) {
		ret = -2;
	}
	if (minix_fclose(&f)) {
		ret = -2;
	}
	f.fs = 0;
	if (!minix_fopen_inodenum(fs, inode, &f)) {
		ret = -3;
		goto error_etc;
	}

	// Vähennetään inoden viittauksia yhdellä
	--f.inode->num_refs;
	f.written = 1;

	// Jos viittauksia on nolla, vapautetaan zonet
	if (!f.inode->num_refs) {
		if (minix_free_all_zones_from_inode(fs, f.inode)) {
			ret = -3;
		}
		minix_free_inode(fs, inode);
		list_item(f.inode_iter).inode_n = 0;
	}

return_etc:
	if (f.fs) {
		if (minix_fclose(&f) && !ret) {
			ret = -1;
		}
	}
	return ret;

error_etc:
	if (!ret) {
		ret = -1;
	}
	goto return_etc;
}

int minix_getprops (struct minix_fs *fs, const char *name, struct file_props *val)
{
	struct minix_file f = {.alloced = 0};

	if (!minix_fopen_all(fs, name, FILE_MODE_READ | FILE_MODE_DONT_CREATE, 0, &f)) {
		return -1;
	}
	*val = (struct file_props) {
		.rights = MINIX_GET_RIGHTS(f.inode->flags),
		.uid = f.inode->uid,
		.gid = f.inode->gid,
		.type = MINIX_MK_DIRENTRY_TYPE(f.inode->flags),

		.flags = FILE_PROP_RIGHTS | FILE_PROP_UID | FILE_PROP_GID | FILE_PROP_TYPE,
	};

	minix_fclose(&f);
	return 0;
}

int minix_setprops (struct minix_fs *fs, const char *name, const struct file_props *val)
{
	struct minix_file f = {.alloced = 0};
	uint_t ret = 0;

	if (!minix_fopen_all(fs, name, FILE_MODE_WRITE | FILE_MODE_DONT_CREATE, 0, &f)) {
		return -1;
		return ret;
	}
	// TODO: minix_setprops: if (val->flags & FILE_PROP_TYPE) { }
	if (val->flags & FILE_PROP_RIGHTS) {
		MINIX_SET_RIGHTS(f.inode->flags, val->rights);
		ret |= FILE_PROP_RIGHTS;
	}
	if (val->flags & FILE_PROP_UID) {
		f.inode->uid = val->uid;
		ret |= FILE_PROP_UID;
	}
	if (val->flags & FILE_PROP_GID) {
		f.inode->gid = val->gid;
		ret |= FILE_PROP_GID;
	}
	f.written = 1;

	if (!minix_fclose(&f)) {
		// TODO: minix_setprops: minix_fclose failed!
	}
	return 0;
	return ret;
}

