/* thermal-imaging-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * config_lepton.c: Configuration for Lepton thermal imaging camera module
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

#ifndef CONFIG_LEPTON_H
#define CONFIG_LEPTON_H

#include "xmc_gpio.h"

#define LEPTON_RESET_TIME           100
#define LEPTON_BOOTUP_TIME          1000 // Datasheet says we need to wait at least 950ms

#define LEPTON_SPI_BAUDRATE         4400000 // Use double the minimum speed, so we have half a frame time for other stuff inbetween
#define LEPTON_SPI_CHANNEL          USIC0_CH1
#define LEPTON_SPI                  XMC_SPI0_CH1

#define LEPTON_SCLK_PIN             P0_8
#define LEPTON_SCLK_PIN_AF          (XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT7 | P0_8_AF_U0C1_SCLKOUT)

#define LEPTON_SELECT_PIN           P0_9
#define LEPTON_SELECT_PIN_AF        (XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT7 | P0_9_AF_U0C1_SELO0)

#define LEPTON_MISO_PIN             P0_6
#define LEPTON_MISO_INPUT           XMC_USIC_CH_INPUT_DX0
#define LEPTON_MISO_SOURCE          0b010 // DX0C


#define LEPTON_I2C_BAUDRATE         400000

#define LEPTON_I2C_ADDRESS          0x2A
#define LEPTON_I2C_CHANNEL          USIC0_CH1
#define LEPTON_I2C                  XMC_I2C0_CH1

#define LEPTON_SCL_PIN              P1_3
#define LEPTON_SCL_PIN_MODE         XMC_GPIO_MODE_OUTPUT_OPEN_DRAIN_ALT6
#define LEPTON_SCL_INPUT            XMC_USIC_CH_INPUT_DX1
#define LEPTON_SCL_SOURCE           0

#define LEPTON_SDA_PIN              P1_2
#define LEPTON_SDA_PIN_MODE         XMC_GPIO_MODE_OUTPUT_OPEN_DRAIN_ALT7
#define LEPTON_SDA_INPUT            XMC_USIC_CH_INPUT_DX0
#define LEPTON_SDA_SOURCE           1

#define LEPTON_SYNC_PIN             P0_0
#define LEPTON_NRESET_PIN           P0_5

#define LEPTON_SERVICE_REQUEST_RX   2

#define LEPTON_IRQ_RX               11
#define LEPTON_IRQ_RX_PRIORITY      0

#endif
