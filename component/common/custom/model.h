#ifndef MODEL_H
#define MODEL_H

#include "FreeRTOS.h"
#include "outlet\outlet.h"

/* Enable Homekit flow */
#define CONFIG_HOMEKIT          1

/* Store version in image */
#define CONFIG_CUSTOM_VERSION   1

//OTA
#define OTA_PERCENT_MIN                         0
#define OTA_PERCENT_MAX                         100
#define OTA_STATE                               0
#define OTA_NAME                                "OTA"
#define OTA_RESULT                              "OTA Result"
#define OTA_PROGRESS                            "OTA Progress"
#define OTA_UPGRADE                             "Firmware Start Upgrade"
#if FIRST_VERSION
#define kServiceUUID_Ota                        "3A4B59DC-CA78-4800-A2EF-6E2E187861E2"
#define kCharacteristicUUID_OtaStart            "FF6F58D2-B705-460B-BAFB-5AE9063B3B59"
#define kCharacteristicUUID_OtaProgress         "4EF78115-F355-4103-BC5E-340F3112AD52"
#define kCharacteristicUUID_OtaResult           "AAB71668-A4E4-49B3-AE21-46912C9B870E"
#else
#define kServiceUUID_Ota                        "A9E34ECA-527D-4F06-838F-4708F639836B"
#define kCharacteristicUUID_OtaStart            "1ED6A66A-6BDF-451C-8B6D-375F928A5C20"
#define kCharacteristicUUID_OtaProgress         "FF35970F-AA18-482E-B916-1A6DB62332C4"
#define kCharacteristicUUID_OtaResult           "6CCB325F-6B94-4341-B3DC-895999FC45C7"
#endif

//System
#define SYS_RESET_MIN                           0
#define SYS_RESET_MAX                           4
#define SYS_NAME                                "System"
#define SYS_RESET                               "Reset To Default"
#define SYS_CONSOLE                             "Console Command"
#define SYS_ALARM_SETTING                       "Alarm Setting"
#define kServiceUUID_System                     "E09E9199-3953-4B3C-9563-B0B25DA84B21"
#define kCharacteristicUUID_SysAlarmSetting     "C2B691E3-177D-495C-B504-03C921538B96"
#define kCharacteristicUUID_SysReset            "D387B0B0-8859-4688-ADBE-142939DFC17F"
#define kCharacteristicUUID_SysConsole          "5C661C9B-AAC5-4B08-9531-04F22657554A"

//0:lightbulb, 1:thermostat example profile
#define OUTLET         2
//#define ADITION_OUTLET 3
//#define CVILUX_ALD     4        // Aroma Lamp Diffuser
//#define CVILUX_AW      5        // Air Washer
//#define MIDEA_FAN      6        //

#define MODEL           OUTLET
#ifndef MODEL
#define MODEL           0
#endif

#if MODEL == OUTLET
#include "other/outlet/outlet.h"
//#elif MODEL == ADITION_OUTLET
//#include "adition/adition_outlet.h"
//#elif MODEL == CVILUX_ALD
//#include "cvilux/cvilux_ald.h"
//#elif MODEL == CVILUX_AW
//#include "cvilux/cvilux_aw.h"
//#elif MODEL == MIDEA_FAN
//#include "midea/midea_fan.h"
#endif

cJSON *HAPCreateOtaService();
cJSON *HAPCreateSystemService();
cJSON *ES_HAPCreateOtaService();
cJSON *ES_HAPCreateSystemService();
int phytrex_OtherHandler(cJSON *pDB, cJSON *pAccessory, int aid, int iid, cJSON *valueJSObject);
void HAPCreatePhytrexServiceGroup(cJSON *pAccessory);
void HAPUpdateReset(int value);
void HAPSystemError(int value);
void HAPAlarmSetting();
void get_pushid(char *pushid);
#endif
