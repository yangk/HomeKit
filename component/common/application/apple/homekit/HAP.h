/*
 *  Copyright (c) 2014 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#ifndef _HAP_H
#define _HAP_H

#include <stdint.h>

typedef enum {
	HAPNotifyNone,
	HAPNotifyReady,
} HAPNotify_t;

typedef struct {
	uint8_t   identifier[64];
	uint32_t  identifier_len;
	uint8_t   pubkey[32];
	uint32_t  admin;
	uint32_t  valid;
} HAPPersistentPairing_t;

typedef struct {
	uint8_t   skey[32];
	uint8_t   pubkey[32];
	uint32_t  valid;
} HAPPersistentKeypair_t;

typedef struct {
	char      *name;
	char      *model;
	char      *setupcode;
	uint8_t   use_MFi;
	uint32_t  config_number;
} HAPParameter_t;

extern int HAPStart(HAPParameter_t *param);
extern int HAPGetMaxPairings(void);
extern int HAPServerGetMaxPeers(void);
extern int RenameHAPBonjourService(char *name);
extern void HAPSetupDebug(uint8_t debug);

#endif  /* _HAP_H */
