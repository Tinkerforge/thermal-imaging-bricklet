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
#include "xmc_eru.h"

#define lepton_rx_read_irq_handler   IRQ_Hdlr_11
#define lepton_rx_remove_irq_handler IRQ_Hdlr_12
#define lepton_sync_handler          IRQ_Hdlr_3

extern Lepton lepton;

uint16_t * const lepton_frame_start = lepton.frame.buffer;
uint16_t * const lepton_frame_end = lepton.frame.buffer + LEPTON_FRAME_SIZE_WITH_TELEMETRY;
uint16_t * const lepton_frame_end_of_first_packet = lepton.frame.buffer + LEPTON_PACKET_SIZE;
uint16_t * const lepton_frame_last_full = lepton.frame.buffer + LEPTON_FRAME_SIZE_WITH_TELEMETRY-12;
uint16_t * const lepton_frame_first_packet_last_full = lepton.frame.buffer + LEPTON_PACKET_SIZE-2;
uint16_t *lepton_frame_pointer = lepton.frame.buffer;
uint16_t *lepton_frame_pointer_dummy = lepton.frame.buffer;

#if 0
uint16_t * const lepton_packet_start = lepton.packet.buffer;
uint16_t * const lepton_packet_end = lepton.packet.buffer + LEPTON_PACKET_SIZE;
uint16_t * const lepton_packet_last_full = lepton.packet.buffer + LEPTON_PACKET_SIZE-2;
uint16_t *lepton_packet_pointer = lepton.packet.buffer;
#endif

#define LEPTON_SET_SPI_LIMIT(limit) do {\
	LEPTON_SPI->RBCTR = (uint32_t)((LEPTON_SPI->RBCTR & (uint32_t)~USIC_CH_RBCTR_LIMIT_Msk) | ((limit) << USIC_CH_RBCTR_LIMIT_Pos));\
} while(0)

#define LEPTON_SPI_WRITE_1() do {\
	LEPTON_SPI->IN[0] = 0;\
} while(0)


#define LEPTON_SPI_WRITE_2() do {\
	LEPTON_SPI_WRITE_1();\
	LEPTON_SPI_WRITE_1();\
} while(0)

#define LEPTON_SPI_WRITE_4() do {\
	LEPTON_SPI_WRITE_2();\
	LEPTON_SPI_WRITE_2();\
} while(0)

#define LEPTON_SPI_WRITE_6() do {\
	LEPTON_SPI_WRITE_4();\
	LEPTON_SPI_WRITE_2();\
} while(0)

#define LEPTON_SPI_WRITE_8() do {\
	LEPTON_SPI_WRITE_6();\
	LEPTON_SPI_WRITE_2();\
} while(0)

#define LEPTON_SPI_WRITE_10() do {\
	LEPTON_SPI_WRITE_8();\
	LEPTON_SPI_WRITE_2();\
} while(0)

#define LEPTON_SPI_WRITE_12() do {\
	LEPTON_SPI_WRITE_10();\
	LEPTON_SPI_WRITE_2();\
} while(0)

#define LEPTON_SPI_WRITE_14() do {\
	LEPTON_SPI_WRITE_12();\
	LEPTON_SPI_WRITE_2();\
} while(0)

#define LEPTON_SPI_WRITE_16() do {\
	LEPTON_SPI_WRITE_14();\
	LEPTON_SPI_WRITE_2();\
} while(0)

#define LEPTON_SPI_READ_1() do {\
	*lepton_frame_pointer++ = LEPTON_SPI->OUTR;\
} while(0)

#define LEPTON_SPI_READ_2() do {\
	LEPTON_SPI_READ_1();\
	LEPTON_SPI_READ_1();\
} while(0)

#define LEPTON_SPI_READ_4() do {\
	LEPTON_SPI_READ_2();\
	LEPTON_SPI_READ_2();\
} while(0)

#define LEPTON_SPI_READ_6() do {\
	LEPTON_SPI_READ_4();\
	LEPTON_SPI_READ_2();\
} while(0)

