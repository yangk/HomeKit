#include "model.h"

#ifdef PHYTREX
#include "main.h"
#include <lwip_netconf.h>
#include <phytrex_homekit_flow.h>
#include <homekit/HAPAccessory.h>
#include <homekit/HAPAccessoryAPI.h>
#include "phytrex_model.h"
#include "phytrex_update.h"
#endif

PhytrexParameter_t ex_param = {
    SERIAL_NUMBER,
    "",
    HARDWARE_REVISION,
    SOFTWARE_REVISION,
	{
		HOST_NAME,
		SOCKET_PORT,
		INTERVAL,
		OFFSET
	},
	FOTA_PATH
};

char setupcode[10+1] = SETUP_CODE;

#if CONFIG_CUSTOM_VERSION
// This version structure can be used to store the version of the image
// It will be located in fixed location in application image
#pragma location=".custom.validate.rodata"
const MyVersion cur_ver = {PHYTREX_SDK_VERSION, SOFTWARE_REVISION};
#endif

#if SUPPORT_LOG_SERVICE
extern char log_buf[LOG_SERVICE_BUFLEN];
extern xSemaphoreHandle	log_rx_interrupt_sema;
#endif
extern xSemaphoreHandle uart_rx_interrupt_sema;

extern PhytrexParameter_t ex_param;

cJSON *MyOtaService = NULL,
      *MySysService = NULL;

void HapUpdateResult(int value)
{
    HAPUpdateServiceCharacteristic(MyOtaService, kCharacteristicUUID_OtaResult, cJSON_CreateNumber(value*1.0f));
}

void HapUpdateProgress(int value)
{
    HAPUpdateServiceCharacteristic(MyOtaService, kCharacteristicUUID_OtaProgress, cJSON_CreateNumber(value*1.0f));
}

void HapUpdateReset(int value)
{
    HAPUpdateServiceCharacteristic(MyOtaService,
                                   kCharacteristicUUID_SysReset,
                                   cJSON_CreateNumber((value == 0)?0:1));
}

void HapSystemError(int value)
{
	HAPUpdateServiceCharacteristic(MySysService, "77", cJSON_CreateNumber(value));
}

void HapAlarmSetting()
{
#if CONFIG_ALRARM
	HAPUpdateServiceCharacteristic(MySysService, kCharacteristicUUID_SysAlarmSetting, cJSON_CreateAlarmArray());
#endif
}

cJSON *HAPCreateOtaService()
{
    cJSON *serviceJSObject = NULL;
    cJSON *characteristicJSArray, *characteristicJSObject;
    char *rPerms[1] = {kCharacteristicPermission_PairedRead};
    char *wPerms[1] = {kCharacteristicPermission_PairedWrite};
    char *rePerms[2] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_Events};
    char *rwePerms[3] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_PairedWrite, kCharacteristicPermission_Events};

    if((serviceJSObject = cJSON_CreateObject()) != NULL) {
        cJSON_AddItemToObject(serviceJSObject, kServiceObject_Type, cJSON_CreateString(kServiceUUID_Ota));
        cJSON_AddItemToObject(serviceJSObject, kServiceObject_InstanceID, cJSON_CreateNull());
        characteristicJSArray = cJSON_CreateArray();
        cJSON_AddItemToObject(serviceJSObject, kServiceObject_Characteristics, characteristicJSArray);
        
        //Start Upgrade
#ifdef OTA_BOOL_START
		characteristicJSObject = cJSON_CreateObject();
        cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_OtaStart));
        cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(0));
        cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
        cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Boolean));
        cJSON_AddStringToObject(characteristicJSObject, kCharacteristicObject_Description, OTA_UPGRADE);
		HAPAddCustomServiceCharacteristic(serviceJSObject, characteristicJSObject);
#else
        characteristicJSObject = cJSON_CreateObject();
        cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_OtaStart));
        cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(""));
        cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
        cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaxLength, cJSON_CreateNumber(128));
        cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
        cJSON_AddStringToObject(characteristicJSObject, kCharacteristicObject_Description, OTA_UPGRADE);
		HAPAddCustomServiceCharacteristic(serviceJSObject, characteristicJSObject);
