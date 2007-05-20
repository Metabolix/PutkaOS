#include <devices/blockdev/blockdev.h>
#include <string.h>
#include <malloc.h>
#include <int64.h>

#include <screen.h>

static int blockdev_getblock(BD_FILE *device);
static void blockdev_update_pos(BD_FILE *device);

int blockdev_dummy_read_one_block(BD_DEVICE *self, uint64_t num, void * buf);
int blockdev_dummy_write_one_block(BD_DEVICE *self, uint64_t num, const void * buf);
size_t blockdev_dummy_read_blocks(BD_DEVICE *self, uint64_t num, size_t count, void * buf);
size_t blockdev_dummy_write_blocks(BD_DEVICE *self, uint64_t num, size_t count, const void * buf);

struct filefunc blockdev_func = {
	(fopen_t)   blockdev_fopen,
	(fclose_t)  blockdev_fclose,
	(fread_t)   blockdev_fread,
	(fwrite_t)  blockdev_fwrite,
	(fflush_t)  blockdev_fflush,
	(fgetpos_t) blockdev_fgetpos,
	(fsetpos_t) blockdev_fsetpos
};

void blockdev_update_pos(BD_FILE *device)
{
	device->std.pos =
		+ device->pos_in_block
		+ device->block_in_dev * device->phys->block_size;
}

int blockdev_fgetpos(BD_FILE *device, fpos_t *pos)
{
	*pos = device->std.pos;
	return 0;
}

int blockdev_fsetpos(BD_FILE *device, const fpos_t *pos)
{
	uint64_t block, pos_in;
	block = uint64_div_rem(*pos, device->phys->block_size, &pos_in);
	if (block > device->phys->block_count) {
		return -1;
	}
	if (block == device->phys->block_count) {
		if (pos_in > 0) {
			return -1;
		} else {
			device->std.eof = EOF;
		}
	}
	if (block != device->block_in_dev) {
		blockdev_fflush(device);
		device->block_in_dev = block;
		device->has_read = 0;
	}
	device->pos_in_block = pos_in;
	device->std.pos = *pos;
	return 0;
}

int blockdev_getblock(BD_FILE *device)
{
	int retval;
	blockdev_fflush(device);
	device->has_read = 0;
	if (device->block_in_dev >= device->phys->block_count) {
		return EOF;
	}
	retval = device->phys->read_one_block(device->phys, device->phys->first_block_num + device->block_in_dev, device->buffer);
	if (!retval) {
		device->has_read = 1;
	}
	return retval;
}

int blockdev_fflush(BD_FILE *device)
{
	if (device->has_read && device->has_written) {
		if (device->phys->write_one_block(device->phys, device->phys->first_block_num + device->block_in_dev, device->buffer)) {
			kprintf("blockdev_fflush: kirjoitus laitteeseen %s kohtaan %#010x ei onnistunut.\n", device->phys->std.name, device->block_in_dev);
			return -1;
		}
		device->has_written = 0;
	}
	return 0;
}

/* TODO: Mode */
BD_FILE *blockdev_fopen(BD_DEVICE *phys, uint_t mode)
{
	BD_FILE *retval;

	if (!phys ||
	(phys->std.dev_class != DEV_CLASS_BLOCK && phys->std.dev_class != DEV_CLASS_OTHER) ||
	phys->std.dev_type == DEV_TYPE_NONE ||
	phys->std.dev_type == DEV_TYPE_ERROR ||
	!phys->block_size) {
		return 0;
	}

	if (!phys->write_one_block) {
		if (!phys->write_blocks) {
			if (mode & FILE_MODE_WRITE) {
				return 0;
			}
		} else {
			phys->write_one_block = blockdev_dummy_write_one_block;
		}
	} else {
		if (!phys->write_blocks) {
			phys->write_blocks = blockdev_dummy_write_blocks;
		}
	}
	if (!phys->read_one_block) {
		if (!phys->read_blocks) {
			if (mode & FILE_MODE_READ) {
				return 0;
			}
		} else {
			phys->read_one_block = blockdev_dummy_read_one_block;
		}
	} else {
		if (!phys->read_blocks) {
			phys->read_blocks = blockdev_dummy_read_blocks;
		}
	}

	retval = kcalloc(1, sizeof(BD_FILE) + phys->block_size);
	if (!retval) {
		return 0;
	}
	retval->std.size = phys->block_size * phys->block_count;
	retval->std.func = &blockdev_func;
	retval->phys = phys;

	retval->buffer = (char*)retval + sizeof(BD_FILE);

	return retval;
}

int blockdev_fclose(BD_FILE *device)
{
	int retval;
	retval = blockdev_fflush(device);
	kfree(device);
	return retval;
}

