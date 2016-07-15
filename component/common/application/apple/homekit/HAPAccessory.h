/*
 *  Copyright (c) 2014 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#ifndef __HAP_ACCESSORY_H__
#define __HAP_ACCESSORY_H__

#include <homekit/HAP.h>
#include <cJSON.h>

/* Accessory Category */
#define kAccessoryCategory_Other               "1"
#define kAccessoryCategory_Bridge              "2"
#define kAccessoryCategory_Fan                 "3"
#define kAccessoryCategory_GarageDoorOpener    "4"
#define kAccessoryCategory_Lightbulb           "5"
#define kAccessoryCategory_DoorLock            "6"
#define kAccessoryCategory_Outlet              "7"
#define kAccessoryCategory_Switch              "8"
#define kAccessoryCategory_Thermostat          "9"

/* Service UUID */
#define kServiceUUID_AccessoryInformation      "3E"
#define kServiceUUID_Fan                       "40"
#define kServiceUUID_GarageDoorOpener          "41"
#define kServiceUUID_Lightbulb                 "43"
#define kServiceUUID_LockManagement            "44"
#define kServiceUUID_LockMechanism             "45"
#define kServiceUUID_Outlet                    "47"
#define kServiceUUID_Switch                    "49"
#define kServiceUUID_Thermostat                "4A"

/* Characteristic UUID */
#define kCharacteristicUUID_AdministratorOnlyAccess       "1"
#define kCharacteristicUUID_AudioFeedback                 "5"
#define kCharacteristicUUID_Brightness                    "8"
#define kCharacteristicUUID_CoolingThresholdTemperature   "D"
#define kCharacteristicUUID_CurrentDoorState              "E"
#define kCharacteristicUUID_CurrentHeatingCoolingMode     "F"
#define kCharacteristicUUID_CurrentRelativeHumidity       "10"
#define kCharacteristicUUID_CurrentTemperature            "11"
#define kCharacteristicUUID_HeatingThresholdTemperature   "12"
#define kCharacteristicUUID_Hue                           "13"
#define kCharacteristicUUID_Identify                      "14"
#define kCharacteristicUUID_LockControlPoint              "19"
#define kCharacteristicUUID_LockAutoSecureTimeout         "1A"
#define kCharacteristicUUID_LockLastKnownAction           "1C"
#define kCharacteristicUUID_LockCurrentState              "1D"
#define kCharacteristicUUID_LockTargetState               "1E"
#define kCharacteristicUUID_Logs                          "1F"
#define kCharacteristicUUID_Manufacturer                  "20"
#define kCharacteristicUUID_Model                         "21"
#define kCharacteristicUUID_MotionDetected                "22"
#define kCharacteristicUUID_Name                          "23"
#define kCharacteristicUUID_ObstructionDetected           "24"
#define kCharacteristicUUID_On                            "25"
#define kCharacteristicUUID_OutletInUse                   "26"
#define kCharacteristicUUID_OutputVolume                  "27"
#define kCharacteristicUUID_RotationDirection             "28"
#define kCharacteristicUUID_RotationSpeed                 "29"
#define kCharacteristicUUID_Saturation                    "2F"
#define kCharacteristicUUID_SerialNumber                  "30"
#define kCharacteristicUUID_TargetDoorState               "32"
#define kCharacteristicUUID_TargetHeatingCoolingMode      "33"
#define kCharacteristicUUID_TargetRelativeHumidity        "34"
#define kCharacteristicUUID_TargetTemperature             "35"
#define kCharacteristicUUID_TemperatureUnits              "36"
#define kCharacteristicUUID_Version                       "37"
#define kCharacteristicUUID_FirmwareRevision              "52"
#define kCharacteristicUUID_HardwareRevision              "53"
#define kCharacteristicUUID_SoftwareRevision              "54"

/* Accessories in JSON */
#define kDatabase_Accessories                  "accessories"

/* Accessory Object */
#define kAccessoryObject_InstanceID            "aid"
#define kAccessoryObject_Services              "services"

/* Service Object */
#define kServiceObject_Type                    "type"
#define kServiceObject_InstanceID              "iid"
#define kServiceObject_Characteristics         "characteristics"

/* Characteristic Object */
#define kCharacteristicObject_Type             "type"
#define kCharacteristicObject_InstanceID       "iid"
#define kCharacteristicObject_Value            "value"
#define kCharacteristicObject_Permissions      "perms"
#define kCharacteristicObject_Event            "ev"
#define kCharacteristicObject_Description      "description"
#define kCharacteristicObject_Format           "format"
#define kCharacteristicObject_Unit             "unit"
#define kCharacteristicObject_MinimumValue     "minValue"
#define kCharacteristicObject_MaximumValue     "maxValue"
#define kCharacteristicObject_StepValue        "minStep"
#define kCharacteristicObject_MaxLength        "maxLen"
#define kCharacteristicObject_MaxDataLength    "maxDataLen"

/* Characteristic Permission */
#define kCharacteristicPermission_PairedRead   "pr"
#define kCharacteristicPermission_PairedWrite  "pw"
#define kCharacteristicPermission_Events       "ev"

/* Characteristic Format Type */
#define kCharacteristicFormat_Boolean          "bool"
#define kCharacteristicFormat_Integer          "int"
#define kCharacteristicFormat_Uint8            "uint8"
#define kCharacteristicFormat_Uint16           "uint16"
#define kCharacteristicFormat_Uint32           "uint32"
#define kCharacteristicFormat_Float            "float"
#define kCharacteristicFormat_String           "string"
#define kCharacteristicFormat_UTF8Date         "date"
#define kCharacteristicFormat_Base64TLV8       "tlv8"
#define kCharacteristicFormat_Base64Data       "data"
#define kCharacteristicFormat_JSONArray        "array"
#define kCharacteristicFormat_NestedJSON       "dict"

/* Characteristic Unit Type */
#define kCharacteristicUnit_DegreesCelsius     "celsius"
#define kCharacteristicUnit_Percentage         "percentage"
#define kCharacteristicUnit_ArcDegrees         "arcdegrees"
#define kCharacteristicUnit_Seconds            "seconds"

/* HAP Status Code */
#define kHAP_StatusCode                        "status"

#define kHAPStatus_Success                     0            /* This Specifies a success for the request */
#define kHAPStatus_Failure                     -1

#define kHAPStatus_RequestDenied               -70401       /* Request denied due to insufficient privileges */
#define kHAPStatus_ServiceUnreachable          -70402       /* Unable to communicate with requested service */
#define kHAPStatus_ResourceBusy                -70403       /* Resource is busy, try again */
#define kHAPStatus_CharacteristicReadOnly      -70404       /* Cannot write to read only characteristic */
#define kHAPStatus_CharacteristicWriteOnly     -70405       /* Cannot read from a write only characteristic */
#define kHAPStatus_NotificationNotSupport      -70406       /* Notification is not supported for characteristic */
#define kHAPStatus_OutOfResource               -70407       /* Out of resources to process request */
#define kHAPStatus_OperationTimeout            -70408       /* Operation timed out */
#define kHAPStatus_ResourceNotExist            -70409       /* Resource does not exist */
#define kHAPStatus_InvalidValue                -70410       /* Accessory received an invalid value in a write request */

void IdentifyRoutine(void);
int AccessoryPlatformInitialize(void);
cJSON *AccessoryDatabaseSetup(HAPParameter_t *param);
int AccessoryOperationHandler(int aid, int iid, cJSON *valueJSObject);

#endif	/* __HAP_ACCESSORY_H__ */

