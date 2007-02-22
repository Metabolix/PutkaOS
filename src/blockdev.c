#include "blockdev.h"
#include <string.h>
#include <screen.h>
#include <malloc.h>

int blockdev_getblock(BD_FILE *device);

struct filefunc blockdev_func = {
	(fopen_t)   blockdev_fopen,
	(fclose_t)  blockdev_fclose,
	(fread_t)   blockdev_fread,
	(fwrite_t)  blockdev_fwrite,
	(fflush_t)  blockdev_fflush,
	(ftell_t)   blockdev_ftell,
	(fseek_t)   blockdev_fseek,
	(fgetpos_t) blockdev_fgetpos,
	(fsetpos_t) blockdev_fsetpos
};

/* TODO: Tuki yli 4 Gt laitteille */
int blockdev_fgetpos(BD_FILE *device, fpos_t *pos)
{
	pos->lo_dword =
		+ device->pos_in_block
		+ device->block_in_dev * device->phys->block_size;
	pos->hi_dword = 0;
	*pos = device->std.pos;
	return 0;
}

int blockdev_fsetpos(BD_FILE *device, const fpos_t *pos)
{
	size_t block, pos_in;
	pos_in = pos->lo_dword % device->phys->block_size;
	block = pos->lo_dword / device->phys->block_size;
	if (block > device->phys->block_count) {
		return -1;
	}
	if (block != device->block_in_dev) {
		blockdev_fflush(device);
		device->block_in_dev = block;
		device->has_read = 0;
	}
	device->pos_in_block = pos_in;
	return 0;
}

int blockdev_fseek(BD_FILE *device, long offset, int origin)
{
	size_t block, pos_in;
	switch (origin) {
		case SEEK_SET:
			block = offset / device->phys->block_size;
			pos_in = offset % device->phys->block_size;
			break;
		case SEEK_END:
			offset = (device->phys->block_count * device->phys->block_size) - offset;
			block = offset / device->phys->block_size;
			pos_in = offset % device->phys->block_size;
			break;
		case SEEK_CUR:
			offset += device->pos_in_block + (device->block_in_dev * device->phys->block_size);
			block = offset / device->phys->block_size;
			pos_in = offset % device->phys->block_size;
			break;
		default:
			return -1;
	}
	if (block > device->phys->block_count) {
		return -1;
	}

	device->std.pos.hi_dword = 0;
	device->std.pos.lo_dword = offset;

	if (block != device->block_in_dev) {
		blockdev_fflush(device);
		device->block_in_dev = block;
		device->has_read = 0;
	}
	device->pos_in_block = pos_in;
	return 0;
}

long blockdev_ftell(BD_FILE *device)
{
	return device->block_in_dev * device->phys->block_size + device->pos_in_block;
}

int blockdev_getblock(BD_FILE *device)
{
	int retval;
	blockdev_fflush(device);
	device->has_read = 0;
	if (device->block_in_dev >= device->phys->block_count) {
		return EOF;
	}
	retval = device->phys->read_block(device->phys, device->block_in_dev, device->buffer);
	if (!retval) {
		device->has_read = 1;
	}
	return retval;
}

void blockdev_fflush(BD_FILE *device)
{
	if (device->has_read && device->has_written) {
		if (device->phys->write_block(device->phys, device->block_in_dev, device->buffer)) {
			kprintf("blockdev_fflush: kirjoitus laitteeseen %s kohtaan %#010x ei onnistunut.\n", device->phys->std.name, device->block_in_dev);
		}
		device->has_written = 0;
	}
}

/* TODO: Mode */
BD_FILE *blockdev_fopen(BD_DEVICE *phys, uint_t mode)
{
	BD_FILE *retval;

	if (!phys ||
	(phys->std.dev_class != DEV_CLASS_BLOCK && phys->std.dev_class != DEV_CLASS_OTHER) ||
	phys->std.dev_type == DEV_TYPE_NONE ||
	phys->std.dev_type == DEV_TYPE_ERROR) {
		return 0;
	}

	retval = kcalloc(1, sizeof(BD_FILE) + phys->block_size);
	if (!retval) {
		return 0;
	}
	retval->std.func = &blockdev_func;
	retval->phys = phys;

	retval->buffer = (char*)retval + sizeof(BD_FILE);

	return retval;
}

void blockdev_fclose(BD_FILE *device)
{
	blockdev_fflush(device);
	kfree(device);
}

