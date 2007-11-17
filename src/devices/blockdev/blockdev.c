#include <devices/blockdev/blockdev.h>
#include <memory/kmalloc.h>
#include <string.h>
#include <int64.h>

#include <kprintf.h>

static int blockdev_fill_buffer(BD_FILE *f);
static size_t blockdev_freadwrite(void *buffer, size_t size, size_t count, BD_FILE *f, int write);

int blockdev_dummy_read_one_block(BD_DEVICE *dev, uint64_t num, void * buf);
int blockdev_dummy_write_one_block(BD_DEVICE *dev, uint64_t num, const void * buf);
size_t blockdev_dummy_read_blocks(BD_DEVICE *dev, uint64_t num, size_t count, void * buf);
size_t blockdev_dummy_write_blocks(BD_DEVICE *dev, uint64_t num, size_t count, const void * buf);

struct filefunc blockdev_func = {
	.fopen = (fopen_t) blockdev_fopen,
	.fclose = (fclose_t) blockdev_fclose,
	.fread = (fread_t) blockdev_fread,
	.fwrite = (fwrite_t) blockdev_fwrite,
	.fflush = (fflush_t) blockdev_fflush,
	.fsetpos = (fsetpos_t) blockdev_fsetpos,
	.ioctl = (ioctl_t) blockdev_ioctl
};

int blockdev_ioctl(BD_FILE *f, int request, uintptr_t param)
{
	if (!f->dev->refs) {
		return -1;
	}
	// TODO: blockdev_ioctl
	return -1;
}

int blockdev_fsetpos(BD_FILE *f, const fpos_t *pos)
{
	if (!f->dev->refs) {
		return EOF;
	}
	uint64_t block, pos_in;
	block = uint64_div_rem(*pos, f->dev->block_size, &pos_in);
	if (block > f->dev->block_count) {
		return -1;
	}
	if (block == f->dev->block_count) {
		if (pos_in > 0) {
			return -1;
		}
		f->std.eof = EOF;
	}
	f->block_in_file = block;
	f->pos_in_block = pos_in;
	f->std.pos = *pos;
	return 0;
}

int blockdev_fflush(BD_FILE *f)
{
	if (!f->dev->refs) {
		return EOF;
	}
	if (f->buffer_changed) {
		if (f->dev->write_one_block(f->dev, f->dev->first_block_num + f->block_in_buffer, f->buf)) {
			kprintf("blockdev_fflush: failure: device %s block %#010llx\n", f->dev->std.name, f->block_in_buffer);
			return EOF;
		}
		f->buffer_changed = 0;
	}
	return 0;
}

static int blockdev_fill_buffer(BD_FILE *f)
{
	if (f->block_in_file == f->block_in_buffer) {
		return 0;
	}
	if (f->block_in_file >= f->dev->block_count) {
		return EOF;
	}
	if (f->buffer_changed) {
		blockdev_fflush(f); // f->buffer_changed = 0;
	}
	f->block_in_buffer = BLOCKDEV_NO_BLOCK;

	if (f->dev->read_one_block(f->dev, f->dev->first_block_num + f->block_in_file, f->buf) != 0) {
		return -1;
	}
	f->block_in_buffer = f->block_in_file;
	return 0;
}

/* TODO: Mode */
BD_FILE *blockdev_fopen(BD_DEVICE *dev, uint_t mode)
{
	if (!dev ||
	(dev->std.dev_class != DEV_CLASS_BLOCK && dev->std.dev_class != DEV_CLASS_OTHER) ||
	(dev->refresh && dev->refresh(dev)) ||
	(dev->std.dev_type == DEV_TYPE_NONE || dev->std.dev_type == DEV_TYPE_ERROR) ||
	!dev->block_size) {
		return 0;
	}

	BD_FILE real = {
		.std.mode = mode,
		.std.size = dev->block_size * dev->block_count,
		.std.func = &blockdev_func,
		.dev = dev,
		.block_in_buffer = BLOCKDEV_NO_BLOCK,
		.buffer_changed = 0,
	}, *f = &real;

	if (!dev->write_one_block) {
		if (!dev->write_blocks) {
			if (mode & FILE_MODE_WRITE) {
				return 0;
			}
		} else {
			dev->write_one_block = blockdev_dummy_write_one_block;
		}
	} else {
		if (!dev->write_blocks) {
			dev->write_blocks = blockdev_dummy_write_blocks;
		}
	}
	if (!dev->read_one_block) {
		if (!dev->read_blocks) {
			if (mode & FILE_MODE_READ) {
				return 0;
			}
		} else {
			dev->read_one_block = blockdev_dummy_read_one_block;
		}
	} else {
		if (!dev->read_blocks) {
			dev->read_blocks = blockdev_dummy_read_blocks;
		}
	}

	f = kcalloc(1, sizeof(BD_FILE) + dev->block_size);
	if (!f) {
		return 0;
	}
	memcpy(f, &real, sizeof(BD_FILE));
	f->buf = (char*)f + sizeof(BD_FILE);

	++dev->refs;
	return f;
}

