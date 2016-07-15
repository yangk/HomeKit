#include <platform/platform_stdlib.h>
#include <WACServer/WAC.h>
#include <homekit/HAP.h>
#include <mDNS/mDNS.h>

#include <flash_api.h>            // For flash
#include <wifi_conf.h>            // For wifi connect
#include <sys_api.h>              // For system reset

#include <lwip_netconf.h>         // For LWIP
#include <lwip/netif.h>
#include "main.h"

#include "rtc_api.h"
#include <time.h>

#include "other/outlet/outlet.h"
#include "smart_plc.h"

extern struct netif xnetif[];

static int hap_initailized = 0;

void cmd_wac(int argc, char **argv)
{
	if(argc == 1) goto error;

	if(strcmp(argv[1], "init") == 0) {
		if(hap_initailized) {
			printf("HAP in used. Please reboot system");
			return;
		}

		if(argc == 3) {
			WACDevice_t dev = {0};
			dev.name = argv[2];
			dev.manufacturer = "Realtek";
			dev.model = "Ameba";
			WACSetupDebug(1);
			WACStart(&dev);
		}
		else {
			printf("Usage: wac init NAME\n");
		}
	}
	else if(strcmp(argv[1], "deinit") == 0) {
		WACStop();
	}
	else if(strcmp(argv[1], "info") == 0) {
		char buf[32];
		WACPersistentConfig_t config;
		WACPlatformReadConfig(&config);
		printf("WAC Info\n");
		memset(buf, 0, sizeof(buf));
		memcpy(buf, config.ssid, (config.ssid_len < 32) ? config.ssid_len : 31);
		printf("    Network SSID     : %s\n", buf);
		memset(buf, 0, sizeof(buf));
		memcpy(buf, config.password, (config.password_len < 32) ? config.password_len : 31);
		printf("    Network Password : %s\n", buf);
		memset(buf, 0, sizeof(buf));
		memcpy(buf, config.name, (config.name_len < 32) ? config.name_len : 31);
		printf("    Accessory Name   : %s\n", buf);
	}
	else if(strcmp(argv[1], "mfi_test") == 0) {
		MFi_auth_test();
	}
	else {
error:
		printf("Usage: wac init|info\n");
	}
}

void cmd_hap(int argc, char **argv)
{
	if(argc == 1) goto error;

	if(strcmp(argv[1], "init") == 0) {
		if(hap_initailized) {
			printf("HAP in used. Please reboot system");
			return;
		}

		if(argc == 4) {
			HAPParameter_t hap_param;
			hap_param.name = argv[2];
			hap_param.model = "Ameba";
			hap_param.setupcode = argv[3];
			hap_param.config_number = 1;
			hap_param.use_MFi = 0;
			WACSetupDebug(1);
			HAPSetupDebug(1);
			HAPStart(&hap_param);
			hap_initailized = 1;
		}
		else if((argc == 5) && (strcmp(argv[4], "mfi") == 0)) {
			HAPParameter_t hap_param;
			hap_param.name = argv[2];
			hap_param.model = "Ameba";
			hap_param.setupcode = argv[3];
			hap_param.config_number = 1;
			hap_param.use_MFi = 1;
			WACSetupDebug(1);
			HAPSetupDebug(1);
			HAPStart(&hap_param);
			hap_initailized = 1;
		}
		else {
			printf("Usage: hap init NAME SETUP_CODE [mfi]\n");
		}
	}
	else if(strcmp(argv[1], "clear") == 0) {
		HAPPersistentKeypair_t keypair;
		HAPPersistentPairing_t *pairing = (HAPPersistentPairing_t *) malloc(sizeof(HAPPersistentPairing_t) * HAPGetMaxPairings());
		memset(&keypair, 0, sizeof(HAPPersistentKeypair_t));
		memset(pairing, 0, sizeof(HAPPersistentPairing_t) * HAPGetMaxPairings());
		HAPPlatformSaveKeypair(&keypair);
		HAPPlatformSavePairings(pairing, HAPGetMaxPairings());
		printf("HAP persistent data (%d + %d bytes) cleared\n", sizeof(HAPPersistentKeypair_t), sizeof(HAPPersistentPairing_t) * HAPGetMaxPairings());
		free(pairing);
	}
	else if(strcmp(argv[1], "info") == 0) {
		int i, pair_count = 0;
		HAPPersistentPairing_t *pairing = (HAPPersistentPairing_t *) malloc(sizeof(HAPPersistentPairing_t) * HAPGetMaxPairings());
		HAPPlatformLoadPairings(pairing, HAPGetMaxPairings());

		for(i = 0; i < HAPGetMaxPairings(); i ++)
			if(pairing[i].valid == 1) pair_count ++;

		printf("HAP Info\n");
		printf("    Accessory Paired State    : %s\n", (pair_count) ? "true" : "false");
		printf("    Number of Pairing Existed : %d\n", pair_count);
		printf("    Max Controller's Pairings : %d\n", HAPGetMaxPairings());
		printf("    Max Client Connections    : %d\n", HAPServerGetMaxPeers());
		free(pairing);
	}
	else if(strcmp(argv[1], "setupcode") == 0) {
		if((argc == 3) && (strlen(argv[2]) == 10)) {
			FlashSetupcodeWrite(argv[2]);
			printf("Update setupcode in flash to %s\n", argv[2]);
		}
		else if((argc == 3) && (strcmp(argv[2], "info") == 0)) {
			char setup_code[11];
			FlashSetupcodeRead(setup_code);
			printf("Setupcode in flash: %s\n", setup_code);
		}
		else {
			char setup_code[11];
			random_setupcode(setup_code);
			FlashSetupcodeWrite(setup_code);
			printf("Update setupcode in flash to random %s\n", setup_code);
		}
	}
	else {
error:
		printf("Usage: hap init|clear|info\n");
	}
}