#endif
        
		//OTA Progress
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_OtaProgress));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(OTA_PERCENT_MIN));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Float));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(OTA_PERCENT_MIN));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(OTA_PERCENT_MAX));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_StepValue, cJSON_CreateNumber(1));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Unit, cJSON_CreateString(kCharacteristicUnit_Percentage));
		cJSON_AddStringToObject(characteristicJSObject, kCharacteristicObject_Description, OTA_PROGRESS);
		HAPAddCustomServiceCharacteristic(serviceJSObject, characteristicJSObject);

		//OTA Result
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_OtaResult));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(OTA_STATE));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Integer));
		cJSON_AddStringToObject(characteristicJSObject, kCharacteristicObject_Description, OTA_RESULT);
		HAPAddCustomServiceCharacteristic(serviceJSObject, characteristicJSObject);

		// public.hap.characteristic.name (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Name));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(OTA_NAME));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
		HAPAddCustomServiceCharacteristic(serviceJSObject, characteristicJSObject);
    }
    
    return serviceJSObject;
}

#if FIRST_VERSION
cJSON *ES_HAPCreateOtaService()
{
    cJSON *serviceJSObject = NULL;
    cJSON *chaJSArray, *chaJSObject;
    char *rPerms[1] = {kChaPerm_PR};
    char *wPerms[1] = {kChaPerm_PW};
    char *rePerms[2] = {kChaPerm_PR, kChaPerm_EV};
    cJSON *rPerm = cJSON_CreateStringArray(rPerms, 1);
    cJSON *wPerm = cJSON_CreateStringArray(wPerms, 1);
    cJSON *rePerm = cJSON_CreateStringArray(rePerms, 2);
    
    if((serviceJSObject = cJSON_CreateObject()) != NULL)
    {
        cJSON_AddStringToObject(serviceJSObject, kServObj_Type, kServUUID_FV_Ota);
        cJSON_AddNullToObject(serviceJSObject, kServObj_IID);
        chaJSArray = cJSON_CreateArray();
        cJSON_AddItemToObject(serviceJSObject, kServObj_Cha, chaJSArray);

        //custom.esoft.hap.characteristic.update-start
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_FV_Ota_Start);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, wPerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_Bool);

        //custom.esoft.hap.characteristic.update-wifi-progress
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_FV_Ota_Prog);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        cJSON_AddNumberToObject(chaJSObject, kChaObj_Val, 0);
        cJSON_AddNumberToObject(chaJSObject, kChaObj_MinVal, 0);
        cJSON_AddNumberToObject(chaJSObject, kChaObj_MaxVal, 100);
        cJSON_AddNumberToObject(chaJSObject, kChaObj_StepVal, 1);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Unit, kChaUnit_Percent);
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, rePerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_Int);

        //custom.esoft.hap.characteristic.update-wifi-result
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_FV_Ota_Res);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        cJSON_AddBoolToObject(chaJSObject, kChaObj_Val, 0);
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, rePerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_Uint8);

        // public.hap.characteristic.name (required)
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kCharacteristicUUID_Name);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Val, UpdateName);
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, rPerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_String);

    }
    
    return serviceJSObject;
}
#else
cJSON *ES_HAPCreateOtaService()
{
    cJSON *serviceJSObject = NULL;
    cJSON *chaJSArray, *chaJSObject;
    char *rPerms[1] = {kChaPerm_PR};
    char *wPerms[1] = {kChaPerm_PW};
    char *rePerms[2] = {kChaPerm_PR, kChaPerm_EV};
    cJSON *rPerm = cJSON_CreateStringArray(rPerms, 1);
    cJSON *wPerm = cJSON_CreateStringArray(wPerms, 1);
    cJSON *rePerm = cJSON_CreateStringArray(rePerms, 2);
    
    if((serviceJSObject = cJSON_CreateObject()) != NULL)
    {
        cJSON_AddStringToObject(serviceJSObject, kServObj_Type, kServUUID_Update);
        cJSON_AddNullToObject(serviceJSObject, kServObj_IID);
        chaJSArray = cJSON_CreateArray();
        cJSON_AddItemToObject(serviceJSObject, kServObj_Cha, chaJSArray);


        //custom.esoft.hap.characteristic.update-wifi-package
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_WF_Ota_Data);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        //cJSON_AddStringToObject(chaJSObject, kChaObj_Val, "");
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, wPerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_Data);

        //custom.esoft.hap.characteristic.update-mcu-package
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_MCU_Ota_Data);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        //cJSON_AddStringToObject(chaJSObject, kChaObj_Val, "");
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, wPerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_Data);

        //custom.esoft.hap.characteristic.update-wifi-progress
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_WF_Ota_Prog);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        cJSON_AddNumberToObject(chaJSObject, kChaObj_Val, 0);
        cJSON_AddNumberToObject(chaJSObject, kChaObj_MinVal, 0);
        cJSON_AddNumberToObject(chaJSObject, kChaObj_MaxVal, 100);
        cJSON_AddNumberToObject(chaJSObject, kChaObj_StepVal, 1);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Unit, kChaUnit_Percent);
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, rePerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_Int);

        //custom.esoft.hap.characteristic.update-mcu-progress
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_MCU_Ota_Prog);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        cJSON_AddNumberToObject(chaJSObject, kChaObj_Val, 0);
        cJSON_AddNumberToObject(chaJSObject, kChaObj_MinVal, 0);
        cJSON_AddNumberToObject(chaJSObject, kChaObj_MaxVal, 100);
        cJSON_AddNumberToObject(chaJSObject, kChaObj_StepVal, 1);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Unit, kChaUnit_Percent);
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, rePerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_Int);

        //custom.esoft.hap.characteristic.update-wifi-result
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_WF_Ota_Res);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        cJSON_AddBoolToObject(chaJSObject, kChaObj_Val, 0);
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, rePerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_Bool);

        //custom.esoft.hap.characteristic.update-wifi-result
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_MCU_Ota_Res);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        cJSON_AddBoolToObject(chaJSObject, kChaObj_Val, 0);
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, rePerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_Bool);

        // public.hap.characteristic.name (required)
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kCharacteristicUUID_Name);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Val, UpdateName);
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, rPerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_String);

    }
    
    return serviceJSObject;
}
#endif
cJSON *HAPCreateSystemService()
{
    cJSON *serviceJSObject = NULL;
    cJSON *characteristicJSArray, *characteristicJSObject;
    char *rPerms[1] = {kCharacteristicPermission_PairedRead};
    char *wPerms[1] = {kCharacteristicPermission_PairedWrite};
    char *rePerms[2] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_Events};
    char *rwePerms[3] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_PairedWrite, kCharacteristicPermission_Events};

    if((serviceJSObject = cJSON_CreateObject()) != NULL) {
        cJSON_AddItemToObject(serviceJSObject, kServiceObject_Type, cJSON_CreateString(kServiceUUID_System));
        cJSON_AddItemToObject(serviceJSObject, kServiceObject_InstanceID, cJSON_CreateNull());
        characteristicJSArray = cJSON_CreateArray();
        cJSON_AddItemToObject(serviceJSObject, kServiceObject_Characteristics, characteristicJSArray);
        
#if CONFIG_ALARM
        // Add/Remove Alarm
        characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_SysAlarmSetting));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
        cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateArray());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_JSONArray));
        cJSON_AddStringToObject(characteristicJSObject, kCharacteristicObject_Description, SYS_ALARM_SETTING);
		HAPAddCustomServiceCharacteristic(serviceJSObject, characteristicJSObject);
