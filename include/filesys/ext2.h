/*
 *  from linux/include/linux/ext2_fs.h
 */

#ifndef _EXT2_FS_H
#define _EXT2_FS_H

#include <filesys/filesystem.h>
#include <spinlock.h>

/*
 * The second extended filesystem constants/structures
 */

#undef EXT2_DEBUG

/*
 * Define EXT2_PREALLOCATE to preallocate data blocks for expanding files
 */
#define EXT2_PREALLOCATE
#define EXT2_DEFAULT_PREALLOC_BLOCKS	8

/*
 * The second extended file system version
 */
#define EXT2FS_DATE		"95/08/09"
#define EXT2FS_VERSION		"0.5b"


/*
 * Special inode numbers
 */
#define	EXT2_BAD_INO		 1	/* Bad blocks inode */
#define EXT2_ROOT_INO		 2	/* Root inode */
#define EXT2_BOOT_LOADER_INO	 5	/* Boot loader inode */
#define EXT2_UNDEL_DIR_INO	 6	/* Undelete directory inode */

/* First non-reserved inode for old ext2 filesystems */
#define EXT2_GOOD_OLD_FIRST_INO	11

/*
 * The second extended file system magic number
 */
#define EXT2_SUPER_MAGIC	0xEF53

/*
 * Maximal count of links to a file
 */
#define EXT2_LINK_MAX		32000

/*
 * Macro-instructions used to manage several block sizes
 */
#define EXT2_MIN_BLOCK_SIZE		1024
#define	EXT2_MAX_BLOCK_SIZE		4096
#define EXT2_MIN_BLOCK_LOG_SIZE		  10
#ifdef __KERNEL__
# define EXT2_BLOCK_SIZE(s)		((s)->s_blocksize)
#else
# define EXT2_BLOCK_SIZE(s)		(EXT2_MIN_BLOCK_SIZE << (s)->s_log_block_size)
#endif
#define	EXT2_ADDR_PER_BLOCK(s)		(EXT2_BLOCK_SIZE(s) / sizeof (unsigned int))
#ifdef __KERNEL__
# define EXT2_BLOCK_SIZE_BITS(s)	((s)->s_blocksize_bits)
#else
# define EXT2_BLOCK_SIZE_BITS(s)	((s)->s_log_block_size + 10)
#endif
#ifdef __KERNEL__
#define	EXT2_ADDR_PER_BLOCK_BITS(s)	(EXT2_SB(s)->s_addr_per_block_bits)
#define EXT2_INODE_SIZE(s)		(EXT2_SB(s)->s_inode_size)
#define EXT2_FIRST_INO(s)		(EXT2_SB(s)->s_first_ino)
#else
#define EXT2_INODE_SIZE(s)	(((s)->s_rev_level == EXT2_GOOD_OLD_REV) ? \
				 EXT2_GOOD_OLD_INODE_SIZE : \
				 (s)->s_inode_size)
#define EXT2_FIRST_INO(s)	(((s)->s_rev_level == EXT2_GOOD_OLD_REV) ? \
				 EXT2_GOOD_OLD_FIRST_INO : \
				 (s)->s_first_ino)
#endif

/*
 * Macro-instructions used to manage fragments
 */
#define EXT2_MIN_FRAG_SIZE		1024
#define	EXT2_MAX_FRAG_SIZE		4096
#define EXT2_MIN_FRAG_LOG_SIZE		  10
#ifdef __KERNEL__
# define EXT2_FRAG_SIZE(s)		(EXT2_SB(s)->s_frag_size)
# define EXT2_FRAGS_PER_BLOCK(s)	(EXT2_SB(s)->s_frags_per_block)
#else
# define EXT2_FRAG_SIZE(s)		(EXT2_MIN_FRAG_SIZE << (s)->s_log_frag_size)
# define EXT2_FRAGS_PER_BLOCK(s)	(EXT2_BLOCK_SIZE(s) / EXT2_FRAG_SIZE(s))
#endif

/*
 * Structure of a blocks group descriptor
 */
struct ext2_group_desc
{
	unsigned int	bg_block_bitmap;		/* Blocks bitmap block */
	unsigned int	bg_inode_bitmap;		/* Inodes bitmap block */
	unsigned int	bg_inode_table;		/* Inodes table block */
	unsigned short int	bg_free_blocks_count;	/* Free blocks count */
	unsigned short int	bg_free_inodes_count;	/* Free inodes count */
	unsigned short int	bg_used_dirs_count;	/* Directories count */
	unsigned short int	bg_pad;
	unsigned int	bg_reserved[3];
};

