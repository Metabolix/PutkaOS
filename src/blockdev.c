#include "blockdev.h"
#include <string.h>

int dgetblock(BD_DESC *device);

#define DESCIEN_MAARA 1
BD_DESC descit[DESCIEN_MAARA];
int desc_kaytossa[DESCIEN_MAARA] = {0};
char yhteinen_buf[512];

int dseek(BD_DESC *device, long offset, int origin)
{
	size_t block, pos_in;
	if (origin == SEEK_SET) {
		block = offset / device->phys->block_size;
		pos_in = offset % device->phys->block_size;
	} else if (origin == SEEK_END) {
		offset = (device->phys->block_count * device->phys->block_size) - offset;
		block = offset / device->phys->block_size;
		pos_in = offset % device->phys->block_size;
	} else {
		offset += device->pos_in_block + (device->block_in_dev * device->phys->block_size);
		block = offset / device->phys->block_size;
		pos_in = offset % device->phys->block_size;
	}
	if (block > device->phys->block_count) {
		return -1;
	}
	if (block != device->block_in_dev) {
		dflush(device);
		device->block_in_dev = block;
		device->has_read = 0;
	}
	device->pos_in_block = pos_in;
	return 0;
}

long dtell(BD_DESC *device)
{
	return device->block_in_dev * device->phys->block_size + device->pos_in_block;
}

int dgetblock(BD_DESC *device)
{
	int retval;
	device->has_read = 0;
	if (device->block_in_dev == device->phys->block_count) {
		return EOD;
	}
	retval = device->phys->read_block(device->phys, device->block_in_dev, device->buffer);
	if (!retval) {
		device->has_read = 1;
	}
	return retval;
}

void dflush(BD_DESC *device)
{
	if (!device->has_read || !device->has_written) {
		return;
	}
	device->phys->write_block(device->phys, device->block_in_dev, device->buffer);
	device->has_written = 0;
}

BD_DESC *dopen(BLOCK_DEVICE *phys)
{
	int i;
	BD_DESC *retval;
	for (i = 0; i < DESCIEN_MAARA; ++i) {
		if (!desc_kaytossa[i]) {
			retval = descit + i;
			goto laita_asetukset;
		}
	}
	return 0;
laita_asetukset:
	retval->phys = phys;
	retval->block_in_dev = 0;
	retval->pos_in_block = 0;
	retval->has_read = 0;
	retval->has_written = 0;
	//retval->buffer = malloc(phys->block_size);
	retval->buffer = yhteinen_buf;
	if (!retval->buffer) {
		// free(retval);
		return 0;
	}

	desc_kaytossa[i] = 1;

	return retval;
}

void dclose(BD_DESC *device)
{
	int i;
	device->phys = 0;
	for (i = 0; i < DESCIEN_MAARA; ++i) {
		if ((descit + i) == device) {
			desc_kaytossa[i] = 0;
			return;
		}
	}
}

int dread(void *buffer, size_t size, size_t count, BD_DESC *device)
{
#define dread_READ_BYTES (dtell(device) - pos_aluksi)
	char *buf = (char*)buffer;
	size_t tavuja_yhteensa, kokonaisia_paloja, lue;
	size_t pos_aluksi;
	pos_aluksi = dtell(device);

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
			if (dgetblock(device)) {
				return 0;
			}
		}

		// Otetaan puskurista sen verran, kuin saadaan... Kaikkiko?
		lue = device->phys->block_size - device->pos_in_block;
		if (lue >= tavuja_yhteensa) {
			if (lue == tavuja_yhteensa) {
				memcpy(buffer, device->buffer + device->pos_in_block, tavuja_yhteensa);
				dflush(device);
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

		dflush(device);

		++device->block_in_dev;
		device->pos_in_block = 0;
		device->has_read = 0;
	}

	while (kokonaisia_paloja) {
		if (device->phys->read_block(device->phys, device->block_in_dev, buf)) {
			return dread_READ_BYTES / size;
		}
		++device->block_in_dev;
		buf += device->phys->block_size;
		--kokonaisia_paloja;
	}

	lue = tavuja_yhteensa - dread_READ_BYTES;
	if (lue) {
		if (dgetblock(device)) {
			return dread_READ_BYTES / size;
		}
		memcpy(buf, device->buffer, lue);
		device->pos_in_block += lue;
	}

	return dread_READ_BYTES / size;
}

int dwrite(const void *buffer, size_t size, size_t count, BD_DESC *device)
{
#define dwrite_WRITTEN_BYTES (dtell(device) - pos_aluksi)
	char *buf = (char*)buffer;
	size_t tavuja_yhteensa, kokonaisia_paloja, kirjoita;

	size_t pos_aluksi;
	pos_aluksi = dtell(device);

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
			if (dgetblock(device)) {
				return 0;
			}
		}

		// Otetaan puskurista sen verran, kuin saadaan... Kaikkiko?
		kirjoita = device->phys->block_size - device->pos_in_block;
		if (kirjoita >= tavuja_yhteensa) {
			if (kirjoita == tavuja_yhteensa) {
				memcpy(device->buffer + device->pos_in_block, buffer, tavuja_yhteensa);
				dflush(device);
				++device->block_in_dev;
				device->pos_in_block = 0;
				device->has_read = 0;
				return count;
			}
			memcpy(device->buffer + device->pos_in_block, buffer, tavuja_yhteensa);
			device->pos_in_block += tavuja_yhteensa;
			device->has_written = 1;
			return count;
		}
		memcpy(device->buffer + device->pos_in_block, buf, kirjoita);
		buf += kirjoita;
		device->has_written = 1;
		dflush(device);

		++device->block_in_dev;
		device->pos_in_block = 0;
		device->has_read = 0;
	}

	while (kokonaisia_paloja) {
		if (device->phys->write_block(device->phys, device->block_in_dev, buf)) {
			return dwrite_WRITTEN_BYTES / size;
		}
		++device->block_in_dev;
		buf += device->phys->block_size;
		--kokonaisia_paloja;
	}

	kirjoita = tavuja_yhteensa - dwrite_WRITTEN_BYTES;
	if (kirjoita) {
		if (dgetblock(device)) {
			return dwrite_WRITTEN_BYTES / size;
		}
		memcpy(device->buffer, buf, kirjoita);
		device->pos_in_block += kirjoita;
		device->has_written = 1;
	}

	return dwrite_WRITTEN_BYTES / size;
}