#endif
        
        // Reset To Default
        characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_SysReset));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(wPerms, 1));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Boolean));
        cJSON_AddStringToObject(characteristicJSObject, kCharacteristicObject_Description, SYS_RESET);
		HAPAddCustomServiceCharacteristic(serviceJSObject, characteristicJSObject);
        
#if CONFIG_CONSOLE
        //Console Command
        characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_SysConsole));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(wPerms, 1));
        cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaxLength, cJSON_CreateNumber(128));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
        cJSON_AddStringToObject(characteristicJSObject, kCharacteristicObject_Description, SYS_CONSOLE);
		HAPAddCustomServiceCharacteristic(serviceJSObject, characteristicJSObject);
#endif
		
#if CONFIG_FAULT
        //Status Fault
        characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString("77"));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(0));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Uint8));
		HAPAddCustomServiceCharacteristic(serviceJSObject, characteristicJSObject);
#endif
                
        // public.hap.characteristic.name (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Name));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(SYS_NAME));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
		HAPAddCustomServiceCharacteristic(serviceJSObject, characteristicJSObject);
    }
    
    return serviceJSObject;
}
extern struct netif xnetif[];
void get_pushid(char *pushid)
{
    uint8_t *mac = LwIP_GetMAC(&xnetif[0]);
    sprintf(pushid, PushID, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
cJSON *ES_HAPCreateSystemService()
{
    cJSON *serviceJSObject = NULL;
    cJSON *chaJSArray, *chaJSObject;
    char *rPerms[1] = {kChaPerm_PR};
    char *wPerms[1] = {kChaPerm_PW};
    char *rePerms[2] = {kChaPerm_PR, kChaPerm_EV};
    char *rwPerms[2] = {kChaPerm_PR, kChaPerm_PW};
    cJSON *rPerm = cJSON_CreateStringArray(rPerms, 1);
    cJSON *wPerm = cJSON_CreateStringArray(wPerms, 1);
    cJSON *rePerm = cJSON_CreateStringArray(rePerms, 2);
    cJSON *rwPerm = cJSON_CreateStringArray(rwPerms, 2);
    
    if((serviceJSObject = cJSON_CreateObject()) != NULL)
    {
        cJSON_AddStringToObject(serviceJSObject, kServObj_Type, kServUUID_System);
        cJSON_AddNullToObject(serviceJSObject, kServObj_IID);
        chaJSArray = cJSON_CreateArray();
        cJSON_AddItemToObject(serviceJSObject, kServObj_Cha, chaJSArray);
        
        //custom.esoft.hap.characteristic.network-status
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_Status);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        //cJSON_AddBoolToObject(chaJSObject, kChaObj_Val, 1);
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, wPerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_Bool);

        //public.hap.characteristic.current-time
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_Time);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Val, "");
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, rwPerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_String);

        //custom.esoft.characteristic.system.reset
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_Reset);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        //cJSON_AddBoolToObject(chaJSObject, kChaObj_Val, 0);
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, wPerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_Bool);

        //custom.esoft.hap.characteristic.accessoryIdentify
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_PushID);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        char pushid[32];
	 get_pushid(pushid);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Val, pushid);
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, rPerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_String);

        // public.hap.characteristic.name (required)
        chaJSObject = cJSON_CreateObject();
        cJSON_AddItemToArray(chaJSArray, chaJSObject);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kCharacteristicUUID_Name);
        cJSON_AddNullToObject(chaJSObject, kChaObj_IID);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Val, SystemName);
        cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, rPerm);
        cJSON_AddStringToObject(chaJSObject, kChaObj_Format, kChaFormat_String);

    }
    
    return serviceJSObject;
}
void HAPCreatePhytrexServiceGroup(cJSON *pAccessory)
{
#if CONFIG_OTA
    //for OTA custom service
    //#if FIRST_VERSION
    //MySysService = HAPCreateOtaService();
    //#else
    MyOtaService = ES_HAPCreateOtaService();
    //#endif
    HAPAddAccessoryService(pAccessory, MyOtaService);
#endif
    
#if CONFIG_SYSTEM
    //for System custom service
    #if FIRST_VERSION
    MySysService = HAPCreateSystemService();
    #else
    MySysService = ES_HAPCreateSystemService();
    #endif
    HAPAddAccessoryService(pAccessory, MySysService);
#endif
}