/*
 * Macro-instructions used to manage group descriptors
 */
#ifdef __KERNEL__
# define EXT2_BLOCKS_PER_GROUP(s)	(EXT2_SB(s)->s_blocks_per_group)
# define EXT2_DESC_PER_BLOCK(s)		(EXT2_SB(s)->s_desc_per_block)
# define EXT2_INODES_PER_GROUP(s)	(EXT2_SB(s)->s_inodes_per_group)
# define EXT2_DESC_PER_BLOCK_BITS(s)	(EXT2_SB(s)->s_desc_per_block_bits)
#else
# define EXT2_BLOCKS_PER_GROUP(s)	((s)->s_blocks_per_group)
# define EXT2_DESC_PER_BLOCK(s)		(EXT2_BLOCK_SIZE(s) / sizeof (struct ext2_group_desc))
# define EXT2_INODES_PER_GROUP(s)	((s)->s_inodes_per_group)
#endif

/*
 * Constants relative to the data blocks
 */
#define	EXT2_NDIR_BLOCKS		12
#define	EXT2_IND_BLOCK			EXT2_NDIR_BLOCKS
#define	EXT2_DIND_BLOCK			(EXT2_IND_BLOCK + 1)
#define	EXT2_TIND_BLOCK			(EXT2_DIND_BLOCK + 1)
#define	EXT2_N_BLOCKS			(EXT2_TIND_BLOCK + 1)

/*
 * Inode flags
 */
#define	EXT2_SECRM_FL			0x00000001 /* Secure deletion */
#define	EXT2_UNRM_FL			0x00000002 /* Undelete */
#define	EXT2_COMPR_FL			0x00000004 /* Compress file */
#define EXT2_SYNC_FL			0x00000008 /* Synchronous updates */
#define EXT2_IMMUTABLE_FL		0x00000010 /* Immutable file */
#define EXT2_APPEND_FL			0x00000020 /* writes to file may only append */
#define EXT2_NODUMP_FL			0x00000040 /* do not dump file */
#define EXT2_NOATIME_FL			0x00000080 /* do not update atime */
/* Reserved for compression usage... */
#define EXT2_DIRTY_FL			0x00000100
#define EXT2_COMPRBLK_FL		0x00000200 /* One or more compressed clusters */
#define EXT2_NOCOMP_FL			0x00000400 /* Don't compress */
#define EXT2_ECOMPR_FL			0x00000800 /* Compression error */
/* End compression flags --- maybe not all used */
#define EXT2_BTREE_FL			0x00001000 /* btree format dir */
#define EXT2_INDEX_FL			0x00001000 /* hash-indexed directory */
#define EXT2_IMAGIC_FL			0x00002000 /* AFS directory */
#define EXT2_JOURNAL_DATA_FL		0x00004000 /* Reserved for ext3 */
#define EXT2_NOTAIL_FL			0x00008000 /* file tail should not be merged */
#define EXT2_DIRSYNC_FL			0x00010000 /* dirsync behaviour (directories only) */
#define EXT2_TOPDIR_FL			0x00020000 /* Top of directory hierarchies*/
#define EXT2_RESERVED_FL		0x80000000 /* reserved for ext2 lib */

#define EXT2_FL_USER_VISIBLE		0x0003DFFF /* User visible flags */
#define EXT2_FL_USER_MODIFIABLE		0x000380FF /* User modifiable flags */

/*
 * Structure of an inode on the disk
 */