void cmd_mdns(int argc, char **argv)
{
	static DNSServiceRef dnsServiceRef = NULL;
	static char hostname[32];

	if(argc == 1) goto error;

	if(strcmp(argv[1], "init") == 0) {
		WACPersistentConfig_t config;
		WACPlatformReadConfig(&config);
		if((config.name_len > 0) && (config.name_len < 32)) {
			strcpy(hostname, config.name);
		}
		else {
			uint8_t *mac = LwIP_GetMAC(&xnetif[0]);
			sprintf(hostname, "AMEBA%02x%02x%02x", mac[3], mac[4], mac[5]);
		}
		netif_set_hostname(&xnetif[0], hostname);

		if(mDNSResponderInit() == 0)
			printf("mDNS initialized\n");
		else
			printf("mDNS init failed\n");
	}
	else if(strcmp(argv[1], "deinit") == 0) {
		mDNSResponderDeinit();
		printf("mDNS de-initialized\n");
	}
	else if(strcmp(argv[1], "reg") == 0) {
		if(argc != 5) {
			printf("Usage: mdns reg NAME TYPE PORT\n");
			return;
		}

		if(atoi(argv[4]) == 0)
			printf("mDNS service incorrect port\n");

		if(dnsServiceRef) {
			mDNSDeregisterService(dnsServiceRef);
			dnsServiceRef = NULL;
		}

		if((dnsServiceRef = mDNSRegisterService(argv[2], argv[3], "local", atoi(argv[4]), NULL)) != NULL)
			printf("mDNS service %s.%s registered\n", argv[2], argv[3]);
		else
			printf("mDNS service %s.%s register failed\n", argv[2], argv[3]);
	}
	else if(strcmp(argv[1], "dereg") == 0) {
		if(dnsServiceRef) {
			mDNSDeregisterService(dnsServiceRef);
			dnsServiceRef = NULL;
			printf("mDNS service de-registered\n");
		}
		else {
			printf("mDNS no service\n");
		}
	}
	else if(strcmp(argv[1], "update") == 0) {
		if(argc != 5) {
			printf("Usage: mdns update TXT_KEY TXT_VALUE TTL\n");
			return;
		}

		if(dnsServiceRef) {
			TXTRecordRef txtRecord;
			unsigned char txt_buf[100];
			TXTRecordCreate(&txtRecord, sizeof(txt_buf), txt_buf);
			TXTRecordSetValue(&txtRecord, argv[2], strlen(argv[3]), argv[3]);
			mDNSUpdateService(dnsServiceRef, &txtRecord, atoi(argv[4]));
			TXTRecordDeallocate(&txtRecord);
			printf("mDNS service updated\n");
		}
		else {
			printf("mDNS no service\n");
		}
	}
	// For BCT mDNS manual name change test
	else if(strcmp(argv[1], "name_test") == 0) {
		char *new_name = "New - Bonjour Service Name";
		printf("Rename HAP service to \"%s\"._hap._tcp.local. %s\n", 
		       new_name, (RenameHAPBonjourService(new_name) == 0) ? "success":"fail");
	}
	else {
error:
		printf("Usage: mdns init|deinit|reg|dereg|name_test\n");
	}
}

