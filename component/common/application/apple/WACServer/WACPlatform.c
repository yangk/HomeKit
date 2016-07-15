#include <platform_stdlib.h>
#include <WACServer/WAC.h>
#include <flash_api.h>
#include "model.h"
#include <phytrex_model.h>
#include <lwip_netconf.h>      // LWIP API and netif
#include <lwip/netif.h>
#include "webserver.h"
#include <phytrex_homekit_flow.h>

extern struct netif xnetif[];
#ifdef PHYTREX
extern HomekitFlow_t hk_flow;
#endif

/* An example of WAC/HAP data offset in one block. It can be modified based on requirement.
 * For flash data at 0xFF000 block
 */
/*#define FLASH_DATA_LEN              4096
#define FLASH_DATA_ADDR             0xFF000
#define WAC_FLASH_OFFSET            0x0*/

/*-----------------------------------------------------------------------
 * Mandatory functions
 *-----------------------------------------------------------------------*/

// Mandatory function to handle notification from WAC engine
// called when WAC state changed
void WACPlatformNotify(WACNotify_t notify)
{
	switch(notify) {
		case WACNotifyReady:
                        phytrex_WACPlatformNotify(notify);
			break;
		case WACNotifyConfiguring:
                        phytrex_WACPlatformNotify(notify);
			break;
		case WACNotifyComplete:
                        phytrex_WACPlatformNotify(notify);
			break;
		case WACNotifyError:
                        phytrex_WACPlatformNotify(notify);
			break;
		default:
			break;
	}
}

// Mandatory function to save WAC configuration to persistent storage
// called when WAC configuration for network connection validated
void WACPlatformSaveConfig(WACPersistentConfig_t *config)
{
	uint8_t *flash_buffer = (uint8_t *) malloc(FLASH_DATA_LEN);
	flash_stream_read(&flash, FLASH_DATA_ADDR, FLASH_DATA_LEN, flash_buffer);
	flash_erase_sector(&flash, FLASH_DATA_ADDR);
	memcpy(flash_buffer + WAC_FLASH_OFFSET, config, sizeof(WACPersistentConfig_t));
	flash_stream_write(&flash, FLASH_DATA_ADDR, FLASH_DATA_LEN, flash_buffer);
	free(flash_buffer);
}

// Mandatory function to get SSID for WAC Soft AP
// called when WAC configuration starts Soft AP mode
char *WACPlatformSWAPName(void)
{
	static char ssid[32];
	uint8_t *mac = LwIP_GetMAC(&xnetif[0]);
#ifdef PHYTREX
	sprintf(ssid, hk_flow.Wac.swap_name, mac[4], mac[5]);
#else
	sprintf(ssid, "RealtekAmeba%02x%02x%02x", mac[3], mac[4], mac[5]);
#endif
	return ssid;
}

/*-----------------------------------------------------------------------
 * Functions not called by WAC server
 *-----------------------------------------------------------------------*/

// Example to read WAC configuration from persistent storage
void WACPlatformReadConfig(WACPersistentConfig_t *config)
{
	flash_stream_read(&flash, FLASH_DATA_ADDR + WAC_FLASH_OFFSET , sizeof(WACPersistentConfig_t), config);
}