int phytrex_OtaHandler(cJSON *pDB, cJSON *pAccessory, int aid, int iid, cJSON *valueJSObject)
{
	int status = kHAPStatus_InvalidValue;
#if CONFIG_OTA
    cJSON *accessoryJSObject, *serviceJSObject, *characteristicJSObject;
    
    accessoryJSObject = HAPSearchDatabaseAID(pDB, aid);
    characteristicJSObject = HAPSearchDatabaseIID(pDB, accessoryJSObject, iid);
    serviceJSObject = HAPSearchCharacteristicService(pDB, characteristicJSObject);

    if((accessoryJSObject == pAccessory) &&
       (serviceJSObject == MyOtaService)) {
		char *characteristic_type = cJSON_GetObjectItem(characteristicJSObject, kCharacteristicObject_Type)->valuestring;
		if(serviceJSObject == MyOtaService) {
			printf("\nOta::");
			printf(kCharacteristicUUID_OtaStart);
			printf(characteristic_type);
			if(strcmp(characteristic_type, kCharacteristicUUID_OtaStart) == 0) {
				printf("OtaStart::");
#ifdef OTA_BOOL_START
				if(valueJSObject->valueint == 0 ||
				   valueJSObject->valueint == 1) {
					printf("%d\n", valueJSObject->valueint);
					if(valueJSObject->valueint == 1) {
						phytrex_update_cfg_cloud_t update_res = {0};
						phytrex_update_set_result(&update_res, 0, 1, 2, 3, 4);
						memcpy(update_res.full_path, ex_param.fota_path, sizeof(update_res.full_path));
						if(phytrex_get_latest_version(&update_res))
							phytrex_update_ota_cloud(&update_res);
						else {
							//1  : Firmware is latest version
							printf("Firmware verison is latest\r\n");
							HAPUpdateServiceCharacteristic(serviceJSObject,
														   kCharacteristicUUID_OtaResult,
														   cJSON_CreateNumber(update_res.update_result[1]));
						}
					}
					
					// Update target characteristic value
					if(HAPUpdateServiceCharacteristic(serviceJSObject, characteristic_type, cJSON_Duplicate(valueJSObject, 0)) == kHAPStatus_Failure) {
						printf("\ninvalid value\n");
					}
					else
						status = kHAPStatus_Success;
				}
#else
				if(valueJSObject->valuestring != NULL) {
					printf("%s\n", valueJSObject->valuestring);
					phytrex_update_cfg_cloud_t update_res = {0};
					phytrex_update_set_result(&update_res, 0, 1, 2, 3, 4);
					memcpy(update_res.full_path, valueJSObject->valuestring, sizeof(update_res.full_path));
					if(phytrex_get_latest_version(&update_res))
						phytrex_update_ota_cloud(&update_res);
					else {
						//1  : Firmware is latest version
						printf("Firmware verison is latest\r\n");
						HAPUpdateServiceCharacteristic(serviceJSObject,
													   kCharacteristicUUID_OtaResult,
													   cJSON_CreateNumber(update_res.update_result[1]));
					}
				
					// Update target characteristic value
					if(HAPUpdateServiceCharacteristic(serviceJSObject, characteristic_type, cJSON_Duplicate(valueJSObject, 0)) == kHAPStatus_Failure) {
						printf("\ninvalid value\n");
					}
					else
						status = kHAPStatus_Success;
				}
#endif
			}
		}
	}
#endif
    return status;
}

