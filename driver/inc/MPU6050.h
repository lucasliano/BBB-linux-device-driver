// I2Cdev library collection - MPU6050 I2C device class
// Based on InvenSense MPU-6050 register map document rev. 2.0, 5/19/2011 (RM-MPU-6000A-00)
// 10/3/2011 by Jeff Rowberg <jeff@rowberg.net>
// 11/28/2014 by Marton Sebok <sebokmarton@gmail.com>
//
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//     ...        - ongoing debug release
//     2014-11-28 - ported to PIC18 peripheral library from Arduino code

// NOTE: THIS IS ONLY A PARIAL RELEASE. THIS DEVICE CLASS IS CURRENTLY UNDERGOING ACTIVE
// DEVELOPMENT AND IS STILL MISSING SOME IMPORTANT FEATURES. PLEASE KEEP THIS IN MIND IF
// YOU DECIDE TO USE THIS PARTICULAR CODE FOR ANYTHING.

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg
Copyright (c) 2014 Marton Sebok

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#ifndef __MPU6050_H_
#define __MPU6050_H_

#include <linux/types.h>
#include "i2c.h"

#if ((defined MPU6050_INCLUDE_DMP_MOTIONAPPS20) || (defined MPU6050_INCLUDE_DMP_MOTIONAPPS41))
    #error DMP is not supported yet
#endif

#define MPU6050_ADDRESS_AD0_LOW     0x68 // address pin low (GND), default for InvenSense evaluation board
#define MPU6050_ADDRESS_AD0_HIGH    0x69 // address pin high (VCC)
#define MPU6050_DEFAULT_ADDRESS     MPU6050_ADDRESS_AD0_LOW