#define LEPTON_SPI_READ_8() do {\
	LEPTON_SPI_READ_6();\
	LEPTON_SPI_READ_2();\
} while(0)

#define LEPTON_SPI_READ_10() do {\
	LEPTON_SPI_READ_8();\
	LEPTON_SPI_READ_2();\
} while(0)

#define LEPTON_SPI_READ_12() do {\
	LEPTON_SPI_READ_10();\
	LEPTON_SPI_READ_2();\
} while(0)

#define LEPTON_SPI_READ_14() do {\
	LEPTON_SPI_READ_12();\
	LEPTON_SPI_READ_2();\
} while(0)

#define LEPTON_SPI_READ_16() do {\
	LEPTON_SPI_READ_14();\
	LEPTON_SPI_READ_2();\
} while(0)

#define LEPTON_SPI_REMOVE_1() do {\
	LEPTON_SPI->OUTR;\
	lepton_frame_pointer_dummy++;\
} while(0)

#define LEPTON_SPI_REMOVE_2() do {\
	LEPTON_SPI_REMOVE_1();\
	LEPTON_SPI_REMOVE_1();\
} while(0)

#define LEPTON_SPI_REMOVE_4() do {\
	LEPTON_SPI_REMOVE_2();\
	LEPTON_SPI_REMOVE_2();\
} while(0)

#define LEPTON_SPI_REMOVE_6() do {\
	LEPTON_SPI_REMOVE_4();\
	LEPTON_SPI_REMOVE_2();\
} while(0)

#define LEPTON_SPI_REMOVE_8() do {\
	LEPTON_SPI_REMOVE_6();\
	LEPTON_SPI_REMOVE_2();\
} while(0)

#define LEPTON_SPI_REMOVE_10() do {\
	LEPTON_SPI_REMOVE_8();\
	LEPTON_SPI_REMOVE_2();\
} while(0)

#define LEPTON_SPI_REMOVE_12() do {\
	LEPTON_SPI_REMOVE_10();\
	LEPTON_SPI_REMOVE_2();\
} while(0)

#define LEPTON_SPI_REMOVE_14() do {\
	LEPTON_SPI_REMOVE_12();\
	LEPTON_SPI_REMOVE_2();\
} while(0)

#define LEPTON_SPI_REMOVE_16() do {\
	LEPTON_SPI_REMOVE_14();\
	LEPTON_SPI_REMOVE_2();\
} while(0)

typedef enum {
	LEPTON_IRQ_STATE_SYNC,
	LEPTON_IRQ_STATE_DATA
} LeptonIRQState;

static LeptonIRQState lepton_irq_state = LEPTON_IRQ_STATE_SYNC;

void __attribute__((optimize("-O3"))) lepton_sync_handler(void) {
	if(lepton.sync_done) {
		if(lepton.state == LEPTON_STATE_READ_FRAME) {
			XMC_USIC_CH_RXFIFO_SetInterruptNodePointer(LEPTON_SPI, XMC_USIC_CH_RXFIFO_INTERRUPT_NODE_POINTER_STANDARD, LEPTON_SERVICE_REQUEST_READ_RX);   // IRQ LEPTON_IRQ_READ_RX
			NVIC_DisableIRQ((IRQn_Type)LEPTON_IRQ_REMOVE_RX);
			NVIC_EnableIRQ((IRQn_Type)LEPTON_IRQ_READ_RX);
			lepton_frame_pointer = lepton_frame_start;
		} else {
			XMC_USIC_CH_RXFIFO_SetInterruptNodePointer(LEPTON_SPI, XMC_USIC_CH_RXFIFO_INTERRUPT_NODE_POINTER_STANDARD, LEPTON_SERVICE_REQUEST_REMOVE_RX); // IRQ LEPTON_IRQ_REMOVE_RX
			NVIC_DisableIRQ((IRQn_Type)LEPTON_IRQ_READ_RX);
			NVIC_EnableIRQ((IRQn_Type)LEPTON_IRQ_REMOVE_RX);
			lepton_frame_pointer_dummy = lepton_frame_start;
		}
		XMC_GPIO_SetOutputLow(LEPTON_SELECT_PIN);

		LEPTON_SET_SPI_LIMIT(15);
		lepton_irq_state = LEPTON_IRQ_STATE_SYNC;
		LEPTON_SPI_WRITE_16();
	}
}