struct ext2_inode {
	unsigned short int	i_mode;		/* File mode */
	unsigned short int	i_uid;		/* Low 16 bits of Owner Uid */
	unsigned int	i_size;		/* Size in bytes */
	unsigned int	i_atime;	/* Access time */
	unsigned int	i_ctime;	/* Creation time */
	unsigned int	i_mtime;	/* Modification time */
	unsigned int	i_dtime;	/* Deletion Time */
	unsigned short int	i_gid;		/* Low 16 bits of Group Id */
	unsigned short int	i_links_count;	/* Links count */
	unsigned int	i_blocks;	/* Blocks count */
	unsigned int	i_flags;	/* File flags */
	union {
		struct {
			unsigned int  l_i_reserved1;
		} linux1;
		struct {
			unsigned int  h_i_translator;
		} hurd1;
		struct {
			unsigned int  m_i_reserved1;
		} masix1;
	} osd1;				/* OS dependent 1 */
	unsigned int	i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
	unsigned int	i_generation;	/* File version (for NFS) */
	unsigned int	i_file_acl;	/* File ACL */
	unsigned int	i_dir_acl;	/* Directory ACL */
	unsigned int	i_faddr;	/* Fragment address */
	union {
		struct {
			unsigned char	l_i_frag;	/* Fragment number */
			unsigned char	l_i_fsize;	/* Fragment size */
			unsigned short int	i_pad1;
			unsigned short int	l_i_uid_high;	/* these 2 fields    */
			unsigned short int	l_i_gid_high;	/* were reserved2[0] */
			unsigned int	l_i_reserved2;
		} linux2;
		struct {
			unsigned char	h_i_frag;	/* Fragment number */
			unsigned char	h_i_fsize;	/* Fragment size */
			unsigned short int	h_i_mode_high;
			unsigned short int	h_i_uid_high;
			unsigned short int	h_i_gid_high;
			unsigned int	h_i_author;
		} hurd2;
		struct {
			unsigned char	m_i_frag;	/* Fragment number */
			unsigned char	m_i_fsize;	/* Fragment size */
			unsigned short int	m_pad1;
			unsigned int	m_i_reserved2[2];
		} masix2;
	} osd2;				/* OS dependent 2 */
};

#define i_size_high	i_dir_acl

#if defined(__KERNEL__) || defined(__linux__)
#define i_reserved1	osd1.linux1.l_i_reserved1
#define i_frag		osd2.linux2.l_i_frag
#define i_fsize		osd2.linux2.l_i_fsize
#define i_uid_low	i_uid
#define i_gid_low	i_gid
#define i_uid_high	osd2.linux2.l_i_uid_high
#define i_gid_high	osd2.linux2.l_i_gid_high
#define i_reserved2	osd2.linux2.l_i_reserved2
#endif

#ifdef	__hurd__
#define i_translator	osd1.hurd1.h_i_translator
#define i_frag		osd2.hurd2.h_i_frag;
#define i_fsize		osd2.hurd2.h_i_fsize;
#define i_uid_high	osd2.hurd2.h_i_uid_high
#define i_gid_high	osd2.hurd2.h_i_gid_high
#define i_author	osd2.hurd2.h_i_author
#endif

#ifdef	__masix__
#define i_reserved1	osd1.masix1.m_i_reserved1
#define i_frag		osd2.masix2.m_i_frag
#define i_fsize		osd2.masix2.m_i_fsize
#define i_reserved2	osd2.masix2.m_i_reserved2
#endif

/*
 * File system states
 */
#define	EXT2_VALID_FS			0x0001	/* Unmounted cleanly */
#define	EXT2_ERROR_FS			0x0002	/* Errors detected */

/*
 * Mount flags
 */
#define EXT2_MOUNT_CHECK		0x0001	/* Do mount-time checks */
#define EXT2_MOUNT_OLDALLOC		0x0002  /* Don't use the new Orlov allocator */
#define EXT2_MOUNT_GRPID		0x0004	/* Create files with directory's group */
#define EXT2_MOUNT_DEBUG		0x0008	/* Some debugging messages */
#define EXT2_MOUNT_ERRORS_CONT		0x0010	/* Continue on errors */
#define EXT2_MOUNT_ERRORS_RO		0x0020	/* Remount fs ro on errors */
#define EXT2_MOUNT_ERRORS_PANIC		0x0040	/* Panic on errors */
#define EXT2_MOUNT_MINIX_DF		0x0080	/* Mimics the Minix statfs */
#define EXT2_MOUNT_NOBH			0x0100	/* No buffer_heads */
#define EXT2_MOUNT_NO_UID32		0x0200  /* Disable 32-bit UIDs */
#define EXT2_MOUNT_XATTR_USER		0x4000	/* Extended user attributes */
#define EXT2_MOUNT_POSIX_ACL		0x8000	/* POSIX Access Control Lists */