size_t blockdev_fread(void *buffer, size_t size, size_t count, BD_FILE *device)
{
#define POS_MUUTOS (device->std.pos - pos_aluksi)
#define RETURN blockdev_update_pos(device); return
	char *buf = (char*)buffer;
	uint64_t tavuja_yhteensa, kokonaisia_paloja;
	size_t lue;
	fpos_t pos_aluksi = device->std.pos;

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
				RETURN 0;
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

				RETURN count;
			}
			memcpy(buffer, device->buffer + device->pos_in_block, tavuja_yhteensa);
			device->pos_in_block += tavuja_yhteensa;

			RETURN count;
		}
		memcpy(buf, device->buffer, lue);
		buf += lue;

		blockdev_fflush(device);

		++device->block_in_dev;
		device->pos_in_block = 0;
	}

	device->has_read = 0;
	while (kokonaisia_paloja) {
		if (device->phys->read_one_block(device->phys, device->block_in_dev, buf)) {
			kprintf("dread: luku laittesta %s kohdasta %#010x ei onnistunut.\n", device->phys->std.name, device->block_in_dev);
			RETURN POS_MUUTOS / size;
		}
		++device->block_in_dev;
		if (device->block_in_dev >= device->phys->block_count) {
			kprintf("dread: levy loppui.\n");
			RETURN POS_MUUTOS / size;
		}
		buf += device->phys->block_size;
		--kokonaisia_paloja;
	}

	lue = tavuja_yhteensa - POS_MUUTOS;
	if (lue) {
		if (blockdev_getblock(device)) {
			kprintf("dread: luku laittesta %s kohdasta %#010x ei onnistunut.\n", device->phys->std.name, device->block_in_dev);
			RETURN POS_MUUTOS / size;
		}
		memcpy(buf, device->buffer, lue);
		device->pos_in_block += lue;
	}

	RETURN POS_MUUTOS / size;
}

size_t blockdev_fwrite(const void *buffer, size_t size, size_t count, BD_FILE *device)
{
	char *buf = (char*)buffer;
	uint64_t tavuja_yhteensa, kokonaisia_paloja;
	size_t kirjoita;
	fpos_t pos_aluksi = device->std.pos;

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
				RETURN 0;
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

				RETURN count;
			}
			// Kirjoitellaan puskuriin vain.
			memcpy(device->buffer + device->pos_in_block, buffer, tavuja_yhteensa);
			device->pos_in_block += tavuja_yhteensa;
			device->has_written = 1;
			RETURN count;
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
		if (device->phys->write_one_block(device->phys, device->block_in_dev, buf)) {
			kprintf("dwrite: kirjoitus laitteeseen %s kohtaan %#010x ei onnistunut.\n", device->phys->std.name, device->block_in_dev);
			RETURN POS_MUUTOS / size;
		}
		++device->block_in_dev;
		if (device->block_in_dev >= device->phys->block_count) {
			kprintf("dwrite: tila loppui.\n");
			RETURN POS_MUUTOS / size;
		}
		buf += device->phys->block_size;
		--kokonaisia_paloja;
	}

	// Jos on jäljellä, lataillaan se seuraava blokki
	kirjoita = tavuja_yhteensa - POS_MUUTOS;
	if (kirjoita) {
		if (blockdev_getblock(device)) {
			kprintf("dwrite: luku laittesta %s kohdasta %#010x ei onnistunut.\n", device->phys->std.name, device->block_in_dev);
			RETURN POS_MUUTOS / size;
		}
		memcpy(device->buffer, buf, kirjoita);
		device->pos_in_block += kirjoita;
		device->has_written = 1;
	}

	RETURN POS_MUUTOS / size;
}

int blockdev_dummy_read_one_block(BD_DEVICE *self, uint64_t num, void * buf)
{
	return self->read_blocks(self, num, 1, buf) ? 0 : -1;
}

int blockdev_dummy_write_one_block(BD_DEVICE *self, uint64_t num, const void * buf)
{
	return self->write_blocks(self, num, 1, buf) ? 0 : -1;
}

size_t blockdev_dummy_read_blocks(BD_DEVICE *self, uint64_t num, size_t count, void * buf)
{
	size_t i;
	for (i = 0; count; ++i, --count, ++num, buf = (char*)buf + self->block_size) {
		if (self->read_one_block(self, num, buf)) {
			return i;
		}
	}
	return i;
}

size_t blockdev_dummy_write_blocks(BD_DEVICE *self, uint64_t num, size_t count, const void * buf)
{
	size_t i;
	for (i = 0; count; ++i, --count, ++num, buf = (const char*)buf + self->block_size) {
		if (self->write_one_block(self, num, buf)) {
			return i;
		}
	}
	return i;
}

