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


// Function and callback IDs and structs
#define FID_SET_CALLBACK_CONFIG 1
#define FID_GET_CALLBACK_CONFIG 2

#define FID_CALLBACK_GREY_SCALE_IMAGE_LOW_LEVEL 3
#define FID_CALLBACK_TEMPERATURE_IMAGE_LOW_LEVEL 4

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
} __attribute__((__packed__)) GreyScaleImageLowLevelCallback;

typedef struct {
	TFPMessageHeader header;
	uint16_t stream_chunk_offset;
	uint16_t stream_chunk_data[31];
} __attribute__((__packed__)) TemperatureImageLowLevelCallback;


// Function prototypes
BootloaderHandleMessageResponse set_callback_config(const SetCallbackConfig *data);
BootloaderHandleMessageResponse get_callback_config(const GetCallbackConfig *data, GetCallbackConfigResponse *response);

// Callbacks
bool handle_grey_scale_image_low_level_callback(void);
bool handle_temperature_image_low_level_callback(void);

#define COMMUNICATION_CALLBACK_TICK_WAIT_MS 1
#define COMMUNICATION_CALLBACK_HANDLER_NUM 2
#define COMMUNICATION_CALLBACK_LIST_INIT \
	handle_grey_scale_image_low_level_callback, \
	handle_temperature_image_low_level_callback, \


#endif