#define MPU6050_RA_XG_OFFS_TC       0x00 //[7] PWR_MODE, [6:1] XG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU6050_RA_YG_OFFS_TC       0x01 //[7] PWR_MODE, [6:1] YG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU6050_RA_ZG_OFFS_TC       0x02 //[7] PWR_MODE, [6:1] ZG_OFFS_TC, [0] OTP_BNK_VLD
#define MPU6050_RA_X_FINE_GAIN      0x03 //[7:0] X_FINE_GAIN
#define MPU6050_RA_Y_FINE_GAIN      0x04 //[7:0] Y_FINE_GAIN
#define MPU6050_RA_Z_FINE_GAIN      0x05 //[7:0] Z_FINE_GAIN
#define MPU6050_RA_XA_OFFS_H        0x06 //[15:0] XA_OFFS
#define MPU6050_RA_XA_OFFS_L_TC     0x07
#define MPU6050_RA_YA_OFFS_H        0x08 //[15:0] YA_OFFS
#define MPU6050_RA_YA_OFFS_L_TC     0x09
#define MPU6050_RA_ZA_OFFS_H        0x0A //[15:0] ZA_OFFS
#define MPU6050_RA_ZA_OFFS_L_TC     0x0B
#define MPU6050_RA_XG_OFFS_USRH     0x13 //[15:0] XG_OFFS_USR
#define MPU6050_RA_XG_OFFS_USRL     0x14
#define MPU6050_RA_YG_OFFS_USRH     0x15 //[15:0] YG_OFFS_USR
#define MPU6050_RA_YG_OFFS_USRL     0x16
#define MPU6050_RA_ZG_OFFS_USRH     0x17 //[15:0] ZG_OFFS_USR
#define MPU6050_RA_ZG_OFFS_USRL     0x18
#define MPU6050_RA_SMPLRT_DIV       0x19
#define MPU6050_RA_CONFIG           0x1A
#define MPU6050_RA_GYRO_CONFIG      0x1B
#define MPU6050_RA_ACCEL_CONFIG     0x1C
#define MPU6050_RA_FF_THR           0x1D
#define MPU6050_RA_FF_DUR           0x1E
#define MPU6050_RA_MOT_THR          0x1F
#define MPU6050_RA_MOT_DUR          0x20
#define MPU6050_RA_ZRMOT_THR        0x21
#define MPU6050_RA_ZRMOT_DUR        0x22
#define MPU6050_RA_FIFO_EN          0x23
#define MPU6050_RA_I2C_MST_CTRL     0x24
#define MPU6050_RA_I2C_SLV0_ADDR    0x25
#define MPU6050_RA_I2C_SLV0_REG     0x26
#define MPU6050_RA_I2C_SLV0_CTRL    0x27
#define MPU6050_RA_I2C_SLV1_ADDR    0x28
#define MPU6050_RA_I2C_SLV1_REG     0x29
#define MPU6050_RA_I2C_SLV1_CTRL    0x2A
#define MPU6050_RA_I2C_SLV2_ADDR    0x2B
#define MPU6050_RA_I2C_SLV2_REG     0x2C
#define MPU6050_RA_I2C_SLV2_CTRL    0x2D
#define MPU6050_RA_I2C_SLV3_ADDR    0x2E
#define MPU6050_RA_I2C_SLV3_REG     0x2F
#define MPU6050_RA_I2C_SLV3_CTRL    0x30
#define MPU6050_RA_I2C_SLV4_ADDR    0x31
#define MPU6050_RA_I2C_SLV4_REG     0x32
#define MPU6050_RA_I2C_SLV4_DO      0x33
#define MPU6050_RA_I2C_SLV4_CTRL    0x34
#define MPU6050_RA_I2C_SLV4_DI      0x35
#define MPU6050_RA_I2C_MST_STATUS   0x36
#define MPU6050_RA_INT_PIN_CFG      0x37
#define MPU6050_RA_INT_ENABLE       0x38
#define MPU6050_RA_DMP_INT_STATUS   0x39
#define MPU6050_RA_INT_STATUS       0x3A
#define MPU6050_RA_ACCEL_XOUT_H     0x3B
#define MPU6050_RA_ACCEL_XOUT_L     0x3C
#define MPU6050_RA_ACCEL_YOUT_H     0x3D
#define MPU6050_RA_ACCEL_YOUT_L     0x3E
#define MPU6050_RA_ACCEL_ZOUT_H     0x3F
#define MPU6050_RA_ACCEL_ZOUT_L     0x40
#define MPU6050_RA_TEMP_OUT_H       0x41
#define MPU6050_RA_TEMP_OUT_L       0x42
#define MPU6050_RA_GYRO_XOUT_H      0x43
#define MPU6050_RA_GYRO_XOUT_L      0x44
#define MPU6050_RA_GYRO_YOUT_H      0x45
#define MPU6050_RA_GYRO_YOUT_L      0x46
#define MPU6050_RA_GYRO_ZOUT_H      0x47
#define MPU6050_RA_GYRO_ZOUT_L      0x48
#define MPU6050_RA_EXT_SENS_DATA_00 0x49
#define MPU6050_RA_EXT_SENS_DATA_01 0x4A
#define MPU6050_RA_EXT_SENS_DATA_02 0x4B
#define MPU6050_RA_EXT_SENS_DATA_03 0x4C
#define MPU6050_RA_EXT_SENS_DATA_04 0x4D
#define MPU6050_RA_EXT_SENS_DATA_05 0x4E
#define MPU6050_RA_EXT_SENS_DATA_06 0x4F
#define MPU6050_RA_EXT_SENS_DATA_07 0x50
#define MPU6050_RA_EXT_SENS_DATA_08 0x51
#define MPU6050_RA_EXT_SENS_DATA_09 0x52
#define MPU6050_RA_EXT_SENS_DATA_10 0x53
#define MPU6050_RA_EXT_SENS_DATA_11 0x54
#define MPU6050_RA_EXT_SENS_DATA_12 0x55
#define MPU6050_RA_EXT_SENS_DATA_13 0x56
#define MPU6050_RA_EXT_SENS_DATA_14 0x57
#define MPU6050_RA_EXT_SENS_DATA_15 0x58
#define MPU6050_RA_EXT_SENS_DATA_16 0x59
#define MPU6050_RA_EXT_SENS_DATA_17 0x5A
#define MPU6050_RA_EXT_SENS_DATA_18 0x5B
#define MPU6050_RA_EXT_SENS_DATA_19 0x5C
#define MPU6050_RA_EXT_SENS_DATA_20 0x5D
#define MPU6050_RA_EXT_SENS_DATA_21 0x5E
#define MPU6050_RA_EXT_SENS_DATA_22 0x5F
#define MPU6050_RA_EXT_SENS_DATA_23 0x60
#define MPU6050_RA_MOT_DETECT_STATUS    0x61
#define MPU6050_RA_I2C_SLV0_DO      0x63
#define MPU6050_RA_I2C_SLV1_DO      0x64
#define MPU6050_RA_I2C_SLV2_DO      0x65
#define MPU6050_RA_I2C_SLV3_DO      0x66
#define MPU6050_RA_I2C_MST_DELAY_CTRL   0x67
#define MPU6050_RA_SIGNAL_PATH_RESET    0x68
#define MPU6050_RA_MOT_DETECT_CTRL      0x69
#define MPU6050_RA_USER_CTRL        0x6A
#define MPU6050_RA_PWR_MGMT_1       0x6B
#define MPU6050_RA_PWR_MGMT_2       0x6C
#define MPU6050_RA_BANK_SEL         0x6D
#define MPU6050_RA_MEM_START_ADDR   0x6E
#define MPU6050_RA_MEM_R_W          0x6F
#define MPU6050_RA_DMP_CFG_1        0x70
#define MPU6050_RA_DMP_CFG_2        0x71
#define MPU6050_RA_FIFO_COUNTH      0x72
#define MPU6050_RA_FIFO_COUNTL      0x73
#define MPU6050_RA_FIFO_R_W         0x74
#define MPU6050_RA_WHO_AM_I         0x75

#define MPU6050_TC_PWR_MODE_BIT     7
#define MPU6050_TC_OFFSET_BIT       6
#define MPU6050_TC_OFFSET_LENGTH    6
#define MPU6050_TC_OTP_BNK_VLD_BIT  0

#define MPU6050_VDDIO_LEVEL_VLOGIC  0
#define MPU6050_VDDIO_LEVEL_VDD     1

#define MPU6050_CFG_EXT_SYNC_SET_BIT    5
#define MPU6050_CFG_EXT_SYNC_SET_LENGTH 3
#define MPU6050_CFG_DLPF_CFG_BIT    2
#define MPU6050_CFG_DLPF_CFG_LENGTH 3

