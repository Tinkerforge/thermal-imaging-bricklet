/* thermal-imaging-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * lepton.c: Lepton thermal imaging camera module communication
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

#include "lepton.h"

#include <stdbool.h>

#include "configs/config.h"
#include "configs/config_lepton.h"

#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/hal/uartbb/uartbb.h"
#include "bricklib2/utility/crc16.h"

#include "xmc_i2c.h"
#include "xmc_spi.h"
#include "xmc_gpio.h"
#include "xmc_usic.h"

#define lepton_rx_irq_handler IRQ_Hdlr_11

extern Lepton lepton;

uint16_t * const lepton_packet_start = lepton.packet.buffer;
uint16_t * const lepton_packet_end = lepton.packet.buffer + LEPTON_PACKET_SIZE;
uint16_t *lepton_packet_pointer = lepton.packet.buffer;


void __attribute__((optimize("-O3"))) lepton_rx_irq_handler(void) {
	XMC_GPIO_SetOutputHigh(UARTBB_TX_PIN);
	*lepton_packet_pointer++ = LEPTON_SPI->OUTR;
	*lepton_packet_pointer++ = LEPTON_SPI->OUTR;
	*lepton_packet_pointer++ = LEPTON_SPI->OUTR;
	*lepton_packet_pointer++ = LEPTON_SPI->OUTR;
	*lepton_packet_pointer++ = LEPTON_SPI->OUTR;
	*lepton_packet_pointer++ = LEPTON_SPI->OUTR;
	*lepton_packet_pointer++ = LEPTON_SPI->OUTR;
	*lepton_packet_pointer++ = LEPTON_SPI->OUTR;
	XMC_GPIO_SetOutputLow(UARTBB_TX_PIN);
}

void lepton_i2c_write(Lepton *lepton, const uint16_t reg, const uint16_t length, const uint16_t *data, bool send_stop) {
	// Send address
	XMC_I2C_CH_MasterStart(LEPTON_I2C, LEPTON_I2C_ADDRESS << 1, XMC_I2C_CH_CMD_WRITE);

	// Wait for ACK
	while(!(XMC_I2C_CH_GetStatusFlag(LEPTON_I2C) & XMC_I2C_CH_STATUS_FLAG_ACK_RECEIVED));
	XMC_I2C_CH_ClearStatusFlag(LEPTON_I2C, XMC_I2C_CH_STATUS_FLAG_ACK_RECEIVED);

	// Transmit 16 bit register
	XMC_I2C_CH_MasterTransmit(LEPTON_I2C, reg >> 8);
	XMC_I2C_CH_MasterTransmit(LEPTON_I2C, reg & 0xFF);

	// Transmit data
	uint32_t index = 0;
	bool high = true;
	while(index < length) {
		while(!XMC_USIC_CH_TXFIFO_IsFull(LEPTON_I2C)) {
			if(index < length) {
				if(high) {
					// Send high byte first
					XMC_I2C_CH_MasterTransmit(LEPTON_I2C, data[index] >> 8);
					high = false;
				} else {
					// and low byte second
					XMC_I2C_CH_MasterTransmit(LEPTON_I2C, data[index] & 0xFF);
					high = true;
					index++;
				}
			} else {
				break;
			}
		}
	}

	// Wait for data to be written
	while(!XMC_USIC_CH_TXFIFO_IsEmpty(LEPTON_I2C));

	if(send_stop) {
		// Send stop
		XMC_I2C_CH_MasterStop(LEPTON_I2C);
	}
}

void lepton_i2c_read(Lepton *lepton, const uint16_t reg, const uint32_t length, uint16_t *data) {
	// Write register address to read from without stop bit
	lepton_i2c_write(lepton, reg, 0, NULL, false);

	// Repeated start
	XMC_I2C_CH_MasterRepeatedStart(LEPTON_I2C, LEPTON_I2C_ADDRESS << 1, XMC_I2C_CH_CMD_READ);

	// Wait for ACK
	while(!(XMC_I2C_CH_GetStatusFlag(LEPTON_I2C) & XMC_I2C_CH_STATUS_FLAG_ACK_RECEIVED));
	XMC_I2C_CH_ClearStatusFlag(LEPTON_I2C, XMC_I2C_CH_STATUS_FLAG_ACK_RECEIVED);

	uint32_t fifo_index = 0;
	uint32_t data_index = 0;
	bool high = true;
	while(fifo_index < length*2) {
		while(!XMC_USIC_CH_RXFIFO_IsFull(LEPTON_I2C)) {
			if(fifo_index < length*2) {
				if(fifo_index+1 == length*2) {
					XMC_I2C_CH_MasterReceiveNack(LEPTON_I2C);
				} else {
					XMC_I2C_CH_MasterReceiveAck(LEPTON_I2C);
				}
				fifo_index++;
			} else {
				break;
			}
		}

		while(data_index < fifo_index) {
			while(XMC_USIC_CH_RXFIFO_IsEmpty(LEPTON_I2C));
			if(high) {
				// Get high byte first
				data[data_index/2] = XMC_I2C_CH_GetReceivedData(LEPTON_I2C) << 8;
				high = false;
			} else {
				// and low byte second
				data[data_index/2] |= XMC_I2C_CH_GetReceivedData(LEPTON_I2C);
				high = true;
			}
			data_index++;
		}
	}
	XMC_I2C_CH_MasterStop(LEPTON_I2C);
}

void lepton_spi_select(Lepton *lepton) {
//	XMC_SPI_CH_EnableSlaveSelect(LEPTON_SPI, XMC_SPI_CH_SLAVE_SELECT_0);
	XMC_GPIO_SetOutputLow(LEPTON_SELECT_PIN);
}

void lepton_spi_deselect(Lepton *lepton) {
	XMC_GPIO_SetOutputHigh(LEPTON_SELECT_PIN);
//	XMC_SPI_CH_DisableSlaveSelect(LEPTON_SPI);
}

uint16_t lepton_get_status(Lepton *lepton) {
	uint16_t status = 0;
	lepton_i2c_read(lepton, LEPTON_I2C_STATUS_REG, 1, &status);
	return status;
}

bool lepton_is_busy(Lepton *lepton) {
	uint16_t status = lepton_get_status(lepton);
	return status & LEPTON_I2C_STATUS_BUSY_BITMASK;
}

void lepton_command_run(Lepton *lepton, const uint16_t command_id) {
	while(lepton_is_busy(lepton));

	// Write command
	uint16_t command = command_id | LEPTON_I2C_COMMAND_TYPE_RUN;
	lepton_i2c_write(lepton, LEPTON_I2C_COMMAND_REG, 1, &command, true);

	while(lepton_is_busy(lepton));
}

void lepton_attribute_read(Lepton *lepton, const uint16_t command_id, const uint16_t length, uint16_t *data) {
	while(lepton_is_busy(lepton));

	// Write length to be read
	lepton_i2c_write(lepton, LEPTON_I2C_DATA_LENGTH_REG, 1, &length, true);

	// Write command
	uint16_t command = command_id | LEPTON_I2C_COMMAND_TYPE_GET;
	lepton_i2c_write(lepton, LEPTON_I2C_COMMAND_REG, 1, &command, true);

	while(lepton_is_busy(lepton));

	// Read data
	lepton_i2c_read(lepton, LEPTON_I2C_DATA_0_REG, length, data);
}

void lepton_attribute_write(Lepton *lepton, const uint16_t command_id, const uint16_t length, uint16_t *data) {
	while(lepton_is_busy(lepton));

	// Write data (we assume length <= 16 here)
	lepton_i2c_write(lepton, LEPTON_I2C_DATA_0_REG, length, data, true);

	// Write length
	lepton_i2c_write(lepton, LEPTON_I2C_DATA_LENGTH_REG, 1, &length, true);

	// Write command
	uint16_t command = command_id | LEPTON_I2C_COMMAND_TYPE_SET;
	lepton_i2c_write(lepton, LEPTON_I2C_COMMAND_REG, 1, &command, true);

	while(lepton_is_busy(lepton));
}

void lepton_reset(Lepton *lepton) {
	XMC_GPIO_SetOutputLow(LEPTON_NRESET_PIN);
	lepton->reset_start_time = system_timer_get_ms();
	lepton->state = LEPTON_STATE_RESET;
}

void lepton_init_spi(Lepton *lepton) {
	XMC_I2C_CH_Stop(LEPTON_I2C);

	const XMC_GPIO_CONFIG_t sda_pin_config =  {
		.mode         = XMC_GPIO_MODE_INPUT_TRISTATE,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH,
	};

	const XMC_GPIO_CONFIG_t scl_pin_config = {
		.mode         = XMC_GPIO_MODE_INPUT_TRISTATE,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH,
	};

	XMC_GPIO_Init(LEPTON_SDA_PIN, &sda_pin_config);
	XMC_GPIO_Init(LEPTON_SCL_PIN, &scl_pin_config);


	// USIC channel configuration
	const XMC_SPI_CH_CONFIG_t channel_config = {
		.baudrate       = LEPTON_SPI_BAUDRATE,
		.bus_mode       = XMC_SPI_CH_BUS_MODE_MASTER,
		.selo_inversion = XMC_SPI_CH_SLAVE_SEL_INV_TO_MSLS,
		.parity_mode    = XMC_USIC_CH_PARITY_MODE_NONE
	};

	// MISO pin configuration
	const XMC_GPIO_CONFIG_t miso_pin_config = {
		.mode             = XMC_GPIO_MODE_INPUT_TRISTATE,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD
	};

	// SCLK pin configuration
	const XMC_GPIO_CONFIG_t sclk_pin_config = {
		.mode             = LEPTON_SCLK_PIN_AF,
		.output_level     = XMC_GPIO_OUTPUT_LEVEL_HIGH
	};

	// SELECT pin configuration
	const XMC_GPIO_CONFIG_t select_pin_config = {
		.mode             = XMC_GPIO_MODE_OUTPUT_PUSH_PULL, // we set cs high/low by hand
		.output_level     = XMC_GPIO_OUTPUT_LEVEL_HIGH
	};

	// Configure MISO pin
	XMC_GPIO_Init(LEPTON_MISO_PIN, &miso_pin_config);

	// Initialize USIC channel in SPI master mode
	XMC_SPI_CH_Init(LEPTON_SPI, &channel_config);
	LEPTON_SPI->SCTR &= ~USIC_CH_SCTR_PDL_Msk; // Set passive data level to 0
	LEPTON_SPI->PCR_SSCMode &= ~USIC_CH_PCR_SSCMode_TIWEN_Msk; // Disable time between bytes

	XMC_SPI_CH_SetBitOrderMsbFirst(LEPTON_SPI);

	XMC_SPI_CH_SetWordLength(LEPTON_SPI, 16);
	XMC_SPI_CH_SetFrameLength(LEPTON_SPI, 64);

	XMC_SPI_CH_SetTransmitMode(LEPTON_SPI, XMC_SPI_CH_MODE_STANDARD);

	// Configure the clock polarity and clock delay
	XMC_SPI_CH_ConfigureShiftClockOutput(LEPTON_SPI,
									     XMC_SPI_CH_BRG_SHIFT_CLOCK_PASSIVE_LEVEL_1_DELAY_DISABLED,
									     XMC_SPI_CH_BRG_SHIFT_CLOCK_OUTPUT_SCLK);
	// Configure Leading/Trailing delay
	XMC_SPI_CH_SetSlaveSelectDelay(LEPTON_SPI, 2);

	// Set input source path
	XMC_SPI_CH_SetInputSource(LEPTON_SPI, LEPTON_MISO_INPUT, LEPTON_MISO_SOURCE);

	// SPI Mode: CPOL=1 and CPHA=1
	LEPTON_SPI_CHANNEL->DX1CR |= USIC_CH_DX1CR_DPOL_Msk;

	// Configure transmit FIFO
	XMC_USIC_CH_TXFIFO_Configure(LEPTON_SPI, 48, XMC_USIC_CH_FIFO_SIZE_16WORDS, 0);

	// Configure receive FIFO
	XMC_USIC_CH_RXFIFO_Configure(LEPTON_SPI, 32, XMC_USIC_CH_FIFO_SIZE_16WORDS, 8);

	// Set service request for rx FIFO receive interrupt
	XMC_USIC_CH_RXFIFO_SetInterruptNodePointer(LEPTON_SPI, XMC_USIC_CH_RXFIFO_INTERRUPT_NODE_POINTER_STANDARD, LEPTON_SERVICE_REQUEST_RX);  // IRQ LEPTON_IRQ_RX
//	XMC_USIC_CH_RXFIFO_SetInterruptNodePointer(LEPTON_SPI, XMC_USIC_CH_RXFIFO_INTERRUPT_NODE_POINTER_ALTERNATE, LEPTON_SERVICE_REQUEST_RX); // IRQ LEPTON_IRQ_RX

	// Set priority and enable NVIC node for receive interrupt
	NVIC_SetPriority((IRQn_Type)LEPTON_IRQ_RX, LEPTON_IRQ_RX_PRIORITY);
	NVIC_EnableIRQ((IRQn_Type)LEPTON_IRQ_RX);

	// Start SPI
	XMC_SPI_CH_Start(LEPTON_SPI);

	// Configure SCLK pin
	XMC_GPIO_Init(LEPTON_SCLK_PIN, &sclk_pin_config);

	// Configure slave select pin
	XMC_GPIO_Init(LEPTON_SELECT_PIN, &select_pin_config);

	XMC_USIC_CH_RXFIFO_EnableEvent(LEPTON_SPI, XMC_USIC_CH_RXFIFO_EVENT_CONF_STANDARD | XMC_USIC_CH_RXFIFO_EVENT_CONF_ALTERNATE);

	lepton->lepton_active_interface = LEPTON_INTERFACE_SPI;
}

void lepton_init_i2c(Lepton *lepton) {
	XMC_SPI_CH_Stop(LEPTON_SPI);

	const XMC_I2C_CH_CONFIG_t master_channel_config = {
		.baudrate = LEPTON_I2C_BAUDRATE,
		.address  = 0
	};

	const XMC_GPIO_CONFIG_t sda_pin_config =  {
		.mode         = LEPTON_SDA_PIN_MODE,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH,
	};

	const XMC_GPIO_CONFIG_t scl_pin_config = {
		.mode         = LEPTON_SCL_PIN_MODE,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH,
	};

	XMC_I2C_CH_Init(LEPTON_I2C, &master_channel_config);

	XMC_USIC_CH_SetInputSource(LEPTON_I2C, LEPTON_SDA_INPUT, LEPTON_SDA_SOURCE);
	XMC_USIC_CH_SetInputSource(LEPTON_I2C, LEPTON_SCL_INPUT, LEPTON_SCL_SOURCE);

	XMC_USIC_CH_TXFIFO_Configure(LEPTON_I2C, 48, XMC_USIC_CH_FIFO_SIZE_16WORDS, 0);
	XMC_USIC_CH_RXFIFO_Configure(LEPTON_I2C, 32,  XMC_USIC_CH_FIFO_SIZE_16WORDS, 0);
	XMC_I2C_CH_Start(LEPTON_I2C);

	XMC_GPIO_Init(LEPTON_SDA_PIN, &sda_pin_config);
	XMC_GPIO_Init(LEPTON_SCL_PIN, &scl_pin_config);

	lepton->lepton_active_interface = LEPTON_INTERFACE_I2C;
}

void lepton_init_gpio(Lepton *lepton) {
	const XMC_GPIO_CONFIG_t sync_pin_config = {
		.mode             = XMC_GPIO_MODE_INPUT_TRISTATE,
		.output_level     = XMC_GPIO_OUTPUT_LEVEL_HIGH,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD
	};

	const XMC_GPIO_CONFIG_t nreset_pin_config = {
		.mode         = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_LOW,
	};

	XMC_GPIO_Init(LEPTON_SYNC_PIN,   &sync_pin_config);
	XMC_GPIO_Init(LEPTON_NRESET_PIN, &nreset_pin_config);
}

void lepton_init(Lepton *lepton) {
	memset(lepton, 0, sizeof(Lepton));
	lepton_init_gpio(lepton);
	lepton_init_i2c(lepton);
//	lepton_init_spi(lepton);
	lepton_reset(lepton);
	XMC_GPIO_SetOutputLow(UARTBB_TX_PIN);
}

bool lepton_check_crc_of_packet(Lepton *lepton) {
	// First sanity check the id.
	if(((lepton->packet.vospi.id & LEPTON_SPI_DISCARD_PACKET_ID_MASK) != LEPTON_SPI_DISCARD_PACKET_ID_MASK) &&
	   ((lepton->packet.vospi.id & 0x00FF) > LEPTON_PIXEL_ROWS)) {
		return false;
	}

	// Remove first 4 bits of id, set crc to 0 in packet to calculate CRC
	// See p52ff in datasheet.
	lepton->packet.vospi.id &= LEPTON_SPI_PACKET_ID_MASK;
	const uint16_t crc_lepton = lepton->packet.vospi.crc;
	lepton->packet.vospi.crc = 0;

	return crc_lepton == crc16_ccitt_16in(lepton->packet.buffer, LEPTON_PACKET_SIZE);
}

void lepton_copy_packet_to_buffer(Lepton *lepton) {
	// Start from beginning if packet is "discard packet"
	// Note: Discard packet does not seem to have valid CRC, so we have to
	//       check for this before we do the CRC calculation...
	if((lepton->packet.vospi.id & LEPTON_SPI_DISCARD_PACKET_ID_MASK) == LEPTON_SPI_DISCARD_PACKET_ID_MASK) {
		lepton->packet_next_id = 0;
		return;
	}

	// Discard packet with invalid crc.
	if((!lepton_check_crc_of_packet(lepton))) {
		lepton->state = LEPTON_STATE_SYNC;
		return;
	}

	// Start from beginning in case of id mismatch
	if((lepton->packet.vospi.id & 0x00FF) != lepton->packet_next_id) {
		// We missed a packet, we are out of sync!
		// We resync by waiting for the next beginning of a frame
		lepton->packet_next_id = 0;
		return;
	}

	const uint32_t start_index = LEPTON_PIXEL_LINES*lepton->packet_next_id;
	for(uint8_t i = 0; i < LEPTON_PIXEL_ROWS; i++) {
		lepton->image_buffer[start_index + i] = lepton->packet.vospi.payload[i];
	}

	lepton->packet_next_id++;

	lepton->image_buffer_receive_index = start_index + LEPTON_PIXEL_ROWS;
	if(lepton->image_buffer_receive_index == LEPTON_IMAGE_BUFFER_SIZE) {
		lepton->state = LEPTON_STATE_WAIT_FOR_SEND;
		lepton->image_buffer_receive_index = 0;
		lepton->packet_next_id = 0;
	}
}


void lepton_handle_reset(Lepton *lepton) {
	// To reset the Lepton we assert the reset line for LEPTON_RESET_TIME ms
	if(lepton->reset_start_time != 0) {
		if(system_timer_is_time_elapsed_ms(lepton->reset_start_time, LEPTON_RESET_TIME)) {
			lepton->reset_start_time = 0;

			XMC_GPIO_SetOutputHigh(LEPTON_NRESET_PIN);
			lepton->boot_start_time = system_timer_get_ms();
		}

		return;
	}

	// Make sure that we only talk to the Lepton after the specified bootup time
	if(lepton->boot_start_time != 0) {
		if(system_timer_is_time_elapsed_ms(lepton->boot_start_time, LEPTON_BOOTUP_TIME)) {
			const uint16_t status = lepton_get_status(lepton);
			// Check if camera really has bootet
			if(status & LEPTON_I2C_STATUS_BOOT_STATUS_BITMASK) {
				lepton->boot_start_time = 0;
				lepton->state = LEPTON_STATE_INIT;
			}
		}
	}
}

void lepton_handle_init(Lepton *lepton) {
	uint32_t enable = 1;
	lepton_attribute_write(lepton, LEPTON_CID_AGC_ENABLE_STATE, 2, (uint16_t*)&enable);

	lepton->state = LEPTON_STATE_SYNC;
}

void lepton_handle_sync(Lepton *lepton) {
	if(lepton->sync_start_time == 0) {
		lepton_init_spi(lepton);
		lepton->packet_next_id = 0;
		lepton->image_buffer_receive_index = 0;
		lepton->image_buffer_stream_index = 0;

		lepton->sync_start_time = system_timer_get_ms();

		lepton_spi_select(lepton);

	}

	if(system_timer_is_time_elapsed_ms(lepton->sync_start_time, 190)) {

		lepton_spi_deselect(lepton);

		lepton->sync_start_time = 0;
		lepton->state = LEPTON_STATE_READ_PACKET;
	}
}

void lepton_handle_read_packet(Lepton *lepton) {
	// Check if packet is at the end and FIFO has less than 8 words
	// In this case we don't get any interrupts anymore, we read the
	// data directly
	if((lepton_packet_end - lepton_packet_pointer) < 8) {
		XMC_GPIO_SetOutputHigh(UARTBB_TX_PIN);
		XMC_GPIO_SetOutputLow(UARTBB_TX_PIN);
		XMC_GPIO_SetOutputHigh(UARTBB_TX_PIN);
		while(!XMC_USIC_CH_RXFIFO_IsEmpty(LEPTON_SPI)) {
			*lepton_packet_pointer++ = LEPTON_SPI->OUTR;
		}
		XMC_GPIO_SetOutputLow(UARTBB_TX_PIN);
		XMC_GPIO_SetOutputHigh(UARTBB_TX_PIN);
		XMC_GPIO_SetOutputLow(UARTBB_TX_PIN);
	}

	// If we send a whole packet of SPI clocks and we reached the end of our
	// packet pointer, we are done with this packet and can copy it
	if(lepton->packet_index == LEPTON_PACKET_SIZE) {
		if(lepton_packet_pointer == lepton_packet_end) {
			lepton->packet_index = 0;
			lepton_packet_pointer = lepton_packet_start;
			lepton_spi_deselect(lepton);

			// The next state depends on the result of the copying,
			// so the copy function will set the next state itself.
			// If no new state is set, we will keep on receiving packets
			lepton_copy_packet_to_buffer(lepton);
		}
		return;
	}

	// If the current packet index is 0, we have not yet started to read this
	// packet and we still have to enable the SPI chip select
	if(lepton->packet_index == 0) {
		// TODO: Start timeout for a maximum time a packet should take
		//       if timeout is broken, reset state machine and re-sync
		lepton_spi_select(lepton);
	}

	// Put 0 into SPI FIFO until it is full (this is just here to generate a
	// clock, there is no MOSI connected to anything)
	while(lepton->packet_index < LEPTON_PACKET_SIZE && !XMC_USIC_CH_TXFIFO_IsFull(LEPTON_SPI)) {
		LEPTON_SPI->IN[0] = 0;
		lepton->packet_index++;
	}
}

void lepton_handle_configure(Lepton *lepton) {

}

void lepton_handle_wait_for_send(Lepton *lepton) {
#if 0
	if(lepton->image_buffer_stream_index == LEPTON_IMAGE_BUFFER_SIZE) {
		lepton->image_buffer_stream_index = 0;
		lepton->image_buffer_receive_index = 0;
		lepton->state = LEPTON_STATE_READ_PACKET;
	}
#endif
}

uint32_t last_time = 0;

void lepton_tick(Lepton *lepton) {
	switch(lepton->state) {
		case LEPTON_STATE_RESET: lepton_handle_reset(lepton); break;
		case LEPTON_STATE_INIT: lepton_handle_init(lepton); break;
		case LEPTON_STATE_SYNC:  lepton_handle_sync(lepton); break;
		case LEPTON_STATE_READ_PACKET: lepton_handle_read_packet(lepton); break;
		case LEPTON_STATE_CONFIGURE: lepton_handle_configure(lepton); break;
		case LEPTON_STATE_WAIT_FOR_SEND: lepton_handle_wait_for_send(lepton); break;
	}


	if(system_timer_is_time_elapsed_ms(last_time, 1000)) {
		last_time = system_timer_get_ms();


#if 0
		uint32_t enable_state;
		lepton_attribute_read(lepton, LEPTON_CID_AGC_ENABLE_STATE, 2, (uint16_t*)&enable_state);
		uartbb_puts("es: "); uartbb_putu(enable_state); uartbb_putnl();

		uint32_t policy;
		lepton_attribute_read(lepton, LEPTON_CID_AGC_POLICY, 2, (uint16_t*)&policy);
		uartbb_puts("p: "); uartbb_putu(policy); uartbb_putnl();

		LeptonAGCHistogramStatistics hist;

		lepton_attribute_read(lepton, LEPTON_CID_AGC_STATISTICS, 4, (uint16_t*)&hist);
		uartbb_puts("st: ");
		uartbb_putu(hist.min_intensity); uartbb_puts(" ");
		uartbb_putu(hist.max_intensity); uartbb_puts(" ");
		uartbb_putu(hist.mean_intensity); uartbb_puts(" ");
		uartbb_putu(hist.num_pixels); uartbb_puts(" ");
		uartbb_putnl();
#endif
	}
}

bool lepton_has_started(Lepton *lepton) {
	return (lepton->reset_start_time == 0) && (lepton->boot_start_time == 0);
}
