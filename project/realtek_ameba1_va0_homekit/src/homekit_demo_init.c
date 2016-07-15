#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h" 

#include "main.h"
#include "main_test.h"
#include "wifi_conf.h"
#include "wlan_intf.h"
#include "lwip_netconf.h"
#include <platform/platform_stdlib.h>

#include <WACServer/WAC.h>
#include <homekit/HAP.h>

#define STACKSIZE                   1024

extern struct netif xnetif[];
static char dev_name[32];

typedef int (*wlan_init_done_ptr)(void);
extern wlan_init_done_ptr p_wlan_init_done_callback;

void init_sequence(void *param)
{
	int wac_unconfigured = 0;
	WACPersistentConfig_t config;

	// Clear WLAN init done callback to prevent re-enter init sequence at each wlan init
	p_wlan_init_done_callback = NULL;

	// Get existed WAC configuration
	WACPlatformReadConfig(&config);
	memset(dev_name, 0, sizeof(dev_name));

	if((config.name_len > 0) && (config.name_len < 32)) {
		memcpy(dev_name, config.name, config.name_len);
	}
	else {
		uint8_t *mac = LwIP_GetMAC(&xnetif[0]);
		sprintf(dev_name, "RealtekAmeba%02x%02x%02x", mac[3], mac[4], mac[5]);
		wac_unconfigured = 1;
	}

	// Setup LWIP hostname before connecting netowrk
	netif_set_hostname(&xnetif[0], dev_name);

	if(wac_unconfigured) {
		// Start WAC server
		WACDevice_t dev = {0};
		dev.name = dev_name;
		dev.manufacturer = "Realtek";
		dev.model = "Ameba";
		WACSetupDebug(1);
		WACStart(&dev);
	}
	else {
		if((config.ssid_len > 0) && (config.ssid_len < 32) && (config.password_len >= 0) && (config.password_len < 32)) {
			if(wifi_connect((unsigned char *) config.ssid, (config.password_len) ? RTW_SECURITY_WPA2_AES_PSK : RTW_SECURITY_OPEN, 
			                (unsigned char *) config.password, config.ssid_len, config.password_len, 0, NULL) == RTW_SUCCESS) {
#if LWIP_IPV6
				LwIP_AUTOIP_IPv6(&xnetif[0]);
#endif
				printf("\nIPv4 DHCP ...");
				LwIP_DHCP(0, DHCP_START);
				uint8_t *ip = LwIP_GetIP(&xnetif[0]);

				if((ip[0] == IP_ADDR0) && (ip[1] == IP_ADDR1) && (ip[2] == IP_ADDR2) && (ip[3] == IP_ADDR3)) {
					printf("\n\nIPv4 AUTOIP ...");
					LwIP_AUTOIP(&xnetif[0]);
				}

				char setup_code[11];
				FlashSetupcodeRead(setup_code);
				if(strlen(setup_code) != 10) {
					random_setupcode(setup_code);
					printf("\nNo setupcode in flash, use random setupcode: %s\n", setup_code);
				}
				printf("\nSETUP CODE for Test: %s\n", setup_code);

				// Start HAP server
				HAPParameter_t hap_param;
				hap_param.name = dev_name;
				hap_param.model = "Ameba";
				hap_param.setupcode = setup_code;
				hap_param.config_number = 1;
				hap_param.use_MFi = 1;
				WACSetupDebug(1);
				HAPSetupDebug(1);
				HAPStart(&hap_param);
			}
		}
		else {
			printf("\nInvalid network config\n");
		}
	}

	vTaskDelete(NULL);
}

int start_init_sequence(void)
{
	if(xTaskCreate(init_sequence, ((const char*)"init_sequence"), STACKSIZE, NULL, tskIDLE_PRIORITY + 3, NULL) != pdPASS)
		printf("\n\r%s xTaskCreate failed", __FUNCTION__);

	return 0;
}

void setup_init_sequence(void)
{
	// Call back from wlan driver after wlan init done
	p_wlan_init_done_callback = start_init_sequence;
}
