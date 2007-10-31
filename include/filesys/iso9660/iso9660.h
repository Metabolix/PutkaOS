#ifndef _ISO9660_FS_H
#define _ISO9660_FS_H 1

#include <filesys/filesystem.h>

#ifndef BIG_ENDIAN
#define iso9660_dbl_uint32_t(x) uint32_t x , x ## _wrong_endian
#define iso9660_dbl_uint16_t(x) uint16_t x , x ## _wrong_endian
#else
#define iso9660_dbl_uint32_t(x) uint32_t x ## _wrong_endian , x
#define iso9660_dbl_uint16_t(x) uint16_t x ## _wrong_endian , x
#endif

struct iso9660_primary_descriptor {
	uint8_t type, id[5], version, _unused01;
	char system_id[32], volume_id[32];
	char _unused02[8];
	iso9660_dbl_uint32_t (volume_space_size);
	char _unused03[32];
	iso9660_dbl_uint16_t (volume_set_size);
	iso9660_dbl_uint16_t (volume_sequence_number);
	iso9660_dbl_uint16_t (logical_block_size);
	iso9660_dbl_uint32_t (path_table_size);
	uint32_t type_l_path_table, opt_type_l_path_table;
	uint32_t type_m_path_table, opt_type_m_path_table;
	// TODO
};

#endif