#define MPU6050_EXT_SYNC_DISABLED       0x0
#define MPU6050_EXT_SYNC_TEMP_OUT_L     0x1
#define MPU6050_EXT_SYNC_GYRO_XOUT_L    0x2
#define MPU6050_EXT_SYNC_GYRO_YOUT_L    0x3
#define MPU6050_EXT_SYNC_GYRO_ZOUT_L    0x4
#define MPU6050_EXT_SYNC_ACCEL_XOUT_L   0x5
#define MPU6050_EXT_SYNC_ACCEL_YOUT_L   0x6
#define MPU6050_EXT_SYNC_ACCEL_ZOUT_L   0x7

#define MPU6050_DLPF_BW_256         0x00
#define MPU6050_DLPF_BW_188         0x01
#define MPU6050_DLPF_BW_98          0x02
#define MPU6050_DLPF_BW_42          0x03
#define MPU6050_DLPF_BW_20          0x04
#define MPU6050_DLPF_BW_10          0x05
#define MPU6050_DLPF_BW_5           0x06

#define MPU6050_GCONFIG_FS_SEL_BIT      4
#define MPU6050_GCONFIG_FS_SEL_LENGTH   2

#define MPU6050_GYRO_FS_250         0x00
#define MPU6050_GYRO_FS_500         0x01
#define MPU6050_GYRO_FS_1000        0x02
#define MPU6050_GYRO_FS_2000        0x03

#define MPU6050_ACONFIG_XA_ST_BIT           7
#define MPU6050_ACONFIG_YA_ST_BIT           6
#define MPU6050_ACONFIG_ZA_ST_BIT           5
#define MPU6050_ACONFIG_AFS_SEL_BIT         4
#define MPU6050_ACONFIG_AFS_SEL_LENGTH      2
#define MPU6050_ACONFIG_ACCEL_HPF_BIT       2
#define MPU6050_ACONFIG_ACCEL_HPF_LENGTH    3

#define MPU6050_ACCEL_FS_2          0x00
#define MPU6050_ACCEL_FS_4          0x01
#define MPU6050_ACCEL_FS_8          0x02
#define MPU6050_ACCEL_FS_16         0x03

#define MPU6050_DHPF_RESET          0x00
#define MPU6050_DHPF_5              0x01
#define MPU6050_DHPF_2P5            0x02
#define MPU6050_DHPF_1P25           0x03
#define MPU6050_DHPF_0P63           0x04
#define MPU6050_DHPF_HOLD           0x07

#define MPU6050_TEMP_FIFO_EN_BIT    7
#define MPU6050_XG_FIFO_EN_BIT      6
#define MPU6050_YG_FIFO_EN_BIT      5
#define MPU6050_ZG_FIFO_EN_BIT      4
#define MPU6050_ACCEL_FIFO_EN_BIT   3
#define MPU6050_SLV2_FIFO_EN_BIT    2
#define MPU6050_SLV1_FIFO_EN_BIT    1
#define MPU6050_SLV0_FIFO_EN_BIT    0

#define MPU6050_MULT_MST_EN_BIT     7
#define MPU6050_WAIT_FOR_ES_BIT     6
#define MPU6050_SLV_3_FIFO_EN_BIT   5
#define MPU6050_I2C_MST_P_NSR_BIT   4
#define MPU6050_I2C_MST_CLK_BIT     3
#define MPU6050_I2C_MST_CLK_LENGTH  4

#define MPU6050_CLOCK_DIV_348       0x0
#define MPU6050_CLOCK_DIV_333       0x1
#define MPU6050_CLOCK_DIV_320       0x2
#define MPU6050_CLOCK_DIV_308       0x3
#define MPU6050_CLOCK_DIV_296       0x4
#define MPU6050_CLOCK_DIV_286       0x5
#define MPU6050_CLOCK_DIV_276       0x6
#define MPU6050_CLOCK_DIV_267       0x7
#define MPU6050_CLOCK_DIV_258       0x8
#define MPU6050_CLOCK_DIV_500       0x9
#define MPU6050_CLOCK_DIV_471       0xA
#define MPU6050_CLOCK_DIV_444       0xB
#define MPU6050_CLOCK_DIV_421       0xC
#define MPU6050_CLOCK_DIV_400       0xD
#define MPU6050_CLOCK_DIV_381       0xE
#define MPU6050_CLOCK_DIV_364       0xF

#define MPU6050_I2C_SLV_RW_BIT      7
#define MPU6050_I2C_SLV_ADDR_BIT    6
#define MPU6050_I2C_SLV_ADDR_LENGTH 7
#define MPU6050_I2C_SLV_EN_BIT      7
#define MPU6050_I2C_SLV_BYTE_SW_BIT 6
#define MPU6050_I2C_SLV_REG_DIS_BIT 5
#define MPU6050_I2C_SLV_GRP_BIT     4
#define MPU6050_I2C_SLV_LEN_BIT     3
#define MPU6050_I2C_SLV_LEN_LENGTH  4

