#include <filesys/minix/minix.h>
#include <filesys/minix/fileutils.h>

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
		return -1;
	}
	if (*dend == '/') ++dend;
	if (!minix_insert_direntry(fs, pinode, inode, dend)) {
		minix_fclose(&f);
		return -1;
	}
	++f.inode->num_refs;
	f.written = 1;
	if (!minix_fclose(&f)) {
		return -1;
	}
	return 0;
}

int minix_symlink (struct minix_fs *fs, const char *name, const char *linkname)
{
	struct minix_file f = {.alloced = 0};

	if (!minix_fopen_all(fs, linkname, FILE_MODE_WRITE | FILE_MODE_MUST_BE_NEW, MINIX_FLAG_SYMLINK, &f)) {
		return -1;
	}
	if (minix_fwrite(name, strlen(name), 1, &f) != 1) {
		minix_fclose(&f);
		return -1;
	}
	minix_fclose(&f);
	return 0;
}

int minix_unlink (struct minix_fs *fs, const char *name)
{
	uint16_t inode;

	// Hajotetaan se ensin paloiksi, etsitään pinode ja finode!

	if (!(inode = minix_locate_inode(fs, name, 0, MINIX_ROOT_INODE))) {
		return -1;
	}
	// TODO: minix_unlink
	/*
	// Poistetaan direntry ja vähennetään inoden viittauksia yhdellä
	// Jos viittauksia on nolla, vapautetaan zonet
	if (!minix_free_all_zones_from_inode(fs, inode)) {
		return -1;
	}
	minix_free_inode(fs, inode);
	*/
	return -1;
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

	if (!minix_fopen_all(fs, name, FILE_MODE_WRITE | FILE_MODE_DONT_CREATE, 0, &f)) {
		return -1;
	}
	// TODO: minix_setprops: if (val->flags & FILE_PROP_TYPE) { }
	if (val->flags & FILE_PROP_RIGHTS) {
		MINIX_SET_RIGHTS(f.inode->flags, val->rights);
	}
	if (val->flags & FILE_PROP_UID) {
		f.inode->uid = val->uid;
	}
	if (val->flags & FILE_PROP_GID) {
		f.inode->gid = val->gid;
	}
	f.written = 1;

	minix_fclose(&f);
	return 0;
}