size_t blockdev_fread(void *buffer, size_t size, size_t count, BD_FILE *device)
{
#define POS_MUUTOS (blockdev_ftell(device) - pos_aluksi)
	char *buf = (char*)buffer;
	size_t tavuja_yhteensa, kokonaisia_paloja, lue;
	size_t pos_aluksi;
	pos_aluksi = blockdev_ftell(device);

	// Voidaan lukea suoraan laitteelta annettuun bufferiin
	tavuja_yhteensa = size * count;
	if (tavuja_yhteensa < device->phys->block_size) {
		kokonaisia_paloja = 0;
	} else {
		kokonaisia_paloja = (tavuja_yhteensa - (device->phys->block_size - device->pos_in_block)) / device->phys->block_size;
	}

	if (!kokonaisia_paloja || device->pos_in_block) {
		// Jos se ei ole vielä muistissa
		if (!device->has_read) {
			if (blockdev_getblock(device)) {
				return 0;
			}
		}

		// Otetaan puskurista sen verran, kuin saadaan... Kaikkiko?
		lue = device->phys->block_size - device->pos_in_block;
		if (lue >= tavuja_yhteensa) {
			if (lue == tavuja_yhteensa) {
				memcpy(buffer, device->buffer + device->pos_in_block, tavuja_yhteensa);
				blockdev_fflush(device);

				++device->block_in_dev;
				device->pos_in_block = 0;
				device->has_read = 0;

				return count;
			}
			memcpy(buffer, device->buffer + device->pos_in_block, tavuja_yhteensa);
			device->pos_in_block += tavuja_yhteensa;

			return count;
		}
		memcpy(buf, device->buffer, lue);
		buf += lue;

		blockdev_fflush(device);

		++device->block_in_dev;
		device->pos_in_block = 0;
	}

	device->has_read = 0;
	while (kokonaisia_paloja) {
		if (device->phys->read_block(device->phys, device->block_in_dev, buf)) {
			kprintf("dread: luku laittesta %s kohdasta %#010x ei onnistunut.\n", device->phys->std.name, device->block_in_dev);
			return POS_MUUTOS / size;
		}
		++device->block_in_dev;
		if (device->block_in_dev >= device->phys->block_count) {
			kprintf("dread: levy loppui.\n");
			return POS_MUUTOS / size;
		}
		buf += device->phys->block_size;
		--kokonaisia_paloja;
	}

	lue = tavuja_yhteensa - POS_MUUTOS;
	if (lue) {
		if (blockdev_getblock(device)) {
			kprintf("dread: luku laittesta %s kohdasta %#010x ei onnistunut.\n", device->phys->std.name, device->block_in_dev);
			return POS_MUUTOS / size;
		}
		memcpy(buf, device->buffer, lue);
		device->pos_in_block += lue;
	}

	return POS_MUUTOS / size;
}

size_t blockdev_fwrite(const void *buffer, size_t size, size_t count, BD_FILE *device)
{
	char *buf = (char*)buffer;
	size_t tavuja_yhteensa, kokonaisia_paloja, kirjoita;

	size_t pos_aluksi;
	pos_aluksi = blockdev_ftell(device);

	// Voidaan lukea suoraan laitteelta annettuun bufferiin
	tavuja_yhteensa = size * count;
	if (tavuja_yhteensa < device->phys->block_size) {
		kokonaisia_paloja = 0;
	} else {
		if (device->pos_in_block) {
			kokonaisia_paloja = (tavuja_yhteensa + device->pos_in_block) / device->phys->block_size - 1;
		} else {
			kokonaisia_paloja = tavuja_yhteensa / device->phys->block_size;
		}
	}

	if (!kokonaisia_paloja || device->pos_in_block) {
		// Jos se ei ole vielä muistissa
		if (!device->has_read) {
			if (blockdev_getblock(device)) {
				return 0;
			}
		}

		// Otetaan puskurista sen verran, kuin saadaan... Kaikkiko?
		kirjoita = device->phys->block_size - device->pos_in_block;
		if (kirjoita >= tavuja_yhteensa) {
			if (kirjoita == tavuja_yhteensa) {
				// Jos sattuu juuri kohdalleen, kirjoitellaan ja valitaan seuraava sektori
				memcpy(device->buffer + device->pos_in_block, buffer, tavuja_yhteensa);
				device->has_written = 1;
				blockdev_fflush(device);

				++device->block_in_dev;
				device->pos_in_block = 0;
				device->has_read = 0;

				return count;
			}
			// Kirjoitellaan puskuriin vain.
			memcpy(device->buffer + device->pos_in_block, buffer, tavuja_yhteensa);
			device->pos_in_block += tavuja_yhteensa;
			device->has_written = 1;
			return count;
		}
		// Kirjoitellaan sen verran, kuin mahtuu
		memcpy(device->buffer + device->pos_in_block, buf, kirjoita);
		buf += kirjoita;
		device->has_written = 1;
		blockdev_fflush(device);

		++device->block_in_dev;
		device->pos_in_block = 0;
	}

	device->has_read = 0;

	while (kokonaisia_paloja) {
		if (device->phys->write_block(device->phys, device->block_in_dev, buf)) {
			kprintf("dwrite: kirjoitus laitteeseen %s kohtaan %#010x ei onnistunut.\n", device->phys->std.name, device->block_in_dev);
			return POS_MUUTOS / size;
		}
		++device->block_in_dev;
		if (device->block_in_dev >= device->phys->block_count) {
			kprintf("dwrite: tila loppui.\n");
			return POS_MUUTOS / size;
		}
		buf += device->phys->block_size;
		--kokonaisia_paloja;
	}

	// Jos on jäljellä, lataillaan se seuraava blokki
	kirjoita = tavuja_yhteensa - POS_MUUTOS;
	if (kirjoita) {
		if (blockdev_getblock(device)) {
			kprintf("dwrite: luku laittesta %s kohdasta %#010x ei onnistunut.\n", device->phys->std.name, device->block_in_dev);
			return POS_MUUTOS / size;
		}
		memcpy(device->buffer, buf, kirjoita);
		device->pos_in_block += kirjoita;
		device->has_written = 1;
	}

	return POS_MUUTOS / size;
}