#define MPU6050_I2C_SLV4_RW_BIT         7
#define MPU6050_I2C_SLV4_ADDR_BIT       6
#define MPU6050_I2C_SLV4_ADDR_LENGTH    7
#define MPU6050_I2C_SLV4_EN_BIT         7
#define MPU6050_I2C_SLV4_INT_EN_BIT     6
#define MPU6050_I2C_SLV4_REG_DIS_BIT    5
#define MPU6050_I2C_SLV4_MST_DLY_BIT    4
#define MPU6050_I2C_SLV4_MST_DLY_LENGTH 5

#define MPU6050_MST_PASS_THROUGH_BIT    7
#define MPU6050_MST_I2C_SLV4_DONE_BIT   6
#define MPU6050_MST_I2C_LOST_ARB_BIT    5
#define MPU6050_MST_I2C_SLV4_NACK_BIT   4
#define MPU6050_MST_I2C_SLV3_NACK_BIT   3
#define MPU6050_MST_I2C_SLV2_NACK_BIT   2
#define MPU6050_MST_I2C_SLV1_NACK_BIT   1
#define MPU6050_MST_I2C_SLV0_NACK_BIT   0

#define MPU6050_INTCFG_INT_LEVEL_BIT        7
#define MPU6050_INTCFG_INT_OPEN_BIT         6
#define MPU6050_INTCFG_LATCH_INT_EN_BIT     5
#define MPU6050_INTCFG_INT_RD_CLEAR_BIT     4
#define MPU6050_INTCFG_FSYNC_INT_LEVEL_BIT  3
#define MPU6050_INTCFG_FSYNC_INT_EN_BIT     2
#define MPU6050_INTCFG_I2C_BYPASS_EN_BIT    1
#define MPU6050_INTCFG_CLKOUT_EN_BIT        0

#define MPU6050_INTMODE_ACTIVEHIGH  0x00
#define MPU6050_INTMODE_ACTIVELOW   0x01

#define MPU6050_INTDRV_PUSHPULL     0x00
#define MPU6050_INTDRV_OPENDRAIN    0x01

#define MPU6050_INTLATCH_50USPULSE  0x00
#define MPU6050_INTLATCH_WAITCLEAR  0x01

#define MPU6050_INTCLEAR_STATUSREAD 0x00
#define MPU6050_INTCLEAR_ANYREAD    0x01

#define MPU6050_INTERRUPT_FF_BIT            7
#define MPU6050_INTERRUPT_MOT_BIT           6
#define MPU6050_INTERRUPT_ZMOT_BIT          5
#define MPU6050_INTERRUPT_FIFO_OFLOW_BIT    4
#define MPU6050_INTERRUPT_I2C_MST_INT_BIT   3
#define MPU6050_INTERRUPT_PLL_RDY_INT_BIT   2
#define MPU6050_INTERRUPT_DMP_INT_BIT       1
#define MPU6050_INTERRUPT_DATA_RDY_BIT      0

// TODO: figure out what these actually do
// UMPL source code is not very obivous
#define MPU6050_DMPINT_5_BIT            5
#define MPU6050_DMPINT_4_BIT            4
#define MPU6050_DMPINT_3_BIT            3
#define MPU6050_DMPINT_2_BIT            2
#define MPU6050_DMPINT_1_BIT            1
#define MPU6050_DMPINT_0_BIT            0

#define MPU6050_MOTION_MOT_XNEG_BIT     7
#define MPU6050_MOTION_MOT_XPOS_BIT     6
#define MPU6050_MOTION_MOT_YNEG_BIT     5
#define MPU6050_MOTION_MOT_YPOS_BIT     4
#define MPU6050_MOTION_MOT_ZNEG_BIT     3
#define MPU6050_MOTION_MOT_ZPOS_BIT     2
#define MPU6050_MOTION_MOT_ZRMOT_BIT    0

#define MPU6050_DELAYCTRL_DELAY_ES_SHADOW_BIT   7
#define MPU6050_DELAYCTRL_I2C_SLV4_DLY_EN_BIT   4
#define MPU6050_DELAYCTRL_I2C_SLV3_DLY_EN_BIT   3
#define MPU6050_DELAYCTRL_I2C_SLV2_DLY_EN_BIT   2
#define MPU6050_DELAYCTRL_I2C_SLV1_DLY_EN_BIT   1
#define MPU6050_DELAYCTRL_I2C_SLV0_DLY_EN_BIT   0

#define MPU6050_PATHRESET_GYRO_RESET_BIT    2
#define MPU6050_PATHRESET_ACCEL_RESET_BIT   1
#define MPU6050_PATHRESET_TEMP_RESET_BIT    0

#define MPU6050_DETECT_ACCEL_ON_DELAY_BIT       5
#define MPU6050_DETECT_ACCEL_ON_DELAY_LENGTH    2
#define MPU6050_DETECT_FF_COUNT_BIT             3
#define MPU6050_DETECT_FF_COUNT_LENGTH          2
#define MPU6050_DETECT_MOT_COUNT_BIT            1
#define MPU6050_DETECT_MOT_COUNT_LENGTH         2

#define MPU6050_DETECT_DECREMENT_RESET  0x0
#define MPU6050_DETECT_DECREMENT_1      0x1
#define MPU6050_DETECT_DECREMENT_2      0x2
#define MPU6050_DETECT_DECREMENT_4      0x3

