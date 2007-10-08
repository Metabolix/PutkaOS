#ifndef _MINIX_FS_MAPS_H
#define _MINIX_FS_MAPS_H 1

#include <filesys/minix/minix.h>
#include <filesys/minix/zones.h>

extern uint16_t minix_alloc_inode(struct minix_fs *fs, uint16_t type);
extern uint16_t minix_alloc_zone(struct minix_zone_allocer_t * const zal);

extern int minix_free_inode(struct minix_fs *fs, uint16_t n);
extern int minix_free_zone(struct minix_fs *fs, uint16_t n);

#endif

