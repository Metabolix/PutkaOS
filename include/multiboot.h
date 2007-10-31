#ifndef _MULTIBOOT_H
#define _MULTIBOOT_H 1
/* multiboot.h - the header for Multiboot */
/* Copyright (C) 1999, 2001  Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

/* Macros. */

/* The magic number for the Multiboot header. */
#define MULTIBOOT_HEADER_MAGIC          0x1BADB002

/* The flags for the Multiboot header. */
#ifdef __ELF__
# define MULTIBOOT_HEADER_FLAGS         0x00000003
#else
# define MULTIBOOT_HEADER_FLAGS         0x00010003
#endif

/* The magic number passed by a Multiboot-compliant boot loader. */
#define MULTIBOOT_BOOTLOADER_MAGIC      0x2BADB002

/* The size of our stack (8KB). */
#define STACK_SIZE                      0x2000

/* C symbol format. HAVE_ASM_USCORE is defined by configure. */
#ifdef HAVE_ASM_USCORE
# define EXT_C(sym)                     _ ## sym
#else
# define EXT_C(sym)                     sym
#endif

#ifndef ASM
/* Do not include here in boot.S. */

#include <stdint.h>

/* Types. */

/* The Multiboot header. */
typedef struct multiboot_header
{
	uint32_t magic;
	uint32_t flags;
	uint32_t checksum;
	uint32_t header_addr;
	uint32_t load_addr;
	uint32_t load_end_addr;
	uint32_t bss_end_addr;
	uint32_t entry_addr;
} multiboot_header_t;

/* The symbol table for a.out. */
typedef struct aout_symbol_table
{
	uint32_t tabsize;
	uint32_t strsize;
	void * addr;
	uint32_t reserved;
} aout_symbol_table_t;

/* The section header table for ELF. */
typedef struct elf_section_header_table
{
	uint32_t num;
	uint32_t entry_size;
	void * addr;
	void * str_index;
} elf_section_header_table_t;

/* The Multiboot information. */
typedef struct multiboot_info
{
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint8_t boot_device[4];
	char * cmdline;
	uint32_t mods_count;
	void * mods_addr;
	union {
		aout_symbol_table_t aout_sym;
		elf_section_header_table_t elf_sec;
	} u;
	uint32_t mmap_length;
	uint32_t mmap_addr;
} multiboot_info_t;

#define MBI_FLAG_MEMORY (1 << 0)
#define MBI_FLAG_BOOTDEV (1 << 1)
#define MBI_FLAG_CMDLINE (1 << 2)
#define MBI_FLAG_MODS (1 << 3)
#define MBI_FLAG_AOUT_SYMS (1 << 4)
#define MBI_FLAG_ELF_SECS (1 << 5)
#define MBI_FLAG_BIOS_MMAP (1 << 6)
#define MBI_FLAG_BIOS_DRIVES (1 << 7)
#define MBI_FLAG_CONFIG_TABLE (1 << 8)
#define MBI_FLAG_BOOT_LOADER_NAME (1 << 9)
#define MBI_FLAG_APM (1 << 10)
#define MBI_FLAG_VBE (1 << 11)

/* The module structure. */
typedef struct module
{
	uint32_t mod_start;
	uint32_t mod_end;
	uint32_t string;
	uint32_t reserved;
} module_t;

/* The memory map. Be careful that the offset 0 is base_addr_low
but no size. */
typedef struct memory_map
{
	uint32_t size;
	uint32_t base_addr_low;
	uint32_t base_addr_high;
	uint32_t length_low;
	uint32_t length_high;
	uint32_t type;
} memory_map_t;

#endif /* ! ASM */

#endif
