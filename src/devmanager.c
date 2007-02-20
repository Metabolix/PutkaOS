#include <devmanager.h>
#include <malloc.h>

struct fs devfs = {
	(fopen_t) dev_fopen,
	(fclose_t) dev_fclose,
	0,0,0,0,
	0,0
};

FILE *dev_fopen(struct fs *this, const char * filename, uint_t mode)
{
	struct DEVICE *retval = kmalloc(sizeof(struct DEVICE));

	/* TODO: Pitäisi laittaa oikean laitteen funktiot käyttöön. */
	retval->file.fs->fclose = (fclose_t) dev_fclose;

	/* TODO: Hakemistolistaushomma dev-hakemistolle */

	return (FILE *)retval;
}

int dev_fclose(FILE *stream)
{
	struct DEVICE *dev = (struct DEVICE *)stream;

	/* Vapautus */

	kfree(dev);
	return 0;
}