/* Example of network connection in HomeKit
 * Wi-Fi connection with the stored WAC configuration
 * Set LWIP hostname to accessory name before DHCP
 * Setup IPv4 and IPv6 address. Suggest to set IPv6 address before IPv4 when mDNS is working
 */
void HomeKitNetworkConnection(void)
{
	static char hostname[32];
	WACPersistentConfig_t config;
	WACPlatformReadConfig(&config);

	if((config.name_len > 0) && (config.name_len < 32)) {
		memset(hostname, 0, sizeof(hostname));
		memcpy(hostname, config.name, config.name_len);
	}
	else {
		uint8_t *mac = LwIP_GetMAC(&xnetif[0]);
		sprintf(hostname, "RealtekAmeba%02x%02x%02x", mac[3], mac[4], mac[5]);
	}

	netif_set_hostname(&xnetif[0], hostname);

	if((config.ssid_len > 0) && (config.ssid_len < 32) && (config.password_len >= 0) && (config.password_len < 32)) {
		if(wifi_connect((unsigned char *) config.ssid, (config.password_len) ? RTW_SECURITY_WPA2_AES_PSK : RTW_SECURITY_OPEN, 
		                (unsigned char *) config.password, config.ssid_len, config.password_len, 0, NULL) == RTW_SUCCESS) {
#if LWIP_IPV6
			LwIP_AUTOIP_IPv6(&xnetif[0]);
#endif
			printf("\nIPv4 DHCP ...");
			LwIP_DHCP(0, DHCP_START);
#if LWIP_AUTOIP
			uint8_t *ip = LwIP_GetIP(&xnetif[0]);
			if((ip[0] == 0) && (ip[1] == 0) && (ip[2] == 0) && (ip[3] == 0)) {
				printf("\n\nIPv4 AUTOIP ...");
				LwIP_AUTOIP(&xnetif[0]);
			}
#endif
		}
	}
}