#define MPU6050_USERCTRL_DMP_EN_BIT             7
#define MPU6050_USERCTRL_FIFO_EN_BIT            6
#define MPU6050_USERCTRL_I2C_MST_EN_BIT         5
#define MPU6050_USERCTRL_I2C_IF_DIS_BIT         4
#define MPU6050_USERCTRL_DMP_RESET_BIT          3
#define MPU6050_USERCTRL_FIFO_RESET_BIT         2
#define MPU6050_USERCTRL_I2C_MST_RESET_BIT      1
#define MPU6050_USERCTRL_SIG_COND_RESET_BIT     0

#define MPU6050_PWR1_DEVICE_RESET_BIT   7
#define MPU6050_PWR1_SLEEP_BIT          6
#define MPU6050_PWR1_CYCLE_BIT          5
#define MPU6050_PWR1_TEMP_DIS_BIT       3
#define MPU6050_PWR1_CLKSEL_BIT         2
#define MPU6050_PWR1_CLKSEL_LENGTH      3

#define MPU6050_CLOCK_INTERNAL          0x00
#define MPU6050_CLOCK_PLL_XGYRO         0x01
#define MPU6050_CLOCK_PLL_YGYRO         0x02
#define MPU6050_CLOCK_PLL_ZGYRO         0x03
#define MPU6050_CLOCK_PLL_EXT32K        0x04
#define MPU6050_CLOCK_PLL_EXT19M        0x05
#define MPU6050_CLOCK_KEEP_RESET        0x07

#define MPU6050_PWR2_LP_WAKE_CTRL_BIT       7
#define MPU6050_PWR2_LP_WAKE_CTRL_LENGTH    2
#define MPU6050_PWR2_STBY_XA_BIT            5
#define MPU6050_PWR2_STBY_YA_BIT            4
#define MPU6050_PWR2_STBY_ZA_BIT            3
#define MPU6050_PWR2_STBY_XG_BIT            2
#define MPU6050_PWR2_STBY_YG_BIT            1
#define MPU6050_PWR2_STBY_ZG_BIT            0

#define MPU6050_WAKE_FREQ_1P25      0x0
#define MPU6050_WAKE_FREQ_2P5       0x1
#define MPU6050_WAKE_FREQ_5         0x2
#define MPU6050_WAKE_FREQ_10        0x3

#define MPU6050_BANKSEL_PRFTCH_EN_BIT       6
#define MPU6050_BANKSEL_CFG_USER_BANK_BIT   5
#define MPU6050_BANKSEL_MEM_SEL_BIT         4
#define MPU6050_BANKSEL_MEM_SEL_LENGTH      5

#define MPU6050_WHO_AM_I_BIT        6
#define MPU6050_WHO_AM_I_LENGTH     6

#define MPU6050_DMP_MEMORY_BANKS        8
#define MPU6050_DMP_MEMORY_BANK_SIZE    256
#define MPU6050_DMP_MEMORY_CHUNK_SIZE   16

// note: DMP code memory blocks defined at end of header file


// CUSTOM
int mpu6050_init(struct platform_device * i2c_plat_dev);
void mpu6050_deinit(void);



typedef struct MPU6050_t {
    unsigned char devAddr;
    unsigned char buffer[14];
} MPU6050_t;

void MPU6050(unsigned char address);
int MPU6050_testConnection(void);

// WHO_AM_I register
unsigned char MPU6050_getDeviceID(void);
// void MPU6050_setDeviceID(unsigned char id);


// void MPU6050_initialize();

// // AUX_VDDIO register
// unsigned char MPU6050_getAuxVDDIOLevel();
// void MPU6050_setAuxVDDIOLevel(unsigned char level);

// // SMPLRT_DIV register
// unsigned char MPU6050_getRate();
// void MPU6050_setRate(unsigned char rate);

// // CONFIG register
// unsigned char MPU6050_getExternalFrameSync();
// void MPU6050_setExternalFrameSync(unsigned char sync);
// unsigned char MPU6050_getDLPFMode();
// void MPU6050_setDLPFMode(unsigned char bandwidth);

// // GYRO_CONFIG register
// unsigned char MPU6050_getFullScaleGyroRange();
// void MPU6050_setFullScaleGyroRange(unsigned char range);

// // ACCEL_CONFIG register
// char MPU6050_getAccelXSelfTest();
// void MPU6050_setAccelXSelfTest(char enabled);
// char MPU6050_getAccelYSelfTest();
// void MPU6050_setAccelYSelfTest(char enabled);
// char MPU6050_getAccelZSelfTest();
// void MPU6050_setAccelZSelfTest(char enabled);
// unsigned char MPU6050_getFullScaleAccelRange();
// void MPU6050_setFullScaleAccelRange(unsigned char range);
// unsigned char MPU6050_getDHPFMode();
// void MPU6050_setDHPFMode(unsigned char mode);

// // FF_THR register
// unsigned char MPU6050_getFreefallDetectionThreshold();
// void MPU6050_setFreefallDetectionThreshold(unsigned char threshold);

// // FF_DUR register
// unsigned char MPU6050_getFreefallDetectionDuration();
// void MPU6050_setFreefallDetectionDuration(unsigned char duration);

// // MOT_THR register
// unsigned char MPU6050_getMotionDetectionThreshold();
// void MPU6050_setMotionDetectionThreshold(unsigned char threshold);

