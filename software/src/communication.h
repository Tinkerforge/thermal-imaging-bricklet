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
#define THERMAL_IMAGING_CALLBACK_CONFIG_CALLBACK_OFF 0
#define THERMAL_IMAGING_CALLBACK_CONFIG_CALLBACK_IMAGE 1
#define THERMAL_IMAGING_CALLBACK_CONFIG_CALLBACK_RAW_IMAGE 2

// Function and callback IDs and structs
#define FID_SET_AUTOMATIC_GAIN_CONTROL_CONFIG 1
#define FID_GET_AUTOMATIC_GAIN_CONTROL_CONFIG 2
#define FID_SET_CALLBACK_CONFIG 3
#define FID_GET_CALLBACK_CONFIG 4

#define FID_CALLBACK_IMAGE_LOW_LEVEL 5
#define FID_CALLBACK_RAW_IMAGE_LOW_LEVEL 6

typedef struct {
	TFPMessageHeader header;
	uint8_t region_of_interest[4];
	uint16_t dampening_factor;
	uint16_t clip_limit[2];
	uint16_t empty_counts;
} __attribute__((__packed__)) SetAutomaticGainControlConfig;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetAutomaticGainControlConfig;

typedef struct {
	TFPMessageHeader header;
	uint8_t region_of_interest[4];
	uint16_t dampening_factor;
	uint16_t clip_limit[2];
	uint16_t empty_counts;
} __attribute__((__packed__)) GetAutomaticGainControlConfigResponse;

typedef struct {
	TFPMessageHeader header;
	uint8_t callback_config;
} __attribute__((__packed__)) SetCallbackConfig;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetCallbackConfig;

typedef struct {
	TFPMessageHeader header;
	uint8_t callback_config;
} __attribute__((__packed__)) GetCallbackConfigResponse;

typedef struct {
	TFPMessageHeader header;
	uint16_t stream_chunk_offset;
	uint8_t stream_chunk_data[62];
} __attribute__((__packed__)) ImageLowLevelCallback;

typedef struct {
	TFPMessageHeader header;
	uint16_t stream_chunk_offset;
	uint16_t stream_chunk_data[31];
} __attribute__((__packed__)) RawImageLowLevelCallback;


// Function prototypes
BootloaderHandleMessageResponse set_automatic_gain_control_config(const SetAutomaticGainControlConfig *data);
BootloaderHandleMessageResponse get_automatic_gain_control_config(const GetAutomaticGainControlConfig *data, GetAutomaticGainControlConfigResponse *response);
BootloaderHandleMessageResponse set_callback_config(const SetCallbackConfig *data);
BootloaderHandleMessageResponse get_callback_config(const GetCallbackConfig *data, GetCallbackConfigResponse *response);

// Callbacks
bool handle_image_low_level_callback(void);
bool handle_raw_image_low_level_callback(void);

#define COMMUNICATION_CALLBACK_TICK_WAIT_MS 1
#define COMMUNICATION_CALLBACK_HANDLER_NUM 2
#define COMMUNICATION_CALLBACK_LIST_INIT \
	handle_image_low_level_callback, \
	handle_raw_image_low_level_callback, \


#endif
