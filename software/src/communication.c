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
		case FID_GET_HIGH_CONTRAST_IMAGE_LOW_LEVEL: return get_high_contrast_image_low_level(message, response);
		case FID_GET_TEMPERATURE_IMAGE_LOW_LEVEL: return get_temperature_image_low_level(message, response);
		case FID_GET_STATISTICS: return get_statistics(message, response);
		case FID_SET_RESOLUTION: return set_resolution(message);
		case FID_GET_RESOLUTION: return get_resolution(message, response);
		case FID_SET_SPOTMETER_CONFIG: return set_spotmeter_config(message);
		case FID_GET_SPOTMETER_CONFIG: return get_spotmeter_config(message, response);
		case FID_SET_HIGH_CONTRAST_CONFIG: return set_high_contrast_config(message);
		case FID_GET_HIGH_CONTRAST_CONFIG: return get_high_contrast_config(message, response);
		case FID_SET_IMAGE_TRANSFER_CONFIG: return set_image_transfer_config(message);
		case FID_GET_IMAGE_TRANSFER_CONFIG: return get_image_transfer_config(message, response);
		default: return HANDLE_MESSAGE_RESPONSE_NOT_SUPPORTED;
	}
}

BootloaderHandleMessageResponse get_high_contrast_image_low_level(const GetHighContrastImageLowLevel *data, GetHighContrastImageLowLevel_Response *response) {
	static uint32_t packet_payload_index = 0;
	static LeptonPacket *lepton_packet = lepton.frame.data.packets;

	response->header.length = sizeof(GetHighContrastImageLowLevel_Response);

	if(lepton.stream_callback_config != THERMAL_IMAGING_DATA_TRANSFER_MANUAL_HIGH_CONTRAST_IMAGE) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	lepton.manual_transfer_ongoing =  true;

	uint8_t length = 62;
	if(lepton.image_buffer_stream_index + 62 > LEPTON_IMAGE_BUFFER_SIZE) {
		length = LEPTON_IMAGE_BUFFER_SIZE - lepton.image_buffer_stream_index;
	}

	response->stream_chunk_offset = lepton.image_buffer_stream_index;

	if(packet_payload_index + length > LEPTON_PACKET_PAYLOAD_SIZE) {
		uint32_t length_first_packet = LEPTON_PACKET_PAYLOAD_SIZE - packet_payload_index;
		for(uint8_t i = 0; i < length_first_packet; i++) {
			response->stream_chunk_data[i] = lepton_packet->vospi.payload[packet_payload_index];
			packet_payload_index++;
		}

		lepton_packet++;
		packet_payload_index = 0;
		for(uint8_t i = length_first_packet; i < length; i++) {
			response->stream_chunk_data[i] = lepton_packet->vospi.payload[packet_payload_index];
			packet_payload_index++;
		}
	} else {
		for(uint8_t i = 0; i < length; i++) {
			response->stream_chunk_data[i] = lepton_packet->vospi.payload[packet_payload_index];
			packet_payload_index++;
		}
	}

	lepton.image_buffer_stream_index += length;
	if(lepton.image_buffer_stream_index == LEPTON_IMAGE_BUFFER_SIZE) {
		lepton.state = LEPTON_STATE_READ_FRAME;
		lepton.manual_transfer_ongoing =  false;
		lepton.image_buffer_stream_index = 0;
		lepton_packet = lepton.frame.data.packets;
		packet_payload_index = 0;
		lepton.stream_callback_config = lepton.current_callback_config;
	}

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}
BootloaderHandleMessageResponse get_temperature_image_low_level(const GetTemperatureImageLowLevel *data, GetTemperatureImageLowLevel_Response *response) {
	static uint32_t packet_payload_index = 0;
	static LeptonPacket *lepton_packet = lepton.frame.data.packets;

	response->header.length = sizeof(GetTemperatureImageLowLevel_Response);

	if(lepton.stream_callback_config != THERMAL_IMAGING_DATA_TRANSFER_MANUAL_TEMPERATURE_IMAGE) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	lepton.manual_transfer_ongoing =  true;

	uint8_t length = 31;
	if(lepton.image_buffer_stream_index + 31 > LEPTON_IMAGE_BUFFER_SIZE) {
		length = LEPTON_IMAGE_BUFFER_SIZE - lepton.image_buffer_stream_index;
	}

	response->stream_chunk_offset = lepton.image_buffer_stream_index;

	if(packet_payload_index + length > LEPTON_PACKET_PAYLOAD_SIZE) {
		uint32_t length_first_packet = LEPTON_PACKET_PAYLOAD_SIZE - packet_payload_index;
		for(uint8_t i = 0; i < length_first_packet; i++) {
			response->stream_chunk_data[i] = lepton_packet->vospi.payload[packet_payload_index];
			packet_payload_index++;
		}

		lepton_packet++;
		packet_payload_index = 0;
		for(uint8_t i = length_first_packet; i < length; i++) {
			response->stream_chunk_data[i] = lepton_packet->vospi.payload[packet_payload_index];
			packet_payload_index++;
		}
	} else {
		for(uint8_t i = 0; i < length; i++) {
			response->stream_chunk_data[i] = lepton_packet->vospi.payload[packet_payload_index];
			packet_payload_index++;
		}
	}

	lepton.image_buffer_stream_index += length;
	if(lepton.image_buffer_stream_index == LEPTON_IMAGE_BUFFER_SIZE) {
		lepton.state = LEPTON_STATE_READ_FRAME;
		lepton.manual_transfer_ongoing =  false;
		lepton.image_buffer_stream_index = 0;
		lepton_packet = lepton.frame.data.packets;
		packet_payload_index = 0;
		lepton.stream_callback_config = lepton.current_callback_config;
	}

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse get_statistics(const GetStatistics *data, GetStatistics_Response *response) {
	response->header.length = sizeof(GetStatistics_Response);

	// Use double buffered statistics, they are already stored and packed in correct format for this getter
	memcpy(response->spotmeter_statistics, &lepton.statistics, sizeof(LeptonStatistics));

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_resolution(const SetResolution *data) {
	if(data->resolution > THERMAL_IMAGING_RESOLUTION_0_TO_655_KELVIN) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}
	if(data->resolution != lepton.resolution) {
		lepton.resolution      = data->resolution;
		lepton.config_bitmask |= LEPTON_CONFIG_BITMASK_RESOLUTION;
	}

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_resolution(const GetResolution *data, GetResolution_Response *response) {
	response->header.length = sizeof(GetResolution_Response);
	response->resolution    = lepton.resolution;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_spotmeter_config(const SetSpotmeterConfig *data) {
	if((data->region_of_interest[0] >= data->region_of_interest[2]) ||
	   (data->region_of_interest[1] >= data->region_of_interest[3]) ||
	   (data->region_of_interest[2] >= LEPTON_FRAME_COLS) ||
	   (data->region_of_interest[3] >= LEPTON_FRAME_ROWS)) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	if((lepton.spotmeter_roi[0] != data->region_of_interest[0]) ||
	   (lepton.spotmeter_roi[1] != data->region_of_interest[1]) ||
	   (lepton.spotmeter_roi[2] != data->region_of_interest[2]) ||
	   (lepton.spotmeter_roi[3] != data->region_of_interest[3])) {
		lepton.spotmeter_roi[0] = data->region_of_interest[0];
		lepton.spotmeter_roi[1] = data->region_of_interest[1];
		lepton.spotmeter_roi[2] = data->region_of_interest[2];
		lepton.spotmeter_roi[3] = data->region_of_interest[3];
		lepton.config_bitmask  |= LEPTON_CONFIG_BITMASK_SPOTMETER_ROI;
	}

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_spotmeter_config(const GetSpotmeterConfig *data, GetSpotmeterConfig_Response *response) {
	response->header.length = sizeof(GetSpotmeterConfig_Response);
	response->region_of_interest[0] = lepton.spotmeter_roi[0];
	response->region_of_interest[1] = lepton.spotmeter_roi[1];
	response->region_of_interest[2] = lepton.spotmeter_roi[2];
	response->region_of_interest[3] = lepton.spotmeter_roi[3];

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_high_contrast_config(const SetHighContrastConfig *data) {
	if((data->region_of_interest[0] > data->region_of_interest[2]) || // ">" is correct here
	   (data->region_of_interest[1] >= data->region_of_interest[3]) ||
	   (data->region_of_interest[2] >= LEPTON_FRAME_COLS) ||
	   (data->region_of_interest[3] >= LEPTON_FRAME_ROWS) ||
	   (data->dampening_factor > 256) ||
	   (data->clip_limit[0] > 4800) ||
	   (data->clip_limit[1] > 1024) ||
	   (data->empty_counts > 16383)) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	if((lepton.agc.region_of_interest[0] != data->region_of_interest[0]) ||
	   (lepton.agc.region_of_interest[1] != data->region_of_interest[1]) ||
	   (lepton.agc.region_of_interest[2] != data->region_of_interest[2]) ||
	   (lepton.agc.region_of_interest[3] != data->region_of_interest[3])) {
		lepton.agc.region_of_interest[0] = data->region_of_interest[0];
		lepton.agc.region_of_interest[1] = data->region_of_interest[1];
		lepton.agc.region_of_interest[2] = data->region_of_interest[2];
		lepton.agc.region_of_interest[3] = data->region_of_interest[3];
		lepton.config_bitmask |= LEPTON_CONFIG_BITMASK_AGC_ROI;
	}

	if(lepton.agc.dampening_factor != data->dampening_factor) {
		lepton.agc.dampening_factor = data->dampening_factor;
		lepton.config_bitmask |= LEPTON_CONFIG_BITMASK_AGC_DAMPENING_FACTOR;
	}

	if((lepton.agc.clip_limit[0] != data->clip_limit[0]) ||
	   (lepton.agc.clip_limit[1] != data->clip_limit[1])) {
		lepton.agc.clip_limit[0] = data->clip_limit[0];
		lepton.agc.clip_limit[1] = data->clip_limit[1];
		lepton.config_bitmask |= LEPTON_CONFIG_BITMASK_AGC_CLIP_LIMIT;
	}

	if(lepton.agc.empty_counts != data->empty_counts) {
		lepton.agc.empty_counts = data->empty_counts;
		lepton.config_bitmask |= LEPTON_CONFIG_BITMASK_AGC_EMPTY_COUNTS;
	}

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_high_contrast_config(const GetHighContrastConfig *data, GetHighContrastConfig_Response *response) {
	response->header.length = sizeof(GetHighContrastConfig_Response);
	memcpy(response->region_of_interest, &lepton.agc, sizeof(LeptonAutomaticGainControl));

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_image_transfer_config(const SetImageTransferConfig *data) {
	if(data->config > THERMAL_IMAGING_DATA_TRANSFER_CALLBACK_TEMPERATURE_IMAGE) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	if(lepton.current_callback_config != data->config) {
		lepton.config_bitmask |= LEPTON_CONFIG_BITMASK_AGC_ENABLE;
		lepton.current_callback_config = data->config;
	}

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_image_transfer_config(const GetImageTransferConfig *data, GetImageTransferConfig_Response *response) {
	response->header.length = sizeof(GetImageTransferConfig_Response);
	response->config        = lepton.current_callback_config;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}



bool handle_high_contrast_image_low_level_callback(void) {
	static bool is_buffered = false;
	static HighContrastImageLowLevel_Callback cb;
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

		tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(HighContrastImageLowLevel_Callback), FID_CALLBACK_HIGH_CONTRAST_IMAGE_LOW_LEVEL);
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
			lepton.stream_callback_config = lepton.current_callback_config;
		}
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(HighContrastImageLowLevel_Callback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}
	return false;
}

bool handle_temperature_image_low_level_callback(void) {
	static bool is_buffered = false;
	static TemperatureImageLowLevel_Callback cb;
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

		tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(TemperatureImageLowLevel_Callback), FID_CALLBACK_TEMPERATURE_IMAGE_LOW_LEVEL);
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
			lepton.stream_callback_config = lepton.current_callback_config;
		}
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(TemperatureImageLowLevel_Callback));
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

		if((lepton.stream_callback_config != lepton.current_callback_config) &&
		   (lepton.stream_callback_config == THERMAL_IMAGING_DATA_TRANSFER_MANUAL_HIGH_CONTRAST_IMAGE || lepton.stream_callback_config == THERMAL_IMAGING_DATA_TRANSFER_MANUAL_TEMPERATURE_IMAGE) &&
		   (!lepton.manual_transfer_ongoing)) {
			lepton.stream_callback_config = lepton.current_callback_config;
		}

		if(lepton.stream_callback_config == THERMAL_IMAGING_DATA_TRANSFER_MANUAL_HIGH_CONTRAST_IMAGE || lepton.stream_callback_config == THERMAL_IMAGING_DATA_TRANSFER_MANUAL_TEMPERATURE_IMAGE) {
			if(lepton.state == LEPTON_STATE_WRITE_FRAME && !lepton.manual_transfer_ongoing) {
				lepton.state = LEPTON_STATE_READ_FRAME;
				return;
			}
		}

		switch(lepton.stream_callback_config) {
			case THERMAL_IMAGING_DATA_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE: handle_high_contrast_image_low_level_callback(); break;
			case THERMAL_IMAGING_DATA_TRANSFER_CALLBACK_TEMPERATURE_IMAGE: handle_temperature_image_low_level_callback(); break;
			default: break;
		}
	}
}
