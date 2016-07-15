#ifndef OUTLET_H
#define OUTLET_H

#include <homekit/HAP.h>
#include <cJSON.h>
#include "rtc_api.h"
#include <time.h>
#include "config.h"
#include "phytrex_model.h"
#include "flash_api.h"
#include "smart_plc.h"

/* Enable system service */
#define CONFIG_SYSTEM           0

/* Enable alarm with CONFIG_SYSTEM */
#define CONFIG_ALARM            1

/* Enable status fault with CONFIG_SYSTEM */
#define CONFIG_FAULT            0

/* Enable console command with CONFIG_SYSTEM */
#define CONFIG_CONSOLE          0

/* Enable phytrex ota service */
#define CONFIG_OTA           	1

#define OTA_BOOL_START

/*首版认证简化UUID*/
#define FIRST_VERSION           1

//GPIO
#define LED_TIMER_ID	TIMER0
#define BTN_TIMER_ID	TIMER1
#define GPIO_RELAYFB1	PE_2//ok
#define GPIO_RELAYFB2	PE_3//PC_3//
#define GPIO_RELAY1	PE_0//PC_1//
#define GPIO_RELAY2	PE_1//PC_2//
#define GPIO_IDENTY	PC_3//PA_5
#define GPIO_TMRBTN	PE_4//PC_0//
#define GPIO_MNUSW1	PA_6
#define GPIO_MNUSW2	PA_7
#define GPIO_OLINUSE1	PC_5
#define GPIO_OLINUSE2	PC_4
//#define GPIO_JTAG_ENABLE_PIN    PA_0//PC_4

//default
#define SERIAL_NUMBER                           "123456789001"
//#define FIRMWARE_REVISION                       "3.4.11"
//#define HARDWARE_REVISION                       "1.0"
//#define SOFTWARE_REVISION                       "1.0.0;outlet"
//#define FIRMWARE_REVISION                       "3.6"
#define HARDWARE_REVISION                       "1.0"
#define SOFTWARE_REVISION                       "0.0.2"
#define SETUP_CODE                              "000-11-222"
#define HOST_NAME                               "time.apple.com"
#define SOCKET_PORT                             "123"
#define INTERVAL                                "1800"
#define OFFSET                                  "0"
#define FOTA_PATH								"https://s3.amazonaws.com/rickysoung/FOTA/HomeKit/Other/Outlet/VERSION"

//profile
#define ON_STATE                                0
#define OUTLET_IN_USE_STATE                     0
#define OUTLET_1                                "Outlet-A"
#define OUTLET_2                                "Outlet-B"

