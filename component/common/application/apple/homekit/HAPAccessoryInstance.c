/*
 *  Copyright (c) 2014 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#include <homekit/HAPAccessory.h>
#include <homekit/HAPAccessoryAPI.h>
#include <homekit/HAPServiceProfile.h>
#include <homekit/HAP.h>
#include <cJSON.h>
#include "model.h"
#include "objects.h"
#include "platform_opts.h"
#include "update.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <phytrex_homekit_flow.h>

cJSON *database = NULL;
cJSON *accessory1 = NULL;
cJSON *service1 = NULL;
cJSON *Ccharacteristic[6] = {0};
cJSON *CcharacteristicArray = NULL;
cJSON *Cservice1 = NULL, *Cservice2 = NULL, *Cservice3 = NULL;

/*-----------------------------------------------------------------------
 * Mandatory functions
 *-----------------------------------------------------------------------*/

// Mandatory function to locate accessory
// called when HAP identify
void IdentifyRoutine(void)
{
	printf("\n%s\n", __FUNCTION__);
        ModelIdentifyRoutine();
}

// Mandatory function to initialize accessory hardware
// called when HAP initialization
int AccessoryPlatformInitialize(void)
{
	int status = kHAPStatus_Success;

exit:
	return status;
}

// Mandatory function to generate JSON accessory attribute database
// called when HAP initialization
cJSON *AccessoryDatabaseSetup(HAPParameter_t *param)
{
#if MODEL == 0
                HAPInformationService_t info;
                HAPLightbulbService_t light;

                // Generate an accessory with information
                database = HAPCreateAccessoryDatabase();
                memset(&info, 0, sizeof(HAPInformationService_t));
                info.manufacturer = "Realtek";
                info.model = "Ameba";
                info.name = param->name;
		info.serial_number = "123456789001";
		accessory1 = HAPCreateDatabaseAccessory(database, &info);

 		// Add lightblub service
		memset(&light, 0, sizeof(HAPLightbulbService_t));
		light.brightness_valid = 1;
		light.hue_valid = 1;
		light.saturation_valid = 1;
		service1 = HAPCreateLightbulbService(&light);
                
                //***********custom characteristic**************
                //*****cJSON Library*****
                Ccharacteristic1 = cJSON_CreateObject();
                cJSON_AddStringToObject(Ccharacteristic1, kServiceObject_Type, "E83228B6-1F14-460A-8E77-0794FAE5B36E");
                cJSON_AddNumberToObject(Ccharacteristic1, kServiceObject_InstanceID, 12);
                char *perms1[] = {kCharacteristicPermission_PairedRead,
                                  kCharacteristicPermission_PairedWrite,
                                  kCharacteristicPermission_Events};
                cJSON_AddItemToObject(Ccharacteristic1, kCharacteristicObject_Permissions, cJSON_CreateStringArray(perms1, 3));
                cJSON_AddStringToObject(Ccharacteristic1, kCharacteristicObject_Format, kCharacteristicFormat_Boolean);
                cJSON_AddStringToObject(Ccharacteristic1, kCharacteristicObject_Unit, kCharacteristicUnit_Percentage);
                cJSON_AddNumberToObject(Ccharacteristic1, kCharacteristicObject_MinimumValue, 0);
                cJSON_AddNumberToObject(Ccharacteristic1, kCharacteristicObject_MaximumValue, 100);
                cJSON_AddNumberToObject(Ccharacteristic1, kCharacteristicObject_StepValue, 1);
                cJSON_AddNumberToObject(Ccharacteristic1, kCharacteristicObject_Value, 0);
                HAPAddCustomServiceCharacteristic(service1, Ccharacteristic1);
                //printf(cJSON_Print(Ccharacteristic1));
                
                //*****String Parser*****
                Ccharacteristic2 = cJSON_Parse(CcharacteristicStr);
                //printf(cJSON_Print(Ccharacteristic2));
                HAPAddCustomServiceCharacteristic(service1, Ccharacteristic2);
                
                HAPAddAccessoryService(accessory1, service1);
                
                //***********custom service**************
                //*****cJSON Library*****
                Cservice1 = cJSON_CreateObject();
                cJSON_AddStringToObject(Cservice1, kServiceObject_Type, "68006B76-856A-407F-B739-C441921BD74C");
                cJSON_AddNumberToObject(Cservice1, kServiceObject_InstanceID, 14);
                  CcharacteristicArray = cJSON_CreateArray();
                  Ccharacteristic3 = cJSON_CreateObject();
                  cJSON_AddStringToObject(Ccharacteristic3, kServiceObject_Type, "69DFD99C-C364-4859-84E3-D6AD6DF1AF7B");
                  cJSON_AddNumberToObject(Ccharacteristic3, kCharacteristicObject_InstanceID, 15);
                  char *perms2[] = {kCharacteristicPermission_PairedRead,
                                    kCharacteristicPermission_Events};
                  cJSON_AddItemToObject(Ccharacteristic3, kCharacteristicObject_Permissions, cJSON_CreateStringArray(perms2, 2));
                  cJSON_AddStringToObject(Ccharacteristic3, kCharacteristicObject_Format, kCharacteristicFormat_Float);
                  cJSON_AddNumberToObject(Ccharacteristic3, kCharacteristicObject_Value, 0);
                  cJSON_AddItemToArray(CcharacteristicArray, Ccharacteristic3);
                cJSON_AddItemToObject(Cservice1, kServiceObject_Characteristics, CcharacteristicArray);
                
                HAPAddAccessoryService(accessory1, Cservice1);
                
                //*****String Parser*****
                Cservice2 = cJSON_Parse(CserviceStr);
                //printf(cJSON_Print(Cservice2));
                
                HAPAddAccessoryService(accessory1, Cservice2);
#elif MODEL == 1
                HAPInformationService_t info;
                HAPThermostatService_t thermostat;

                // Generate an accessory with information
                database = HAPCreateAccessoryDatabase();
                memset(&info, 0, sizeof(HAPInformationService_t));
                info.manufacturer = "Realtek";
                info.model = "Ameba";
                info.name = param->name;
		info.serial_number = "123456789002";
		accessory1 = HAPCreateDatabaseAccessory(database, &info);

 		// Add thermostat service
		memset(&thermostat, 0, sizeof(HAPThermostatService_t));
		thermostat.temperature_current = 25;
		thermostat.temperature_target = 25;
		service1 = HAPCreateThermostatService(&thermostat);
                printf(cJSON_Print(service1));
                
                HAPAddAccessoryService(accessory1, service1);
#else
                database = phytrex_AccessoryDatabaseSetup(param);
#endif

	HAPRefreshDatabase(database);

	return database;
}