int blockdev_fclose(BD_FILE *f)
{
	int retval;
	BD_DEVICE *dev = f->dev;
	if (dev->refs == 0) {
		return EOF;
	}
	retval = blockdev_fflush(f);
	kfree(f);
	--dev->refs;
	return retval;
}

static size_t blockdev_freadwrite(void *buffer, size_t size, size_t count, BD_FILE *f, int write)
{
	char *buf = (char*) buffer;
	size_t tavuja = size * count;
	const fpos_t alkupos = f->std.pos;
	BD_DEVICE * const dev = f->dev;

	if (!tavuja || !dev->refs) {
		return 0;
	}

	if (f->pos_in_block) {
		const size_t jaljella = dev->block_size - f->pos_in_block;
		const size_t hoida = (tavuja < jaljella) ? tavuja : jaljella;

		if (blockdev_fill_buffer(f) != 0) {
			goto error_etc;
		}
		if (write) {
			f->buffer_changed = 1;
			memcpy(f->buf + f->pos_in_block, buf, hoida);
		} else {
			memcpy(buf, f->buf + f->pos_in_block, hoida);
		}
		buf += hoida;
		tavuja -= hoida;
		if (hoida < jaljella) {
			f->pos_in_block += hoida;
			goto return_etc;
		}
		f->pos_in_block = 0;
		++f->block_in_file;
	}

	if (tavuja >= dev->block_size) {
		const size_t maara = tavuja / (size_t)dev->block_size;
		const uint64_t block = dev->first_block_num + f->block_in_file;
		size_t onnistui;
		size_t tavuja_onnistui;

		if (write) {
			onnistui = dev->write_blocks(dev, block, maara, buf);
		} else {
			onnistui = dev->read_blocks(dev, block, maara, buf);
		}
		tavuja_onnistui = onnistui * dev->block_size;
		tavuja -= tavuja_onnistui;
		buf += tavuja_onnistui;
		f->block_in_file += onnistui;
		if (onnistui < maara) {
			goto error_etc;
		}
	}
	if (!tavuja) {
		goto return_etc;
	}
	if (blockdev_fill_buffer(f) != 0) {
		goto error_etc;
	}
	if (write) {
		f->buffer_changed = 1;
		memcpy(f->buf, buf, tavuja);
	} else {
		memcpy(buf, f->buf, tavuja);
	}
	buf += tavuja;
	f->pos_in_block += tavuja;
	tavuja = 0;
	goto return_etc;

error_etc:
return_etc:
	f->std.pos = f->pos_in_block + f->block_in_file * dev->block_size;
	return (size_t)(f->std.pos - alkupos) / size;
}

size_t blockdev_fread(void *buffer, size_t size, size_t count, BD_FILE *f)
{
	return blockdev_freadwrite((void*) buffer, size, count, f, 0);
}
size_t blockdev_fwrite(const void *buffer, size_t size, size_t count, BD_FILE *f)
{
	return blockdev_freadwrite((void*) buffer, size, count, f, 1);
}

int blockdev_dummy_read_one_block(BD_DEVICE *dev, uint64_t num, void * buf)
{
	return (dev->read_blocks(dev, num, 1, buf) == 1) ? 0 : -1;
}

int blockdev_dummy_write_one_block(BD_DEVICE *dev, uint64_t num, const void * buf)
{
	return (dev->write_blocks(dev, num, 1, buf) == 1) ? 0 : -1;
}

size_t blockdev_dummy_read_blocks(BD_DEVICE *dev, uint64_t num, size_t count, void * buf)
{
	char *ptr = buf;
	size_t i = 0;
	while (count--) {
		if (dev->read_one_block(dev, num, ptr) != 0) {
			return i;
		}
		++i;
		++num;
		ptr += dev->block_size;
	}
	return i;
}

size_t blockdev_dummy_write_blocks(BD_DEVICE *dev, uint64_t num, size_t count, const void * buf)
{
	const char *ptr = buf;
	size_t i = 0;
	while (count--) {
		if (dev->write_one_block(dev, num, ptr) != 0) {
			return i;
		}
		++i;
		++num;
		ptr += dev->block_size;
	}
	return i;
}