void __attribute__((optimize("-O3"))) lepton_rx_read_irq_handler(void) {
	if(lepton_irq_state == LEPTON_IRQ_STATE_SYNC) {
		LEPTON_SPI_READ_2();
		if(lepton_frame_pointer != lepton_frame_end_of_first_packet) {
			LEPTON_SPI_READ_14();
			LEPTON_SPI_WRITE_2();
			if(lepton_frame_pointer != lepton_frame_first_packet_last_full) {
				LEPTON_SPI_WRITE_14();
			} else {
				LEPTON_SET_SPI_LIMIT(1);
			}
		} else {
			if((lepton.frame.data.packets[0].vospi.id & LEPTON_SPI_DISCARD_PACKET_ID_MASK) == LEPTON_SPI_DISCARD_PACKET_ID_MASK) {
				lepton_frame_pointer = lepton_frame_start;
			} else {
				lepton_irq_state = LEPTON_IRQ_STATE_DATA;
			}
			LEPTON_SET_SPI_LIMIT(15);
			LEPTON_SPI_WRITE_16();
		}
	} else {
		LEPTON_SPI_READ_12();
		if(lepton_frame_pointer != lepton_frame_end) {
			// Write 10 words already before we read the next 4, we can save some waiting time between packets with this.
			// We don't write the full 12 bytes, we may get an early interrupt if we do.
			LEPTON_SPI_WRITE_10();
			LEPTON_SPI_READ_4();
			LEPTON_SPI_WRITE_2();
			if(lepton_frame_pointer != lepton_frame_last_full) {
				LEPTON_SPI_WRITE_4();
			} else {
				LEPTON_SET_SPI_LIMIT(11);
			}
		} else {
			lepton.state = LEPTON_STATE_WRITE_FRAME;
			lepton.config_handle_now = true;
			XMC_GPIO_SetOutputHigh(LEPTON_SELECT_PIN);
		}
	}
}

void __attribute__((optimize("-O3"))) lepton_rx_remove_irq_handler(void) {
	if(lepton_irq_state == LEPTON_IRQ_STATE_SYNC) {
		LEPTON_SPI_REMOVE_2();
		if(lepton_frame_pointer_dummy != lepton_frame_end_of_first_packet) {
			LEPTON_SPI_REMOVE_14();
			LEPTON_SPI_WRITE_2();
			if(lepton_frame_pointer_dummy != lepton_frame_first_packet_last_full) {
				LEPTON_SPI_WRITE_14();
			} else {
				LEPTON_SET_SPI_LIMIT(1);
			}
		} else {
			if((lepton.frame.data.packets[0].vospi.id & LEPTON_SPI_DISCARD_PACKET_ID_MASK) == LEPTON_SPI_DISCARD_PACKET_ID_MASK) {
				lepton_frame_pointer_dummy = lepton_frame_start;
			} else {
				lepton_irq_state = LEPTON_IRQ_STATE_DATA;
			}
			LEPTON_SET_SPI_LIMIT(15);
			LEPTON_SPI_WRITE_16();
		}
	} else {
		LEPTON_SPI_REMOVE_12();
		if(lepton_frame_pointer_dummy != lepton_frame_end) {
			// Write 10 words already before we read the next 4, we can save some waiting time between packets with this.
			// We don't write the full 12 bytes, we may get an early interrupt if we do.
			LEPTON_SPI_WRITE_10();
			LEPTON_SPI_REMOVE_4();
			LEPTON_SPI_WRITE_2();
			if(lepton_frame_pointer_dummy != lepton_frame_last_full) {
				LEPTON_SPI_WRITE_4();
			} else {
				LEPTON_SET_SPI_LIMIT(11);
			}
		} else {
			XMC_GPIO_SetOutputHigh(LEPTON_SELECT_PIN);
		}
	}
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
	XMC_GPIO_SetOutputLow(LEPTON_SELECT_PIN);
}