// // MOT_DUR register
// unsigned char MPU6050_getMotionDetectionDuration();
// void MPU6050_setMotionDetectionDuration(unsigned char duration);

// // ZRMOT_THR register
// unsigned char MPU6050_getZeroMotionDetectionThreshold();
// void MPU6050_setZeroMotionDetectionThreshold(unsigned char threshold);

// // ZRMOT_DUR register
// unsigned char MPU6050_getZeroMotionDetectionDuration();
// void MPU6050_setZeroMotionDetectionDuration(unsigned char duration);

// // FIFO_EN register
// char MPU6050_getTempFIFOEnabled();
// void MPU6050_setTempFIFOEnabled(char enabled);
// char MPU6050_getXGyroFIFOEnabled();
// void MPU6050_setXGyroFIFOEnabled(char enabled);
// char MPU6050_getYGyroFIFOEnabled();
// void MPU6050_setYGyroFIFOEnabled(char enabled);
// char MPU6050_getZGyroFIFOEnabled();
// void MPU6050_setZGyroFIFOEnabled(char enabled);
// char MPU6050_getAccelFIFOEnabled();
// void MPU6050_setAccelFIFOEnabled(char enabled);
// char MPU6050_getSlave2FIFOEnabled();
// void MPU6050_setSlave2FIFOEnabled(char enabled);
// char MPU6050_getSlave1FIFOEnabled();
// void MPU6050_setSlave1FIFOEnabled(char enabled);
// char MPU6050_getSlave0FIFOEnabled();
// void MPU6050_setSlave0FIFOEnabled(char enabled);

// // I2C_MST_CTRL register
// char MPU6050_getMultiMasterEnabled();
// void MPU6050_setMultiMasterEnabled(char enabled);
// char MPU6050_getWaitForExternalSensorEnabled();
// void MPU6050_setWaitForExternalSensorEnabled(char enabled);
// char MPU6050_getSlave3FIFOEnabled();
// void MPU6050_setSlave3FIFOEnabled(char enabled);
// char MPU6050_getSlaveReadWriteTransitionEnabled();
// void MPU6050_setSlaveReadWriteTransitionEnabled(char enabled);
// unsigned char MPU6050_getMasterClockSpeed();
// void MPU6050_setMasterClockSpeed(unsigned char speed);

// // I2C_SLV* registers (Slave 0-3)
// unsigned char MPU6050_getSlaveAddress(unsigned char num);
// void MPU6050_setSlaveAddress(unsigned char num, unsigned char address);
// unsigned char MPU6050_getSlaveRegister(unsigned char num);
// void MPU6050_setSlaveRegister(unsigned char num, unsigned char reg);
// char MPU6050_getSlaveEnabled(unsigned char num);
// void MPU6050_setSlaveEnabled(unsigned char num, char enabled);
// char MPU6050_getSlaveWordByteSwap(unsigned char num);
// void MPU6050_setSlaveWordByteSwap(unsigned char num, char enabled);
// char MPU6050_getSlaveWriteMode(unsigned char num);
// void MPU6050_setSlaveWriteMode(unsigned char num, char mode);
// char MPU6050_getSlaveWordGroupOffset(unsigned char num);
// void MPU6050_setSlaveWordGroupOffset(unsigned char num, char enabled);
// unsigned char MPU6050_getSlaveDataLength(unsigned char num);
// void MPU6050_setSlaveDataLength(unsigned char num, unsigned char length);

// // I2C_SLV* registers (Slave 4)
// unsigned char MPU6050_getSlave4Address();
// void MPU6050_setSlave4Address(unsigned char address);
// unsigned char MPU6050_getSlave4Register();
// void MPU6050_setSlave4Register(unsigned char reg);
// void MPU6050_setSlave4OutputByte(unsigned char data);
// char MPU6050_getSlave4Enabled();
// void MPU6050_setSlave4Enabled(char enabled);
// char MPU6050_getSlave4InterruptEnabled();
// void MPU6050_setSlave4InterruptEnabled(char enabled);
// char MPU6050_getSlave4WriteMode();
// void MPU6050_setSlave4WriteMode(char mode);
// unsigned char MPU6050_getSlave4MasterDelay();
// void MPU6050_setSlave4MasterDelay(unsigned char delay);
// unsigned char MPU6050_getSlate4InputByte();

// // I2C_MST_STATUS register
// char MPU6050_getPassthroughStatus();
// char MPU6050_getSlave4IsDone();
// char MPU6050_getLostArbitration();
// char MPU6050_getSlave4Nack();
// char MPU6050_getSlave3Nack();
// char MPU6050_getSlave2Nack();
// char MPU6050_getSlave1Nack();
// char MPU6050_getSlave0Nack();

// // INT_PIN_CFG register
// char MPU6050_getInterruptMode();
// void MPU6050_setInterruptMode(char mode);
// char MPU6050_getInterruptDrive();
// void MPU6050_setInterruptDrive(char drive);
// char MPU6050_getInterruptLatch();
// void MPU6050_setInterruptLatch(char latch);
// char MPU6050_getInterruptLatchClear();
// void MPU6050_setInterruptLatchClear(char clear);
// char MPU6050_getFSyncInterruptLevel();
// void MPU6050_setFSyncInterruptLevel(char level);
// char MPU6050_getFSyncInterruptEnabled();
// void MPU6050_setFSyncInterruptEnabled(char enabled);
// char MPU6050_getI2CBypassEnabled();
// void MPU6050_setI2CBypassEnabled(char enabled);
// char MPU6050_getClockOutputEnabled();
// void MPU6050_setClockOutputEnabled(char enabled);

