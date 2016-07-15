/*
 *  Copyright (c) 2014 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#ifndef _HAP_SERVICE_PROFILE_H
#define _HAP_SERVICE_PROFILE_H

#include <stdint.h>
#include <cJSON.h>

/* Accessory Information Service */
typedef struct {
	char *manufacturer;        // public.hap.characteristic.manufacturer (required)
	char *model;               // public.hap.characteristic.model (required)
	char *name;                // public.hap.characteristic.name (required)
	char *serial_number;       // public.hap.characteristic.serial-number (required)
	char *firmware_revision;   // public.hap.characteristic.firmware.revision (optional)
	char *hardware_revision;   // public.hap.characteristic.hardware.revision (optional)
	char *software_revision;   // public.hap.characteristic.software.revision (optional)
} HAPInformationService_t;

cJSON *HAPCreateInformationService(HAPInformationService_t *service);

/* Fan Service */
typedef struct {
	uint8_t on;                         // public.hap.characteristic.on (required)
	int     rotation_direction;         // public.hap.characteristic.rotation.direction (optional)
	uint8_t rotation_direction_valid;
	float   rotation_speed;             // public.hap.characteristic.rotation.speed (optional)
	uint8_t rotation_speed_valid;
        char*   name;                       // public.hap.characteristic.name (optional)
} HAPFanService_t;

cJSON *HAPCreateFanService(HAPFanService_t *service);

/* Garage Door Opener Service */
typedef struct {
	int     door_state_current;         // public.hap.characteristic.door-state.current (required)
	int     door_state_target;          // public.hap.characteristic.door-state.target (required)
	uint8_t obstruction_detected;       // public.hap.characteristic.obstruction-detected (required)
	int     lock_current_state;         // public.hap.characteristic.lock-mechanism.current-state (optional)
	uint8_t lock_current_state_valid;
	int     lock_target_state;          // public.hap.characteristic.lock-mechanism.target-state (optional)
	uint8_t lock_target_state_valid;
	char*   name;                       // public.hap.characteristic.name (optional)
} HAPGarageDoorOpenerService_t;

cJSON *HAPCreateGarageDoorOpenerService(HAPGarageDoorOpenerService_t *service);

/* Lightbulb Service */
typedef struct {
	uint8_t on;                  // public.hap.characteristic.on (required)
	int     brightness;          // public.hap.characteristic.brightness (optional)
	uint8_t brightness_valid;
	float   hue;                 // public.hap.characteristic.hue (optional)
	uint8_t hue_valid;
	char*   name;                // public.hap.characteristic.name (optional)
	float   saturation;          // public.hap.characteristic.saturation (optional)
	uint8_t saturation_valid;
} HAPLightbulbService_t;

cJSON *HAPCreateLightbulbService(HAPLightbulbService_t *service);

/* Lock Management Service */
typedef struct {
	char*    version;                   // public.hap.characteristic.version (required)
	char*    logs;                      // public.hap.characteristic.logs (optional)
	uint8_t  audio_feedback;            // public.hap.characteristic.audio-feedback (optional)
	uint8_t  audio_feedback_valid;
	uint32_t auto_secure_timeout;       // public.hap.characteristic.lock-management.auto-secure-timeout (optional)
	uint8_t  auto_secure_timeout_valid;
	uint8_t  admin_only_access;         // public.hap.characteristic.administrator-only-access (optional)
	uint8_t  admin_only_access_valid;
	uint8_t  last_known_action;         // public.hap.characteristic.lock-mechanism.last-known-action (optional)
	uint8_t  last_known_action_valid;
	uint8_t  current_door_state;        // public.hap.characteristic.door-state.current (optional)
	uint8_t  current_door_state_valid;
        char*   name;                       // public.hap.characteristic.name (optional)
} HAPLockManagementService_t;

cJSON *HAPCreateLockManagementService(HAPLockManagementService_t *service);

/* Lock Mechanism Service */
typedef struct {
	int lock_current_state;    // public.hap.characteristic.lock-mechanism.current-state (required)
	int lock_target_state;     // public.hap.characteristic.lock-mechanism.target-state (required)
        char*   name;                       // public.hap.characteristic.name (optional)
} HAPLockMechanismService_t;

cJSON *HAPCreateLockMechanismService(HAPLockMechanismService_t *service);

/* Outlet Service */
typedef struct {
	uint8_t on;                  // public.hap.characteristic.on (required)
	uint8_t outlet_in_use;       // public.hap.characteristic.outlet-in-use (required)
	char*   name;                // public.hap.characteristic.name (optional)
} HAPOutletService_t;

cJSON *HAPCreateOutletService(HAPOutletService_t *service);

/* Switch Service */
typedef struct {
	uint8_t on;                  // public.hap.characteristic.on (required)
	char*   name;                // public.hap.characteristic.name (optional)
} HAPSwitchService_t;

cJSON *HAPCreateSwitchService(HAPSwitchService_t *service);

/* Thermostat Service */
typedef struct {
	int     heating_cooling_current;              // public.hap.characteristic.heating-cooling.current (required)
	int     heating_cooling_target;               // public.hap.characteristic.heating-cooling.target (required)
	float   temperature_current;                  // public.hap.characteristic.temperature.current (required)
	float   temperature_target;                   // public.hap.characteristic.temperature.target (required)
	int     temperature_units;                    // public.hap.characteristic.temperature.units (required)
	char*   name;                                 // public.hap.characteristic.name (optional)
	float   relative_humidity_current;            // public.hap.characteristic.relative-humidity.current (optional)
	uint8_t relative_humidity_current_valid;
	float   relative_humidity_target;             // public.hap.characteristic.relative-humidity.target (optional)
	uint8_t relative_humidity_target_valid;
	float   temperature_cooling_threshold;        // public.hap.characteristic.temperature.cooling-threshold (optional)
	uint8_t temperature_cooling_threshold_valid;
	float   temperature_heating_threshold;        // public.hap.characteristic.temperature.heating-threshold (optional)
	uint8_t temperature_heating_threshold_valid;
} HAPThermostatService_t;

cJSON *HAPCreateThermostatService(HAPThermostatService_t *service);
cJSON *ES_HAPCreateOutletService(HAPOutletService_t *service);
cJSON *ES_HAPCreateMeterService(HAPOutletService_t *service);
#endif  /* _HAP_SERVICE_PROFILE_H */