void lepton_spi_deselect(Lepton *lepton) {
	XMC_GPIO_SetOutputHigh(LEPTON_SELECT_PIN);
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
	if(lepton->active_interface == LEPTON_INTERFACE_SPI) {
		return;
	}

	// -- First disable I2C --
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


	// -- Then enable SPI --

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

	XMC_USIC_CH_TXFIFO_Flush(LEPTON_SPI);
	XMC_USIC_CH_RXFIFO_Flush(LEPTON_SPI);

	// Configure transmit FIFO
	XMC_USIC_CH_TXFIFO_Configure(LEPTON_SPI, 48, XMC_USIC_CH_FIFO_SIZE_16WORDS, 0);

	// Configure receive FIFO
	XMC_USIC_CH_RXFIFO_Configure(LEPTON_SPI, 32, XMC_USIC_CH_FIFO_SIZE_16WORDS, 15);

	// Set priority and enable NVIC node for receive interrupt
	NVIC_SetPriority((IRQn_Type)LEPTON_IRQ_READ_RX, LEPTON_IRQ_RX_PRIORITY);
	NVIC_SetPriority((IRQn_Type)LEPTON_IRQ_REMOVE_RX, LEPTON_IRQ_RX_PRIORITY);

	// Start SPI
	XMC_SPI_CH_Start(LEPTON_SPI);

	// Configure SCLK pin
	XMC_GPIO_Init(LEPTON_SCLK_PIN, &sclk_pin_config);

	// Configure slave select pin
	XMC_GPIO_Init(LEPTON_SELECT_PIN, &select_pin_config);

	XMC_USIC_CH_RXFIFO_EnableEvent(LEPTON_SPI, XMC_USIC_CH_RXFIFO_EVENT_CONF_STANDARD | XMC_USIC_CH_RXFIFO_EVENT_CONF_ALTERNATE);

	lepton->active_interface = LEPTON_INTERFACE_SPI;
}

void lepton_init_i2c(Lepton *lepton) {
	if(lepton->active_interface == LEPTON_INTERFACE_I2C) {
		return;
	}

	// -- First disable SPI --

	XMC_SPI_CH_Stop(LEPTON_SPI);
	lepton_spi_deselect(lepton);

	NVIC_DisableIRQ((IRQn_Type)LEPTON_IRQ_REMOVE_RX);
	NVIC_DisableIRQ((IRQn_Type)LEPTON_IRQ_READ_RX);

	XMC_USIC_CH_RXFIFO_DisableEvent(LEPTON_SPI, XMC_USIC_CH_RXFIFO_EVENT_CONF_STANDARD | XMC_USIC_CH_RXFIFO_EVENT_CONF_ALTERNATE);

	// MISO pin configuration
	const XMC_GPIO_CONFIG_t miso_pin_config = {
		.mode             = XMC_GPIO_MODE_INPUT_TRISTATE,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD
	};

	// SCLK pin configuration
	const XMC_GPIO_CONFIG_t sclk_pin_config = {
		.mode             = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
		.output_level     = XMC_GPIO_OUTPUT_LEVEL_HIGH
	};

	XMC_GPIO_Init(LEPTON_SCLK_PIN, &sclk_pin_config);
	XMC_GPIO_Init(LEPTON_MISO_PIN, &miso_pin_config);

	LEPTON_SPI->SCTR |= USIC_CH_SCTR_PDL_Msk; // Set passive data level to 1
	LEPTON_SPI_CHANNEL->DX1CR &= ~USIC_CH_DX1CR_DPOL_Msk; // Set input control register back to default


	// -- Then enable I2C --

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

	XMC_USIC_CH_TXFIFO_Flush(LEPTON_I2C);
	XMC_USIC_CH_RXFIFO_Flush(LEPTON_I2C);

	XMC_USIC_CH_TXFIFO_Configure(LEPTON_I2C, 48, XMC_USIC_CH_FIFO_SIZE_16WORDS, 0);
	XMC_USIC_CH_RXFIFO_Configure(LEPTON_I2C, 32, XMC_USIC_CH_FIFO_SIZE_16WORDS, 0);

	XMC_I2C_CH_Start(LEPTON_I2C);

	XMC_GPIO_Init(LEPTON_SDA_PIN, &sda_pin_config);
	XMC_GPIO_Init(LEPTON_SCL_PIN, &scl_pin_config);

	lepton->active_interface = LEPTON_INTERFACE_I2C;
}

