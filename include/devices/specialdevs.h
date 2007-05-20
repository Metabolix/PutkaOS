#ifndef _SPECIAL_DEVICES_H
#define _SPECIAL_DEVICES_H 1

#define SPECIAL_OPENFUNC_NAME(name) open_ ## name
#define SPECIAL_RMFUNC_NAME(name) rm_ ## name

#define SPECIAL_OPENFUNC(name) \
	FILE * SPECIAL_OPENFUNC_NAME(name) (DEVICE *device, uint_t mode)
#define SPECIAL_RMFUNC(name) \
	int SPECIAL_RMFUNC_NAME(name) (DEVICE *device)

#define SPECIAL_DECLARE_FUNCS(name) \
	extern SPECIAL_OPENFUNC(name); \
	extern SPECIAL_RMFUNC(name);

extern void init_special_devices(void);

#endif