// // INT_ENABLE register
// unsigned char MPU6050_getIntEnabled();
// void MPU6050_setIntEnabled(unsigned char enabled);
// char MPU6050_getIntFreefallEnabled();
// void MPU6050_setIntFreefallEnabled(char enabled);
// char MPU6050_getIntMotionEnabled();
// void MPU6050_setIntMotionEnabled(char enabled);
// char MPU6050_getIntZeroMotionEnabled();
// void MPU6050_setIntZeroMotionEnabled(char enabled);
// char MPU6050_getIntFIFOBufferOverflowEnabled();
// void MPU6050_setIntFIFOBufferOverflowEnabled(char enabled);
// char MPU6050_getIntI2CMasterEnabled();
// void MPU6050_setIntI2CMasterEnabled(char enabled);
// char MPU6050_getIntDataReadyEnabled();
// void MPU6050_setIntDataReadyEnabled(char enabled);

// // INT_STATUS register
// unsigned char MPU6050_getIntStatus();
// char MPU6050_getIntFreefallStatus();
// char MPU6050_getIntMotionStatus();
// char MPU6050_getIntZeroMotionStatus();
// char MPU6050_getIntFIFOBufferOverflowStatus();
// char MPU6050_getIntI2CMasterStatus();
// char MPU6050_getIntDataReadyStatus();

// // ACCEL_*OUT_* registers
// void MPU6050_getMotion9(int* ax, int* ay, int* az, int* gx, int* gy, int* gz, int* mx, int* my, int* mz);
// void MPU6050_getMotion6(int* ax, int* ay, int* az, int* gx, int* gy, int* gz);
// void MPU6050_getAcceleration(int* x, int* y, int* z);
// int MPU6050_getAccelerationX();
// int MPU6050_getAccelerationY();
// int MPU6050_getAccelerationZ();

// // TEMP_OUT_* registers
// int MPU6050_getTemperature();

// // GYRO_*OUT_* registers
// void MPU6050_getRotation(int* x, int* y, int* z);
// int MPU6050_getRotationX();
// int MPU6050_getRotationY();
// int MPU6050_getRotationZ();

// // EXT_SENS_DATA_* registers
// unsigned char MPU6050_getExternalSensorByte(int position);
// uint MPU6050_getExternalSensorWord(int position);
// uint32_t getExternalSensorDWord(int position);

// // MOT_DETECT_STATUS register
// char MPU6050_getXNegMotionDetected();
// char MPU6050_getXPosMotionDetected();
// char MPU6050_getYNegMotionDetected();
// char MPU6050_getYPosMotionDetected();
// char MPU6050_getZNegMotionDetected();
// char MPU6050_getZPosMotionDetected();
// char MPU6050_getZeroMotionDetected();

// // I2C_SLV*_DO register
// void MPU6050_setSlaveOutputByte(unsigned char num, unsigned char data);

// // I2C_MST_DELAY_CTRL register
// char MPU6050_getExternalShadowDelayEnabled();
// void MPU6050_setExternalShadowDelayEnabled(char enabled);
// char MPU6050_getSlaveDelayEnabled(unsigned char num);
// void MPU6050_setSlaveDelayEnabled(unsigned char num, char enabled);

// // SIGNAL_PATH_RESET register
// void MPU6050_resetGyroscopePath();
// void MPU6050_resetAccelerometerPath();
// void MPU6050_resetTemperaturePath();

// // MOT_DETECT_CTRL register
// unsigned char MPU6050_getAccelerometerPowerOnDelay();
// void MPU6050_setAccelerometerPowerOnDelay(unsigned char delay);
// unsigned char MPU6050_getFreefallDetectionCounterDecrement();
// void MPU6050_setFreefallDetectionCounterDecrement(unsigned char decrement);
// unsigned char MPU6050_getMotionDetectionCounterDecrement();
// void MPU6050_setMotionDetectionCounterDecrement(unsigned char decrement);

// // USER_CTRL register
// char MPU6050_getFIFOEnabled();
// void MPU6050_setFIFOEnabled(char enabled);
// char MPU6050_getI2CMasterModeEnabled();
// void MPU6050_setI2CMasterModeEnabled(char enabled);
// void MPU6050_switchSPIEnabled(char enabled);
// void MPU6050_resetFIFO();
// void MPU6050_resetI2CMaster();
// void MPU6050_resetSensors();

// // PWR_MGMT_1 register
// void MPU6050_reset();
// char MPU6050_getSleepEnabled();
// void MPU6050_setSleepEnabled(char enabled);
// char MPU6050_getWakeCycleEnabled();
// void MPU6050_setWakeCycleEnabled(char enabled);
// char MPU6050_getTempSensorEnabled();
// void MPU6050_setTempSensorEnabled(char enabled);
// unsigned char MPU6050_getClockSource();
// void MPU6050_setClockSource(unsigned char source);