#define clear_opt(o, opt)		o &= ~EXT2_MOUNT_##opt
#define set_opt(o, opt)			o |= EXT2_MOUNT_##opt
#define test_opt(sb, opt)		(EXT2_SB(sb)->s_mount_opt & \
					 EXT2_MOUNT_##opt)
/*
 * Maximal mount counts between two filesystem checks
 */
#define EXT2_DFL_MAX_MNT_COUNT		20	/* Allow 20 mounts */
#define EXT2_DFL_CHECKINTERVAL		0	/* Don't use interval check */

/*
 * Behaviour when detecting errors
 */
#define EXT2_ERRORS_CONTINUE		1	/* Continue execution */
#define EXT2_ERRORS_RO			2	/* Remount fs read-only */
#define EXT2_ERRORS_PANIC		3	/* Panic */
#define EXT2_ERRORS_DEFAULT		EXT2_ERRORS_CONTINUE

/*
 * Structure of the super block
 */
struct ext2_super_block {
	unsigned int	s_inodes_count;		/* Inodes count */
	unsigned int	s_blocks_count;		/* Blocks count */
	unsigned int	s_r_blocks_count;	/* Reserved blocks count */
	unsigned int	s_free_blocks_count;	/* Free blocks count */
	unsigned int	s_free_inodes_count;	/* Free inodes count */
	unsigned int	s_first_data_block;	/* First Data Block */
	unsigned int	s_log_block_size;	/* Block size */
	unsigned int	s_log_frag_size;	/* Fragment size */
	unsigned int	s_blocks_per_group;	/* # Blocks per group */
	unsigned int	s_frags_per_group;	/* # Fragments per group */
	unsigned int	s_inodes_per_group;	/* # Inodes per group */
	unsigned int	s_mtime;		/* Mount time */
	unsigned int	s_wtime;		/* Write time */
	unsigned short int	s_mnt_count;		/* Mount count */
	unsigned short int	s_max_mnt_count;	/* Maximal mount count */
	unsigned short int	s_magic;		/* Magic signature */
	unsigned short int	s_state;		/* File system state */
	unsigned short int	s_errors;		/* Behaviour when detecting errors */
	unsigned short int	s_minor_rev_level; 	/* minor revision level */
	unsigned int	s_lastcheck;		/* time of last check */
	unsigned int	s_checkinterval;	/* max. time between checks */
	unsigned int	s_creator_os;		/* OS */
	unsigned int	s_rev_level;		/* Revision level */
	unsigned short int	s_def_resuid;		/* Default uid for reserved blocks */
	unsigned short int	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 *
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	unsigned int	s_first_ino; 		/* First non-reserved inode */
	unsigned short int   s_inode_size; 		/* size of inode structure */
	unsigned short int	s_block_group_nr; 	/* block group # of this superblock */
	unsigned int	s_feature_compat; 	/* compatible feature set */
	unsigned int	s_feature_incompat; 	/* incompatible feature set */
	unsigned int	s_feature_ro_compat; 	/* readonly-compatible feature set */
	unsigned char	s_uuid[16];		/* 128-bit uuid for volume */
	char	s_volume_name[16]; 	/* volume name */
	char	s_last_mounted[64]; 	/* directory where last mounted */
	unsigned int	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	unsigned char	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	unsigned char	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	unsigned short int	s_padding1;
	/*
	 * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	unsigned char	s_journal_uuid[16];	/* uuid of journal superblock */
	unsigned int	s_journal_inum;		/* inode number of journal file */
	unsigned int	s_journal_dev;		/* device number of journal file */
	unsigned int	s_last_orphan;		/* start of list of inodes to delete */
	unsigned int	s_hash_seed[4];		/* HTREE hash seed */
	unsigned char	s_def_hash_version;	/* Default hash version to use */
	unsigned char	s_reserved_char_pad;
	unsigned short int	s_reserved_word_pad;
	unsigned int	s_default_mount_opts;
 	unsigned int	s_first_meta_bg; 	/* First metablock block group */
	unsigned int	s_reserved[190];	/* Padding to the end of the block */
};

/*
 * Codes for operating systems
 */
#define EXT2_OS_LINUX		0
#define EXT2_OS_HURD		1
#define EXT2_OS_MASIX		2
#define EXT2_OS_FREEBSD		3
#define EXT2_OS_LITES		4

/*
 * Revision levels
 */
#define EXT2_GOOD_OLD_REV	0	/* The good old (original) format */
#define EXT2_DYNAMIC_REV	1 	/* V2 format w/ dynamic inode sizes */

