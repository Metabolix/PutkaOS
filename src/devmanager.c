#include <devmanager.h>

FILE *dev_fopen(const char * restrict filename, const char * restrict mode)
{
	struct DEVICE *retval = kmalloc(sizeof(struct DEVICE));

	return (FILE *)retval;
}

int dev_fclose(FILE *stream)
{
	DEVICE *dev = (DEVICE *)stream;

	/* Vapautus */

	kfree(dev);
	return 0;
}