// // PWR_MGMT_2 register
// unsigned char MPU6050_getWakeFrequency();
// void MPU6050_setWakeFrequency(unsigned char frequency);
// char MPU6050_getStandbyXAccelEnabled();
// void MPU6050_setStandbyXAccelEnabled(char enabled);
// char MPU6050_getStandbyYAccelEnabled();
// void MPU6050_setStandbyYAccelEnabled(char enabled);
// char MPU6050_getStandbyZAccelEnabled();
// void MPU6050_setStandbyZAccelEnabled(char enabled);
// char MPU6050_getStandbyXGyroEnabled();
// void MPU6050_setStandbyXGyroEnabled(char enabled);
// char MPU6050_getStandbyYGyroEnabled();
// void MPU6050_setStandbyYGyroEnabled(char enabled);
// char MPU6050_getStandbyZGyroEnabled();
// void MPU6050_setStandbyZGyroEnabled(char enabled);

// // FIFO_COUNT_* registers
// uint MPU6050_getFIFOCount();

// // FIFO_R_W register
// unsigned char MPU6050_getFIFOByte();
// void MPU6050_setFIFOByte(unsigned char data);
// void MPU6050_getFIFOBytes(unsigned char *data, unsigned char length);

// // ======== UNDOCUMENTED/DMP REGISTERS/METHODS ========

// // XG_OFFS_TC register
// unsigned char MPU6050_getOTPBankValid();
// void MPU6050_setOTPBankValid(char enabled);
// signed char getXGyroOffsetTC();
// void MPU6050_setXGyroOffsetTC(signed char offset);

// // YG_OFFS_TC register
// signed char getYGyroOffsetTC();
// void MPU6050_setYGyroOffsetTC(signed char offset);

// // ZG_OFFS_TC register
// signed char getZGyroOffsetTC();
// void MPU6050_setZGyroOffsetTC(signed char offset);

// // X_FINE_GAIN register
// signed char getXFineGain();
// void MPU6050_setXFineGain(signed char gain);

// // Y_FINE_GAIN register
// signed char getYFineGain();
// void MPU6050_setYFineGain(signed char gain);

// // Z_FINE_GAIN register
// signed char getZFineGain();
// void MPU6050_setZFineGain(signed char gain);

// // XA_OFFS_* registers
// int MPU6050_getXAccelOffset();
// void MPU6050_setXAccelOffset(int offset);

// // YA_OFFS_* register
// int MPU6050_getYAccelOffset();
// void MPU6050_setYAccelOffset(int offset);

// // ZA_OFFS_* register
// int MPU6050_getZAccelOffset();
// void MPU6050_setZAccelOffset(int offset);

// // XG_OFFS_USR* registers
// int MPU6050_getXGyroOffset();
// void MPU6050_setXGyroOffset(int offset);

// // YG_OFFS_USR* register
// int MPU6050_getYGyroOffset();
// void MPU6050_setYGyroOffset(int offset);

// // ZG_OFFS_USR* register
// int MPU6050_getZGyroOffset();
// void MPU6050_setZGyroOffset(int offset);

// // INT_ENABLE register (DMP functions)
// char MPU6050_getIntPLLReadyEnabled();
// void MPU6050_setIntPLLReadyEnabled(char enabled);
// char MPU6050_getIntDMPEnabled();
// void MPU6050_setIntDMPEnabled(char enabled);

// // DMP_INT_STATUS
// char MPU6050_getDMPInt5Status();
// char MPU6050_getDMPInt4Status();
// char MPU6050_getDMPInt3Status();
// char MPU6050_getDMPInt2Status();
// char MPU6050_getDMPInt1Status();
// char MPU6050_getDMPInt0Status();

// // INT_STATUS register (DMP functions)
// char MPU6050_getIntPLLReadyStatus();
// char MPU6050_getIntDMPStatus();

// // USER_CTRL register (DMP functions)
// char MPU6050_getDMPEnabled();
// void MPU6050_setDMPEnabled(char enabled);
// void MPU6050_resetDMP();

// // BANK_SEL register
// void MPU6050_setMemoryBank(unsigned char bank, char prefetchEnabled, char userBank);

// // MEM_START_ADDR register
// void MPU6050_setMemoryStartAddress(unsigned char address);

// // MEM_R_W register
// unsigned char MPU6050_readMemoryByte();
// void MPU6050_writeMemoryByte(unsigned char data);
// void MPU6050_readMemoryBlock(unsigned char *data, uint dataSize, unsigned char bank, unsigned char address);
// //char MPU6050_writeMemoryBlock(const unsigned char *data, uint dataSize, unsigned char bank, unsigned char address, char verify, char useProgMem);
// //char MPU6050_writeProgMemoryBlock(const unsigned char *data, uint dataSize, unsigned char bank, unsigned char address, char verify);

// //char MPU6050_writeDMPConfigurationSet(const unsigned char *data, uint dataSize, char useProgMem);
// //char MPU6050_writeProgDMPConfigurationSet(const unsigned char *data, uint dataSize);

// // DMP_CFG_1 register
// unsigned char MPU6050_getDMPConfig1();
// void MPU6050_setDMPConfig1(unsigned char config);

// // DMP_CFG_2 register
// unsigned char MPU6050_getDMPConfig2();
// void MPU6050_setDMPConfig2(unsigned char config);

#endif /* __MPU6050_H_ */