/*
 *  Copyright (c) 2014 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#include <homekit/HAPAccessory.h>
#include <homekit/HAPServiceProfile.h>
#include <cJSON.h>
#include "model.h"

cJSON *HAPCreateInformationService(HAPInformationService_t *service)
{
	cJSON *serviceJSObject = NULL;
	cJSON *characteristicJSArray, *characteristicJSObject;
	char *rPerms[1] = {kCharacteristicPermission_PairedRead};
	char *wPerms[1] = {kCharacteristicPermission_PairedWrite};

	if(!service->manufacturer || !service->model || !service->name || !service->serial_number) return NULL;

	if((serviceJSObject = cJSON_CreateObject()) != NULL) {
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Type, cJSON_CreateString(kServiceUUID_AccessoryInformation));
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_InstanceID, cJSON_CreateNull());
		characteristicJSArray = cJSON_CreateArray();
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Characteristics, characteristicJSArray);

		// public.hap.characteristic.identify (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Identify));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
//		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(wPerms, 1));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Boolean));

		// public.hap.characteristic.manufacturer (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Manufacturer));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->manufacturer));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));

		// public.hap.characteristic.model (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Model));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->model));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));

		// public.hap.characteristic.name (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Name));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->name));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));

		// public.hap.characteristic.serial-number (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_SerialNumber));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->serial_number));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));

		// public.hap.characteristic.firmware.revision (optional)
		if(service->firmware_revision) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_FirmwareRevision));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->firmware_revision));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
		}

		// public.hap.characteristic.hardware.revision (optional)
		if(service->hardware_revision) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_HardwareRevision));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->hardware_revision));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
		}

		// public.hap.characteristic.software.revision (optional)
		if(service->software_revision) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_SoftwareRevision));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->software_revision));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
		}
	}

	return serviceJSObject;
}

cJSON *HAPCreateFanService(HAPFanService_t *service)
{
	cJSON *serviceJSObject = NULL;
	cJSON *characteristicJSArray, *characteristicJSObject;
	char *rPerms[1] = {kCharacteristicPermission_PairedRead};
	char *rwePerms[3] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_PairedWrite, kCharacteristicPermission_Events};

	if((serviceJSObject = cJSON_CreateObject()) != NULL) {
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Type, cJSON_CreateString(kServiceUUID_Fan));
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_InstanceID, cJSON_CreateNull());
		characteristicJSArray = cJSON_CreateArray();
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Characteristics, characteristicJSArray);

		// public.hap.characteristic.on (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_On));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, (service->on) ? cJSON_CreateBool(cJSON_True) : cJSON_CreateBool(cJSON_False));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Boolean));

		// public.hap.characteristic.rotation.direction (optional)
		if(service->rotation_direction_valid) {
			int rotation_direction = service->rotation_direction;
			if((rotation_direction < 0) || (rotation_direction > 1)) rotation_direction = 0;

			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_RotationDirection));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(rotation_direction));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Uint8));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(1));
		}

		// public.hap.characteristic.rotation.speed (optional)
		if(service->rotation_speed_valid) {
			float rotation_speed = service->rotation_speed;
			if((rotation_speed < 0) || (rotation_speed > 100)) rotation_speed = 0;

			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_RotationSpeed));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(rotation_speed));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Float));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(100));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_StepValue, cJSON_CreateNumber(1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Unit, cJSON_CreateString(kCharacteristicUnit_Percentage));
		}
		
		// public.hap.characteristic.name (optional)
		if(service->name) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
		}
	}

	return serviceJSObject;
}

cJSON *HAPCreateGarageDoorOpenerService(HAPGarageDoorOpenerService_t *service)
{
	cJSON *serviceJSObject = NULL;
	cJSON *characteristicJSArray, *characteristicJSObject;
	char *rPerms[1] = {kCharacteristicPermission_PairedRead};
	char *rePerms[2] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_Events};
	char *rwePerms[3] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_PairedWrite, kCharacteristicPermission_Events};
	int door_state_current, door_state_target;

	if((serviceJSObject = cJSON_CreateObject()) != NULL) {
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Type, cJSON_CreateString(kServiceUUID_GarageDoorOpener));
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_InstanceID, cJSON_CreateNull());
		characteristicJSArray = cJSON_CreateArray();
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Characteristics, characteristicJSArray);

		// public.hap.characteristic.door-state.current (required)
		door_state_current = service->door_state_current;
		if((door_state_current < 0) || (door_state_current > 255)) door_state_current = 0;
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_CurrentDoorState));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(door_state_current));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Integer));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(255));

		// public.hap.characteristic.door-state.target (required)
		door_state_target = service->door_state_target;
		if((door_state_target < 0) || (door_state_target > 1)) door_state_target = 0;
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_TargetDoorState));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(door_state_target));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Integer));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(1));

		// public.hap.characteristic.obstruction-detected (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_ObstructionDetected));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, (service->obstruction_detected) ? cJSON_CreateBool(cJSON_True) : cJSON_CreateBool(cJSON_False));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Boolean));

		// public.hap.characteristic.lock-mechanism.current-state (optional)
		if(service->lock_current_state_valid) {
			int lock_current_state = service->lock_current_state;
			if((lock_current_state < 0) || (lock_current_state > 255)) lock_current_state = 0;

			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_LockCurrentState));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(lock_current_state));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Integer));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(255));
		}

		// public.hap.characteristic.lock-mechanism.target-state (optional)
		if(service->lock_target_state_valid) {
			int lock_target_state = service->lock_target_state;
			if((lock_target_state < 0) || (lock_target_state > 255)) lock_target_state = 0;

			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_LockTargetState));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(lock_target_state));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Integer));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(255));
		}

		// public.hap.characteristic.name (optional)
		if(service->name) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
		}
	}

	return serviceJSObject;
}

cJSON *HAPCreateLightbulbService(HAPLightbulbService_t *service)
{
	cJSON *serviceJSObject = NULL;
	cJSON *characteristicJSArray, *characteristicJSObject;
	char *rPerms[1] = {kCharacteristicPermission_PairedRead};
	char *rwePerms[3] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_PairedWrite, kCharacteristicPermission_Events};

	if((serviceJSObject = cJSON_CreateObject()) != NULL) {
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Type, cJSON_CreateString(kServiceUUID_Lightbulb));
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_InstanceID, cJSON_CreateNull());
		characteristicJSArray = cJSON_CreateArray();
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Characteristics, characteristicJSArray);

		// public.hap.characteristic.on (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_On));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, (service->on) ? cJSON_CreateBool(cJSON_True) : cJSON_CreateBool(cJSON_False));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Boolean));

		// public.hap.characteristic.brightness (optional)
		if(service->brightness_valid) {
			int brightness = service->brightness;
			if((brightness < 0) || (brightness > 100)) brightness = 0;

			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Brightness));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(brightness));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Integer));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(100));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_StepValue, cJSON_CreateNumber(1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Unit, cJSON_CreateString(kCharacteristicUnit_Percentage));
		}

		// public.hap.characteristic.hue (optional)
		if(service->hue_valid) {
			float hue = service->hue;
			if((hue < 0) || (hue > 360)) hue = 0;

			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Hue));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(hue));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Float));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(360));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_StepValue, cJSON_CreateNumber(1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Unit, cJSON_CreateString(kCharacteristicUnit_ArcDegrees));
		}

		// public.hap.characteristic.name (optional)
		if(service->name) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
		}

		// public.hap.characteristic.saturation (optional)
		if(service->saturation_valid) {
			float saturation = service->saturation;
			if((saturation < 0) || (saturation > 100)) saturation = 0;

			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Saturation));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(saturation));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Float));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(100));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_StepValue, cJSON_CreateNumber(1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Unit, cJSON_CreateString(kCharacteristicUnit_Percentage));
		}
	}

	return serviceJSObject;
}

cJSON *HAPCreateLockManagementService(HAPLockManagementService_t *service)
{
	cJSON *serviceJSObject = NULL;
	cJSON *characteristicJSArray, *characteristicJSObject;
	char *rePerms[2] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_Events};
	char *wPerms[1] = {kCharacteristicPermission_PairedWrite};
	char *rwePerms[3] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_PairedWrite, kCharacteristicPermission_Events};

	if(!service->version) return NULL;

	if((serviceJSObject = cJSON_CreateObject()) != NULL) {
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Type, cJSON_CreateString(kServiceUUID_LockManagement));
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_InstanceID, cJSON_CreateNull());
		characteristicJSArray = cJSON_CreateArray();
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Characteristics, characteristicJSArray);

		// public.hap.characteristic.lock-management.control-point (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_LockControlPoint));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
//		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(wPerms, 1));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Base64TLV8));

		// public.hap.characteristic.version (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Version));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->version));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));

		// public.hap.characteristic.logs (optional)
		if(service->logs) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Logs));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->logs));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Base64TLV8));
		}

		// public.hap.characteristic.audio-feedback (optional)
		if(service->audio_feedback_valid) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_AudioFeedback));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, (service->audio_feedback) ? cJSON_CreateBool(cJSON_True) : cJSON_CreateBool(cJSON_False));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Boolean));
		}

		// public.hap.characteristic.lock-management.auto-secure-timeout (optional)
		if(service->auto_secure_timeout_valid) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_LockAutoSecureTimeout));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(service->auto_secure_timeout));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Uint32));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Unit, cJSON_CreateString(kCharacteristicUnit_Seconds));
		}

		// public.hap.characteristic.administrator-only-access (optional)
		if(service->admin_only_access_valid) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_AdministratorOnlyAccess));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, (service->admin_only_access) ? cJSON_CreateBool(cJSON_True) : cJSON_CreateBool(cJSON_False));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Boolean));
		}

		// public.hap.characteristic.lock-mechanism.last-known-action (optional)
		if(service->last_known_action_valid) {
			uint8_t last_known_action = service->last_known_action;
			if((last_known_action < 0) || (last_known_action > 10)) last_known_action = 0;

			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_LockLastKnownAction));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(last_known_action));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Uint8));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(10));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_StepValue, cJSON_CreateNumber(1));
		}

		// public.hap.characteristic.door-state.current (optional)
		if(service->current_door_state_valid) {
			uint8_t current_door_state = service->current_door_state;
			if((current_door_state < 0) || (current_door_state > 4)) current_door_state = 0;

			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_CurrentDoorState));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(current_door_state));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Uint8));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(4));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_StepValue, cJSON_CreateNumber(1));
		}
	}

	return serviceJSObject;
}

cJSON *HAPCreateLockMechanismService(HAPLockMechanismService_t *service)
{
	cJSON *serviceJSObject = NULL;
	cJSON *characteristicJSArray, *characteristicJSObject;
	char *rePerms[2] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_Events};
	char *rwePerms[3] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_PairedWrite, kCharacteristicPermission_Events};
	int lock_current_state, lock_target_state;

	if((serviceJSObject = cJSON_CreateObject()) != NULL) {
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Type, cJSON_CreateString(kServiceUUID_LockMechanism));
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_InstanceID, cJSON_CreateNull());
		characteristicJSArray = cJSON_CreateArray();
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Characteristics, characteristicJSArray);

		// public.hap.characteristic.lock-mechanism.current-state (required)
		lock_current_state = service->lock_current_state;
		if((lock_current_state < 0) || (lock_current_state > 255)) lock_current_state = 0;
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_LockCurrentState));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(lock_current_state));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Integer));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(255));

		// public.hap.characteristic.lock-mechanism.target-state (required)
		lock_target_state = service->lock_target_state;
		if((lock_target_state < 0) || (lock_target_state > 255)) lock_target_state = 0;
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_LockTargetState));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(lock_target_state));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Integer));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(255));
	}

	return serviceJSObject;
}

cJSON *HAPCreateOutletService(HAPOutletService_t *service)
{
	cJSON *serviceJSObject = NULL;
	cJSON *characteristicJSArray, *characteristicJSObject;
	char *rPerms[1] = {kCharacteristicPermission_PairedRead};
	char *rePerms[2] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_Events};
	char *rwePerms[3] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_PairedWrite, kCharacteristicPermission_Events};

	if((serviceJSObject = cJSON_CreateObject()) != NULL) {
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Type, cJSON_CreateString(kServiceUUID_Outlet));
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_InstanceID, cJSON_CreateNull());
		characteristicJSArray = cJSON_CreateArray();
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Characteristics, characteristicJSArray);

		// public.hap.characteristic.on (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_On));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, (service->on) ? cJSON_CreateBool(cJSON_True) : cJSON_CreateBool(cJSON_False));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Boolean));

		// public.hap.characteristic.outlet-in-use (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_OutletInUse));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, (service->outlet_in_use) ? cJSON_CreateBool(cJSON_True) : cJSON_CreateBool(cJSON_False));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Boolean));

		// public.hap.characteristic.name (optional)
		if(service->name) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
		}
	}

	return serviceJSObject;
}
cJSON *ES_HAPCreateOutletService(HAPOutletService_t *service)
{
	cJSON *serviceJSObject = NULL;
	cJSON *characteristicJSArray, *characteristicJSObject, *chaJSObject;
	char *rPerms[1] = {kCharacteristicPermission_PairedRead};
	char *rePerms[2] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_Events};
	char *rwePerms[3] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_PairedWrite, kCharacteristicPermission_Events};
       char *wPerms[1] = {kCharacteristicPermission_PairedWrite};
	if((serviceJSObject = cJSON_CreateObject()) != NULL) {
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Type, cJSON_CreateString(kServiceUUID_Outlet));
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_InstanceID, cJSON_CreateNull());
		characteristicJSArray = cJSON_CreateArray();
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Characteristics, characteristicJSArray);

		// public.hap.characteristic.on (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_On));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, (service->on) ? cJSON_CreateBool(cJSON_True) : cJSON_CreateBool(cJSON_False));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Boolean));

		// public.hap.characteristic.outlet-in-use (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_OutletInUse));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, (service->outlet_in_use) ? cJSON_CreateBool(cJSON_True) : cJSON_CreateBool(cJSON_False));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Boolean));
#if !FIRST_VERSION       
            //custom.esoft.hap.characteristic.ohd.month
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_Month_Data);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddItemToObject(chaJSObject, kChaObj_Val, cJSON_CreateString(""));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rePerms, 2));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Data));
            
            //custom.esoft.hap.characteristic.timer
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_Timing_On);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddItemToObject(chaJSObject, kChaObj_Val, cJSON_CreateString(""));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rwePerms, 3));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Data));
      
            //custom.esoft.hap.characteristic.meterage.quantity
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_Met_Total_Quantity);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddNumberToObject(chaJSObject, kChaObj_Val, 0);
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rePerms, 2));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Float));
            
            //custom.esoft.hap.characteristic.meterage.voltage
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_Met_Voltage);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddNumberToObject(chaJSObject, kChaObj_Val, 0);
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rePerms, 2));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Float));
            
            //custom.esoft.hap.characteristic.meterage.current
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_Met_Current);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddNumberToObject(chaJSObject, kChaObj_Val, 0);
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rePerms, 2));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Float));
            
            //custom.esoft.hap.characteristic.meterage.power
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_Met_Power);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddNumberToObject(chaJSObject, kChaObj_Val, 0);
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rePerms, 2));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Float));
            
            //custom.esoft.hap.characteristic.threshold.voltage
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_Thr_overVoltage);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddNumberToObject(chaJSObject, kChaObj_Val, 220);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_MinVal, 176);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_MaxVal, 264);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_StepVal, Quantity_Step);
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rwePerms, 3));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Float));

            //custom.esoft.hap.characteristic.threshold.voltage
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_Thr_underVoltage);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddNumberToObject(chaJSObject, kChaObj_Val, 220);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_MinVal, 176);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_MaxVal, 264);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_StepVal, Quantity_Step);
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rwePerms, 3));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Float));
            
            //custom.esoft.hap.characteristic.threshold.current
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_Thr_Current);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddNumberToObject(chaJSObject, kChaObj_Val, 10);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_MinVal, 0);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_MaxVal, 10);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_StepVal, Quantity_Step);
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rwePerms, 3));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Float));
            
            //custom.esoft.hap.characteristic.threshold.power
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_Thr_Power);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddNumberToObject(chaJSObject, kChaObj_Val, 2.2);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_MinVal, 0);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_MaxVal, 2.2);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_StepVal, Quantity_Step);
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rwePerms, 3));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Float));
            
            //custom.esoft.hap.characteristic.firstHistory.data
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_1st_Hisdata);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddItemToObject(chaJSObject, kChaObj_Val, cJSON_CreateString(""));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rePerms, 2));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Data));
            
            //custom.esoft.hap.characteristic.secondHistory.data
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_2nd_Hisdata);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddItemToObject(chaJSObject, kChaObj_Val, cJSON_CreateString(""));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rePerms, 2));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Data));
            
            //custom.esoft.hap.characteristic.thirdHistory.data
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_3rd_Hisdata);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddItemToObject(chaJSObject, kChaObj_Val, cJSON_CreateString(""));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rePerms, 2));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Data));

            //custom.esoft.hap.characteristic.fourthHistory.data
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_4th_Hisdata);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddItemToObject(chaJSObject, kChaObj_Val, cJSON_CreateString(""));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rePerms, 2));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Data));

            //custom.esoft.hap.characteristic.fifthHistory.data
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_5th_Hisdata);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddItemToObject(chaJSObject, kChaObj_Val, cJSON_CreateString(""));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rePerms, 2));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Data));

            //custom.esoft.hap.characteristic.sixthHistory.data
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_6th_Hisdata);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddItemToObject(chaJSObject, kChaObj_Val, cJSON_CreateString(""));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rePerms, 2));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Data));

            //custom.esoft.hap.characteristic.seventhHistory.data
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_7th_Hisdata);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddItemToObject(chaJSObject, kChaObj_Val, cJSON_CreateString(""));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rePerms, 2));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Data));

            //custom.esoft.hap.characteristic.eighthHistory.data
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_8th_Hisdata);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddItemToObject(chaJSObject, kChaObj_Val, cJSON_CreateString(""));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rePerms, 2));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Data));       
#endif
		// public.hap.characteristic.name (optional)
		if(service->name) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
		}
	}

	return serviceJSObject;
}

#if FIRST_VERSION 
cJSON *ES_HAPCreateMeterService(HAPOutletService_t *service)
{
	cJSON *serviceJSObject = NULL;
	cJSON *characteristicJSArray, *characteristicJSObject, *chaJSObject;
	char *rPerms[1] = {kCharacteristicPermission_PairedRead};
	char *rePerms[2] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_Events};
	char *rwePerms[3] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_PairedWrite, kCharacteristicPermission_Events};
       char *wPerms[1] = {kCharacteristicPermission_PairedWrite};
	if((serviceJSObject = cJSON_CreateObject()) != NULL) {
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Type, cJSON_CreateString(kServUUID_FV_Meter));
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_InstanceID, cJSON_CreateNull());
		characteristicJSArray = cJSON_CreateArray();
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Characteristics, characteristicJSArray);
            
            //custom.esoft.hap.characteristic.meterage.voltage
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_FV_Met_Voltage);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddNumberToObject(chaJSObject, kChaObj_Val, 0);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_MinVal, 0);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_MaxVal, 300);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_StepVal, 0.1);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Unit, "volt");
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rPerms, 1));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Float));
            
            //custom.esoft.hap.characteristic.meterage.current
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_FV_Met_Current);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddNumberToObject(chaJSObject, kChaObj_Val, 0);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_MinVal, 0);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_MaxVal, 15);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_StepVal, 0.1);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Unit, "amp");
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rPerms, 1));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Float));
            
            //custom.esoft.hap.characteristic.meterage.power
            chaJSObject = cJSON_CreateObject();
            cJSON_AddItemToArray(characteristicJSArray, chaJSObject);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Type, kChaUUID_FV_Met_Power);
            cJSON_AddItemToObject(chaJSObject, kChaObj_IID, cJSON_CreateNull());
            cJSON_AddNumberToObject(chaJSObject, kChaObj_Val, 0);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_MinVal, 0);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_MaxVal, 2000);
            cJSON_AddNumberToObject(chaJSObject, kChaObj_StepVal, 0.1);
            cJSON_AddStringToObject(chaJSObject, kChaObj_Unit, "watt");
            cJSON_AddItemToObject(chaJSObject, kChaObj_Perms, cJSON_CreateStringArray(rPerms, 1));
            cJSON_AddItemToObject(chaJSObject, kChaObj_Format, cJSON_CreateString(kChaFormat_Float));

		// public.hap.characteristic.name (optional)
		if(service->name) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
		}
	}

	return serviceJSObject;
}
#endif


cJSON *HAPCreateSwitchService(HAPSwitchService_t *service)
{
	cJSON *serviceJSObject = NULL;
	cJSON *characteristicJSArray, *characteristicJSObject;
	char *rPerms[1] = {kCharacteristicPermission_PairedRead};
	char *rwePerms[3] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_PairedWrite, kCharacteristicPermission_Events};

	if((serviceJSObject = cJSON_CreateObject()) != NULL) {
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Type, cJSON_CreateString(kServiceUUID_Switch));
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_InstanceID, cJSON_CreateNull());
		characteristicJSArray = cJSON_CreateArray();
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Characteristics, characteristicJSArray);

		// public.hap.characteristic.on (required)
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_On));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, (service->on) ? cJSON_CreateBool(cJSON_True) : cJSON_CreateBool(cJSON_False));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Boolean));

		// public.hap.characteristic.name (optional)
		if(service->name) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
		}
	}

	return serviceJSObject;
}

cJSON *HAPCreateThermostatService(HAPThermostatService_t *service)
{
	cJSON *serviceJSObject = NULL;
	cJSON *characteristicJSArray, *characteristicJSObject;
	char *rPerms[1] = {kCharacteristicPermission_PairedRead};
	char *rePerms[2] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_Events};
	char *rwePerms[3] = {kCharacteristicPermission_PairedRead, kCharacteristicPermission_PairedWrite, kCharacteristicPermission_Events};
	int heating_cooling_current, heating_cooling_target, temperature_units;
	float temperature_current, temperature_target;

	if((serviceJSObject = cJSON_CreateObject()) != NULL) {
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Type, cJSON_CreateString(kServiceUUID_Thermostat));
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_InstanceID, cJSON_CreateNull());
		characteristicJSArray = cJSON_CreateArray();
		cJSON_AddItemToObject(serviceJSObject, kServiceObject_Characteristics, characteristicJSArray);

		// public.hap.characteristic.heating-cooling.current (required)
		heating_cooling_current = service->heating_cooling_current;
		if((heating_cooling_current < 0) || (heating_cooling_current > 2)) heating_cooling_current = 0;
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_CurrentHeatingCoolingMode));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(heating_cooling_current));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Integer));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(2));

		// public.hap.characteristic.heating-cooling.target (required)
		heating_cooling_target = service->heating_cooling_target;
		if((heating_cooling_target < 0) || (heating_cooling_target > 3)) heating_cooling_target = 0;
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_TargetHeatingCoolingMode));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(heating_cooling_target));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Integer));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(3));

		// public.hap.characteristic.temperature.current (required)
		temperature_current = service->temperature_current;
		if((temperature_current < 0) || (temperature_current > 100)) temperature_current = 0;
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_CurrentTemperature));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(temperature_current));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Float));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(100));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_StepValue, cJSON_CreateNumber(0.1));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Unit, cJSON_CreateString(kCharacteristicUnit_DegreesCelsius));

		// public.hap.characteristic.temperature.target (required)
		temperature_target = service->temperature_target;
		if((temperature_target < 10) || (temperature_target > 38)) temperature_target = 10;
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_TargetTemperature));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(temperature_target));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Float));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(10));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(38));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_StepValue, cJSON_CreateNumber(0.1));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Unit, cJSON_CreateString(kCharacteristicUnit_DegreesCelsius));

		// public.hap.characteristic.temperature.units (required)
		temperature_units = service->temperature_units;
		if((temperature_units < 0) || (temperature_units > 1)) temperature_units = 0;
		characteristicJSObject = cJSON_CreateObject();
		cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_TemperatureUnits));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(temperature_units));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Integer));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
		cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(1));

		// public.hap.characteristic.name (optional)
		if(service->name) {
			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_Name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateString(service->name));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rPerms, 1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_String));
		}

		// public.hap.characteristic.relative-humidity.current (optional)
		if(service->relative_humidity_current_valid) {
			float relative_humidity_current = service->relative_humidity_current;
			if((relative_humidity_current < 0) || (relative_humidity_current > 100)) relative_humidity_current = 0;

			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_CurrentRelativeHumidity));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(relative_humidity_current));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rePerms, 2));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Float));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(100));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_StepValue, cJSON_CreateNumber(1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Unit, cJSON_CreateString(kCharacteristicUnit_Percentage));
		}

		// public.hap.characteristic.relative-humidity.target (optional)
		if(service->relative_humidity_target_valid) {
			float relative_humidity_target = service->relative_humidity_target;
			if((relative_humidity_target < 0) || (relative_humidity_target > 100)) relative_humidity_target = 0;

			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_TargetRelativeHumidity));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(relative_humidity_target));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Float));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(100));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_StepValue, cJSON_CreateNumber(1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Unit, cJSON_CreateString(kCharacteristicUnit_Percentage));
		}

		// public.hap.characteristic.temperature.cooling-threshold (optional)
		if(service->temperature_cooling_threshold_valid) {
			float temperature_cooling_threshold = service->temperature_cooling_threshold;
			if((temperature_cooling_threshold < 10) || (temperature_cooling_threshold > 35)) temperature_cooling_threshold = 10;

			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_CoolingThresholdTemperature));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(temperature_cooling_threshold));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Float));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(10));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(35));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_StepValue, cJSON_CreateNumber(0.1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Unit, cJSON_CreateString(kCharacteristicUnit_DegreesCelsius));
		}

		// public.hap.characteristic.temperature.heating-threshold (optional)
		if(service->temperature_heating_threshold_valid) {
			float temperature_heating_threshold = service->temperature_heating_threshold;
			if((temperature_heating_threshold < 0) || (temperature_heating_threshold > 25)) temperature_heating_threshold = 0;

			characteristicJSObject = cJSON_CreateObject();
			cJSON_AddItemToArray(characteristicJSArray, characteristicJSObject);
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Type, cJSON_CreateString(kCharacteristicUUID_HeatingThresholdTemperature));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_InstanceID, cJSON_CreateNull());
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Value, cJSON_CreateNumber(temperature_heating_threshold));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Permissions, cJSON_CreateStringArray(rwePerms, 3));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Format, cJSON_CreateString(kCharacteristicFormat_Float));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MinimumValue, cJSON_CreateNumber(0));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_MaximumValue, cJSON_CreateNumber(25));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_StepValue, cJSON_CreateNumber(0.1));
			cJSON_AddItemToObject(characteristicJSObject, kCharacteristicObject_Unit, cJSON_CreateString(kCharacteristicUnit_DegreesCelsius));
		}
	}

	return serviceJSObject;
}
