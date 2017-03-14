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

#include "configs/config_lepton.h"

#include "bricklib2/hal/system_timer/system_timer.h"

#include "xmc_i2c.h"
#include "xmc_spi.h"
#include "xmc_gpio.h"
#include "xmc_usic.h"

void lepton_i2c_write(Lepton *lepton, const uint16_t reg, const uint32_t length, const uint8_t *data, bool send_stop) {
	// Send address
	XMC_I2C_CH_MasterStart(LEPTON_I2C, LEPTON_I2C_ADDRESS, XMC_I2C_CH_CMD_WRITE);

	// Wait for ACK
	while(!(XMC_I2C_CH_GetStatusFlag(LEPTON_I2C) & XMC_I2C_CH_STATUS_FLAG_ACK_RECEIVED));
	XMC_I2C_CH_ClearStatusFlag(LEPTON_I2C, XMC_I2C_CH_STATUS_FLAG_ACK_RECEIVED);

	// Transmit 16 bit register
	XMC_I2C_CH_MasterTransmit(LEPTON_I2C, reg >> 8);
	XMC_I2C_CH_MasterTransmit(LEPTON_I2C, reg & 0xFF);

	// Transmit data
	uint32_t index = 0;
	while(index < length) {
		while(!XMC_USIC_CH_TXFIFO_IsFull(LEPTON_I2C)) {
			if(index < length) {
				XMC_I2C_CH_MasterTransmit(LEPTON_I2C, data[index]);
				index++;
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

void lepton_i2c_read(Lepton *lepton, const uint16_t reg, const uint32_t length, uint8_t *data) {
	// Write register address to read from without stop bit
	lepton_i2c_write(lepton, reg, 0, NULL, false);

	// Repeated start
	XMC_I2C_CH_MasterRepeatedStart(LEPTON_I2C, LEPTON_I2C_ADDRESS, XMC_I2C_CH_CMD_READ);

	// Wait for ACK
	while(!(XMC_I2C_CH_GetStatusFlag(LEPTON_I2C) & XMC_I2C_CH_STATUS_FLAG_ACK_RECEIVED));
	XMC_I2C_CH_ClearStatusFlag(LEPTON_I2C, XMC_I2C_CH_STATUS_FLAG_ACK_RECEIVED);

	uint32_t fifo_index = 0;
	uint32_t data_index = 0;
	while(fifo_index < length) {
		while(!XMC_USIC_CH_RXFIFO_IsFull(LEPTON_I2C)) {
			if(fifo_index < length) {
				if(fifo_index+1 == length) {
					XMC_I2C_CH_MasterReceiveNack(LEPTON_I2C);
				} else {
					XMC_I2C_CH_MasterReceiveAck(LEPTON_I2C);
				}
			} else {
				break;
			}
		}

		while(data_index < fifo_index) {
			while(XMC_USIC_CH_RXFIFO_IsEmpty(LEPTON_I2C)) {
				data[data_index] = XMC_I2C_CH_GetReceivedData(LEPTON_I2C);
				data_index++;
			}
		}
	}
}

void lepton_reset(Lepton *lepton) {
	XMC_GPIO_SetOutputLow(LEPTON_NRESET_PIN);
	lepton->reset_start_time = system_timer_get_ms();
}

void lepton_init_spi(Lepton *lepton) {
	XMC_I2C_CH_Stop(LEPTON_I2C);

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
		.mode             = LEPTON_SELECT_PIN_AF,
		.output_level     = XMC_GPIO_OUTPUT_LEVEL_HIGH
	};

	// Configure MISO pin
	XMC_GPIO_Init(LEPTON_MISO_PIN, &miso_pin_config);

	// Initialize USIC channel in SPI master mode
	XMC_SPI_CH_Init(LEPTON_SPI, &channel_config);
	LEPTON_SPI->SCTR &= ~USIC_CH_SCTR_PDL_Msk; // Set passive data level to 0

	XMC_SPI_CH_SetBitOrderMsbFirst(LEPTON_SPI);

	XMC_SPI_CH_SetWordLength(LEPTON_SPI, (uint8_t)8U);
	XMC_SPI_CH_SetFrameLength(LEPTON_SPI, (uint8_t)64U);

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

	// Configure receive FIFO
	XMC_USIC_CH_RXFIFO_Configure(LEPTON_SPI, 32, XMC_USIC_CH_FIFO_SIZE_32WORDS, 0);

	// Set service request for rx FIFO receive interrupt
	XMC_USIC_CH_RXFIFO_SetInterruptNodePointer(LEPTON_SPI, XMC_USIC_CH_RXFIFO_INTERRUPT_NODE_POINTER_STANDARD, LEPTON_SERVICE_REQUEST_RX);  // IRQ LEPTON_IRQ_RX
	XMC_USIC_CH_RXFIFO_SetInterruptNodePointer(LEPTON_SPI, XMC_USIC_CH_RXFIFO_INTERRUPT_NODE_POINTER_ALTERNATE, LEPTON_SERVICE_REQUEST_RX); // IRQ LEPTON_IRQ_RX

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
}

void lepton_init_i2c(Lepton *lepton) {
	XMC_SPI_CH_Stop(LEPTON_SPI);

	const XMC_I2C_CH_CONFIG_t master_channel_config = {
		.baudrate = 400000,
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
	XMC_USIC_CH_SetInputSource(LEPTON_I2C, LEPTON_SDA_INPUT, LEPTON_SDA_SOURCE);

	XMC_USIC_CH_TXFIFO_Configure(LEPTON_I2C, 48, XMC_USIC_CH_FIFO_SIZE_16WORDS, 1);
	XMC_USIC_CH_RXFIFO_Configure(LEPTON_I2C, 32,  XMC_USIC_CH_FIFO_SIZE_16WORDS, 15);
	XMC_I2C_CH_Start(LEPTON_I2C);

	XMC_GPIO_Init(LEPTON_SDA_PIN, &sda_pin_config);
	XMC_GPIO_Init(LEPTON_SCL_PIN, &scl_pin_config);
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
	lepton_init_gpio(lepton);
	lepton_init_i2c(lepton);
	lepton_reset(lepton);
}

uint32_t last_time = 0;

void lepton_tick(Lepton *lepton) {
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
			lepton->boot_start_time = 0;
		}

		return;
	}

	if(system_timer_is_time_elapsed_ms(last_time, 1000)) {
		last_time = system_timer_get_ms();

	}
}

bool lepton_is_ready(Lepton *lepton) {
	return (lepton->reset_start_time == 0) && (lepton->boot_start_time == 0);
}