// Mandatory function to handle writing characteristic
// called when aid and iid are found in accessory attribute database
int AccessoryOperationHandler(int aid, int iid, cJSON *valueJSObject)
{
        int status = kHAPStatus_InvalidValue;
	cJSON *accessoryJSObject, *serviceJSObject, *characteristicJSObject;

	accessoryJSObject = HAPSearchDatabaseAID(database, aid);
	characteristicJSObject = HAPSearchDatabaseIID(database, accessoryJSObject, iid);
	serviceJSObject = HAPSearchCharacteristicService(database, characteristicJSObject);

#if MODEL == 0
	// Handle for Lightbulb
        if((accessoryJSObject == accessory1) && (serviceJSObject == service1)) {
		char *characteristic_type = cJSON_GetObjectItem(characteristicJSObject, kCharacteristicObject_Type)->valuestring;

		if(strcmp(characteristic_type, kCharacteristicUUID_On) == 0) {
			cJSON *current_valueJSObject = HAPGetServiceCharacteristicValue(service1, kCharacteristicUUID_On);
			if(current_valueJSObject->type == cJSON_True) printf("\ncurrent on = true\n");
			if(current_valueJSObject->type == cJSON_False) printf("\ncurrent on = false\n");
			if(current_valueJSObject->type == cJSON_Number) printf("\ncurrent on = %d\n", current_valueJSObject->valueint);

			if(valueJSObject->type == cJSON_True) printf("\nset on = true\n");
			if(valueJSObject->type == cJSON_False) printf("\nset on = false\n");
			if(valueJSObject->type == cJSON_Number) printf("\nset on = %d\n", valueJSObject->valueint);
		}
		else if(strcmp(characteristic_type, kCharacteristicUUID_Brightness) == 0) {
			printf("\nset brightness = %d\n", valueJSObject->valueint);
		}
		else if(strcmp(characteristic_type, kCharacteristicUUID_Hue) == 0) {
			printf("\nset hue = %f\n", valueJSObject->valuedouble);
		}
		else if(strcmp(characteristic_type, kCharacteristicUUID_Saturation) == 0) {
			printf("\nset saturation = %f\n", valueJSObject->valuedouble);
		}
                
                // Update target characteristic value
                HAPUpdateServiceCharacteristic(service1, characteristic_type, cJSON_Duplicate(valueJSObject, 0));

                status = kHAPStatus_Success;
	}
#elif MODEL == 1
	// Handle for Thermostat
	if((accessoryJSObject == accessory1) && (serviceJSObject == service1)) {
		char *characteristic_type = cJSON_GetObjectItem(characteristicJSObject, kCharacteristicObject_Type)->valuestring;

		if(strcmp(characteristic_type, kCharacteristicUUID_TargetHeatingCoolingMode) == 0) {
			if(valueJSObject->valueint == 0) printf("\noff\n");
			if(valueJSObject->valueint == 1) printf("\nheater on\n");
			if(valueJSObject->valueint == 2) printf("\ncooler on\n");

			// Also apply target mode to current mode
			HAPUpdateServiceCharacteristic(service1, kCharacteristicUUID_CurrentHeatingCoolingMode, cJSON_CreateNumber(valueJSObject->valueint));
		}
		else if(strcmp(characteristic_type, kCharacteristicUUID_TargetTemperature) == 0) {
			printf("\nset target = %f Celsius\n", valueJSObject->valuedouble);
		}
                
                // Update target characteristic value
                HAPUpdateServiceCharacteristic(service1, characteristic_type, cJSON_Duplicate(valueJSObject, 0));
                
                status = kHAPStatus_Success;
	}
#else
        status = phytrex_AccessoryOperationHandler(aid, iid, valueJSObject);
#endif

exit:
	return status;
}




/*-----------------------------------------------------------------------*/