#define EXT2_CURRENT_REV	EXT2_GOOD_OLD_REV
#define EXT2_MAX_SUPP_REV	EXT2_DYNAMIC_REV

#define EXT2_GOOD_OLD_INODE_SIZE 128

/*
 * Feature set definitions
 */

#define EXT2_HAS_COMPAT_FEATURE(sb,mask)			\
	( EXT2_SB(sb)->s_es->s_feature_compat & cpu_to_le32(mask) )
#define EXT2_HAS_RO_COMPAT_FEATURE(sb,mask)			\
	( EXT2_SB(sb)->s_es->s_feature_ro_compat & cpu_to_le32(mask) )
#define EXT2_HAS_INCOMPAT_FEATURE(sb,mask)			\
	( EXT2_SB(sb)->s_es->s_feature_incompat & cpu_to_le32(mask) )
#define EXT2_SET_COMPAT_FEATURE(sb,mask)			\
	EXT2_SB(sb)->s_es->s_feature_compat |= cpu_to_le32(mask)
#define EXT2_SET_RO_COMPAT_FEATURE(sb,mask)			\
	EXT2_SB(sb)->s_es->s_feature_ro_compat |= cpu_to_le32(mask)
#define EXT2_SET_INCOMPAT_FEATURE(sb,mask)			\
	EXT2_SB(sb)->s_es->s_feature_incompat |= cpu_to_le32(mask)
#define EXT2_CLEAR_COMPAT_FEATURE(sb,mask)			\
	EXT2_SB(sb)->s_es->s_feature_compat &= ~cpu_to_le32(mask)
#define EXT2_CLEAR_RO_COMPAT_FEATURE(sb,mask)			\
	EXT2_SB(sb)->s_es->s_feature_ro_compat &= ~cpu_to_le32(mask)
#define EXT2_CLEAR_INCOMPAT_FEATURE(sb,mask)			\
	EXT2_SB(sb)->s_es->s_feature_incompat &= ~cpu_to_le32(mask)

#define EXT2_FEATURE_COMPAT_DIR_PREALLOC	0x0001
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES	0x0002
#define EXT3_FEATURE_COMPAT_HAS_JOURNAL		0x0004
#define EXT2_FEATURE_COMPAT_EXT_ATTR		0x0008
#define EXT2_FEATURE_COMPAT_RESIZE_INO		0x0010
#define EXT2_FEATURE_COMPAT_DIR_INDEX		0x0020
#define EXT2_FEATURE_COMPAT_ANY			0xffffffff

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER	0x0001
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE	0x0002
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR	0x0004
#define EXT2_FEATURE_RO_COMPAT_ANY		0xffffffff

#define EXT2_FEATURE_INCOMPAT_COMPRESSION	0x0001
#define EXT2_FEATURE_INCOMPAT_FILETYPE		0x0002
#define EXT3_FEATURE_INCOMPAT_RECOVER		0x0004
#define EXT3_FEATURE_INCOMPAT_JOURNAL_DEV	0x0008
#define EXT2_FEATURE_INCOMPAT_META_BG		0x0010
#define EXT2_FEATURE_INCOMPAT_ANY		0xffffffff

#define EXT2_FEATURE_COMPAT_SUPP	EXT2_FEATURE_COMPAT_EXT_ATTR
#define EXT2_FEATURE_INCOMPAT_SUPP	(EXT2_FEATURE_INCOMPAT_FILETYPE| \
					 EXT2_FEATURE_INCOMPAT_META_BG)
#define EXT2_FEATURE_RO_COMPAT_SUPP	(EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER| \
					 EXT2_FEATURE_RO_COMPAT_LARGE_FILE| \
					 EXT2_FEATURE_RO_COMPAT_BTREE_DIR)
#define EXT2_FEATURE_RO_COMPAT_UNSUPPORTED	~EXT2_FEATURE_RO_COMPAT_SUPP
#define EXT2_FEATURE_INCOMPAT_UNSUPPORTED	~EXT2_FEATURE_INCOMPAT_SUPP

/*
 * Default values for user and/or group using reserved blocks
 */
#define	EXT2_DEF_RESUID		0
#define	EXT2_DEF_RESGID		0

/*
 * Default mount options
 */
