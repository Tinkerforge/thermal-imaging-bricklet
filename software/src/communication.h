/* thermal-imaging-bricklet
 * Copyright (C) 2023 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.h: TFP protocol message handling
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>

#include "bricklib2/protocols/tfp/tfp.h"
#include "bricklib2/bootloader/bootloader.h"

// Default functions
BootloaderHandleMessageResponse handle_message(const void *data, void *response);
void communication_tick(void);
void communication_init(void);

// Constants

#define THERMAL_IMAGING_RESOLUTION_0_TO_6553_KELVIN 0
#define THERMAL_IMAGING_RESOLUTION_0_TO_655_KELVIN 1

#define THERMAL_IMAGING_FFC_STATUS_NEVER_COMMANDED 0
#define THERMAL_IMAGING_FFC_STATUS_IMMINENT 1
#define THERMAL_IMAGING_FFC_STATUS_IN_PROGRESS 2
#define THERMAL_IMAGING_FFC_STATUS_COMPLETE 3

#define THERMAL_IMAGING_IMAGE_TRANSFER_MANUAL_HIGH_CONTRAST_IMAGE 0
#define THERMAL_IMAGING_IMAGE_TRANSFER_MANUAL_TEMPERATURE_IMAGE 1
#define THERMAL_IMAGING_IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE 2
#define THERMAL_IMAGING_IMAGE_TRANSFER_CALLBACK_TEMPERATURE_IMAGE 3

#define THERMAL_IMAGING_SHUTTER_MODE_MANUAL 0
#define THERMAL_IMAGING_SHUTTER_MODE_AUTO 1
#define THERMAL_IMAGING_SHUTTER_MODE_EXTERNAL 2

#define THERMAL_IMAGING_SHUTTER_LOCKOUT_INACTIVE 0
#define THERMAL_IMAGING_SHUTTER_LOCKOUT_HIGH 1
#define THERMAL_IMAGING_SHUTTER_LOCKOUT_LOW 2

#define THERMAL_IMAGING_BOOTLOADER_MODE_BOOTLOADER 0
#define THERMAL_IMAGING_BOOTLOADER_MODE_FIRMWARE 1
#define THERMAL_IMAGING_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2
#define THERMAL_IMAGING_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3
#define THERMAL_IMAGING_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

#define THERMAL_IMAGING_BOOTLOADER_STATUS_OK 0
#define THERMAL_IMAGING_BOOTLOADER_STATUS_INVALID_MODE 1
#define THERMAL_IMAGING_BOOTLOADER_STATUS_NO_CHANGE 2
#define THERMAL_IMAGING_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3
#define THERMAL_IMAGING_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4
#define THERMAL_IMAGING_BOOTLOADER_STATUS_CRC_MISMATCH 5

#define THERMAL_IMAGING_STATUS_LED_CONFIG_OFF 0
#define THERMAL_IMAGING_STATUS_LED_CONFIG_ON 1
#define THERMAL_IMAGING_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2
#define THERMAL_IMAGING_STATUS_LED_CONFIG_SHOW_STATUS 3

// Function and callback IDs and structs
#define FID_GET_HIGH_CONTRAST_IMAGE_LOW_LEVEL 1
#define FID_GET_TEMPERATURE_IMAGE_LOW_LEVEL 2
#define FID_GET_STATISTICS 3
#define FID_SET_RESOLUTION 4
#define FID_GET_RESOLUTION 5
#define FID_SET_SPOTMETER_CONFIG 6
#define FID_GET_SPOTMETER_CONFIG 7
#define FID_SET_HIGH_CONTRAST_CONFIG 8
#define FID_GET_HIGH_CONTRAST_CONFIG 9
#define FID_SET_IMAGE_TRANSFER_CONFIG 10
#define FID_GET_IMAGE_TRANSFER_CONFIG 11
#define FID_SET_FLUX_LINEAR_PARAMETERS 14
#define FID_GET_FLUX_LINEAR_PARAMETERS 15
#define FID_SET_FFC_SHUTTER_MODE 16
#define FID_GET_FFC_SHUTTER_MODE 17
#define FID_RUN_FFC_NORMALIZATION 18

#define FID_CALLBACK_HIGH_CONTRAST_IMAGE_LOW_LEVEL 12
#define FID_CALLBACK_TEMPERATURE_IMAGE_LOW_LEVEL 13

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetHighContrastImageLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint16_t image_chunk_offset;
	uint8_t image_chunk_data[62];
} __attribute__((__packed__)) GetHighContrastImageLowLevel_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetTemperatureImageLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint16_t image_chunk_offset;
	uint16_t image_chunk_data[31];
} __attribute__((__packed__)) GetTemperatureImageLowLevel_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetStatistics;

typedef struct {
	TFPMessageHeader header;
	uint16_t spotmeter_statistics[4];
	uint16_t temperatures[4];
	uint8_t resolution;
	uint8_t ffc_status;
	uint8_t temperature_warning[1];
} __attribute__((__packed__)) GetStatistics_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t resolution;
} __attribute__((__packed__)) SetResolution;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetResolution;

typedef struct {
	TFPMessageHeader header;
	uint8_t resolution;
} __attribute__((__packed__)) GetResolution_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t region_of_interest[4];
} __attribute__((__packed__)) SetSpotmeterConfig;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetSpotmeterConfig;

typedef struct {
	TFPMessageHeader header;
	uint8_t region_of_interest[4];
} __attribute__((__packed__)) GetSpotmeterConfig_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t region_of_interest[4];
	uint16_t dampening_factor;
	uint16_t clip_limit[2];
	uint16_t empty_counts;
} __attribute__((__packed__)) SetHighContrastConfig;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetHighContrastConfig;

typedef struct {
	TFPMessageHeader header;
	uint8_t region_of_interest[4];
	uint16_t dampening_factor;
	uint16_t clip_limit[2];
	uint16_t empty_counts;
} __attribute__((__packed__)) GetHighContrastConfig_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t config;
} __attribute__((__packed__)) SetImageTransferConfig;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetImageTransferConfig;

typedef struct {
	TFPMessageHeader header;
	uint8_t config;
} __attribute__((__packed__)) GetImageTransferConfig_Response;

typedef struct {
	TFPMessageHeader header;
	uint16_t image_chunk_offset;
	uint8_t image_chunk_data[62];
} __attribute__((__packed__)) HighContrastImageLowLevel_Callback;

typedef struct {
	TFPMessageHeader header;
	uint16_t image_chunk_offset;
	uint16_t image_chunk_data[31];
} __attribute__((__packed__)) TemperatureImageLowLevel_Callback;

typedef struct {
	TFPMessageHeader header;
	uint16_t scene_emissivity;
	uint16_t temperature_background;
	uint16_t tau_window;
	uint16_t temperatur_window;
	uint16_t tau_atmosphere;
	uint16_t temperature_atmosphere;
	uint16_t reflection_window;
	uint16_t temperature_reflection;
} __attribute__((__packed__)) SetFluxLinearParameters;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetFluxLinearParameters;

typedef struct {
	TFPMessageHeader header;
	uint16_t scene_emissivity;
	uint16_t temperature_background;
	uint16_t tau_window;
	uint16_t temperatur_window;
	uint16_t tau_atmosphere;
	uint16_t temperature_atmosphere;
	uint16_t reflection_window;
	uint16_t temperature_reflection;
} __attribute__((__packed__)) GetFluxLinearParameters_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t shutter_mode;
	uint8_t temp_lockout_state;
	bool video_freeze_during_ffc;
	bool ffc_desired;
	uint32_t elapsed_time_since_last_ffc;
	uint32_t desired_ffc_period;
	bool explicit_cmd_to_open;
	uint16_t desired_ffc_temp_delta;
	uint16_t imminent_delay;
} __attribute__((__packed__)) SetFFCShutterMode;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetFFCShutterMode;

typedef struct {
	TFPMessageHeader header;
	uint8_t shutter_mode;
	uint8_t temp_lockout_state;
	bool video_freeze_during_ffc;
	bool ffc_desired;
	uint32_t elapsed_time_since_last_ffc;
	uint32_t desired_ffc_period;
	bool explicit_cmd_to_open;
	uint16_t desired_ffc_temp_delta;
	uint16_t imminent_delay;
} __attribute__((__packed__)) GetFFCShutterMode_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) RunFFCNormalization;


// Function prototypes
BootloaderHandleMessageResponse get_high_contrast_image_low_level(const GetHighContrastImageLowLevel *data, GetHighContrastImageLowLevel_Response *response);
BootloaderHandleMessageResponse get_temperature_image_low_level(const GetTemperatureImageLowLevel *data, GetTemperatureImageLowLevel_Response *response);
BootloaderHandleMessageResponse get_statistics(const GetStatistics *data, GetStatistics_Response *response);
BootloaderHandleMessageResponse set_resolution(const SetResolution *data);
BootloaderHandleMessageResponse get_resolution(const GetResolution *data, GetResolution_Response *response);
BootloaderHandleMessageResponse set_spotmeter_config(const SetSpotmeterConfig *data);
BootloaderHandleMessageResponse get_spotmeter_config(const GetSpotmeterConfig *data, GetSpotmeterConfig_Response *response);
BootloaderHandleMessageResponse set_high_contrast_config(const SetHighContrastConfig *data);
BootloaderHandleMessageResponse get_high_contrast_config(const GetHighContrastConfig *data, GetHighContrastConfig_Response *response);
BootloaderHandleMessageResponse set_image_transfer_config(const SetImageTransferConfig *data);
BootloaderHandleMessageResponse get_image_transfer_config(const GetImageTransferConfig *data, GetImageTransferConfig_Response *response);
BootloaderHandleMessageResponse set_flux_linear_parameters(const SetFluxLinearParameters *data);
BootloaderHandleMessageResponse get_flux_linear_parameters(const GetFluxLinearParameters *data, GetFluxLinearParameters_Response *response);
BootloaderHandleMessageResponse set_ffc_shutter_mode(const SetFFCShutterMode *data);
BootloaderHandleMessageResponse get_ffc_shutter_mode(const GetFFCShutterMode *data, GetFFCShutterMode_Response *response);
BootloaderHandleMessageResponse run_ffc_normalization(const RunFFCNormalization *data);

// Callbacks
bool handle_high_contrast_image_low_level_callback(void);
bool handle_temperature_image_low_level_callback(void);

#define COMMUNICATION_CALLBACK_TICK_WAIT_MS 1
#define COMMUNICATION_CALLBACK_HANDLER_NUM 2
#define COMMUNICATION_CALLBACK_LIST_INIT \
	handle_high_contrast_image_low_level_callback, \
	handle_temperature_image_low_level_callback, \


#endif
