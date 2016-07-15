#ifndef PHYTREX_MODEL_H
#define PHYTREX_MODEL_H

#include <platform_stdlib.h>
#include "phytrex_clock.h"
#include "phytrex_version.h"

//flash address setting
#define FLASH_DATA_LEN            4096

#define WAC_FLASH_OFFSET          0x0
#define HAP_FLASH_KEYPAIR_OFFSET  0x70
#define HAP_FLASH_PAIRING_OFFSET  0xC0
#define FLASH_SETUPCODE_OFFSET    0x930

#define BACKUP_SECTOR             (FLASH_SYSTEM_DATA_ADDR - 0x1000)

#define OTA_SECTOR                0x00100000
#define FLASH_LOG_ADDR            0x001A0000	//0x001A0000 ~ 0x001EFFF
#define LOG_DATA_LEN              	0x00050000
                                  //0x001F000 ~ 0x001F9FFF
#define FLASH_USER_ADDR           0x001FA000
#define FLASH_CLOCK_ADDR          0x001FB000
#define WEB_SECTOR                0x001FC000
#define FLASH_EX_ADDR             0x001FE000
#define FLASH_DATA_ADDR           0x001FF000

typedef struct{
    uint8_t firmware[16];
    uint8_t software[16];
} MyVersion;

typedef struct {
	uint8_t serial_number[16];
	uint8_t device_id[16];
	uint8_t hardware_revision[16];
	uint8_t software_revision[16];	//deprecated
	PhytrexNtpServer ntp;
	uint8_t fota_path[256];
} PhytrexParameter_t;

/* phytrex_wifi_manager														*/
/* if wifi signal is out of range, use this task function to do reconnect	*/
void phytrex_wifi_manager(int times, int interval);

/* phytrex_reset																					*/
/* type - 1 : Reset to factory default (Homekit reset/extension reset/System Data Offset reset/)	*/
/* type - 2 : Reset to default (Homekit data reset : WAC/HAP Data in flash)							*/
/* type - 3 : Reset ota system data offset to ~0x0													*/
/* type - 4 : Reboot system																			*/
void phytrex_reset(int type);

/* phytrex_is_auth																					*/
/* return Auth Flag																					*/
BOOL phytrex_is_auth();

#endif
