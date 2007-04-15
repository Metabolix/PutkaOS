#include <storage/hdd.h>
#include <storage/ide.h>
#include <storage/blockdev.h>
#include <debugprint.h>
#include <panic.h>

void hdd_read_logicals(ide_device_t *dev, struct hdd_bootrec_entry *logical_entry, BD_FILE *f)
{
	struct hdd_bootrec bootrec;
	fpos_t pos;

	pos = logical_entry->start_lba;
	pos *= dev->blockdev.block_size;
	pos += HDD_BOOTREC_SKIP;
	if (blockdev_fsetpos(f, &pos)
	|| blockdev_fread(&bootrec, sizeof(bootrec), 1, f) != 1
	|| bootrec.xAA55 != 0xAA55) {
		DEBUGP("hdd_read_logicals: Corrupted bootrec...\n");
		return;
	}
}

void hdd_read_partitions(ide_device_t *dev)
{
	BD_FILE *f;
	struct hdd_bootrec bootrec;
	fpos_t pos;
	int i;

	if (sizeof(bootrec) != 512) {
		panic("sizeof(struct hdd_mbr) != 512, O_O");
	}

	pos = HDD_BOOTREC_SKIP;

	// MBR & kelpoisuustarkistus
	if (!(f = blockdev_fopen(&dev->blockdev, FILE_MODE_READ))) {
		DEBUGP("hdd_read_partitions: Can't open blockdev...\n");
		return;
	}

	if (blockdev_fsetpos(f, &pos)
	|| blockdev_fread(&bootrec, sizeof(bootrec), 1, f) != 1
	|| bootrec.xAA55 != 0xAA55) {
		DEBUGP("hdd_read_partitions: Corrupted bootrec...\n");
		return;
	}

	for (i = 0; i < 4; ++i) {
		if (!bootrec.osio[i].type) {
			continue;
		}
		if (bootrec.osio[i].type == 5) {
			hdd_read_logicals(dev, &bootrec.osio[i], f);
		}
	}

	blockdev_fclose(f);
}