int phytrex_SystemHandler(cJSON *pDB, cJSON *pAccessory, int aid, int iid, cJSON *valueJSObject)
{
	int status = kHAPStatus_InvalidValue;
#if CONFIG_SYSTEM
    cJSON *accessoryJSObject, *serviceJSObject, *characteristicJSObject;
    
    accessoryJSObject = HAPSearchDatabaseAID(pDB, aid);
    characteristicJSObject = HAPSearchDatabaseIID(pDB, accessoryJSObject, iid);
    serviceJSObject = HAPSearchCharacteristicService(pDB, characteristicJSObject);

    if((accessoryJSObject == pAccessory) &&
       (serviceJSObject == MySysService)) {
		char *characteristic_type = cJSON_GetObjectItem(characteristicJSObject, kCharacteristicObject_Type)->valuestring;
		if(serviceJSObject == MySysService) {
		printf("\nSystem::");
		if(strcmp(characteristic_type, kCharacteristicUUID_SysAlarmSetting) == 0) {
			printf("AlarmSetting::");
			if(valueJSObject != NULL) {
				char *str = cJSON_Print(valueJSObject);
				printf("\n\r%s\n\r", str);
				free(str);
#if CONFIG_ALARM
				PhytrexClockAlarm CA[ALARM_COUNT] = {0};
				Parse_Alarm_JSON(CA, sizeof(CA), valueJSObject);
				phytrex_FlashClockWrite(CA, sizeof(CA));
#endif
			}
			// Update target characteristic value
			if(HAPUpdateServiceCharacteristic(serviceJSObject, characteristic_type, cJSON_Duplicate(valueJSObject, 0)) == kHAPStatus_Failure) {
				printf("\ninvalid value\n");
			}
			else
				status = kHAPStatus_Success;
		}
		else if(strcmp(characteristic_type, kCharacteristicUUID_SysReset) == 0) {
			printf("Reset::%d\n", valueJSObject->valueint);
			if(valueJSObject->valueint == 1) {
				phytrex_reset(2);
				}
				/*else if(valueJSObject->valueint == 2) {
				    phytrex_reset(1);
				}
				else if(valueJSObject->valueint == 3) {
				    phytrex_reset(3);
				}
				else if(valueJSObject->valueint == 4) phytrex_reset(4);*/
				// Update target characteristic value
				if(HAPUpdateServiceCharacteristic(serviceJSObject, characteristic_type, cJSON_Duplicate(valueJSObject, 0)) == kHAPStatus_Failure) {
					printf("\ninvalid value\n");
				}
				else
					status = kHAPStatus_Success;
		    }
		    else if(strcmp(characteristic_type, kCharacteristicUUID_SysConsole) == 0) {
		        printf("Console::");
		        if(valueJSObject->valuestring != NULL) {
		            printf("%s\n", valueJSObject->valuestring);
		            memset(log_buf, 0, sizeof(log_buf));
		            strcpy(log_buf, valueJSObject->valuestring);
		            xSemaphoreGive(log_rx_interrupt_sema);
		            xSemaphoreGive(uart_rx_interrupt_sema);
		        }
				// Update target characteristic value
				if(HAPUpdateServiceCharacteristic(serviceJSObject, characteristic_type, cJSON_Duplicate(valueJSObject, 0)) == kHAPStatus_Failure) {
					printf("\ninvalid value\n");
				}
				else
					status = kHAPStatus_Success;
		    }
		}
    }
#endif
    return status;
}

int phytrex_OtherHandler(cJSON *pDB, cJSON *pAccessory, int aid, int iid, cJSON *valueJSObject)
{
	int status = kHAPStatus_InvalidValue;
	
	if(phytrex_OtaHandler(pDB, pAccessory, aid, iid, valueJSObject) == kHAPStatus_Success)
		status = kHAPStatus_Success;
	else if(phytrex_SystemHandler(pDB, pAccessory, aid, iid, valueJSObject) == kHAPStatus_Success)
		status = kHAPStatus_Success;
	
	return status;
}

int phytrex_version_compare(char *ver1, char *ver2)
{
	int x1, y1, z1;
	int x2, y2, z2;
	
	sscanf(ver1, "%d.%d.%d", &x1, &y1, &z1);
	sscanf(ver2, "%d.%d.%d", &x2, &y2, &z2);
	
	return (10000*x1+100*y1+z1) - (10000*x2+100*y2+z2);
}