//Homekit flow
#if 0
#define HK_NAME                    "SmartOutlet-%02X%02X"
#define HK_MANUFACTURER            "Phytrex-Outlet"
#define HK_MODEL                   "Phytrex-Outlet"
#define HK_SWAP_NAME               "PhytrexOutlet-%02X%02X"
#else
#define HK_NAME                    "EastSmartOutlet-%02X%02X"
#define HK_MANUFACTURER            "Eastsoft-Outlet"
#define HK_MODEL                   "Eastsoft-Outlet"
#define HK_SWAP_NAME               "EastsoftOutlet-%02X%02X"
#endif
#define SOFT_VER             "ESSP-2S10A-WF-EM(v1.0)-WiFi-20160216"
/*Outlet custom UUID*/
#define kChaUUID_Hour_Data 		"9205D1B8-1E29-4656-92D2-5F28261FC8B1"
#define kChaUUID_Day_Data		"47072759-D675-4291-89A3-3D2C4B3D9D03"
#define kChaUUID_Month_Data		"CC81AC28-7BBD-48EC-A048-9B06C41A0820"
#define kChaUUID_Timing_On		"A645E4C7-BA8A-4A6D-9512-CCA21BB493C3"
#define kChaUUID_Timing_Off		"D0AC1170-A2E8-469A-8DC7-F052A0235451"
#define kChaUUID_Met_Total_Quantity       "93E9FD86-E8A4-43AB-AB63-55F1C12EF991"
#define kChaUUID_Met_Voltage		"CC6B66E8-D218-4B5D-A6B6-213F708C7F5F"
#define kChaUUID_Met_Current		"23120672-6B13-4728-84A0-AF6EEA357C37"
#define kChaUUID_Met_Power		"A0CB82BB-486B-43F4-B9D6-F0793FF71E6C"
#define kChaUUID_Thr_overVoltage		"0973DA57-E7C4-430F-9762-A0B65A852573"
#define kChaUUID_Thr_underVoltage		"A622908C-3042-43C5-AD21-E78444696C75"
#define kChaUUID_Thr_Current		"20B4C71C-4D8D-4C6F-AECC-B6E677D90BA4"
#define kChaUUID_Thr_Power		"C9B074A3-FFEB-4A86-B7BC-FF85A40FD802"
#define kChaUUID_1st_Hisdata		"A767DF15-7E25-49C6-B7D0-7263C297787D"
#define kChaUUID_2nd_Hisdata	"AEE2AC25-2C67-4CAE-AE32-8D814D7F9161"
#define kChaUUID_3rd_Hisdata		"68EF4D9D-24C0-4F7D-A2EC-9ACF619E5103"
#define kChaUUID_4th_Hisdata		"5201E7F0-996E-4327-93E0-09E00A47E4C4"
#define kChaUUID_5th_Hisdata	        "C849069F-C033-476E-9DB0-176FF73176FD"
#define kChaUUID_6th_Hisdata        "48A57CFC-1072-4521-A05E-7DA0D5F2387C"
#define kChaUUID_7th_Hisdata	        "5C5403EA-E642-44E8-B1E1-4AD9795E36B8"
#define kChaUUID_8th_Hisdata	        "C3540191-100E-431D-BCE9-27D422D46F91"

//Update Service UUID
#define kServUUID_Update              "A9E34ECA-527D-4F06-838F-4708F639836B"
#define kChaUUID_WF_Ota_Data        "1ED6A66A-6BDF-451C-8B6D-375F928A5C20"
#define kChaUUID_MCU_Ota_Data       "81C93D06-D736-41D1-BF32-FBAE7C30A3D4"
#define kChaUUID_WF_Ota_Prog        "6848F38C-C2AC-46D6-9FB4-2A20DAF4B373"
#define kChaUUID_MCU_Ota_Prog       "FF35970F-AA18-482E-B916-1A6DB62332C4"
#define kChaUUID_WF_Ota_Res         "6CCB325F-6B94-4341-B3DC-895999FC45C7"
#define kChaUUID_MCU_Ota_Res        "D5E196AB-3A26-41FF-A791-FE04B2046FA5"
#define UpdateName          "Update-FW"

//System Service UUID
#define kServUUID_System                "2A5332F6-6BD1-45D2-8C67-648C67D8454A"
#define kChaUUID_Status                    "58090EED-8DA5-47BB-89FD-DEB6FE62ED6B"
#define kChaUUID_Time                      "695CBB0E-56F6-4B9E-83E6-38AC6C2456C4"
#define kChaUUID_Reset                     "D387B0B0-8859-4688-ADBE-142939DFC17F"
#define kChaUUID_PushID                  "363F4FEE-EC1A-4E3F-B3E1-24FFDEF7E784"
#define SystemName          "System"
#define PushID      "ESOutlet%02x%02x%02x%02x%02x%02x"

#if FIRST_VERSION
#define kServUUID_FV_Meter                "14FA9D31-FC94-4F98-B00D-4AE878523748"
#define kChaUUID_FV_Met_Voltage     "032B12CF-D4E8-4277-9021-188816FD00C6"
#define kChaUUID_FV_Met_Current     "08874E2E-5B63-4EEC-A146-4E2D93E5642A"
#define kChaUUID_FV_Met_Power       "D7467855-8B65-42EC-98E1-496FF153F342"

#define kServUUID_FV_Ota               "3A4B59DC-CA78-4800-A2EF-6E2E187861E2"
#define kChaUUID_FV_Ota_Start       "FF6F58D2-B705-460B-BAFB-5AE9063B3B59"
#define kChaUUID_FV_Ota_Prog       "4EF78115-F355-4103-BC5E-340F3112AD52"
#define kChaUUID_FV_Ota_Res        "AAB71668-A4E4-49B3-AE21-46912C9B870E"
#endif
//steps
#define Quantity_Step        0.1