void lepton_init_gpio(Lepton *lepton) {
	const XMC_GPIO_CONFIG_t nreset_pin_config = {
		.mode         = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_LOW,
	};
	XMC_GPIO_Init(LEPTON_NRESET_PIN, &nreset_pin_config);

	const XMC_GPIO_CONFIG_t sync_pin_config = {
		.mode             = XMC_GPIO_MODE_INPUT_TRISTATE,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD
	};
	XMC_GPIO_Init(P0_0, &sync_pin_config); // TODO: Remove me for release version

	const XMC_ERU_ETL_CONFIG_t sync_etl_config = {
		.input_a                = (uint32_t)XMC_ERU_ETL_INPUT_A0,
		.input_b                = (uint32_t)XMC_ERU_ETL_INPUT_B0,
		.enable_output_trigger  = (uint32_t)1,
		.edge_detection         = XMC_ERU_ETL_EDGE_DETECTION_RISING,
		.output_trigger_channel = XMC_ERU_ETL_OUTPUT_TRIGGER_CHANNEL0, // Select the source for event
		.source                 = XMC_ERU_ETL_SOURCE_B
	};

	// Initializes input pin characteristics
	XMC_GPIO_Init(LEPTON_SYNC_PIN, &sync_pin_config);
	// ERU Event Trigger Logic Hardware initialization based on UI
	XMC_ERU_ETL_Init(XMC_ERU0, LEPTON_SYNC_ETL_CHANNEL, &sync_etl_config);
	// OGU is configured to generate event on configured trigger edge
	XMC_ERU_OGU_SetServiceRequestMode(XMC_ERU0, LEPTON_SYNC_OGU_CHANNEL, XMC_ERU_OGU_SERVICE_REQUEST_ON_TRIGGER);
	// Configure NVIC node and priority
	NVIC_SetPriority(LEPTON_SYNC_IRQ_N, LEPTON_SYNC_IRQ_PRIO);

	// Clear pending interrupt before enabling it
	NVIC_ClearPendingIRQ(LEPTON_SYNC_IRQ_N);
	// Enable NVIC node
	NVIC_EnableIRQ(LEPTON_SYNC_IRQ_N);
}

void lepton_init(Lepton *lepton) {
	memset(lepton, 0, sizeof(Lepton));
	lepton->active_interface = LEPTON_INTERFACE_NONE;

	lepton->agc.region_of_interest[0] = 0;
	lepton->agc.region_of_interest[1] = 0;
	lepton->agc.region_of_interest[2] = 79;
	lepton->agc.region_of_interest[3] = 59;
	lepton->agc.dampening_factor      = 64;
	lepton->agc.clip_limit[0]         = 4800;
	lepton->agc.clip_limit[1]         = 512;
	lepton->agc.empty_counts          = 2;

	lepton_init_gpio(lepton);
	lepton_init_i2c(lepton);
	lepton_reset(lepton);
	XMC_GPIO_SetOutputLow(UARTBB_TX_PIN);
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

	uint32_t telemetry_location = 1; // footer
	lepton_attribute_write(lepton, LEPTON_CID_SYS_TELEMETRY_LOCATION, 2, (uint16_t*)&telemetry_location);

	uint32_t telemetry_enable = 1;
	lepton_attribute_write(lepton, LEPTON_CID_SYS_TELEMETRY_ENABLE_STATE, 2, (uint16_t*)&telemetry_enable);

	uint32_t oem_status = 1;
	while(oem_status != 0) {
		lepton_attribute_read(lepton, LEPTON_CID_OEM_STATUS, 2, (uint16_t*)&oem_status);
	}
	uint32_t gpio_mode = 5; // vsync
	lepton_attribute_write(lepton, LEPTON_CID_OEM_GPIO_MODE_SELECT, 2, (uint16_t*)&gpio_mode);
	while(oem_status != 0) {
		lepton_attribute_read(lepton, LEPTON_CID_OEM_STATUS, 2, (uint16_t*)&oem_status);
	}
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
		lepton->state = LEPTON_STATE_READ_FRAME;
		lepton->sync_done = true;
	}
}