#define EXT2_DEFM_DEBUG		0x0001
#define EXT2_DEFM_BSDGROUPS	0x0002
#define EXT2_DEFM_XATTR_USER	0x0004
#define EXT2_DEFM_ACL		0x0008
#define EXT2_DEFM_UID16		0x0010
    /* Not used by ext2, but reserved for use by ext3 */
#define EXT3_DEFM_JMODE		0x0060
#define EXT3_DEFM_JMODE_DATA	0x0020
#define EXT3_DEFM_JMODE_ORDERED	0x0040
#define EXT3_DEFM_JMODE_WBACK	0x0060

/*
 * Structure of a directory entry
 */
#define EXT2_NAME_LEN 255

struct ext2_dir_entry {
	unsigned int	inode;			/* Inode number */
	unsigned short int	rec_len;		/* Directory entry length */
	unsigned short int	name_len;		/* Name length */
	char	name[EXT2_NAME_LEN];	/* File name */
};

/*
 * The new version of the directory entry.  Since EXT2 structures are
 * stored in intel byte order, and the name_len field could never be
 * bigger than 255 chars, it's safe to reclaim the extra byte for the
 * file_type field.
 */
struct ext2_dir_entry_2 {
	unsigned int	inode;			/* Inode number */
	unsigned short int	rec_len;		/* Directory entry length */
	unsigned char	name_len;		/* Name length */
	unsigned char	file_type;
	char	name[EXT2_NAME_LEN];	/* File name */
};


/*
 * Ext2 directory file types.  Only the low 3 bits are used.  The
 * other bits are reserved for now.
 */
enum {
	EXT2_FT_UNKNOWN,
	EXT2_FT_REG_FILE,
	EXT2_FT_DIR,
	EXT2_FT_CHRDEV,
	EXT2_FT_BLKDEV,
	EXT2_FT_FIFO,
	EXT2_FT_SOCK,
	EXT2_FT_SYMLINK,
	EXT2_FT_MAX
};

/*
 * EXT2_DIR_PAD defines the directory entries boundaries
 *
 * NOTE: It must be a multiple of 4
 */
#define EXT2_DIR_PAD		 	4
#define EXT2_DIR_ROUND 			(EXT2_DIR_PAD - 1)
#define EXT2_DIR_REC_LEN(name_len)	(((name_len) + 8 + EXT2_DIR_ROUND) & \
					 ~EXT2_DIR_ROUND)

struct ext2_fs {
	struct fs std;

	struct ext2_super_block * super_block;
	struct ext2_group_desc * group_desc;
	char * ext2_inode_bitmap;
	char * ext2_block_bitmap;
	unsigned int inode_bitmap_read;
	unsigned int block_bitmap_read;
	unsigned int group_desc_n;
	unsigned short int block_size;
	unsigned int refs_open;
	uint_t mode;
	struct spinlock lock;

	FILE *device;
};

struct ext2_file {
	FILE std;

	struct ext2_inode *inode;
	int inode_num;
	uint_t mode;
	struct ext2_fs *fs;
};

struct ext2_dir {
	DIR std;

	struct ext2_inode *inode;
	int directory_inode;
	struct ext2_fs *fs;
	int inode_num;

	char * buffer;
	int pos, block;
};

extern struct fs *ext2_mount(FILE *device, uint_t mode);

int ext2_umount(struct ext2_fs *this);

void *ext2_fopen(struct ext2_fs *this, const char * filename, uint_t mode);
int ext2_fclose(struct ext2_file *stream);

size_t ext2_fread(void *buf, size_t size, size_t count, struct ext2_file *stream);
size_t ext2_fwrite(void *buf, size_t size, size_t count, struct ext2_file *stream);

int ext2_fflush(struct ext2_file *stream);
long ext2_ftell(struct ext2_file *stream);
int ext2_fseek(struct ext2_file *stream, long int offset, int origin);

int ext2_fsetpos(struct ext2_file *stream, const fpos_t *pos);
int ext2_ioctl(struct ext2_file *f, int request, uintptr_t param);

int ext2_dmake(struct ext2_fs * this, const char * dirname, uint_t owned, uint_t rights);
DIR *ext2_dopen(struct ext2_fs * this, const char * dirname);
int ext2_dread(struct ext2_dir * listing);
int ext2_dclose(struct ext2_dir *listing);

#endif	/* _EXT2_FS_H */
