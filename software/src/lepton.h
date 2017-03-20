/* thermal-imaging-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * lepton.h: Lepton thermal imaging camera module communication
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

#ifndef LEPTON_H
#define LEPTON_H

#include <stdint.h>
#include <stdbool.h>

#define LEPTON_I2C_COMMAND_MODULE_ID_BITMASK       (uint16_t)0x0F00
#define LEPTON_I2C_COMMAND_ID_BITMASK              (uint16_t)0x00FC
#define LEPTON_I2C_COMMAND_TYPE_BITMASK            (uint16_t)0x0003

#define LEPTON_I2C_COMMAND_TYPE_GET                (uint16_t)0x0000
#define LEPTON_I2C_COMMAND_TYPE_SET                (uint16_t)0x0001
#define LEPTON_I2C_COMMAND_TYPE_RUN                (uint16_t)0x0002

#define LEPTON_I2C_STATUS_BUSY_BITMASK             (uint16_t)0x0001
#define LEPTON_I2C_STATUS_BOOT_MODE_BITMASK        (uint16_t)0x0002
#define LEPTON_I2C_STATUS_BOOT_STATUS_BITMASK      (uint16_t)0x0004
#define LEPTON_I2C_STATUS_ERROR_CODE_BITMASK       (uint16_t)0xFF00
#define LEPTON_I2C_STATUS_ERROR_CODE_BITSHIFT      8

#define LEPTON_I2C_REG_BASE_ADDR                   (uint16_t)0x0000
#define LEPTON_I2C_POWER_REG                       (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0000)
#define LEPTON_I2C_STATUS_REG                      (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0002)
#define LEPTON_I2C_COMMAND_REG                     (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0004)
#define LEPTON_I2C_DATA_LENGTH_REG                 (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0006)
#define LEPTON_I2C_DATA_0_REG                      (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0008)
#define LEPTON_I2C_DATA_1_REG                      (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x000A)
#define LEPTON_I2C_DATA_2_REG                      (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x000C)
#define LEPTON_I2C_DATA_3_REG                      (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x000E)
#define LEPTON_I2C_DATA_4_REG                      (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0010)
#define LEPTON_I2C_DATA_5_REG                      (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0012)
#define LEPTON_I2C_DATA_6_REG                      (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0014)
#define LEPTON_I2C_DATA_7_REG                      (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0016)
#define LEPTON_I2C_DATA_8_REG                      (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0018)
#define LEPTON_I2C_DATA_9_REG                      (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x001A)
#define LEPTON_I2C_DATA_10_REG                     (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x001C)
#define LEPTON_I2C_DATA_11_REG                     (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x001E)
#define LEPTON_I2C_DATA_12_REG                     (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0020)
#define LEPTON_I2C_DATA_13_REG                     (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0022)
#define LEPTON_I2C_DATA_14_REG                     (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0024)
#define LEPTON_I2C_DATA_15_REG                     (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0026)
#define LEPTON_I2C_DATA_CRC_REG                    (uint16_t)(LEPTON_I2C_REG_BASE_ADDR + 0x0028)

#define LEPTON_I2C_DATA_BUFFER                     (uint16_t)0xF800
#define LEPTON_I2C_DATA_BUFFER_LENGTH              (uint16_t)0x0800

#define LEPTON_AGC_MODULE_BASE                     (uint16_t)0x0100
#define LEPTON_CID_AGC_ENABLE_STATE                (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0000)
#define LEPTON_CID_AGC_POLICY                      (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0004)
#define LEPTON_CID_AGC_ROI                         (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0008)
#define LEPTON_CID_AGC_STATISTICS                  (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x000C)
#define LEPTON_CID_AGC_HISTOGRAM_CLIP_PERCENT      (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0010)
#define LEPTON_CID_AGC_HISTOGRAM_TAIL_SIZE         (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0014)
#define LEPTON_CID_AGC_LINEAR_MAX_GAIN             (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0018)
#define LEPTON_CID_AGC_LINEAR_MIDPOINT             (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x001C)
#define LEPTON_CID_AGC_LINEAR_DAMPENING_FACTOR     (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0020)
#define LEPTON_CID_AGC_HEQ_DAMPENING_FACTOR        (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0024)
#define LEPTON_CID_AGC_HEQ_MAX_GAIN                (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0028)
#define LEPTON_CID_AGC_HEQ_CLIP_LIMIT_HIGH         (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x002C)
#define LEPTON_CID_AGC_HEQ_CLIP_LIMIT_LOW          (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0030)
#define LEPTON_CID_AGC_HEQ_BIN_EXTENSION           (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0034)
#define LEPTON_CID_AGC_HEQ_MIDPOINT                (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0038)
#define LEPTON_CID_AGC_HEQ_EMPTY_COUNTS            (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x003C)
#define LEPTON_CID_AGC_HEQ_NORMALIZATION_FACTOR    (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0040)
#define LEPTON_CID_AGC_HEQ_SCALE_FACTOR            (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0044)
#define LEPTON_CID_AGC_CALC_ENABLE_STATE           (uint16_t)(LEPTON_AGC_MODULE_BASE + 0x0048)

#define LEPTON_SYS_MODULE_BASE                     (uint16_t)0x0200
#define LEPTON_CID_SYS_PING                        (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x0000)
#define LEPTON_CID_SYS_CAM_STATUS                  (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x0004)
#define LEPTON_CID_SYS_FLIR_SERIAL_NUMBER          (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x0008)
#define LEPTON_CID_SYS_CAM_UPTIME                  (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x000C)
#define LEPTON_CID_SYS_AUX_TEMPERATURE_KELVIN      (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x0010)
#define LEPTON_CID_SYS_FPA_TEMPERATURE_KELVIN      (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x0014)
#define LEPTON_CID_SYS_TELEMETRY_ENABLE_STATE      (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x0018)
#define LEPTON_CID_SYS_TELEMETRY_LOCATION          (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x001C)
#define LEPTON_CID_SYS_EXECTUE_FRAME_AVERAGE       (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x0020)
#define LEPTON_CID_SYS_NUM_FRAMES_TO_AVERAGE       (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x0024)
#define LEPTON_CID_SYS_CUST_SERIAL_NUMBER          (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x0028)
#define LEPTON_CID_SYS_SCENE_STATISTICS            (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x002C)
#define LEPTON_CID_SYS_SCENE_ROI                   (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x0030)
#define LEPTON_CID_SYS_THERMAL_SHUTDOWN_COUNT      (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x0034)
#define LEPTON_CID_SYS_SHUTTER_POSITION            (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x0038)
#define LEPTON_CID_SYS_FFC_SHUTTER_MODE            (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x003C)
#define LEPTON_CID_SYS_RUN_FFC                     (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x0042)
#define LEPTON_CID_SYS_FFC_STATUS                  (uint16_t)(LEPTON_SYS_MODULE_BASE + 0x0044)

#define LEPTON_VID_MODULE_BASE                     (uint16_t)0x0300
#define LEPTON_CID_VID_POLARITY_SELECT             (uint16_t)(LEPTON_VID_MODULE_BASE + 0x0000)
#define LEPTON_CID_VID_LUT_SELECT                  (uint16_t)(LEPTON_VID_MODULE_BASE + 0x0004)
#define LEPTON_CID_VID_LUT_TRANSFER                (uint16_t)(LEPTON_VID_MODULE_BASE + 0x0008)
#define LEPTON_CID_VID_FOCUS_CALC_ENABLE           (uint16_t)(LEPTON_VID_MODULE_BASE + 0x000C)
#define LEPTON_CID_VID_FOCUS_ROI                   (uint16_t)(LEPTON_VID_MODULE_BASE + 0x0010)
#define LEPTON_CID_VID_FOCUS_THRESHOLD             (uint16_t)(LEPTON_VID_MODULE_BASE + 0x0014)
#define LEPTON_CID_VID_FOCUS_METRIC                (uint16_t)(LEPTON_VID_MODULE_BASE + 0x0018)
#define LEPTON_CID_VID_SBNUC_ENABLE                (uint16_t)(LEPTON_VID_MODULE_BASE + 0x001C)
#define LEPTON_CID_VID_GAMMA_SELECT                (uint16_t)(LEPTON_VID_MODULE_BASE + 0x0020)
#define LEPTON_CID_VID_FREEZE_ENABLE               (uint16_t)(LEPTON_VID_MODULE_BASE + 0x0024)

typedef enum {
    LEPTON_AGC_LINEAR = 0,
    LEPTON_AGC_HEQ,
} LeptonAGCPolicy;

typedef enum {
    LEPTON_SYSTEM_READY = 0,
    LEPTON_SYSTEM_INITIALIZING,
    LEPTON_SYSTEM_IN_LOW_POWER_MODE,
    LEPTON_SYSTEM_GOING_INTO_STANDBY,
    LEPTON_SYSTEM_FLAT_FIELD_IN_PROCESS
} LeptonSysCamStatusStates;

typedef enum {
    LEPTON_TELEMETRY_LOCATION_HEADER = 0,
    LEPTON_TELEMETRY_LOCATION_FOOTER
} LeptonSysTelemetryLocation;

typedef enum {
    LEPTON_SYS_FA_DIV_1 = 0,
    LEPTON_SYS_FA_DIV_2,
    LEPTON_SYS_FA_DIV_4,
    LEPTON_SYS_FA_DIV_8,
    LEPTON_SYS_FA_DIV_16,
    LEPTON_SYS_FA_DIV_32,
    LEPTON_SYS_FA_DIV_64,
    LEPTON_SYS_FA_DIV_128
} LeptonSys_Frame_Average;

typedef enum {
    LEPTON_SYS_SHUTTER_POSITION_UNKNOWN = -1,
    LEPTON_SYS_SHUTTER_POSITION_IDLE = 0,
    LEPTON_SYS_SHUTTER_POSITION_OPEN,
    LEPTON_SYS_SHUTTER_POSITION_CLOSED,
    LEPTON_SYS_SHUTTER_POSITION_BRAKE_ON
} LeptonSysShutterPosition;

typedef enum {
    LEPTON_SYS_FFC_SHUTTER_MODE_MANUAL = 0,
    LEPTON_SYS_FFC_SHUTTER_MODE_AUTO,
    LEPTON_SYS_FFC_SHUTTER_MODE_EXTERNAL
} LeptonSysFFCShutterModeState;

typedef enum {
    LEPTON_SYS_SHUTTER_LOCKOUT_INACTIVE = 0,
    LEPTON_SYS_SHUTTER_LOCKOUT_HIGH,
    LEPTON_SYS_SHUTTER_LOCKOUT_LOW
} LeptonSysShutterTempLockoutState;

typedef enum {
    LEPTON_AGC_SCALE_TO_8_BITS = 0,
    LEPTON_AGC_SCALE_TO_14_BITS
} LeptonAGCHEQScaleFactor;

typedef enum {
    LEPTON_SYS_FFC_STATUS_WRITE_ERROR = -2,
    LEPTON_SYS_FFC_STATUS_ERROR = -1,
    LEPTON_SYS_FFC_STATUS_READY = 0,
    LEPTON_SYS_FFC_STATUS_BUSY,
    LEPTON_SYS_FRAME_AVERAGE_COLLECTING_FRAMES,
} LeptonSysFFCStatus;


typedef enum {
    LEPTON_VID_WHITE_HOT = 0,
    LEPTON_VID_BLACK_HOT
} LeptonVIDPolarity;

typedef enum {
    LEPTON_VID_WHEEL6_LUT = 0,
    LEPTON_VID_FUSION_LUT,
    LEPTON_VID_RAINBOW_LUT,
    LEPTON_VID_GLOBOW_LUT,
    LEPTON_VID_SEPIA_LUT,
    LEPTON_VID_COLOR_LUT,
    LEPTON_VID_ICE_FIRE_LUT,
    LEPTON_VID_RAIN_LUT,
    LEPTON_VID_USER_LUT,
} LeptonVIDPcolorLUT;

typedef enum {
    LEPTON_OK = 0,                           // Camera ok
    LEPTON_COMM_OK = LEPTON_OK,              // Camera comm ok (same as LEPTON_OK)

    LEPTON_ERROR = -1,                       // Camera general error
    LEPTON_NOT_READY = -2,                   // Camera not ready error
    LEPTON_RANGE_ERROR = -3,                 // Camera range error
    LEPTON_CHECKSUM_ERROR = -4,              // Camera checksum error
    LEPTON_BAD_ARG_POINTER_ERROR = -5,       // Camera Bad argument error
    LEPTON_DATA_SIZE_ERROR = -6,             // Camera byte count error
    LEPTON_UNDEFINED_FUNCTION_ERROR = -7,    // Camera undefined function error
    LEPTON_FUNCTION_NOT_SUPPORTED = -8,      // Camera function not yet supported error

    // OTP access errors
    LEPTON_OTP_WRITE_ERROR = -15,            // Camera OTP write error
    LEPTON_OTP_READ_ERROR = -16,             // double bit error detected (uncorrectible)

    LEPTON_OTP_NOT_PROGRAMMED_ERROR = -18,   // Flag read as non-zero

    // I2C Errors
    LEPTON_ERROR_I2C_BUS_NOT_READY = -20,    // I2C Bus Error - Bus Not Avaialble
    LEPTON_ERROR_I2C_BUFFER_OVERFLOW = -22,  // I2C Bus Error - Buffer Overflow
    LEPTON_ERROR_I2C_ARBITRATION_LOST = -23, // I2C Bus Error - Bus Arbitration Lost
    LEPTON_ERROR_I2C_BUS_ERROR = -24,        // I2C Bus Error - General Bus Error
    LEPTON_ERROR_I2C_NACK_RECEIVED = -25,    // I2C Bus Error - NACK Received
    LEPTON_ERROR_I2C_FAIL = -26,             // I2C Bus Error - General Failure

    // Processing Errors
    LEPTON_DIV_ZERO_ERROR = -80,             // Attempted div by zero

    // Comm Errors
    LEPTON_COMM_PORT_NOT_OPEN = -101,        // Comm port not open
    LEPTON_COMM_INVALID_PORT_ERROR = -102,   // Comm port no such port error
    LEPTON_COMM_RANGE_ERROR = -103,          // Comm port range error
    LEPTON_ERROR_CREATING_COMM = -104,       // Error creating comm
    LEPTON_ERROR_STARTING_COMM = -105,       // Error starting comm
    LEPTON_ERROR_CLOSING_COMM = -106,        // Error closing comm
    LEPTON_COMM_CHECKSUM_ERROR = -107,       // Comm checksum error
    LEPTON_COMM_NO_DEV = -108,               // No comm device
    LEPTON_TIMEOUT_ERROR = -109,             // Comm timeout error
    LEPTON_COMM_ERROR_WRITING_COMM = -110,   // Error writing comm
    LEPTON_COMM_ERROR_READING_COMM = -111,   // Error reading comm
    LEPTON_COMM_COUNT_ERROR = -112,          // Comm byte count error

    // Other Errors
    LEPTON_OPERATION_CANCELED = -126,        // Camera operation canceled
    LEPTON_UNDEFINED_ERROR_CODE = -127       // Undefined error
} LeptonResult;


typedef struct {
    uint16_t start_col;
    uint16_t start_row;
    uint16_t end_col;
    uint16_t end_row;
} LeptonAGCHistogramROI;

typedef struct {
    uint16_t min_intensity;
    uint16_t max_intensity;
    uint16_t mean_intensity;
    uint16_t num_pixels; // def: 4800
} LeptonAGCHistogramStatistics;

typedef struct {
    uint32_t cam_status; // LeptonSysCamStatusStates
    uint16_t command_count;
    uint16_t reserved;
} LeptonSysCamStatus;

typedef struct {
    uint16_t mean_intensity;
    uint16_t max_intensity;
    uint16_t min_intensity;
    uint16_t num_pixels;
} LeptonSysSceneStatistics;

typedef struct {
    uint16_t start_col;
    uint16_t start_row;
    uint16_t end_col;
    uint16_t end_row;
} LeptonSysSceneROI;

typedef struct {
    uint32_t shutter_mode;                // LEPTON_SYS_FFC_SHUTTER_MODE_STATE def:LEPTON_SYS_FFC_SHUTTER_MODE_EXTERNAL
    uint32_t temp_lockout_state;          // LEPTON_SYS_SHUTTER_TEMP_LOCKOUT_STATE def:LEPTON_SYS_SHUTTER_LOCKOUT_INACTIVE
    uint32_t video_freeze_during_ffc;     // bool def:enabled
    uint32_t ffc_desired;                 // bool def:disabled
    uint32_t elapsed_time_since_last_ffc; // (ms)
    uint32_t desired_ffc_period;          // def:300000 (ms)
    uint32_t explicit_cmd_to_open;        // bool def:disabled
    uint16_t desired_ffc_temp_delta;      // def:300 (kelvins*100)
    uint16_t imminent_delay;              // def:52 (frame counts)
} LeptonSysFFCShutterMode;

typedef struct {
    uint8_t reserved;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} LeptonVIDLUTPixel;

typedef struct {
	 LeptonVIDLUTPixel bin[256];
} LeptonVIDLUTBuffer;

typedef struct {
    uint16_t start_col;
    uint16_t start_row;
    uint16_t end_col;
    uint16_t end_row;
} LeptonVIDFocusROI;


typedef struct {
	uint32_t reset_start_time; // Set to 0 if reset finished
	uint32_t boot_start_time;  // Set to 0 if boot finished
} Lepton;

void lepton_init(Lepton *lepton);
void lepton_tick(Lepton *lepton);

bool lepton_is_ready(Lepton *lepton);

#endif