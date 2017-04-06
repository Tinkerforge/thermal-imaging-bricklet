/* thermal-imaging-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
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

#include "bricklib2/bootloader/bootloader.h"

// Default functions
BootloaderHandleMessageResponse handle_message(const void *data, void *response);
void communication_tick(void);
void communication_init(void);

// Constants
#define THERMAL_IMAGING_RESOLUTION_0_TO_6553_KELVIN 0
#define THERMAL_IMAGING_RESOLUTION_0_TO_655_KELVIN 1

#define THERMAL_IMAGING_DATA_TRANSFER_MANUAL_HIGH_CONTRAST_IMAGE 0
#define THERMAL_IMAGING_DATA_TRANSFER_MANUAL_TEMPERATURE_IMAGE 1
#define THERMAL_IMAGING_DATA_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE 2
#define THERMAL_IMAGING_DATA_TRANSFER_CALLBACK_TEMPERATURE_IMAGE 3

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

#define FID_CALLBACK_HIGH_CONTRAST_IMAGE_LOW_LEVEL 12
#define FID_CALLBACK_TEMPERATURE_IMAGE_LOW_LEVEL 13

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetHighContrastImageLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint16_t stream_chunk_offset;
	uint8_t stream_chunk_data[62];
} __attribute__((__packed__)) GetHighContrastImageLowLevelResponse;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetTemperatureImageLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint16_t stream_chunk_offset;
	uint16_t stream_chunk_data[31];
} __attribute__((__packed__)) GetTemperatureImageLowLevelResponse;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetStatistics;

typedef struct {
	TFPMessageHeader header;
	uint16_t spotmeter_statistics[4];
	uint16_t temperatures[4];
	uint8_t  resolution;
	uint16_t status;
} __attribute__((__packed__)) GetStatisticsResponse;

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
} __attribute__((__packed__)) GetResolutionResponse;

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
} __attribute__((__packed__)) GetSpotmeterConfigResponse;

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
} __attribute__((__packed__)) GetHighContrastConfigResponse;

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
} __attribute__((__packed__)) GetImageTransferConfigResponse;

typedef struct {
	TFPMessageHeader header;
	uint16_t stream_chunk_offset;
	uint8_t stream_chunk_data[62];
} __attribute__((__packed__)) HighContrastImageLowLevelCallback;

typedef struct {
	TFPMessageHeader header;
	uint16_t stream_chunk_offset;
	uint16_t stream_chunk_data[31];
} __attribute__((__packed__)) TemperatureImageLowLevelCallback;


// Function prototypes
BootloaderHandleMessageResponse get_high_contrast_image_low_level(const GetHighContrastImageLowLevel *data, GetHighContrastImageLowLevelResponse *response);
BootloaderHandleMessageResponse get_temperature_image_low_level(const GetTemperatureImageLowLevel *data, GetTemperatureImageLowLevelResponse *response);
BootloaderHandleMessageResponse get_statistics(const GetStatistics *data, GetStatisticsResponse *response);
BootloaderHandleMessageResponse set_resolution(const SetResolution *data);
BootloaderHandleMessageResponse get_resolution(const GetResolution *data, GetResolutionResponse *response);
BootloaderHandleMessageResponse set_spotmeter_config(const SetSpotmeterConfig *data);
BootloaderHandleMessageResponse get_spotmeter_config(const GetSpotmeterConfig *data, GetSpotmeterConfigResponse *response);
BootloaderHandleMessageResponse set_high_contrast_config(const SetHighContrastConfig *data);
BootloaderHandleMessageResponse get_high_contrast_config(const GetHighContrastConfig *data, GetHighContrastConfigResponse *response);
BootloaderHandleMessageResponse set_image_transfer_config(const SetImageTransferConfig *data);
BootloaderHandleMessageResponse get_image_transfer_config(const GetImageTransferConfig *data, GetImageTransferConfigResponse *response);

// Callbacks
bool handle_high_contrast_image_low_level_callback(void);
bool handle_temperature_image_low_level_callback(void);

#define COMMUNICATION_CALLBACK_TICK_WAIT_MS 1
#define COMMUNICATION_CALLBACK_HANDLER_NUM 2
#define COMMUNICATION_CALLBACK_LIST_INIT \
	handle_high_contrast_image_low_level_callback, \
	handle_temperature_image_low_level_callback, \


#endif
