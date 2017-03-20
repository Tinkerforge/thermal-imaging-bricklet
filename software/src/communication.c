/* thermal-imaging-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.c: TFP protocol message handling
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

#include "communication.h"

#include "bricklib2/utility/communication_callback.h"
#include "bricklib2/protocols/tfp/tfp.h"

#include "lepton.h"

extern Lepton lepton;

BootloaderHandleMessageResponse handle_message(const void *message, void *response) {
	switch(tfp_get_fid_from_message(message)) {
		case FID_SET_CALLBACK_CONFIG: return set_callback_config(message);
		case FID_GET_CALLBACK_CONFIG: return get_callback_config(message, response);
		default: return HANDLE_MESSAGE_RESPONSE_NOT_SUPPORTED;
	}
}


BootloaderHandleMessageResponse set_callback_config(const SetCallbackConfig *data) {

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_callback_config(const GetCallbackConfig *data, GetCallbackConfigResponse *response) {
	response->header.length = sizeof(GetCallbackConfigResponse);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}




bool handle_grey_scale_image_low_level_callback(void) {
	static bool is_buffered = false;
	static GreyScaleImageLowLevelCallback cb;

	if(!is_buffered) {
		uint8_t length = 62;
		if(lepton.image_buffer_stream_index + 62 > LEPTON_IMAGE_BUFFER_SIZE) {
			length = LEPTON_IMAGE_BUFFER_SIZE - lepton.image_buffer_stream_index;
		}

		if(length == 0) {
			return false;
		}

//		if(lepton.image_buffer_stream_index + length <= lepton.image_buffer_receive_index) {
		if(LEPTON_IMAGE_BUFFER_SIZE == lepton.image_buffer_receive_index) {
			tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(GreyScaleImageLowLevelCallback), FID_CALLBACK_GREY_SCALE_IMAGE_LOW_LEVEL);

			cb.stream_chunk_offset = lepton.image_buffer_stream_index;
			memcpy(cb.stream_chunk_data, lepton.image_buffer + lepton.image_buffer_stream_index, length);
			lepton.image_buffer_stream_index += length;
#if 0
			if(lepton.image_buffer_stream_index >= LEPTON_IMAGE_BUFFER_SIZE) {
				lepton.image_buffer_stream_index = 0;
			}
#endif
		} else {
			return false;
		}
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(GreyScaleImageLowLevelCallback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}

	return false;
}

bool handle_temperature_image_low_level_callback(void) {
	static bool is_buffered = false;
	static TemperatureImageLowLevelCallback cb;

	if(!is_buffered) {
		//tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(TemperatureImageLowLevelCallback), FID_CALLBACK_TEMPERATURE_IMAGE_LOW_LEVEL);
		// TODO: Implement TemperatureImageLowLevel callback handling

		return false;
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(TemperatureImageLowLevelCallback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}

	return false;
}

void communication_tick(void) {
	static uint32_t counter = 0;
	counter++;
	if(counter == 2) {
		counter = 0;
		handle_grey_scale_image_low_level_callback();
	}
	//communication_callback_tick();
}

void communication_init(void) {
//	communication_callback_init();
}
