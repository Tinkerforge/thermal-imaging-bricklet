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
	if(data->callback_config > THERMAL_IMAGING_CALLBACK_CONFIG_CALLBACK_TEMPERATURE_IMAGE) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	lepton.current_callback_config = data->callback_config;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_callback_config(const GetCallbackConfig *data, GetCallbackConfigResponse *response) {
	response->header.length   = sizeof(GetCallbackConfigResponse);
	response->callback_config = lepton.current_callback_config;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}




bool handle_grey_scale_image_low_level_callback(void) {
	static bool is_buffered = false;
	static GreyScaleImageLowLevelCallback cb;
	static uint32_t packet_payload_index = 0;
	static LeptonPacket *lepton_packet = lepton.frame.data.packets;

	if(!is_buffered) {
		if(lepton.state != LEPTON_STATE_WRITE_FRAME) {
			return false;
		}

		uint8_t length = 62;
		if(lepton.image_buffer_stream_index + 62 > LEPTON_IMAGE_BUFFER_SIZE) {
			length = LEPTON_IMAGE_BUFFER_SIZE - lepton.image_buffer_stream_index;
		}

		if(length == 0) {
			return false;
		}

		tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(GreyScaleImageLowLevelCallback), FID_CALLBACK_GREY_SCALE_IMAGE_LOW_LEVEL);
		cb.stream_chunk_offset = lepton.image_buffer_stream_index;

		if(packet_payload_index + length > LEPTON_PACKET_PAYLOAD_SIZE) {
			uint32_t length_first_packet = LEPTON_PACKET_PAYLOAD_SIZE - packet_payload_index;
			for(uint8_t i = 0; i < length_first_packet; i++) {
				cb.stream_chunk_data[i] = lepton_packet->vospi.payload[packet_payload_index];
				packet_payload_index++;
			}

			lepton_packet++;
			packet_payload_index = 0;
			for(uint8_t i = length_first_packet; i < length; i++) {
				cb.stream_chunk_data[i] = lepton_packet->vospi.payload[packet_payload_index];
				packet_payload_index++;
			}
		} else {
			for(uint8_t i = 0; i < length; i++) {
				cb.stream_chunk_data[i] = lepton_packet->vospi.payload[packet_payload_index];
				packet_payload_index++;
			}
		}

		lepton.image_buffer_stream_index += length;
		if(lepton.image_buffer_stream_index == LEPTON_IMAGE_BUFFER_SIZE) {
			lepton.state = LEPTON_STATE_READ_FRAME;
			lepton.image_buffer_stream_index = 0;
			lepton_packet = lepton.frame.data.packets;
			packet_payload_index = 0;
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
	static uint32_t packet_payload_index = 0;
	static LeptonPacket *lepton_packet = lepton.frame.data.packets;

	if(!is_buffered) {
		if(lepton.state != LEPTON_STATE_WRITE_FRAME) {
			return false;
		}

		uint8_t length = 31;
		if(lepton.image_buffer_stream_index + 31 > LEPTON_IMAGE_BUFFER_SIZE) {
			length = LEPTON_IMAGE_BUFFER_SIZE - lepton.image_buffer_stream_index;
		}

		if(length == 0) {
			return false;
		}

		tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(TemperatureImageLowLevelCallback), FID_CALLBACK_TEMPERATURE_IMAGE_LOW_LEVEL);
		cb.stream_chunk_offset = lepton.image_buffer_stream_index;

		if(packet_payload_index + length > LEPTON_PACKET_PAYLOAD_SIZE) {
			uint32_t length_first_packet = LEPTON_PACKET_PAYLOAD_SIZE - packet_payload_index;
			for(uint8_t i = 0; i < length_first_packet; i++) {
				cb.stream_chunk_data[i] = lepton_packet->vospi.payload[packet_payload_index];
				packet_payload_index++;
			}

			lepton_packet++;
			packet_payload_index = 0;
			for(uint8_t i = length_first_packet; i < length; i++) {
				cb.stream_chunk_data[i] = lepton_packet->vospi.payload[packet_payload_index];
				packet_payload_index++;
			}
		} else {
			for(uint8_t i = 0; i < length; i++) {
				cb.stream_chunk_data[i] = lepton_packet->vospi.payload[packet_payload_index];
				packet_payload_index++;
			}
		}

		lepton.image_buffer_stream_index += length;
		if(lepton.image_buffer_stream_index == LEPTON_IMAGE_BUFFER_SIZE) {
			lepton.state = LEPTON_STATE_READ_FRAME;
			lepton.image_buffer_stream_index = 0;
			lepton_packet = lepton.frame.data.packets;
			packet_payload_index = 0;
		}
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

	// Only handle callbacks every second call to give bootloader code some time to receive/send other messages
	counter++;
	if(counter == 2) {
		counter = 0;
		switch(lepton.current_callback_config) {
			case THERMAL_IMAGING_CALLBACK_CONFIG_CALLBACK_GREY_SCALE_IMAGE: handle_grey_scale_image_low_level_callback(); break;
			case THERMAL_IMAGING_CALLBACK_CONFIG_CALLBACK_TEMPERATURE_IMAGE: handle_temperature_image_low_level_callback(); break;
			case THERMAL_IMAGING_CALLBACK_CONFIG_MANUAL:
			default: break;
		}
	}
}