// For BCT hut-plugging test
void cmd_interface(int argc, char **argv)
{
	if(argc == 1) goto error;

	if(strcmp(argv[1], "set") == 0) {
		if((argc == 4) || (argc == 5)) {
			struct ip_addr ip_addr;
			struct ip_addr netmask_addr;
			struct ip_addr gw_addr;
			uint32_t ip[4], netmask[4], gw[4];
			char *ptr, *head, *tail;
			int i;

			ptr = argv[2];
			for(i = 0; i < 4; i ++) {
				head = ptr;
				while(*ptr && *ptr != '.') ptr ++;
				tail = ptr ++;
				*tail = 0;
				ip[i] = atoi(head);
			}

			ptr = argv[3];
			for(i = 0; i < 4; i ++) {
				head = ptr;
				while(*ptr && *ptr != '.') ptr ++;
				tail = ptr ++;
				*tail = 0;
				netmask[i] = atoi(head);
			}

			if(argc == 5) {
				ptr = argv[4];
				for(i = 0; i < 4; i ++) {
					head = ptr;
					while(*ptr && *ptr != '.') ptr ++;
					tail = ptr ++;
					*tail = 0;
					gw[i] = atoi(head);
				}
			}
			else {
				memset(gw, 0, sizeof(gw));
			}

			IP4_ADDR(&ip_addr, ip[0], ip[1], ip[2], ip[3]);
			IP4_ADDR(&netmask_addr, netmask[0], netmask[1], netmask[2], netmask[3]);
			IP4_ADDR(&gw_addr, gw[0], gw[1], gw[2], gw[3]);
			netif_set_addr(&xnetif[0], &ip_addr , &netmask_addr, &gw_addr);

			printf("\nSet IP  : %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
			printf(  "    MASK: %d.%d.%d.%d\n", netmask[0], netmask[1], netmask[2], netmask[3]);
			printf(  "    GW  : %d.%d.%d.%d\n", gw[0], gw[1], gw[2], gw[3]);
		}
		else {
			printf("Usage: interface set IP NETMASK [GATEWAY]\n");
		}
	}
	else if(strcmp(argv[1], "info") == 0) {
		u8 *mac = LwIP_GetMAC(&xnetif[0]);
		u8 *ip = LwIP_GetIP(&xnetif[0]);
		u8 *netmask = LwIP_GetMASK(&xnetif[0]);
		u8 *gw = LwIP_GetGW(&xnetif[0]);
		printf("\n MAC Address : %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		printf("\nIPv4 Address : %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
		printf(  "     Netmask : %d.%d.%d.%d\n", netmask[0], netmask[1], netmask[2], netmask[3]);
		printf(  "     Gateway : %d.%d.%d.%d\n", gw[0], gw[1], gw[2], gw[3]);
#if LWIP_IPV6
		uint8_t *ipv6 = (uint8_t *) &(xnetif[0].ip6_addr[0].addr[0]);
		printf("\nIPv6 Address : %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
		       ipv6[0], ipv6[1],  ipv6[2],  ipv6[3],  ipv6[4],  ipv6[5],  ipv6[6], ipv6[7],
		       ipv6[8], ipv6[9], ipv6[10], ipv6[11], ipv6[12], ipv6[13], ipv6[14], ipv6[15]);
#endif
	}
	else if(strcmp(argv[1], "disconnect") == 0) {
		int timeout = 20;
		char essid[33];

		if(wext_get_ssid(WLAN0_NAME, (unsigned char *) essid) < 0) {
			printf("\nWIFI disconnected\n");
			return;
		}

		wifi_disconnect();

		while(1) {
			if(wext_get_ssid(WLAN0_NAME, (unsigned char *) essid) < 0) {
				printf("\nWIFI disconnected\n");
				break;
			}
			if(timeout == 0) {
				printf("\nERROR: Operation failed!\n");
				break;
			}
			vTaskDelay(1000);
			timeout --;
		}
	}
	else if(strcmp(argv[1], "reconnect") == 0) {
		HomeKitNetworkConnection();
	}
	else {
error:
		printf("Usage: interface set|info|disconnect|reconnect\n");
	}
}

/* Example of factory reset
 * For WAC/HAP flash data at 0xFF000 block
 */
#define FLASH_DATA_ADDR             0xFF000

void cmd_factory_reset(int argc, char **argv)
{
	// Read setupcode
	char setup_code[11];
	FlashSetupcodeRead(setup_code);

	flash_erase_sector(&flash, FLASH_DATA_ADDR);

	// Restore setupcode
	if(strlen(setup_code) == 10)
		FlashSetupcodeWrite(setup_code);

	// Turn off Wi-Fi and reboot system
	wifi_off();
	sys_reset();
}
void cmd_time(int argc, char **argv)
{
    struct tm timeinfo;
    read_locoltime(&timeinfo);
    printf("Time as a custom formatted string = %d-%d-%d %d:%d:%d wday:%d yday:%d\n", 
    timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour,
    timeinfo.tm_min,timeinfo.tm_sec,timeinfo.tm_wday,timeinfo.tm_yday);
    printf("==============rtc_isenabled : %d============\n", rtc_isenabled());
}

void cmd_led(int argc, char **argv)
{
    led_blink(100000, 5);
}
void cmd_mcuota(int argc, char **argv)
{
    creatFristPacket();
}
void cmd_mcuver(int argc, char **argv)
{
    getMCU_ver();
}
void cmd_mcustate(int argc, char **argv)
{
    getMCU_state();
}
void cmd_mcusynch(int argc, char **argv)
{
    getMCU_synch();
}
void cmd_jpush(int argc, char **argv)
{
    Jiguang_Push(0);
    Jiguang_Push(1);
    Jiguang_Push(2);
    Jiguang_Push(3);
}
void cmd_ntp(int argc, char **argv)
{
    get_ntp_time();
}
void cmd_reportiot(int argc, char **argv)
{
    struct tm timeinfo;
    read_locoltime(&timeinfo);
    report_hisdata_to_iot(0x14,0x66,timeinfo);
}
extern outlet_state outletstateA;
extern outlet_state outletstateB;
void cmd_reportiotalarm(int argc, char **argv)
{
    report_alarm_to_iot(&outletstateA);
}


void cmd_ota(int argc, char **argv)
{
    ES_phytrex_OtaHandler();
}