void lepton_handle_configuration(Lepton *lepton) {
	if(lepton->config_handle_now) {
		XMC_GPIO_SetOutputHigh(UARTBB_TX_PIN);
		lepton->config_handle_now = false;
		if(lepton->config_agc_bitmask) {
			lepton_init_i2c(lepton);

			if(lepton->config_agc_bitmask & LEPTON_CONFIG_AGC_BITMASK_ENABLE) {
				uint32_t enabled = 0;
				if(lepton->current_callback_config == THERMAL_IMAGING_CALLBACK_CONFIG_CALLBACK_IMAGE) {
					enabled = 1;
				}

				lepton_attribute_write(lepton, LEPTON_CID_AGC_ENABLE_STATE, 2, (uint16_t*)&enabled);
				lepton->config_agc_bitmask &= ~LEPTON_CONFIG_AGC_BITMASK_ENABLE;

			} else if(lepton->config_agc_bitmask & LEPTON_CONFIG_AGC_BITMASK_ROI) {
				uint16_t roi[4] = {lepton->agc.region_of_interest[0], lepton->agc.region_of_interest[1], lepton->agc.region_of_interest[2], lepton->agc.region_of_interest[3]};
				lepton_attribute_write(lepton, LEPTON_CID_AGC_ROI, 4, roi);
				lepton->config_agc_bitmask &= ~LEPTON_CONFIG_AGC_BITMASK_ROI;

			} else if(lepton->config_agc_bitmask & LEPTON_CONFIG_AGC_BITMASK_DAMPENING_FACTOR) {
				lepton_attribute_write(lepton, LEPTON_CID_AGC_HEQ_DAMPENING_FACTOR, 1, &lepton->agc.dampening_factor);
				lepton->config_agc_bitmask &= ~LEPTON_CONFIG_AGC_BITMASK_DAMPENING_FACTOR;

			} else if(lepton->config_agc_bitmask & LEPTON_CONFIG_AGC_BITMASK_CLIP_LIMIT) {
				lepton_attribute_write(lepton, LEPTON_CID_AGC_HEQ_CLIP_LIMIT_HIGH, 1, &lepton->agc.clip_limit[0]);
				lepton_attribute_write(lepton, LEPTON_CID_AGC_HEQ_CLIP_LIMIT_LOW,  1, &lepton->agc.clip_limit[1]);
				lepton->config_agc_bitmask &= ~LEPTON_CONFIG_AGC_BITMASK_CLIP_LIMIT;

			} else if(lepton->config_agc_bitmask & LEPTON_CONFIG_AGC_BITMASK_EMPTY_COUNTS) {
				lepton_attribute_write(lepton, LEPTON_CID_AGC_HEQ_EMPTY_COUNTS, 1, &lepton->agc.empty_counts);
				lepton->config_agc_bitmask &= ~LEPTON_CONFIG_AGC_BITMASK_EMPTY_COUNTS;
			}

			lepton_init_spi(lepton);
		}
		XMC_GPIO_SetOutputLow(UARTBB_TX_PIN);
	}
}



void lepton_tick(Lepton *lepton) {
	switch(lepton->state) {
		case LEPTON_STATE_RESET: lepton_handle_reset(lepton); break;
		case LEPTON_STATE_INIT: lepton_handle_init(lepton); break;
		case LEPTON_STATE_SYNC:  lepton_handle_sync(lepton); break;
		case LEPTON_STATE_READ_FRAME:
		case LEPTON_STATE_WRITE_FRAME: lepton_handle_configuration(lepton); break;
	}
}

bool lepton_has_started(Lepton *lepton) {
	return (lepton->reset_start_time == 0) && (lepton->boot_start_time == 0);
}