/* Service Object */
#define kServObj_Type                    "type"
#define kServObj_IID              "iid"
#define kServObj_Cha         "characteristics"

/* Characteristic Object */
#define kChaObj_Type             "type"
#define kChaObj_IID       "iid"
#define kChaObj_Val            "value"
#define kChaObj_Perms      "perms"
#define kChaObj_Event            "ev"
#define kChaObj_Descrip      "description"
#define kChaObj_Format           "format"
#define kChaObj_Unit             "unit"
#define kChaObj_MinVal     "minValue"
#define kChaObj_MaxVal     "maxValue"
#define kChaObj_StepVal       "minStep"
#define kChaObj_MaxLen        "maxLen"
#define kChaObj_MaxDataLen   "maxDataLen"

/* Characteristic Format Type */
#define kChaFormat_Bool          "bool"
#define kChaFormat_Int          "int"
#define kChaFormat_Uint8            "uint8"
#define kChaFormat_Uint32           "uint32"
#define kChaFormat_Float            "float"
#define kChaFormat_String           "string"
#define kChaFormat_Date         "date"
#define kChaFormat_TLV8       "tlv8"
#define kChaFormat_Data       "data"
#define kChaFormat_Array        "array"
#define kChaFormat_Dict       "dict"

/* Characteristic Unit Type */
#define kChaUnit_Celsius     "celsius"
#define kChaUnit_Percent         "percentage"
#define kChaUnit_ArcDegr         "arcdegrees"
#define kChaUnit_Seconds            "seconds"

/* Characteristic Permission */
#define kChaPerm_PR   "pr"
#define kChaPerm_PW  "pw"
#define kChaPerm_EV       "ev"

//Quantity config
#define HISDATA_ADDR                         0xF6000
#define HISDATA_MONTH_LEN               0x400
#define HISDATA_PEROUTLET_LEN        0x2000
#define HISDATALEN                              771
#define HISDATABASE64LEN                   1500   
//extension structure
//can add user data structure
/*typedef struct {
} ModelParameter_t;*/
typedef struct outlet_state {
    unsigned char on;	
    unsigned char inuse;
    unsigned char alarm;
    float metqua;
    float metvol;
    float metcur;
    float metpow;
    float thrvol_over;
    float thrvol_under;
    float thrcur;
    float thrpow;				
} outlet_state;

typedef struct hisdata_state {
    uint8 channel;
    uint8 year;
    uint8 mon;
    uint8 day;
    uint8 hour;
    uint8 min;
    uint8 quantity;			
} hisdata_state;

#define OUTLET_A_HISADDR    FLASH_USER_ADDR
#define OUTLET_B_HISADDR    OUTLET_A_HISADDR+0x3000
#define OUTLET_MON_HISADDR    OUTLET_B_HISADDR+0x3000
#define OUTLET_TIMER_ADDR    OUTLET_MON_HISADDR+0x1000
#define MCU_OTA_ADDR             OUTLET_TIMER_ADDR+0x1000
#define MCU_OTA_DATA_ADDR   MCU_OTA_ADDR+sizeof(struct UPDATE_FILE)

#define OUTLET_TIMER_ONELEN    700
#define OUTLET_TIMER_TWOLEN    1400

#define HISDATA_15_MIN    1

struct outlet_timer 
{
    uint8_t    repeat;
    uint8_t    onoff;
    uint8_t    hour;
    uint8_t    min;
    uint8_t    enable;
    uint8_t    weekday;
};

struct read_handle 
{
    char* cha_type;
    cJSON (*action)(void *args);
};

void read_locoltime(struct tm *ptm);
void set_locoltime(char *time);
void init_gpio(void);
void ModelIdentifyRoutine(void);
int phytrex_AccessoryOperationHandler(int aid, int iid, cJSON *valueJSObject);
cJSON *phytrex_AccessoryDatabaseSetup(HAPParameter_t *param);
cJSON *cJSON_CreateAlarmArray();
void HAPNotifyHandle(void *param);
void led_blink(uint32_t value, int timeout);
void getMCU_state();
void getMCU_synch();

#endif
