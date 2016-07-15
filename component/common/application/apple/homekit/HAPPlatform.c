/*
 *  Copyright (c) 2014 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#include <platform_stdlib.h>
#include <homekit/HAP.h>
#include <flash_api.h>
#include <model.h>
#include "phytrex_model.h"
#include "phytrex_homekit_flow.h"

/* An example of WAC/HAP data offset in one block. It can be modified based on requirement.
 * For flash data at 0xFF000 block
 */
/*#define FLASH_DATA_LEN              4096
#define FLASH_DATA_ADDR             0xFF000
#define WAC_FLASH_OFFSET            0x0
#define HAP_FLASH_KEYPAIR_OFFSET    0x70
#define HAP_FLASH_PAIRING_OFFSET    0xC0
#define FLASH_SETUPCODE_OFFSET      0x930*/

extern PhytrexParameter_t ex_param;

/*-----------------------------------------------------------------------
 * Mandatory functions
 *-----------------------------------------------------------------------*/

// Mandatory function to handle notification from HAP engine
// called when HAP state changed
void HAPPlatformNotify(HAPNotify_t notify)
{
	switch(notify) {
		case HAPNotifyReady:
			phytrex_HAPPlatformNotify(notify);
			break;
		default:
			break;
	}
}


// Mandatory function to save accessory key pair to persistent storage
// called when HAP accessory key pair generation
void HAPPlatformSaveKeypair(HAPPersistentKeypair_t *keypair)
{
	uint8_t *flash_buffer = (uint8_t *) malloc(FLASH_DATA_LEN);
	flash_stream_read(&flash, FLASH_DATA_ADDR, FLASH_DATA_LEN, flash_buffer);
	flash_erase_sector(&flash, FLASH_DATA_ADDR);
	memcpy(flash_buffer + HAP_FLASH_KEYPAIR_OFFSET, keypair, sizeof(HAPPersistentKeypair_t));
	flash_stream_write(&flash, FLASH_DATA_ADDR, FLASH_DATA_LEN, flash_buffer);
	free(flash_buffer);
}

// Mandatory function to load accessory key pair from persistent storage
// called when HAP initialization
void HAPPlatformLoadKeypair(HAPPersistentKeypair_t *keypair)
{
	flash_stream_read(&flash, FLASH_DATA_ADDR + HAP_FLASH_KEYPAIR_OFFSET , sizeof(HAPPersistentKeypair_t), keypair);
}

// Mandatory function to save controller pairing to persistent storage
// called when HAP controller pairing update
void HAPPlatformSavePairings(HAPPersistentPairing_t *pairing, int num)
{
	uint8_t *flash_buffer = (uint8_t *) malloc(FLASH_DATA_LEN);
	flash_stream_read(&flash, FLASH_DATA_ADDR, FLASH_DATA_LEN, flash_buffer);
	flash_erase_sector(&flash, FLASH_DATA_ADDR);
	memcpy(flash_buffer + HAP_FLASH_PAIRING_OFFSET, pairing, sizeof(HAPPersistentPairing_t) * num);
	flash_stream_write(&flash, FLASH_DATA_ADDR, FLASH_DATA_LEN, flash_buffer);
	free(flash_buffer);
}

// Mandatory function to load controller pairing from persistent storage
// called when HAP initialization
void HAPPlatformLoadPairings(HAPPersistentPairing_t *pairing, int num)
{
	flash_stream_read(&flash, FLASH_DATA_ADDR + HAP_FLASH_PAIRING_OFFSET , sizeof(HAPPersistentPairing_t) * num, pairing);
}

// Mandatory function to save HAP state number to persistent storage
// called when HAP state number changed
void HAPPlatformSaveStateNumber(uint32_t state_number)
{
	printf("\nUpdate s#=%u\n", state_number);
}

// Mandatory function to load HAP state number from persistent storage
// called when HAP initialization
uint32_t HAPPlatformLoadStateNumber(void)
{
	uint32_t state_number;

	state_number = 1;

	return state_number;
}

void HAPPlatformLoadDeviceID(char *id_str_buf)
{
	phytrex_FlashDataRead(&ex_param, sizeof(ex_param));
	sprintf(id_str_buf, "%02X:%02X:%02X:%02X:%02X:%02X",
			ex_param.device_id[0],
			ex_param.device_id[1],
			ex_param.device_id[2],
			ex_param.device_id[3],
			ex_param.device_id[4],
			ex_param.device_id[5]);
#if 1
	printf("\r\n[%s] device_id : %s\r\n", __func__, id_str_buf);
#endif
}

/*-----------------------------------------------------------------------
 * Functions not called by HAP server
 *-----------------------------------------------------------------------*/

// Example of setupcode
// To generate random setup code. To store setup code in flash
void random_setupcode(char *buf)
{
	uint8_t rbytes[8];
	rtw_get_random_bytes(rbytes, sizeof(rbytes));
	sprintf(buf, "%d%d%d-%d%d-%d%d%d", rbytes[0] % 10, rbytes[1] % 10, rbytes[2] % 10, rbytes[3] % 10, 
	                                   rbytes[4] % 10, rbytes[5] % 10, rbytes[6] % 10, rbytes[7] % 10);
}

typedef struct {
	uint8_t   code[16];
	uint32_t  len;
} PersistentSetupcode_t;

void FlashSetupcodeRead(char *buf)
{
	PersistentSetupcode_t setupcode;
	flash_stream_read(&flash, FLASH_DATA_ADDR + FLASH_SETUPCODE_OFFSET , sizeof(PersistentSetupcode_t), &setupcode);

	if(setupcode.len == 10)
		strcpy(buf, setupcode.code);
	else
		*buf = 0;
}

void FlashSetupcodeWrite(char *code)
{
	if(strlen(code) == 10) {
		PersistentSetupcode_t setupcode;
		memset(&setupcode, 0, sizeof(PersistentSetupcode_t));
		setupcode.len = 10;
		strcpy(setupcode.code, code);

		uint8_t *flash_buffer = (uint8_t *) malloc(FLASH_DATA_LEN);
		flash_stream_read(&flash, FLASH_DATA_ADDR, FLASH_DATA_LEN, flash_buffer);
		flash_erase_sector(&flash, FLASH_DATA_ADDR);
		memcpy(flash_buffer + FLASH_SETUPCODE_OFFSET, &setupcode, sizeof(PersistentSetupcode_t));
		flash_stream_write(&flash, FLASH_DATA_ADDR, FLASH_DATA_LEN, flash_buffer);
		free(flash_buffer);
	}
}
