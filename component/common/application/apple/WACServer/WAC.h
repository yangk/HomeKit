#ifndef _WAC_H
#define _WAC_H

#include <stdint.h>

typedef enum {
	WACNotifyNone,
	WACNotifyReady,
	WACNotifyConfiguring,
	WACNotifyComplete,
	WACNotifyError
} WACNotify_t;

typedef struct {
	uint8_t   ssid[32];
	uint32_t  ssid_len;
	uint8_t   password[32];
	uint32_t  password_len;
	uint8_t   name[32];
	uint32_t  name_len;
} WACPersistentConfig_t;

typedef struct {
	char      *name;
	char      *model;
	char      *manufacturer;
	char      *sn;
	char      *fw_ver;
	char      *hw_ver;
	char      **ea_protocols;
	uint8_t   num_ea_protocols;
	char      *bundle_seed_id;
} WACDevice_t;

extern int WACStart(WACDevice_t *dev);
extern void WACStop(void);
extern void WACSetupDebug(uint8_t debug);

#endif  /* _WAC_H */
