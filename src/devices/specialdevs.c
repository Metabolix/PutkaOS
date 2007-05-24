#include <devices/specialdevs.h>
#include <devices/devmanager.h>
#include <screen.h>

/*
struct device {
	const char *name;
	dev_class_t dev_class;
	dev_type_t dev_type;
	size_t index; // device_insert sets this.
	devopen_t devopen;
	devrm_t remove;
};
*/

/*
* Declare functions for devices
**/
#define MKSPECIALDEV(name) SPECIAL_DECLARE_FUNCS(name)
#include <devices/specialdevs_list.h>
#undef MKSPECIALDEV

/*
* Removing them needs nothing special
**/
#define MKSPECIALDEV(name) \
	SPECIAL_RMFUNC(name) { return 0; }
#include <devices/specialdevs_list.h>
#undef MKSPECIALDEV

/*
* List devices
**/
DEVICE specialdev[] = {
	#define MKSPECIALDEV(name) \
		{ \
			# name , \
			DEV_CLASS_OTHER, DEV_TYPE_OTHER, 0, \
			(devopen_t) SPECIAL_OPENFUNC_NAME(name), \
			(devrm_t) SPECIAL_RMFUNC_NAME(name) \
		},
	#include <devices/specialdevs_list.h>
	#undef MKSPECIALDEV
};
const int num_specialdevices = sizeof(specialdev) / sizeof(DEVICE);

void special_devices_init(void)
{
	int i;
	for (i = 0; i < num_specialdevices; ++i) {
		switch (device_insert(&specialdev[i])) {
			default:
				break;
			case DEV_ERR_TOTAL_FAILURE:
			case DEV_ERR_EXISTS:
				kprintf("Inserting special device '%s' failed.\n", specialdev[i].name);
		}
	}
}
