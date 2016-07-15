#ifndef PHYTREX_UPDATE_H
#define PHYTREX_UPDATE_H

#include "phytrex_model.h"

#define HOST_NAME_LEN	64
#define FILE_PATH_LEN	64

typedef struct {
	uint32_t	ip_addr;
	uint16_t	port;
}phytrex_update_cfg_local_t;

typedef struct {
	uint8_t		full_path[256];
	uint32_t	crc32;
	uint8_t		method;
	uint8_t	 	host_name[HOST_NAME_LEN];
	uint8_t		file_path[FILE_PATH_LEN];
	int			update_result[5];	//0  : Update completed
									//1  : Firmware is latest version
									//2  : Download firmware fail
									//3  : Verify firmware fail
									//4  : Upgrade firmware fail
}phytrex_update_cfg_cloud_t;

//--------------------------------------------------------------------------
/* phytrex_FlashUpdateErase								*/
/* Erase Flash depend on PAGE (4096 bytes)				*/
/* address	- start address								*/
/* len		- data length								*/
void phytrex_FlashUpdateErase(uint32_t address, uint32_t len);

//--------------------------------------------------------------------------
/* phytrex_FlashUpdateRead								*/
/* Read OTA data from flash								*/
/* address	- start address								*/
/* offset	- address offset							*/
/* len		- data length								*/
/* data		- data pointer								*/
void phytrex_FlashUpdateRead(uint32_t address, uint32_t offset, uint32_t len, uint8_t *data);

//--------------------------------------------------------------------------
/* phytrex_FlashUpdateWrite								*/
/* Write OTA data to flash								*/
/* address	- start address								*/
/* offset	- address offset							*/
/* len		- data length								*/
/* data		- data pointer								*/
void phytrex_FlashUpdateWrite(uint32_t address, uint32_t offset, uint32_t len, uint8_t *data);

//--------------------------------------------------------------------------
/* phytrex_ReadSwapAddr										*/
/* Read image signature then return New/Old Bank Address	*/
/* NewImg2Addr	- New Bank Address							*/
/* OldImg2Addr	- Old Bank Address							*/
BOOL phytrex_ReadSwapAddr(uint32_t *NewImg2Addr, uint32_t *OldImg2Addr);

//--------------------------------------------------------------------------
/* phytrex_WriteSwapSig									*/
/* Write image signature								*/
/* NewImg2Addr	- New Bank Address						*/
/* OldImg2Addr	- Old Bank Address						*/
BOOL phytrex_WriteSwapSig(uint32_t NewImg2Addr, uint32_t OldImg2Addr);

//--------------------------------------------------------------------------
/* phytrex_read_ota_addr_to_system_data					*/
/* Read ota address from system data offset				*/
/* ota_addr	- OTA Address								*/
int phytrex_read_ota_addr_to_system_data(uint32_t *ota_addr);

//--------------------------------------------------------------------------
/* phytrex_write_ota_addr_to_system_data				*/
/* Write ota address to system data offset				*/
/* ota_addr	- OTA Address								*/
int phytrex_write_ota_addr_to_system_data(uint32_t ota_addr);

//--------------------------------------------------------------------------
/* phytrex_update_ota_local								*/
/* start OTA task with the "DownloadServer"				*/
/* phytrex-sdk-ameba1-v3.4a_117987\tools\DownloadServer	*/
/* ip   - Server ip										*/
/* port - Server port									*/
int phytrex_update_ota_local(char *ip, int port);

//--------------------------------------------------------------------------
/* phytrex_update_ota_cloud						*/
/* start OTA task with Storage Cloud Service		*/
/* UpdateRes   - phytrex_update_cfg_cloud_t Strcucture	*/
int phytrex_update_ota_cloud(phytrex_update_cfg_cloud_t *pUpdateRes);

//----------------------------------------------------------------------------
int phytrex_my_random(void *p_rng, unsigned char *output, size_t output_len);

int phytrex_parse_path(phytrex_update_cfg_cloud_t *res);

#endif
