/*
 *  Copyright (c) 2014 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#ifndef _HAP_ACCESSORY_API_H
#define _HAP_ACCESSORY_API_H

#include <stdint.h>
#include <homekit/HAPServiceProfile.h>
#include <cJSON.h>

cJSON *HAPCreateAccessoryDatabase(void);
void HAPRefreshDatabase(cJSON *database);
cJSON *HAPCreateDatabaseAccessory(cJSON *database, HAPInformationService_t *info);
int HAPAddAccessoryService(cJSON *accessory, cJSON *service);
int HAPAddCustomServiceCharacteristic(cJSON *service, cJSON *characteristic);
int HAPUpdateServiceCharacteristic(cJSON *service, char *type, cJSON *value);
cJSON *HAPGetServiceCharacteristicValue(cJSON *service, char *type);
cJSON *HAPSearchCharacteristicService(cJSON *database, cJSON *characteristic);
cJSON *HAPSearchDatabaseAID(cJSON *database, int aid);
cJSON *HAPSearchDatabaseIID(cJSON *database, cJSON *accessory, int iid);
char *HAPGetAccessoryCategory(cJSON *database);
int HAPSetAccessoryCategory(char *category);
void HAPSetCharacteristicReader(int (*reader)(int, int, cJSON **));

#endif  /* _HAP_ACCESSORY_API_H */
