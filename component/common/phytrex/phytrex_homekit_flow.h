#ifndef PHYTREX_HOMEKIT_FLOW_H
#define PHYTREX_HOMEKIT_FLOW_H

typedef enum {
    PhytrexHomekitNotifyWacStart,
    PhytrexHomekitNotifyWacAlready,
    PhytrexHomekitNotifyWacAfter,
    PhytrexHomekitNotifyHapStart,
} PhytrexHomekitNotify_t;

//wac flow
typedef struct {
    char name[32];
    char manufacturer[32];
    char model[32];
	char fw_ver[32];
	char hw_ver[32];
	char sn[32];
    char swap_name[32];
	char *ea_protocols[64];
	int num_ea_protocols;
	char bundle_seed_id[10];
} WacFlow_t;

//hap flow
typedef struct {
    char name[32];
    char model[32];
} HapFlow_t;

//hap flow
typedef struct {
    WacFlow_t Wac;
    HapFlow_t Hap;
} HomekitFlow_t;

/* phytrex_wac_info					*/
/* this will show WAC information	*/
void phytrex_wac_info(void);

/* phytrex_hap_info					*/
/* this will show HAP information	*/
void phytrex_hap_info(void);

/* phytrex_wac_clear					*/
/* this will clear WAC data from flash	*/
void phytrex_wac_clear(void);

/* phytrex_hap_clear					*/
/* this will clear HAP data from flash	*/
void phytrex_hap_clear(void);

/* phytrex_homekit_reset							*/
/* this will both clear WAC and HAP data from flash	*/
void phytrex_homekit_reset(void);

/* phytrex_data_reset								*/
/* this will clear structure data from flash	*/
void phytrex_data_reset(void);

/* phytrex_HomeKitNetworkConnection						*/
/* this will do Homkit network reconnect with BCT test	*/
void phytrex_HomeKitNetworkConnection(WacFlow_t *pWAC);

/* phytrex_homekit_hap										*/
/* this use HomekitFlow_t structure to start HAP Service	*/
void phytrex_homekit_wac(HomekitFlow_t *pHomeKit);

/* phytrex_homekit_hap									*/
/* this use HapFlow_t structure to start HAP Service	*/
void phytrex_homekit_hap(HapFlow_t *pHAP);

void phytrex_homekit_cb(PhytrexHomekitNotify_t notify);

#endif