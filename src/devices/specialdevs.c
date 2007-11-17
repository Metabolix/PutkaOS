#include <devices/specialdevs.h>
#include <devices/devmanager.h>
#include <kprintf.h>

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
	#define MKSPECIALDEV(name_str) \
		{ \
			.name = # name_str , \
			.dev_class = DEV_CLASS_OTHER, \
			.dev_type = DEV_TYPE_OTHER, \
			.devopen = (devopen_t) SPECIAL_OPENFUNC_NAME(name_str), \
			.remove = (devrm_t) SPECIAL_RMFUNC_NAME(name_str) \
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
