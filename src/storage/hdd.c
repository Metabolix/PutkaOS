#include <storage/hdd.h>
#include <storage/ide.h>
#include <storage/blockdev.h>
#include <debugprint.h>
#include <panic.h>
#include <string.h>
#include <malloc.h>

void hdd_free_partition(hdd_partdev_t *part)
{
	kfree(part);
}

static void hdd_partition_found(ide_device_t *dev, struct hdd_bootrec_entry *entry, int pnum)
{
	int len;
	char *nimi;

	hdd_partdev_t *newdev;

	len = strlen(dev->blockdev.std.name);
	newdev = kmalloc(sizeof(hdd_partdev_t) + len + 12);
	if (!newdev) {
		return;
	}
	newdev->diskdev = *dev;
	newdev->parentdev = dev;

	newdev->diskdev.blockdev.first_block_num = entry->start_lba;
	newdev->diskdev.blockdev.block_count = entry->sector_count;

	nimi = (char*)(newdev + 1);
	memcpy(nimi, dev->blockdev.std.name, len);
	// sprintf(&nimi[len], "_p%d", pnum);
	nimi[len] = 'p'; nimi[len+1] = '0' + pnum; nimi[len+2] = 0;
	newdev->diskdev.blockdev.std.name = nimi;

	device_insert((DEVICE*) newdev);

	DEBUGF("%s\n", nimi);
}

static int hdd_read_logicals(ide_device_t *dev, struct hdd_bootrec_entry *logical_entry, BD_FILE *f, int pnum)
{
	struct hdd_bootrec bootrec;
	fpos_t pos;

	while (HDD_IS_LOGICAL(logical_entry->type)) {
		pos = logical_entry->start_lba;
		pos *= dev->blockdev.block_size;
		pos += HDD_BOOTREC_SKIP;
		if (blockdev_fsetpos(f, &pos)
		|| blockdev_fread(&bootrec, sizeof(bootrec), 1, f) != 1
		|| bootrec.xAA55 != 0xAA55) {
			DEBUGP("Corrupted bootrec...\n");
			return pnum;
		}

		bootrec.osio[0].start_lba += logical_entry->start_lba;
		bootrec.osio[1].start_lba += logical_entry->start_lba;

		if (bootrec.osio[0].type != HDD_PART_EMPTY) {
			hdd_partition_found(dev, &bootrec.osio[0], pnum);
			++pnum;
		}
		logical_entry = &bootrec.osio[1];
	}
	return pnum;
}

static int hdd_read_gpt(ide_device_t *dev, struct hdd_bootrec_entry *logical_entry, BD_FILE *f, int pnum)
{
	DEBUGP("Can't (yet) read GUID Partition Table.\n");
	return pnum;
}

void hdd_read_partitions(ide_device_t *dev)
{
	BD_FILE *f;
	struct hdd_bootrec bootrec;
	fpos_t pos;
	int i;
	int primary_lkm = 0;
	int logical_pnum = 4;

	pos = HDD_BOOTREC_SKIP;

	// MBR & kelpoisuustarkistus
	if (!(f = blockdev_fopen(&dev->blockdev, FILE_MODE_READ))) {
		DEBUGP("Can't open blockdev...\n");
		return;
	}

	if (blockdev_fsetpos(f, &pos)
	|| blockdev_fread(&bootrec, sizeof(bootrec), 1, f) != 1
	|| bootrec.xAA55 != 0xAA55) {
		DEBUGP("Corrupted bootrec...\n");
		return;
	}

	for (i = 0; i < 4; ++i) {
		if (bootrec.osio[i].type == HDD_PART_EMPTY) {
			continue;
		} else if (HDD_IS_LOGICAL(bootrec.osio[i].type)) {
			logical_pnum = hdd_read_logicals(dev, &bootrec.osio[i], f, logical_pnum);
		} else if (bootrec.osio[i].type == HDD_PART_EFI_GPT) {
			if (primary_lkm) {
				logical_pnum = hdd_read_gpt(dev, &bootrec.osio[i], f, logical_pnum);
			} else {
				hdd_read_gpt(dev, &bootrec.osio[i], f, 1);
				break;
			}
		} else {
			hdd_partition_found(dev, &bootrec.osio[i], i);
			++primary_lkm;
		}
	}
	blockdev_fclose(f);
}
