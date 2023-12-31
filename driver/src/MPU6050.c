// I2Cdev library collection - MPU6050 I2C device class
// Based on InvenSense MPU-6050 register map document rev. 2.0, 5/19/2011 (RM-MPU-6000A-00)
// 8/24/2011 by Jeff Rowberg <jeff@rowberg.net>
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

#include "MPU6050.h"

MPU6050_t mpu6050;


/* =========================== R/W ROUTINES ======================================== */
/** Read multiple bytes from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr First register regAddr to read from
 * @param length Number of bytes to read
 * @param data Buffer to store read data in
 * @return Number of bytes read (-1 indicates failure)
 */
int8_t MPU6050_readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data) {
    int retVal = -1;

    if (i2c_write(devAddr, &regAddr, 1) != 0)
        return retVal;
    
    if (i2c_read(devAddr, data, length)!= 0)
        return retVal;

    return length;
}

/** Read single byte from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param data Container for byte value read from device
 * @return Status of read operation (0 = success)
 */
int8_t MPU6050_readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data) {
    return MPU6050_readBytes(devAddr, regAddr, 1, data);
}

/** Read a single bit from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitNum Bit position to read (0-7)
 * @param data Container for single bit value
 * @return Status of read operation (0 = success)
 */
int8_t MPU6050_readBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data) {
    uint8_t b;
    uint8_t count = MPU6050_readByte(devAddr, regAddr, &b);
    *data = b & (1 << bitNum);
    return count;
}

/** Read multiple bits from an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-7)
 * @param length Number of bits to read (not more than 8)
 * @param data Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @return Status of read operation (0 = success)
 */
int8_t MPU6050_readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data) {
    // 01101001 read byte
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    //    010   masked
    //   -> 010 shifted
    uint8_t count, b;
    if ((count = MPU6050_readByte(devAddr, regAddr, &b)) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        b &= mask;
        b >>= (bitStart - length + 1);
        *data = b;
    }
    return count;
}

/** Write single byte to an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register address to write to
 * @param data New byte value to write
 * @return Status of operation (0 = success)
 */
int MPU6050_writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data) {

    uint8_t data_buffer [2] = {regAddr, data};

    return i2c_write(devAddr, data_buffer, 2);
}

/** Write single word to a 16-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register address to write to
 * @param data New word value to write
 * @return Status of operation (0 = success)
 */
int MPU6050_writeWord(uint8_t devAddr, uint8_t regAddr, uint16_t data) {
    uint8_t data_buffer [3] = {regAddr, data};

    data_buffer[0] = regAddr;
    data_buffer[1] = data >> 8;
    data_buffer[2] = data & 0xFF;

    return i2c_write(devAddr, data_buffer, 3);
}

/** write a single bit in an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitNum Bit position to write (0-7)
 * @param value New bit value to write
 * @return Status of operation (0 = success)
 */
int MPU6050_writeBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data) {
    uint8_t b;
    MPU6050_readByte(devAddr, regAddr, &b);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    return MPU6050_writeByte(devAddr, regAddr, b);
}

/** Write multiple bits in an 8-bit device register.
 * @param devAddr I2C slave device address
 * @param regAddr Register regAddr to write to
 * @param bitStart First bit position to write (0-7)
 * @param length Number of bits to write (not more than 8)
 * @param data Right-aligned value to write
 * @return Status of operation (0 = success)
 */
int MPU6050_writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data) {
    //      010 value to write
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    // 00011100 mask byte
    // 10101111 original value (sample)
    // 10100011 original & ~mask
    // 10101011 masked | value
    uint8_t b;
    if (MPU6050_readByte(devAddr, regAddr, &b) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        data <<= (bitStart - length + 1); // shift data into correct position
        data &= mask; // zero all non-important bits in data
        b &= ~(mask); // zero all important bits in existing byte
        b |= data; // combine data with existing byte
        return MPU6050_writeByte(devAddr, regAddr, b);
    } else {
        return false;
    }
}

/* ================================================================================== */



int MPU6050_init(struct platform_device * i2c_plat_dev)
{
    int retVal = -1;
    int16_t ax, ay, az, gx, gy, gz;
    int i;

    MPU6050(0x68);
    pr_info("MPU6050: Testing connection...\n");

    if (!MPU6050_testConnection())
        return retVal;

    MPU6050_reset();
    MPU6050_initialize();

    pr_info("MPU6050: Getting some samples...\n");

    for (i = 0; i < 10; i++)
    {
        MPU6050_getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        pr_info("acc:\t%d\t%d\t%d\n", ax, ay, az);
        pr_info("vel:\t%d\t%d\t%d\n", gx, gy, gz);

        msleep(25);
    }

    pr_info("MPU6050 - DEV ID: %d\n", MPU6050_getDeviceID());

    return 0;
}

void MPU6050_deinit(void)
{
    MPU6050_setSleepEnabled(1);
    return;
}



/** Specific address constructor.
 * @param address I2C address
 * @see MPU6050_DEFAULT_ADDRESS
 * @see MPU6050_ADDRESS_AD0_LOW
 * @see MPU6050_ADDRESS_AD0_HIGH
 */
void MPU6050(uint8_t address) {
    mpu6050.devAddr = address;
}

/** Power on and prepare for general usage.
 * This will activate the device and take it out of sleep mode (which must be done
 * after start-up). This function also sets both the accelerometer and the gyroscope
 * to their most sensitive settings, namely +/- 2g and +/- 250 degrees/sec, and sets
 * the clock source to use the X Gyro for reference, which is slightly better than
 * the default internal clock source.
 */
void MPU6050_initialize(void) {
    MPU6050_setSleepEnabled(0); // thanks to Jack Elston for pointing this one out!
    // MPU6050_setClockSource(MPU6050_CLOCK_PLL_XGYRO);
    MPU6050_setFullScaleGyroRange(MPU6050_GYRO_FS_250);
    MPU6050_setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
}

/** Verify the I2C connection.
 * Make sure the device is connected and responds as expected.
 * @return True if connection is valid, false otherwise
 */
bool MPU6050_testConnection(void) {
    return MPU6050_getDeviceID() == 0x34;
}

// AUX_VDDIO register (InvenSense demo code calls this RA_*G_OFFS_TC)

/** Get the auxiliary I2C supply voltage level.
 * When set to 1, the auxiliary I2C bus high logic level is VDD. When cleared to
 * 0, the auxiliary I2C bus high logic level is VLOGIC. This does not apply to
 * the MPU-6000, which does not have a VLOGIC pin.
 * @return I2C supply voltage level (0=VLOGIC, 1=VDD)
 */
uint8_t MPU6050_getAuxVDDIOLevel(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_YG_OFFS_TC, MPU6050_TC_PWR_MODE_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set the auxiliary I2C supply voltage level.
 * When set to 1, the auxiliary I2C bus high logic level is VDD. When cleared to
 * 0, the auxiliary I2C bus high logic level is VLOGIC. This does not apply to
 * the MPU-6000, which does not have a VLOGIC pin.
 * @param level I2C supply voltage level (0=VLOGIC, 1=VDD)
 */
void MPU6050_setAuxVDDIOLevel(uint8_t level) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_YG_OFFS_TC, MPU6050_TC_PWR_MODE_BIT, level);
}

// SMPLRT_DIV register

/** Get gyroscope output rate divider.
 * The sensor register output, FIFO output, DMP sampling, Motion detection, Zero
 * Motion detection, and Free Fall detection are all based on the Sample Rate.
 * The Sample Rate is generated by dividing the gyroscope output rate by
 * SMPLRT_DIV:
 *
 * Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
 *
 * where Gyroscope Output Rate = 8kHz when the DLPF is disabled (DLPF_CFG = 0 or
 * 7), and 1kHz when the DLPF is enabled (see Register 26).
 *
 * Note: The accelerometer output rate is 1kHz. This means that for a Sample
 * Rate greater than 1kHz, the same accelerometer sample may be output to the
 * FIFO, DMP, and sensor registers more than once.
 *
 * For a diagram of the gyroscope and accelerometer signal paths, see Section 8
 * of the MPU-6000/MPU-6050 Product Specification document.
 *
 * @return Current sample rate
 * @see MPU6050_RA_SMPLRT_DIV
 */
uint8_t MPU6050_getRate(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_SMPLRT_DIV, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set gyroscope sample rate divider.
 * @param rate New sample rate divider
 * @see getRate()
 * @see MPU6050_RA_SMPLRT_DIV
 */
void MPU6050_setRate(uint8_t rate) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_SMPLRT_DIV, rate);
}

// CONFIG register

/** Get external FSYNC configuration.
 * Configures the external Frame Synchronization (FSYNC) pin sampling. An
 * external signal connected to the FSYNC pin can be sampled by configuring
 * EXT_SYNC_SET. Signal changes to the FSYNC pin are latched so that short
 * strobes may be captured. The latched FSYNC signal will be sampled at the
 * Sampling Rate, as defined in register 25. After sampling, the latch will
 * reset to the current FSYNC signal state.
 *
 * The sampled value will be reported in place of the least significant bit in
 * a sensor data register determined by the value of EXT_SYNC_SET according to
 * the following table.
 *
 * <pre>
 * EXT_SYNC_SET | FSYNC Bit Location
 * -------------+-------------------
 * 0            | Input disabled
 * 1            | TEMP_OUT_L[0]
 * 2            | GYRO_XOUT_L[0]
 * 3            | GYRO_YOUT_L[0]
 * 4            | GYRO_ZOUT_L[0]
 * 5            | ACCEL_XOUT_L[0]
 * 6            | ACCEL_YOUT_L[0]
 * 7            | ACCEL_ZOUT_L[0]
 * </pre>
 *
 * @return FSYNC configuration value
 */
uint8_t MPU6050_getExternalFrameSync(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_CONFIG, MPU6050_CFG_EXT_SYNC_SET_BIT, MPU6050_CFG_EXT_SYNC_SET_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set external FSYNC configuration.
 * @see getExternalFrameSync()
 * @see MPU6050_RA_CONFIG
 * @param sync New FSYNC configuration value
 */
void MPU6050_setExternalFrameSync(uint8_t sync) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_CONFIG, MPU6050_CFG_EXT_SYNC_SET_BIT, MPU6050_CFG_EXT_SYNC_SET_LENGTH, sync);
}
/** Get digital low-pass filter configuration.
 * The DLPF_CFG parameter sets the digital low pass filter configuration. It
 * also determines the internal sampling rate used by the device as shown in
 * the table below.
 *
 * Note: The accelerometer output rate is 1kHz. This means that for a Sample
 * Rate greater than 1kHz, the same accelerometer sample may be output to the
 * FIFO, DMP, and sensor registers more than once.
 *
 * <pre>
 *          |   ACCELEROMETER    |           GYROSCOPE
 * DLPF_CFG | Bandwidth | Delay  | Bandwidth | Delay  | Sample Rate
 * ---------+-----------+--------+-----------+--------+-------------
 * 0        | 260Hz     | 0ms    | 256Hz     | 0.98ms | 8kHz
 * 1        | 184Hz     | 2.0ms  | 188Hz     | 1.9ms  | 1kHz
 * 2        | 94Hz      | 3.0ms  | 98Hz      | 2.8ms  | 1kHz
 * 3        | 44Hz      | 4.9ms  | 42Hz      | 4.8ms  | 1kHz
 * 4        | 21Hz      | 8.5ms  | 20Hz      | 8.3ms  | 1kHz
 * 5        | 10Hz      | 13.8ms | 10Hz      | 13.4ms | 1kHz
 * 6        | 5Hz       | 19.0ms | 5Hz       | 18.6ms | 1kHz
 * 7        |   -- Reserved --   |   -- Reserved --   | Reserved
 * </pre>
 *
 * @return DLFP configuration
 * @see MPU6050_RA_CONFIG
 * @see MPU6050_CFG_DLPF_CFG_BIT
 * @see MPU6050_CFG_DLPF_CFG_LENGTH
 */
uint8_t MPU6050_getDLPFMode(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set digital low-pass filter configuration.
 * @param mode New DLFP configuration setting
 * @see getDLPFBandwidth()
 * @see MPU6050_DLPF_BW_256
 * @see MPU6050_RA_CONFIG
 * @see MPU6050_CFG_DLPF_CFG_BIT
 * @see MPU6050_CFG_DLPF_CFG_LENGTH
 */
void MPU6050_setDLPFMode(uint8_t mode) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, mode);
}

// GYRO_CONFIG register

/** Get full-scale gyroscope range.
 * The FS_SEL parameter allows setting the full-scale range of the gyro sensors,
 * as described in the table below.
 *
 * <pre>
 * 0 = +/- 250 degrees/sec
 * 1 = +/- 500 degrees/sec
 * 2 = +/- 1000 degrees/sec
 * 3 = +/- 2000 degrees/sec
 * </pre>
 *
 * @return Current full-scale gyroscope range setting
 * @see MPU6050_GYRO_FS_250
 * @see MPU6050_RA_GYRO_CONFIG
 * @see MPU6050_GCONFIG_FS_SEL_BIT
 * @see MPU6050_GCONFIG_FS_SEL_LENGTH
 */
uint8_t MPU6050_getFullScaleGyroRange(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set full-scale gyroscope range.
 * @param range New full-scale gyroscope range value
 * @see getFullScaleRange()
 * @see MPU6050_GYRO_FS_250
 * @see MPU6050_RA_GYRO_CONFIG
 * @see MPU6050_GCONFIG_FS_SEL_BIT
 * @see MPU6050_GCONFIG_FS_SEL_LENGTH
 */
void MPU6050_setFullScaleGyroRange(uint8_t range) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, range);
}

// ACCEL_CONFIG register

/** Get self-test enabled setting for accelerometer X axis.
 * @return Self-test enabled value
 * @see MPU6050_RA_ACCEL_CONFIG
 */
bool MPU6050_getAccelXSelfTest(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_XA_ST_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get self-test enabled setting for accelerometer X axis.
 * @param enabled Self-test enabled value
 * @see MPU6050_RA_ACCEL_CONFIG
 */
void MPU6050_setAccelXSelfTest(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_XA_ST_BIT, enabled);
}
/** Get self-test enabled value for accelerometer Y axis.
 * @return Self-test enabled value
 * @see MPU6050_RA_ACCEL_CONFIG
 */
bool MPU6050_getAccelYSelfTest(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_YA_ST_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get self-test enabled value for accelerometer Y axis.
 * @param enabled Self-test enabled value
 * @see MPU6050_RA_ACCEL_CONFIG
 */
void MPU6050_setAccelYSelfTest(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_YA_ST_BIT, enabled);
}
/** Get self-test enabled value for accelerometer Z axis.
 * @return Self-test enabled value
 * @see MPU6050_RA_ACCEL_CONFIG
 */
bool MPU6050_getAccelZSelfTest(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_ZA_ST_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set self-test enabled value for accelerometer Z axis.
 * @param enabled Self-test enabled value
 * @see MPU6050_RA_ACCEL_CONFIG
 */
void MPU6050_setAccelZSelfTest(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_ZA_ST_BIT, enabled);
}
/** Get full-scale accelerometer range.
 * The FS_SEL parameter allows setting the full-scale range of the accelerometer
 * sensors, as described in the table below.
 *
 * <pre>
 * 0 = +/- 2g
 * 1 = +/- 4g
 * 2 = +/- 8g
 * 3 = +/- 16g
 * </pre>
 *
 * @return Current full-scale accelerometer range setting
 * @see MPU6050_ACCEL_FS_2
 * @see MPU6050_RA_ACCEL_CONFIG
 * @see MPU6050_ACONFIG_AFS_SEL_BIT
 * @see MPU6050_ACONFIG_AFS_SEL_LENGTH
 */
uint8_t MPU6050_getFullScaleAccelRange(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set full-scale accelerometer range.
 * @param range New full-scale accelerometer range setting
 * @see getFullScaleAccelRange()
 */
void MPU6050_setFullScaleAccelRange(uint8_t range) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, range);
}
/** Get the high-pass filter configuration.
 * The DHPF is a filter module in the path leading to motion detectors (Free
 * Fall, Motion threshold, and Zero Motion). The high pass filter output is not
 * available to the data registers (see Figure in Section 8 of the MPU-6000/
 * MPU-6050 Product Specification document).
 *
 * The high pass filter has three modes:
 *
 * <pre>
 *    Reset: The filter output settles to zero within one sample. This
 *           effectively disables the high pass filter. This mode may be toggled
 *           to quickly settle the filter.
 *
 *    On:    The high pass filter will pass signals above the cut off frequency.
 *
 *    Hold:  When triggered, the filter holds the present sample. The filter
 *           output will be the difference between the input sample and the held
 *           sample.
 * </pre>
 *
 * <pre>
 * ACCEL_HPF | Filter Mode | Cut-off Frequency
 * ----------+-------------+------------------
 * 0         | Reset       | None
 * 1         | On          | 5Hz
 * 2         | On          | 2.5Hz
 * 3         | On          | 1.25Hz
 * 4         | On          | 0.63Hz
 * 7         | Hold        | None
 * </pre>
 *
 * @return Current high-pass filter configuration
 * @see MPU6050_DHPF_RESET
 * @see MPU6050_RA_ACCEL_CONFIG
 */
uint8_t MPU6050_getDHPFMode(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_ACCEL_HPF_BIT, MPU6050_ACONFIG_ACCEL_HPF_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set the high-pass filter configuration.
 * @param bandwidth New high-pass filter configuration
 * @see setDHPFMode()
 * @see MPU6050_DHPF_RESET
 * @see MPU6050_RA_ACCEL_CONFIG
 */
void MPU6050_setDHPFMode(uint8_t bandwidth) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_ACCEL_HPF_BIT, MPU6050_ACONFIG_ACCEL_HPF_LENGTH, bandwidth);
}

// FF_THR register

/** Get free-fall event acceleration threshold.
 * This register configures the detection threshold for Free Fall event
 * detection. The unit of FF_THR is 1LSB = 2mg. Free Fall is detected when the
 * absolute value of the accelerometer measurements for the three axes are each
 * less than the detection threshold. This condition increments the Free Fall
 * duration counter (Register 30). The Free Fall interrupt is triggered when the
 * Free Fall duration counter reaches the time specified in FF_DUR.
 *
 * For more details on the Free Fall detection interrupt, see Section 8.2 of the
 * MPU-6000/MPU-6050 Product Specification document as well as Registers 56 and
 * 58 of this document.
 *
 * @return Current free-fall acceleration threshold value (LSB = 2mg)
 * @see MPU6050_RA_FF_THR
 */
uint8_t MPU6050_getFreefallDetectionThreshold(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_FF_THR, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get free-fall event acceleration threshold.
 * @param threshold New free-fall acceleration threshold value (LSB = 2mg)
 * @see getFreefallDetectionThreshold()
 * @see MPU6050_RA_FF_THR
 */
void MPU6050_setFreefallDetectionThreshold(uint8_t threshold) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_FF_THR, threshold);
}

// FF_DUR register

/** Get free-fall event duration threshold.
 * This register configures the duration counter threshold for Free Fall event
 * detection. The duration counter ticks at 1kHz, therefore FF_DUR has a unit
 * of 1 LSB = 1 ms.
 *
 * The Free Fall duration counter increments while the absolute value of the
 * accelerometer measurements are each less than the detection threshold
 * (Register 29). The Free Fall interrupt is triggered when the Free Fall
 * duration counter reaches the time specified in this register.
 *
 * For more details on the Free Fall detection interrupt, see Section 8.2 of
 * the MPU-6000/MPU-6050 Product Specification document as well as Registers 56
 * and 58 of this document.
 *
 * @return Current free-fall duration threshold value (LSB = 1ms)
 * @see MPU6050_RA_FF_DUR
 */
uint8_t MPU6050_getFreefallDetectionDuration(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_FF_DUR, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get free-fall event duration threshold.
 * @param duration New free-fall duration threshold value (LSB = 1ms)
 * @see getFreefallDetectionDuration()
 * @see MPU6050_RA_FF_DUR
 */
void MPU6050_setFreefallDetectionDuration(uint8_t duration) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_FF_DUR, duration);
}

// MOT_THR register

/** Get motion detection event acceleration threshold.
 * This register configures the detection threshold for Motion interrupt
 * generation. The unit of MOT_THR is 1LSB = 2mg. Motion is detected when the
 * absolute value of any of the accelerometer measurements exceeds this Motion
 * detection threshold. This condition increments the Motion detection duration
 * counter (Register 32). The Motion detection interrupt is triggered when the
 * Motion Detection counter reaches the time count specified in MOT_DUR
 * (Register 32).
 *
 * The Motion interrupt will indicate the axis and polarity of detected motion
 * in MOT_DETECT_STATUS (Register 97).
 *
 * For more details on the Motion detection interrupt, see Section 8.3 of the
 * MPU-6000/MPU-6050 Product Specification document as well as Registers 56 and
 * 58 of this document.
 *
 * @return Current motion detection acceleration threshold value (LSB = 2mg)
 * @see MPU6050_RA_MOT_THR
 */
uint8_t MPU6050_getMotionDetectionThreshold(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_MOT_THR, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set free-fall event acceleration threshold.
 * @param threshold New motion detection acceleration threshold value (LSB = 2mg)
 * @see getMotionDetectionThreshold()
 * @see MPU6050_RA_MOT_THR
 */
void MPU6050_setMotionDetectionThreshold(uint8_t threshold) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_MOT_THR, threshold);
}

// MOT_DUR register

/** Get motion detection event duration threshold.
 * This register configures the duration counter threshold for Motion interrupt
 * generation. The duration counter ticks at 1 kHz, therefore MOT_DUR has a unit
 * of 1LSB = 1ms. The Motion detection duration counter increments when the
 * absolute value of any of the accelerometer measurements exceeds the Motion
 * detection threshold (Register 31). The Motion detection interrupt is
 * triggered when the Motion detection counter reaches the time count specified
 * in this register.
 *
 * For more details on the Motion detection interrupt, see Section 8.3 of the
 * MPU-6000/MPU-6050 Product Specification document.
 *
 * @return Current motion detection duration threshold value (LSB = 1ms)
 * @see MPU6050_RA_MOT_DUR
 */
uint8_t MPU6050_getMotionDetectionDuration(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_MOT_DUR, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set motion detection event duration threshold.
 * @param duration New motion detection duration threshold value (LSB = 1ms)
 * @see getMotionDetectionDuration()
 * @see MPU6050_RA_MOT_DUR
 */
void MPU6050_setMotionDetectionDuration(uint8_t duration) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_MOT_DUR, duration);
}

// ZRMOT_THR register

/** Get zero motion detection event acceleration threshold.
 * This register configures the detection threshold for Zero Motion interrupt
 * generation. The unit of ZRMOT_THR is 1LSB = 2mg. Zero Motion is detected when
 * the absolute value of the accelerometer measurements for the 3 axes are each
 * less than the detection threshold. This condition increments the Zero Motion
 * duration counter (Register 34). The Zero Motion interrupt is triggered when
 * the Zero Motion duration counter reaches the time count specified in
 * ZRMOT_DUR (Register 34).
 *
 * Unlike Free Fall or Motion detection, Zero Motion detection triggers an
 * interrupt both when Zero Motion is first detected and when Zero Motion is no
 * longer detected.
 *
 * When a zero motion event is detected, a Zero Motion Status will be indicated
 * in the MOT_DETECT_STATUS register (Register 97). When a motion-to-zero-motion
 * condition is detected, the status bit is set to 1. When a zero-motion-to-
 * motion condition is detected, the status bit is set to 0.
 *
 * For more details on the Zero Motion detection interrupt, see Section 8.4 of
 * the MPU-6000/MPU-6050 Product Specification document as well as Registers 56
 * and 58 of this document.
 *
 * @return Current zero motion detection acceleration threshold value (LSB = 2mg)
 * @see MPU6050_RA_ZRMOT_THR
 */
uint8_t MPU6050_getZeroMotionDetectionThreshold(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_ZRMOT_THR, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set zero motion detection event acceleration threshold.
 * @param threshold New zero motion detection acceleration threshold value (LSB = 2mg)
 * @see getZeroMotionDetectionThreshold()
 * @see MPU6050_RA_ZRMOT_THR
 */
void MPU6050_setZeroMotionDetectionThreshold(uint8_t threshold) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_ZRMOT_THR, threshold);
}

// ZRMOT_DUR register

/** Get zero motion detection event duration threshold.
 * This register configures the duration counter threshold for Zero Motion
 * interrupt generation. The duration counter ticks at 16 Hz, therefore
 * ZRMOT_DUR has a unit of 1 LSB = 64 ms. The Zero Motion duration counter
 * increments while the absolute value of the accelerometer measurements are
 * each less than the detection threshold (Register 33). The Zero Motion
 * interrupt is triggered when the Zero Motion duration counter reaches the time
 * count specified in this register.
 *
 * For more details on the Zero Motion detection interrupt, see Section 8.4 of
 * the MPU-6000/MPU-6050 Product Specification document, as well as Registers 56
 * and 58 of this document.
 *
 * @return Current zero motion detection duration threshold value (LSB = 64ms)
 * @see MPU6050_RA_ZRMOT_DUR
 */
uint8_t MPU6050_getZeroMotionDetectionDuration(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_ZRMOT_DUR, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set zero motion detection event duration threshold.
 * @param duration New zero motion detection duration threshold value (LSB = 1ms)
 * @see getZeroMotionDetectionDuration()
 * @see MPU6050_RA_ZRMOT_DUR
 */
void MPU6050_setZeroMotionDetectionDuration(uint8_t duration) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_ZRMOT_DUR, duration);
}

// FIFO_EN register

/** Get temperature FIFO enabled value.
 * When set to 1, this bit enables TEMP_OUT_H and TEMP_OUT_L (Registers 65 and
 * 66) to be written into the FIFO mpu6050.buffer.
 * @return Current temperature FIFO enabled value
 * @see MPU6050_RA_FIFO_EN
 */
bool MPU6050_getTempFIFOEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_TEMP_FIFO_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set temperature FIFO enabled value.
 * @param enabled New temperature FIFO enabled value
 * @see getTempFIFOEnabled()
 * @see MPU6050_RA_FIFO_EN
 */
void MPU6050_setTempFIFOEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_TEMP_FIFO_EN_BIT, enabled);
}
/** Get gyroscope X-axis FIFO enabled value.
 * When set to 1, this bit enables GYRO_XOUT_H and GYRO_XOUT_L (Registers 67 and
 * 68) to be written into the FIFO mpu6050.buffer.
 * @return Current gyroscope X-axis FIFO enabled value
 * @see MPU6050_RA_FIFO_EN
 */
bool MPU6050_getXGyroFIFOEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_XG_FIFO_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set gyroscope X-axis FIFO enabled value.
 * @param enabled New gyroscope X-axis FIFO enabled value
 * @see getXGyroFIFOEnabled()
 * @see MPU6050_RA_FIFO_EN
 */
void MPU6050_setXGyroFIFOEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_XG_FIFO_EN_BIT, enabled);
}
/** Get gyroscope Y-axis FIFO enabled value.
 * When set to 1, this bit enables GYRO_YOUT_H and GYRO_YOUT_L (Registers 69 and
 * 70) to be written into the FIFO mpu6050.buffer.
 * @return Current gyroscope Y-axis FIFO enabled value
 * @see MPU6050_RA_FIFO_EN
 */
bool MPU6050_getYGyroFIFOEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_YG_FIFO_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set gyroscope Y-axis FIFO enabled value.
 * @param enabled New gyroscope Y-axis FIFO enabled value
 * @see getYGyroFIFOEnabled()
 * @see MPU6050_RA_FIFO_EN
 */
void MPU6050_setYGyroFIFOEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_YG_FIFO_EN_BIT, enabled);
}
/** Get gyroscope Z-axis FIFO enabled value.
 * When set to 1, this bit enables GYRO_ZOUT_H and GYRO_ZOUT_L (Registers 71 and
 * 72) to be written into the FIFO mpu6050.buffer.
 * @return Current gyroscope Z-axis FIFO enabled value
 * @see MPU6050_RA_FIFO_EN
 */
bool MPU6050_getZGyroFIFOEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_ZG_FIFO_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set gyroscope Z-axis FIFO enabled value.
 * @param enabled New gyroscope Z-axis FIFO enabled value
 * @see getZGyroFIFOEnabled()
 * @see MPU6050_RA_FIFO_EN
 */
void MPU6050_setZGyroFIFOEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_ZG_FIFO_EN_BIT, enabled);
}
/** Get accelerometer FIFO enabled value.
 * When set to 1, this bit enables ACCEL_XOUT_H, ACCEL_XOUT_L, ACCEL_YOUT_H,
 * ACCEL_YOUT_L, ACCEL_ZOUT_H, and ACCEL_ZOUT_L (Registers 59 to 64) to be
 * written into the FIFO mpu6050.buffer.
 * @return Current accelerometer FIFO enabled value
 * @see MPU6050_RA_FIFO_EN
 */
bool MPU6050_getAccelFIFOEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_ACCEL_FIFO_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set accelerometer FIFO enabled value.
 * @param enabled New accelerometer FIFO enabled value
 * @see getAccelFIFOEnabled()
 * @see MPU6050_RA_FIFO_EN
 */
void MPU6050_setAccelFIFOEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_ACCEL_FIFO_EN_BIT, enabled);
}
/** Get Slave 2 FIFO enabled value.
 * When set to 1, this bit enables EXT_SENS_DATA registers (Registers 73 to 96)
 * associated with Slave 2 to be written into the FIFO mpu6050.buffer.
 * @return Current Slave 2 FIFO enabled value
 * @see MPU6050_RA_FIFO_EN
 */
bool MPU6050_getSlave2FIFOEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_SLV2_FIFO_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Slave 2 FIFO enabled value.
 * @param enabled New Slave 2 FIFO enabled value
 * @see getSlave2FIFOEnabled()
 * @see MPU6050_RA_FIFO_EN
 */
void MPU6050_setSlave2FIFOEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_SLV2_FIFO_EN_BIT, enabled);
}
/** Get Slave 1 FIFO enabled value.
 * When set to 1, this bit enables EXT_SENS_DATA registers (Registers 73 to 96)
 * associated with Slave 1 to be written into the FIFO mpu6050.buffer.
 * @return Current Slave 1 FIFO enabled value
 * @see MPU6050_RA_FIFO_EN
 */
bool MPU6050_getSlave1FIFOEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_SLV1_FIFO_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Slave 1 FIFO enabled value.
 * @param enabled New Slave 1 FIFO enabled value
 * @see getSlave1FIFOEnabled()
 * @see MPU6050_RA_FIFO_EN
 */
void MPU6050_setSlave1FIFOEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_SLV1_FIFO_EN_BIT, enabled);
}
/** Get Slave 0 FIFO enabled value.
 * When set to 1, this bit enables EXT_SENS_DATA registers (Registers 73 to 96)
 * associated with Slave 0 to be written into the FIFO mpu6050.buffer.
 * @return Current Slave 0 FIFO enabled value
 * @see MPU6050_RA_FIFO_EN
 */
bool MPU6050_getSlave0FIFOEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_SLV0_FIFO_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Slave 0 FIFO enabled value.
 * @param enabled New Slave 0 FIFO enabled value
 * @see getSlave0FIFOEnabled()
 * @see MPU6050_RA_FIFO_EN
 */
void MPU6050_setSlave0FIFOEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_FIFO_EN, MPU6050_SLV0_FIFO_EN_BIT, enabled);
}

// I2C_MST_CTRL register

/** Get multi-master enabled value.
 * Multi-master capability allows multiple I2C masters to operate on the same
 * bus. In circuits where multi-master capability is required, set MULT_MST_EN
 * to 1. This will increase current drawn by approximately 30uA.
 *
 * In circuits where multi-master capability is required, the state of the I2C
 * bus must always be monitored by each separate I2C Master. Before an I2C
 * Master can assume arbitration of the bus, it must first confirm that no other
 * I2C Master has arbitration of the bus. When MULT_MST_EN is set to 1, the
 * MPU-60X0's bus arbitration detection logic is turned on, enabling it to
 * detect when the bus is available.
 *
 * @return Current multi-master enabled value
 * @see MPU6050_RA_I2C_MST_CTRL
 */
bool MPU6050_getMultiMasterEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_CTRL, MPU6050_MULT_MST_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set multi-master enabled value.
 * @param enabled New multi-master enabled value
 * @see getMultiMasterEnabled()
 * @see MPU6050_RA_I2C_MST_CTRL
 */
void MPU6050_setMultiMasterEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_CTRL, MPU6050_MULT_MST_EN_BIT, enabled);
}
/** Get wait-for-external-sensor-data enabled value.
 * When the WAIT_FOR_ES bit is set to 1, the Data Ready interrupt will be
 * delayed until External Sensor data from the Slave Devices are loaded into the
 * EXT_SENS_DATA registers. This is used to ensure that both the internal sensor
 * data (i.e. from gyro and accel) and external sensor data have been loaded to
 * their respective data registers (i.e. the data is synced) when the Data Ready
 * interrupt is triggered.
 *
 * @return Current wait-for-external-sensor-data enabled value
 * @see MPU6050_RA_I2C_MST_CTRL
 */
bool MPU6050_getWaitForExternalSensorEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_CTRL, MPU6050_WAIT_FOR_ES_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set wait-for-external-sensor-data enabled value.
 * @param enabled New wait-for-external-sensor-data enabled value
 * @see getWaitForExternalSensorEnabled()
 * @see MPU6050_RA_I2C_MST_CTRL
 */
void MPU6050_setWaitForExternalSensorEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_CTRL, MPU6050_WAIT_FOR_ES_BIT, enabled);
}
/** Get Slave 3 FIFO enabled value.
 * When set to 1, this bit enables EXT_SENS_DATA registers (Registers 73 to 96)
 * associated with Slave 3 to be written into the FIFO mpu6050.buffer.
 * @return Current Slave 3 FIFO enabled value
 * @see MPU6050_RA_MST_CTRL
 */
bool MPU6050_getSlave3FIFOEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_CTRL, MPU6050_SLV_3_FIFO_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Slave 3 FIFO enabled value.
 * @param enabled New Slave 3 FIFO enabled value
 * @see getSlave3FIFOEnabled()
 * @see MPU6050_RA_MST_CTRL
 */
void MPU6050_setSlave3FIFOEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_CTRL, MPU6050_SLV_3_FIFO_EN_BIT, enabled);
}
/** Get slave read/write transition enabled value.
 * The I2C_MST_P_NSR bit configures the I2C Master's transition from one slave
 * read to the next slave read. If the bit equals 0, there will be a restart
 * between reads. If the bit equals 1, there will be a stop followed by a start
 * of the following read. When a write transaction follows a read transaction,
 * the stop followed by a start of the successive write will be always used.
 *
 * @return Current slave read/write transition enabled value
 * @see MPU6050_RA_I2C_MST_CTRL
 */
bool MPU6050_getSlaveReadWriteTransitionEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_CTRL, MPU6050_I2C_MST_P_NSR_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set slave read/write transition enabled value.
 * @param enabled New slave read/write transition enabled value
 * @see getSlaveReadWriteTransitionEnabled()
 * @see MPU6050_RA_I2C_MST_CTRL
 */
void MPU6050_setSlaveReadWriteTransitionEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_CTRL, MPU6050_I2C_MST_P_NSR_BIT, enabled);
}
/** Get I2C master clock speed.
 * I2C_MST_CLK is a 4 bit unsigned value which configures a divider on the
 * MPU-60X0 internal 8MHz clock. It sets the I2C master clock speed according to
 * the following table:
 *
 * <pre>
 * I2C_MST_CLK | I2C Master Clock Speed | 8MHz Clock Divider
 * ------------+------------------------+-------------------
 * 0           | 348kHz                 | 23
 * 1           | 333kHz                 | 24
 * 2           | 320kHz                 | 25
 * 3           | 308kHz                 | 26
 * 4           | 296kHz                 | 27
 * 5           | 286kHz                 | 28
 * 6           | 276kHz                 | 29
 * 7           | 267kHz                 | 30
 * 8           | 258kHz                 | 31
 * 9           | 500kHz                 | 16
 * 10          | 471kHz                 | 17
 * 11          | 444kHz                 | 18
 * 12          | 421kHz                 | 19
 * 13          | 400kHz                 | 20
 * 14          | 381kHz                 | 21
 * 15          | 364kHz                 | 22
 * </pre>
 *
 * @return Current I2C master clock speed
 * @see MPU6050_RA_I2C_MST_CTRL
 */
uint8_t MPU6050_getMasterClockSpeed(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_I2C_MST_CTRL, MPU6050_I2C_MST_CLK_BIT, MPU6050_I2C_MST_CLK_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set I2C master clock speed.
 * @reparam speed Current I2C master clock speed
 * @see MPU6050_RA_I2C_MST_CTRL
 */
void MPU6050_setMasterClockSpeed(uint8_t speed) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_I2C_MST_CTRL, MPU6050_I2C_MST_CLK_BIT, MPU6050_I2C_MST_CLK_LENGTH, speed);
}

// I2C_SLV* registers (Slave 0-3)

/** Get the I2C address of the specified slave (0-3).
 * Note that Bit 7 (MSB) controls read/write mode. If Bit 7 is set, it's a read
 * operation, and if it is cleared, then it's a write operation. The remaining
 * bits (6-0) are the 7-bit device address of the slave device.
 *
 * In read mode, the result of the read is placed in the lowest available 
 * EXT_SENS_DATA register. For further information regarding the allocation of
 * read results, please refer to the EXT_SENS_DATA register description
 * (Registers 73 - 96).
 *
 * The MPU-6050 supports a total of five slaves, but Slave 4 has unique
 * characteristics, and so it has its own functions (getSlave4* and setSlave4*).
 *
 * I2C data transactions are performed at the Sample Rate, as defined in
 * Register 25. The user is responsible for ensuring that I2C data transactions
 * to and from each enabled Slave can be completed within a single period of the
 * Sample Rate.
 *
 * The I2C slave access rate can be reduced relative to the Sample Rate. This
 * reduced access rate is determined by I2C_MST_DLY (Register 52). Whether a
 * slave's access rate is reduced relative to the Sample Rate is determined by
 * I2C_MST_DELAY_CTRL (Register 103).
 *
 * The processing order for the slaves is fixed. The sequence followed for
 * processing the slaves is Slave 0, Slave 1, Slave 2, Slave 3 and Slave 4. If a
 * particular Slave is disabled it will be skipped.
 *
 * Each slave can either be accessed at the sample rate or at a reduced sample
 * rate. In a case where some slaves are accessed at the Sample Rate and some
 * slaves are accessed at the reduced rate, the sequence of accessing the slaves
 * (Slave 0 to Slave 4) is still followed. However, the reduced rate slaves will
 * be skipped if their access rate dictates that they should not be accessed
 * during that particular cycle. For further information regarding the reduced
 * access rate, please refer to Register 52. Whether a slave is accessed at the
 * Sample Rate or at the reduced rate is determined by the Delay Enable bits in
 * Register 103.
 *
 * @param num Slave number (0-3)
 * @return Current address for specified slave
 * @see MPU6050_RA_I2C_SLV0_ADDR
 */
uint8_t MPU6050_getSlaveAddress(uint8_t num) {
    if (num > 3) return 0;
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_ADDR + num*3, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set the I2C address of the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param address New address for specified slave
 * @see getSlaveAddress()
 * @see MPU6050_RA_I2C_SLV0_ADDR
 */
void MPU6050_setSlaveAddress(uint8_t num, uint8_t address) {
    if (num > 3) return;
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_ADDR + num*3, address);
}
/** Get the active internal register for the specified slave (0-3).
 * Read/write operations for this slave will be done to whatever internal
 * register address is stored in this MPU register.
 *
 * The MPU-6050 supports a total of five slaves, but Slave 4 has unique
 * characteristics, and so it has its own functions.
 *
 * @param num Slave number (0-3)
 * @return Current active register for specified slave
 * @see MPU6050_RA_I2C_SLV0_REG
 */
uint8_t MPU6050_getSlaveRegister(uint8_t num) {
    if (num > 3) return 0;
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_REG + num*3, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set the active internal register for the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param reg New active register for specified slave
 * @see getSlaveRegister()
 * @see MPU6050_RA_I2C_SLV0_REG
 */
void MPU6050_setSlaveRegister(uint8_t num, uint8_t reg) {
    if (num > 3) return;
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_REG + num*3, reg);
}
/** Get the enabled value for the specified slave (0-3).
 * When set to 1, this bit enables Slave 0 for data transfer operations. When
 * cleared to 0, this bit disables Slave 0 from data transfer operations.
 * @param num Slave number (0-3)
 * @return Current enabled value for specified slave
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
bool MPU6050_getSlaveEnabled(uint8_t num) {
    if (num > 3) return 0;
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_CTRL + num*3, MPU6050_I2C_SLV_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set the enabled value for the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param enabled New enabled value for specified slave
 * @see getSlaveEnabled()
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
void MPU6050_setSlaveEnabled(uint8_t num, bool enabled) {
    if (num > 3) return;
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_CTRL + num*3, MPU6050_I2C_SLV_EN_BIT, enabled);
}
/** Get word pair byte-swapping enabled for the specified slave (0-3).
 * When set to 1, this bit enables byte swapping. When byte swapping is enabled,
 * the high and low bytes of a word pair are swapped. Please refer to
 * I2C_SLV0_GRP for the pairing convention of the word pairs. When cleared to 0,
 * bytes transferred to and from Slave 0 will be written to EXT_SENS_DATA
 * registers in the order they were transferred.
 *
 * @param num Slave number (0-3)
 * @return Current word pair byte-swapping enabled value for specified slave
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
bool MPU6050_getSlaveWordByteSwap(uint8_t num) {
    if (num > 3) return 0;
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_CTRL + num*3, MPU6050_I2C_SLV_BYTE_SW_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set word pair byte-swapping enabled for the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param enabled New word pair byte-swapping enabled value for specified slave
 * @see getSlaveWordByteSwap()
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
void MPU6050_setSlaveWordByteSwap(uint8_t num, bool enabled) {
    if (num > 3) return;
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_CTRL + num*3, MPU6050_I2C_SLV_BYTE_SW_BIT, enabled);
}
/** Get write mode for the specified slave (0-3).
 * When set to 1, the transaction will read or write data only. When cleared to
 * 0, the transaction will write a register address prior to reading or writing
 * data. This should equal 0 when specifying the register address within the
 * Slave device to/from which the ensuing data transaction will take place.
 *
 * @param num Slave number (0-3)
 * @return Current write mode for specified slave (0 = register address + data, 1 = data only)
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
bool MPU6050_getSlaveWriteMode(uint8_t num) {
    if (num > 3) return 0;
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_CTRL + num*3, MPU6050_I2C_SLV_REG_DIS_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set write mode for the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param mode New write mode for specified slave (0 = register address + data, 1 = data only)
 * @see getSlaveWriteMode()
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
void MPU6050_setSlaveWriteMode(uint8_t num, bool mode) {
    if (num > 3) return;
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_CTRL + num*3, MPU6050_I2C_SLV_REG_DIS_BIT, mode);
}
/** Get word pair grouping order offset for the specified slave (0-3).
 * This sets specifies the grouping order of word pairs received from registers.
 * When cleared to 0, bytes from register addresses 0 and 1, 2 and 3, etc (even,
 * then odd register addresses) are paired to form a word. When set to 1, bytes
 * from register addresses are paired 1 and 2, 3 and 4, etc. (odd, then even
 * register addresses) are paired to form a word.
 *
 * @param num Slave number (0-3)
 * @return Current word pair grouping order offset for specified slave
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
bool MPU6050_getSlaveWordGroupOffset(uint8_t num) {
    if (num > 3) return 0;
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_CTRL + num*3, MPU6050_I2C_SLV_GRP_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set word pair grouping order offset for the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param enabled New word pair grouping order offset for specified slave
 * @see getSlaveWordGroupOffset()
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
void MPU6050_setSlaveWordGroupOffset(uint8_t num, bool enabled) {
    if (num > 3) return;
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_CTRL + num*3, MPU6050_I2C_SLV_GRP_BIT, enabled);
}
/** Get number of bytes to read for the specified slave (0-3).
 * Specifies the number of bytes transferred to and from Slave 0. Clearing this
 * bit to 0 is equivalent to disabling the register by writing 0 to I2C_SLV0_EN.
 * @param num Slave number (0-3)
 * @return Number of bytes to read for specified slave
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
uint8_t MPU6050_getSlaveDataLength(uint8_t num) {
    if (num > 3) return 0;
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_CTRL + num*3, MPU6050_I2C_SLV_LEN_BIT, MPU6050_I2C_SLV_LEN_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set number of bytes to read for the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param length Number of bytes to read for specified slave
 * @see getSlaveDataLength()
 * @see MPU6050_RA_I2C_SLV0_CTRL
 */
void MPU6050_setSlaveDataLength(uint8_t num, uint8_t length) {
    if (num > 3) return;
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_CTRL + num*3, MPU6050_I2C_SLV_LEN_BIT, MPU6050_I2C_SLV_LEN_LENGTH, length);
}

// I2C_SLV* registers (Slave 4)

/** Get the I2C address of Slave 4.
 * Note that Bit 7 (MSB) controls read/write mode. If Bit 7 is set, it's a read
 * operation, and if it is cleared, then it's a write operation. The remaining
 * bits (6-0) are the 7-bit device address of the slave device.
 *
 * @return Current address for Slave 4
 * @see getSlaveAddress()
 * @see MPU6050_RA_I2C_SLV4_ADDR
 */
uint8_t MPU6050_getSlave4Address(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_I2C_SLV4_ADDR, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set the I2C address of Slave 4.
 * @param address New address for Slave 4
 * @see getSlave4Address()
 * @see MPU6050_RA_I2C_SLV4_ADDR
 */
void MPU6050_setSlave4Address(uint8_t address) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_I2C_SLV4_ADDR, address);
}
/** Get the active internal register for the Slave 4.
 * Read/write operations for this slave will be done to whatever internal
 * register address is stored in this MPU register.
 *
 * @return Current active register for Slave 4
 * @see MPU6050_RA_I2C_SLV4_REG
 */
uint8_t MPU6050_getSlave4Register(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_I2C_SLV4_REG, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set the active internal register for Slave 4.
 * @param reg New active register for Slave 4
 * @see getSlave4Register()
 * @see MPU6050_RA_I2C_SLV4_REG
 */
void MPU6050_setSlave4Register(uint8_t reg) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_I2C_SLV4_REG, reg);
}
/** Set new byte to write to Slave 4.
 * This register stores the data to be written into the Slave 4. If I2C_SLV4_RW
 * is set 1 (set to read), this register has no effect.
 * @param data New byte to write to Slave 4
 * @see MPU6050_RA_I2C_SLV4_DO
 */
void MPU6050_setSlave4OutputByte(uint8_t data) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_I2C_SLV4_DO, data);
}
/** Get the enabled value for the Slave 4.
 * When set to 1, this bit enables Slave 4 for data transfer operations. When
 * cleared to 0, this bit disables Slave 4 from data transfer operations.
 * @return Current enabled value for Slave 4
 * @see MPU6050_RA_I2C_SLV4_CTRL
 */
bool MPU6050_getSlave4Enabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set the enabled value for Slave 4.
 * @param enabled New enabled value for Slave 4
 * @see getSlave4Enabled()
 * @see MPU6050_RA_I2C_SLV4_CTRL
 */
void MPU6050_setSlave4Enabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_EN_BIT, enabled);
}
/** Get the enabled value for Slave 4 transaction interrupts.
 * When set to 1, this bit enables the generation of an interrupt signal upon
 * completion of a Slave 4 transaction. When cleared to 0, this bit disables the
 * generation of an interrupt signal upon completion of a Slave 4 transaction.
 * The interrupt status can be observed in Register 54.
 *
 * @return Current enabled value for Slave 4 transaction interrupts.
 * @see MPU6050_RA_I2C_SLV4_CTRL
 */
bool MPU6050_getSlave4InterruptEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_INT_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set the enabled value for Slave 4 transaction interrupts.
 * @param enabled New enabled value for Slave 4 transaction interrupts.
 * @see getSlave4InterruptEnabled()
 * @see MPU6050_RA_I2C_SLV4_CTRL
 */
void MPU6050_setSlave4InterruptEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_INT_EN_BIT, enabled);
}
/** Get write mode for Slave 4.
 * When set to 1, the transaction will read or write data only. When cleared to
 * 0, the transaction will write a register address prior to reading or writing
 * data. This should equal 0 when specifying the register address within the
 * Slave device to/from which the ensuing data transaction will take place.
 *
 * @return Current write mode for Slave 4 (0 = register address + data, 1 = data only)
 * @see MPU6050_RA_I2C_SLV4_CTRL
 */
bool MPU6050_getSlave4WriteMode(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_REG_DIS_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set write mode for the Slave 4.
 * @param mode New write mode for Slave 4 (0 = register address + data, 1 = data only)
 * @see getSlave4WriteMode()
 * @see MPU6050_RA_I2C_SLV4_CTRL
 */
void MPU6050_setSlave4WriteMode(bool mode) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_REG_DIS_BIT, mode);
}
/** Get Slave 4 master delay value.
 * This configures the reduced access rate of I2C slaves relative to the Sample
 * Rate. When a slave's access rate is decreased relative to the Sample Rate,
 * the slave is accessed every:
 *
 *     1 / (1 + I2C_MST_DLY) samples
 *
 * This base Sample Rate in turn is determined by SMPLRT_DIV (register 25) and
 * DLPF_CFG (register 26). Whether a slave's access rate is reduced relative to
 * the Sample Rate is determined by I2C_MST_DELAY_CTRL (register 103). For
 * further information regarding the Sample Rate, please refer to register 25.
 *
 * @return Current Slave 4 master delay value
 * @see MPU6050_RA_I2C_SLV4_CTRL
 */
uint8_t MPU6050_getSlave4MasterDelay(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_MST_DLY_BIT, MPU6050_I2C_SLV4_MST_DLY_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Slave 4 master delay value.
 * @param delay New Slave 4 master delay value
 * @see getSlave4MasterDelay()
 * @see MPU6050_RA_I2C_SLV4_CTRL
 */
void MPU6050_setSlave4MasterDelay(uint8_t delay) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_I2C_SLV4_CTRL, MPU6050_I2C_SLV4_MST_DLY_BIT, MPU6050_I2C_SLV4_MST_DLY_LENGTH, delay);
}
/** Get last available byte read from Slave 4.
 * This register stores the data read from Slave 4. This field is populated
 * after a read transaction.
 * @return Last available byte read from to Slave 4
 * @see MPU6050_RA_I2C_SLV4_DI
 */
uint8_t MPU6050_getSlate4InputByte(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_I2C_SLV4_DI, mpu6050.buffer);
    return mpu6050.buffer[0];
}

// I2C_MST_STATUS register

/** Get FSYNC interrupt status.
 * This bit reflects the status of the FSYNC interrupt from an external device
 * into the MPU-60X0. This is used as a way to pass an external interrupt
 * through the MPU-60X0 to the host application processor. When set to 1, this
 * bit will cause an interrupt if FSYNC_INT_EN is asserted in INT_PIN_CFG
 * (Register 55).
 * @return FSYNC interrupt status
 * @see MPU6050_RA_I2C_MST_STATUS
 */
bool MPU6050_getPassthroughStatus(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_PASS_THROUGH_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get Slave 4 transaction done status.
 * Automatically sets to 1 when a Slave 4 transaction has completed. This
 * triggers an interrupt if the I2C_MST_INT_EN bit in the INT_ENABLE register
 * (Register 56) is asserted and if the SLV_4_DONE_INT bit is asserted in the
 * I2C_SLV4_CTRL register (Register 52).
 * @return Slave 4 transaction done status
 * @see MPU6050_RA_I2C_MST_STATUS
 */
bool MPU6050_getSlave4IsDone(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_I2C_SLV4_DONE_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get master arbitration lost status.
 * This bit automatically sets to 1 when the I2C Master has lost arbitration of
 * the auxiliary I2C bus (an error condition). This triggers an interrupt if the
 * I2C_MST_INT_EN bit in the INT_ENABLE register (Register 56) is asserted.
 * @return Master arbitration lost status
 * @see MPU6050_RA_I2C_MST_STATUS
 */
bool MPU6050_getLostArbitration(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_I2C_LOST_ARB_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get Slave 4 NACK status.
 * This bit automatically sets to 1 when the I2C Master receives a NACK in a
 * transaction with Slave 4. This triggers an interrupt if the I2C_MST_INT_EN
 * bit in the INT_ENABLE register (Register 56) is asserted.
 * @return Slave 4 NACK interrupt status
 * @see MPU6050_RA_I2C_MST_STATUS
 */
bool MPU6050_getSlave4Nack(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_I2C_SLV4_NACK_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get Slave 3 NACK status.
 * This bit automatically sets to 1 when the I2C Master receives a NACK in a
 * transaction with Slave 3. This triggers an interrupt if the I2C_MST_INT_EN
 * bit in the INT_ENABLE register (Register 56) is asserted.
 * @return Slave 3 NACK interrupt status
 * @see MPU6050_RA_I2C_MST_STATUS
 */
bool MPU6050_getSlave3Nack(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_I2C_SLV3_NACK_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get Slave 2 NACK status.
 * This bit automatically sets to 1 when the I2C Master receives a NACK in a
 * transaction with Slave 2. This triggers an interrupt if the I2C_MST_INT_EN
 * bit in the INT_ENABLE register (Register 56) is asserted.
 * @return Slave 2 NACK interrupt status
 * @see MPU6050_RA_I2C_MST_STATUS
 */
bool MPU6050_getSlave2Nack(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_I2C_SLV2_NACK_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get Slave 1 NACK status.
 * This bit automatically sets to 1 when the I2C Master receives a NACK in a
 * transaction with Slave 1. This triggers an interrupt if the I2C_MST_INT_EN
 * bit in the INT_ENABLE register (Register 56) is asserted.
 * @return Slave 1 NACK interrupt status
 * @see MPU6050_RA_I2C_MST_STATUS
 */
bool MPU6050_getSlave1Nack(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_I2C_SLV1_NACK_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get Slave 0 NACK status.
 * This bit automatically sets to 1 when the I2C Master receives a NACK in a
 * transaction with Slave 0. This triggers an interrupt if the I2C_MST_INT_EN
 * bit in the INT_ENABLE register (Register 56) is asserted.
 * @return Slave 0 NACK interrupt status
 * @see MPU6050_RA_I2C_MST_STATUS
 */
bool MPU6050_getSlave0Nack(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_STATUS, MPU6050_MST_I2C_SLV0_NACK_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}

// INT_PIN_CFG register

/** Get interrupt logic level mode.
 * Will be set 0 for active-high, 1 for active-low.
 * @return Current interrupt mode (0=active-high, 1=active-low)
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_INT_LEVEL_BIT
 */
bool MPU6050_getInterruptMode(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_LEVEL_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set interrupt logic level mode.
 * @param mode New interrupt mode (0=active-high, 1=active-low)
 * @see getInterruptMode()
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_INT_LEVEL_BIT
 */
void MPU6050_setInterruptMode(bool mode) {
   MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_LEVEL_BIT, mode);
}
/** Get interrupt drive mode.
 * Will be set 0 for push-pull, 1 for open-drain.
 * @return Current interrupt drive mode (0=push-pull, 1=open-drain)
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_INT_OPEN_BIT
 */
bool MPU6050_getInterruptDrive(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_OPEN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set interrupt drive mode.
 * @param drive New interrupt drive mode (0=push-pull, 1=open-drain)
 * @see getInterruptDrive()
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_INT_OPEN_BIT
 */
void MPU6050_setInterruptDrive(bool drive) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_OPEN_BIT, drive);
}
/** Get interrupt latch mode.
 * Will be set 0 for 50us-pulse, 1 for latch-until-int-cleared.
 * @return Current latch mode (0=50us-pulse, 1=latch-until-int-cleared)
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_LATCH_INT_EN_BIT
 */
bool MPU6050_getInterruptLatch(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_LATCH_INT_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set interrupt latch mode.
 * @param latch New latch mode (0=50us-pulse, 1=latch-until-int-cleared)
 * @see getInterruptLatch()
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_LATCH_INT_EN_BIT
 */
void MPU6050_setInterruptLatch(bool latch) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_LATCH_INT_EN_BIT, latch);
}
/** Get interrupt latch clear mode.
 * Will be set 0 for status-read-only, 1 for any-register-read.
 * @return Current latch clear mode (0=status-read-only, 1=any-register-read)
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_INT_RD_CLEAR_BIT
 */
bool MPU6050_getInterruptLatchClear(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_RD_CLEAR_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set interrupt latch clear mode.
 * @param clear New latch clear mode (0=status-read-only, 1=any-register-read)
 * @see getInterruptLatchClear()
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_INT_RD_CLEAR_BIT
 */
void MPU6050_setInterruptLatchClear(bool clear) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_RD_CLEAR_BIT, clear);
}
/** Get FSYNC interrupt logic level mode.
 * @return Current FSYNC interrupt mode (0=active-high, 1=active-low)
 * @see getFSyncInterruptMode()
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_FSYNC_INT_LEVEL_BIT
 */
bool MPU6050_getFSyncInterruptLevel(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_FSYNC_INT_LEVEL_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set FSYNC interrupt logic level mode.
 * @param mode New FSYNC interrupt mode (0=active-high, 1=active-low)
 * @see getFSyncInterruptMode()
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_FSYNC_INT_LEVEL_BIT
 */
void MPU6050_setFSyncInterruptLevel(bool level) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_FSYNC_INT_LEVEL_BIT, level);
}
/** Get FSYNC pin interrupt enabled setting.
 * Will be set 0 for disabled, 1 for enabled.
 * @return Current interrupt enabled setting
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_FSYNC_INT_EN_BIT
 */
bool MPU6050_getFSyncInterruptEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_FSYNC_INT_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set FSYNC pin interrupt enabled setting.
 * @param enabled New FSYNC pin interrupt enabled setting
 * @see getFSyncInterruptEnabled()
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_FSYNC_INT_EN_BIT
 */
void MPU6050_setFSyncInterruptEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_FSYNC_INT_EN_BIT, enabled);
}
/** Get I2C bypass enabled status.
 * When this bit is equal to 1 and I2C_MST_EN (Register 106 bit[5]) is equal to
 * 0, the host application processor will be able to directly access the
 * auxiliary I2C bus of the MPU-60X0. When this bit is equal to 0, the host
 * application processor will not be able to directly access the auxiliary I2C
 * bus of the MPU-60X0 regardless of the state of I2C_MST_EN (Register 106
 * bit[5]).
 * @return Current I2C bypass enabled status
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_I2C_BYPASS_EN_BIT
 */
bool MPU6050_getI2CBypassEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_I2C_BYPASS_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set I2C bypass enabled status.
 * When this bit is equal to 1 and I2C_MST_EN (Register 106 bit[5]) is equal to
 * 0, the host application processor will be able to directly access the
 * auxiliary I2C bus of the MPU-60X0. When this bit is equal to 0, the host
 * application processor will not be able to directly access the auxiliary I2C
 * bus of the MPU-60X0 regardless of the state of I2C_MST_EN (Register 106
 * bit[5]).
 * @param enabled New I2C bypass enabled status
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_I2C_BYPASS_EN_BIT
 */
void MPU6050_setI2CBypassEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_I2C_BYPASS_EN_BIT, enabled);
}
/** Get reference clock output enabled status.
 * When this bit is equal to 1, a reference clock output is provided at the
 * CLKOUT pin. When this bit is equal to 0, the clock output is disabled. For
 * further information regarding CLKOUT, please refer to the MPU-60X0 Product
 * Specification document.
 * @return Current reference clock output enabled status
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_CLKOUT_EN_BIT
 */
bool MPU6050_getClockOutputEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_CLKOUT_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set reference clock output enabled status.
 * When this bit is equal to 1, a reference clock output is provided at the
 * CLKOUT pin. When this bit is equal to 0, the clock output is disabled. For
 * further information regarding CLKOUT, please refer to the MPU-60X0 Product
 * Specification document.
 * @param enabled New reference clock output enabled status
 * @see MPU6050_RA_INT_PIN_CFG
 * @see MPU6050_INTCFG_CLKOUT_EN_BIT
 */
void MPU6050_setClockOutputEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_CLKOUT_EN_BIT, enabled);
}

// INT_ENABLE register

/** Get full interrupt enabled status.
 * Full register byte for all interrupts, for quick reading. Each bit will be
 * set 0 for disabled, 1 for enabled.
 * @return Current interrupt enabled status
 * @see MPU6050_RA_INT_ENABLE
 * @see MPU6050_INTERRUPT_FF_BIT
 **/
uint8_t MPU6050_getIntEnabled(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set full interrupt enabled status.
 * Full register byte for all interrupts, for quick reading. Each bit should be
 * set 0 for disabled, 1 for enabled.
 * @param enabled New interrupt enabled status
 * @see getIntFreefallEnabled()
 * @see MPU6050_RA_INT_ENABLE
 * @see MPU6050_INTERRUPT_FF_BIT
 **/
void MPU6050_setIntEnabled(uint8_t enabled) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, enabled);
}
/** Get Free Fall interrupt enabled status.
 * Will be set 0 for disabled, 1 for enabled.
 * @return Current interrupt enabled status
 * @see MPU6050_RA_INT_ENABLE
 * @see MPU6050_INTERRUPT_FF_BIT
 **/
bool MPU6050_getIntFreefallEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_FF_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Free Fall interrupt enabled status.
 * @param enabled New interrupt enabled status
 * @see getIntFreefallEnabled()
 * @see MPU6050_RA_INT_ENABLE
 * @see MPU6050_INTERRUPT_FF_BIT
 **/
void MPU6050_setIntFreefallEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_FF_BIT, enabled);
}
/** Get Motion Detection interrupt enabled status.
 * Will be set 0 for disabled, 1 for enabled.
 * @return Current interrupt enabled status
 * @see MPU6050_RA_INT_ENABLE
 * @see MPU6050_INTERRUPT_MOT_BIT
 **/
bool MPU6050_getIntMotionEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_MOT_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Motion Detection interrupt enabled status.
 * @param enabled New interrupt enabled status
 * @see getIntMotionEnabled()
 * @see MPU6050_RA_INT_ENABLE
 * @see MPU6050_INTERRUPT_MOT_BIT
 **/
void MPU6050_setIntMotionEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_MOT_BIT, enabled);
}
/** Get Zero Motion Detection interrupt enabled status.
 * Will be set 0 for disabled, 1 for enabled.
 * @return Current interrupt enabled status
 * @see MPU6050_RA_INT_ENABLE
 * @see MPU6050_INTERRUPT_ZMOT_BIT
 **/
bool MPU6050_getIntZeroMotionEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_ZMOT_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Zero Motion Detection interrupt enabled status.
 * @param enabled New interrupt enabled status
 * @see getIntZeroMotionEnabled()
 * @see MPU6050_RA_INT_ENABLE
 * @see MPU6050_INTERRUPT_ZMOT_BIT
 **/
void MPU6050_setIntZeroMotionEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_ZMOT_BIT, enabled);
}
/** Get FIFO Buffer Overflow interrupt enabled status.
 * Will be set 0 for disabled, 1 for enabled.
 * @return Current interrupt enabled status
 * @see MPU6050_RA_INT_ENABLE
 * @see MPU6050_INTERRUPT_FIFO_OFLOW_BIT
 **/
bool MPU6050_getIntFIFOBufferOverflowEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_FIFO_OFLOW_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set FIFO Buffer Overflow interrupt enabled status.
 * @param enabled New interrupt enabled status
 * @see getIntFIFOBufferOverflowEnabled()
 * @see MPU6050_RA_INT_ENABLE
 * @see MPU6050_INTERRUPT_FIFO_OFLOW_BIT
 **/
void MPU6050_setIntFIFOBufferOverflowEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_FIFO_OFLOW_BIT, enabled);
}
/** Get I2C Master interrupt enabled status.
 * This enables any of the I2C Master interrupt sources to generate an
 * interrupt. Will be set 0 for disabled, 1 for enabled.
 * @return Current interrupt enabled status
 * @see MPU6050_RA_INT_ENABLE
 * @see MPU6050_INTERRUPT_I2C_MST_INT_BIT
 **/
bool MPU6050_getIntI2CMasterEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_I2C_MST_INT_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set I2C Master interrupt enabled status.
 * @param enabled New interrupt enabled status
 * @see getIntI2CMasterEnabled()
 * @see MPU6050_RA_INT_ENABLE
 * @see MPU6050_INTERRUPT_I2C_MST_INT_BIT
 **/
void MPU6050_setIntI2CMasterEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_I2C_MST_INT_BIT, enabled);
}
/** Get Data Ready interrupt enabled setting.
 * This event occurs each time a write operation to all of the sensor registers
 * has been completed. Will be set 0 for disabled, 1 for enabled.
 * @return Current interrupt enabled status
 * @see MPU6050_RA_INT_ENABLE
 * @see MPU6050_INTERRUPT_DATA_RDY_BIT
 */
bool MPU6050_getIntDataReadyEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_DATA_RDY_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Data Ready interrupt enabled status.
 * @param enabled New interrupt enabled status
 * @see getIntDataReadyEnabled()
 * @see MPU6050_RA_INT_CFG
 * @see MPU6050_INTERRUPT_DATA_RDY_BIT
 */
void MPU6050_setIntDataReadyEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_DATA_RDY_BIT, enabled);
}

// INT_STATUS register

/** Get full set of interrupt status bits.
 * These bits clear to 0 after the register has been read. Very useful
 * for getting multiple INT statuses, since each single bit read clears
 * all of them because it has to read the whole byte.
 * @return Current interrupt status
 * @see MPU6050_RA_INT_STATUS
 */
uint8_t MPU6050_getIntStatus(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_INT_STATUS, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get Free Fall interrupt status.
 * This bit automatically sets to 1 when a Free Fall interrupt has been
 * generated. The bit clears to 0 after the register has been read.
 * @return Current interrupt status
 * @see MPU6050_RA_INT_STATUS
 * @see MPU6050_INTERRUPT_FF_BIT
 */
bool MPU6050_getIntFreefallStatus(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_FF_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get Motion Detection interrupt status.
 * This bit automatically sets to 1 when a Motion Detection interrupt has been
 * generated. The bit clears to 0 after the register has been read.
 * @return Current interrupt status
 * @see MPU6050_RA_INT_STATUS
 * @see MPU6050_INTERRUPT_MOT_BIT
 */
bool MPU6050_getIntMotionStatus(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_MOT_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get Zero Motion Detection interrupt status.
 * This bit automatically sets to 1 when a Zero Motion Detection interrupt has
 * been generated. The bit clears to 0 after the register has been read.
 * @return Current interrupt status
 * @see MPU6050_RA_INT_STATUS
 * @see MPU6050_INTERRUPT_ZMOT_BIT
 */
bool MPU6050_getIntZeroMotionStatus(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_ZMOT_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get FIFO Buffer Overflow interrupt status.
 * This bit automatically sets to 1 when a Free Fall interrupt has been
 * generated. The bit clears to 0 after the register has been read.
 * @return Current interrupt status
 * @see MPU6050_RA_INT_STATUS
 * @see MPU6050_INTERRUPT_FIFO_OFLOW_BIT
 */
bool MPU6050_getIntFIFOBufferOverflowStatus(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_FIFO_OFLOW_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get I2C Master interrupt status.
 * This bit automatically sets to 1 when an I2C Master interrupt has been
 * generated. For a list of I2C Master interrupts, please refer to Register 54.
 * The bit clears to 0 after the register has been read.
 * @return Current interrupt status
 * @see MPU6050_RA_INT_STATUS
 * @see MPU6050_INTERRUPT_I2C_MST_INT_BIT
 */
bool MPU6050_getIntI2CMasterStatus(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_I2C_MST_INT_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get Data Ready interrupt status.
 * This bit automatically sets to 1 when a Data Ready interrupt has been
 * generated. The bit clears to 0 after the register has been read.
 * @return Current interrupt status
 * @see MPU6050_RA_INT_STATUS
 * @see MPU6050_INTERRUPT_DATA_RDY_BIT
 */
bool MPU6050_getIntDataReadyStatus(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_DATA_RDY_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}

// ACCEL_*OUT_* registers

/** Get raw 9-axis motion sensor readings (accel/gyro/compass).
 * FUNCTION NOT FULLY IMPLEMENTED YET.
 * @param ax 16-bit signed integer container for accelerometer X-axis value
 * @param ay 16-bit signed integer container for accelerometer Y-axis value
 * @param az 16-bit signed integer container for accelerometer Z-axis value
 * @param gx 16-bit signed integer container for gyroscope X-axis value
 * @param gy 16-bit signed integer container for gyroscope Y-axis value
 * @param gz 16-bit signed integer container for gyroscope Z-axis value
 * @param mx 16-bit signed integer container for magnetometer X-axis value
 * @param my 16-bit signed integer container for magnetometer Y-axis value
 * @param mz 16-bit signed integer container for magnetometer Z-axis value
 * @see getMotion6()
 * @see getAcceleration()
 * @see getRotation()
 * @see MPU6050_RA_ACCEL_XOUT_H
 */
void MPU6050_getMotion9(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz, int16_t* mx, int16_t* my, int16_t* mz) {
    MPU6050_getMotion6(ax, ay, az, gx, gy, gz);
    // TODO: magnetometer integration
}
/** Get raw 6-axis motion sensor readings (accel/gyro).
 * Retrieves all currently available motion sensor values.
 * @param ax 16-bit signed integer container for accelerometer X-axis value
 * @param ay 16-bit signed integer container for accelerometer Y-axis value
 * @param az 16-bit signed integer container for accelerometer Z-axis value
 * @param gx 16-bit signed integer container for gyroscope X-axis value
 * @param gy 16-bit signed integer container for gyroscope Y-axis value
 * @param gz 16-bit signed integer container for gyroscope Z-axis value
 * @see getAcceleration()
 * @see getRotation()
 * @see MPU6050_RA_ACCEL_XOUT_H
 */
void MPU6050_getMotion6(uint16_t* ax, uint16_t* ay, uint16_t* az, uint16_t* gx, uint16_t* gy, uint16_t* gz) {
    MPU6050_setSleepEnabled(0);
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_ACCEL_XOUT_H, 14, mpu6050.buffer);
    *ax = (((uint16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
    *ay = (((uint16_t)mpu6050.buffer[2]) << 8) | mpu6050.buffer[3];
    *az = (((uint16_t)mpu6050.buffer[4]) << 8) | mpu6050.buffer[5];
    *gx = (((uint16_t)mpu6050.buffer[8]) << 8) | mpu6050.buffer[9];
    *gy = (((uint16_t)mpu6050.buffer[10]) << 8) | mpu6050.buffer[11];
    *gz = (((uint16_t)mpu6050.buffer[12]) << 8) | mpu6050.buffer[13];
}
/** Get 3-axis accelerometer readings.
 * These registers store the most recent accelerometer measurements.
 * Accelerometer measurements are written to these registers at the Sample Rate
 * as defined in Register 25.
 *
 * The accelerometer measurement registers, along with the temperature
 * measurement registers, gyroscope measurement registers, and external sensor
 * data registers, are composed of two sets of registers: an internal register
 * set and a user-facing read register set.
 *
 * The data within the accelerometer sensors' internal register set is always
 * updated at the Sample Rate. Meanwhile, the user-facing read register set
 * duplicates the internal register set's data values whenever the serial
 * interface is idle. This guarantees that a burst read of sensor registers will
 * read measurements from the same sampling instant. Note that if burst reads
 * are not used, the user is responsible for ensuring a set of single byte reads
 * correspond to a single sampling instant by checking the Data Ready interrupt.
 *
 * Each 16-bit accelerometer measurement has a full scale defined in ACCEL_FS
 * (Register 28). For each full scale setting, the accelerometers' sensitivity
 * per LSB in ACCEL_xOUT is shown in the table below:
 *
 * <pre>
 * AFS_SEL | Full Scale Range | LSB Sensitivity
 * --------+------------------+----------------
 * 0       | +/- 2g           | 8192 LSB/mg
 * 1       | +/- 4g           | 4096 LSB/mg
 * 2       | +/- 8g           | 2048 LSB/mg
 * 3       | +/- 16g          | 1024 LSB/mg
 * </pre>
 *
 * @param x 16-bit signed integer container for X-axis acceleration
 * @param y 16-bit signed integer container for Y-axis acceleration
 * @param z 16-bit signed integer container for Z-axis acceleration
 * @see MPU6050_RA_GYRO_XOUT_H
 */
void MPU6050_getAcceleration(int16_t* x, int16_t* y, int16_t* z) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_ACCEL_XOUT_H, 6, mpu6050.buffer);
    *x = (((int16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
    *y = (((int16_t)mpu6050.buffer[2]) << 8) | mpu6050.buffer[3];
    *z = (((int16_t)mpu6050.buffer[4]) << 8) | mpu6050.buffer[5];
}
/** Get X-axis accelerometer reading.
 * @return X-axis acceleration measurement in 16-bit 2's complement format
 * @see getMotion6()
 * @see MPU6050_RA_ACCEL_XOUT_H
 */
int16_t MPU6050_getAccelerationX(void) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_ACCEL_XOUT_H, 2, mpu6050.buffer);
    return (((int16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}
/** Get Y-axis accelerometer reading.
 * @return Y-axis acceleration measurement in 16-bit 2's complement format
 * @see getMotion6()
 * @see MPU6050_RA_ACCEL_YOUT_H
 */
int16_t MPU6050_getAccelerationY(void) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_ACCEL_YOUT_H, 2, mpu6050.buffer);
    return (((int16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}
/** Get Z-axis accelerometer reading.
 * @return Z-axis acceleration measurement in 16-bit 2's complement format
 * @see getMotion6()
 * @see MPU6050_RA_ACCEL_ZOUT_H
 */
int16_t MPU6050_getAccelerationZ(void) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_ACCEL_ZOUT_H, 2, mpu6050.buffer);
    return (((int16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}

// TEMP_OUT_* registers

/** Get current internal temperature.
 * @return Temperature reading in 16-bit 2's complement format
 * @see MPU6050_RA_TEMP_OUT_H
 */
uint16_t MPU6050_getTemperature(void) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_TEMP_OUT_H, 2, mpu6050.buffer);
    return (((uint16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}

// GYRO_*OUT_* registers

/** Get 3-axis gyroscope readings.
 * These gyroscope measurement registers, along with the accelerometer
 * measurement registers, temperature measurement registers, and external sensor
 * data registers, are composed of two sets of registers: an internal register
 * set and a user-facing read register set.
 * The data within the gyroscope sensors' internal register set is always
 * updated at the Sample Rate. Meanwhile, the user-facing read register set
 * duplicates the internal register set's data values whenever the serial
 * interface is idle. This guarantees that a burst read of sensor registers will
 * read measurements from the same sampling instant. Note that if burst reads
 * are not used, the user is responsible for ensuring a set of single byte reads
 * correspond to a single sampling instant by checking the Data Ready interrupt.
 *
 * Each 16-bit gyroscope measurement has a full scale defined in FS_SEL
 * (Register 27). For each full scale setting, the gyroscopes' sensitivity per
 * LSB in GYRO_xOUT is shown in the table below:
 *
 * <pre>
 * FS_SEL | Full Scale Range   | LSB Sensitivity
 * -------+--------------------+----------------
 * 0      | +/- 250 degrees/s  | 131 LSB/deg/s
 * 1      | +/- 500 degrees/s  | 65.5 LSB/deg/s
 * 2      | +/- 1000 degrees/s | 32.8 LSB/deg/s
 * 3      | +/- 2000 degrees/s | 16.4 LSB/deg/s
 * </pre>
 *
 * @param x 16-bit signed integer container for X-axis rotation
 * @param y 16-bit signed integer container for Y-axis rotation
 * @param z 16-bit signed integer container for Z-axis rotation
 * @see getMotion6()
 * @see MPU6050_RA_GYRO_XOUT_H
 */
void MPU6050_getRotation(int16_t* x, int16_t* y, int16_t* z) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_GYRO_XOUT_H, 6, mpu6050.buffer);
    *x = (((int16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
    *y = (((int16_t)mpu6050.buffer[2]) << 8) | mpu6050.buffer[3];
    *z = (((int16_t)mpu6050.buffer[4]) << 8) | mpu6050.buffer[5];
}
/** Get X-axis gyroscope reading.
 * @return X-axis rotation measurement in 16-bit 2's complement format
 * @see getMotion6()
 * @see MPU6050_RA_GYRO_XOUT_H
 */
int16_t MPU6050_getRotationX(void) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_GYRO_XOUT_H, 2, mpu6050.buffer);
    return (((int16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}
/** Get Y-axis gyroscope reading.
 * @return Y-axis rotation measurement in 16-bit 2's complement format
 * @see getMotion6()
 * @see MPU6050_RA_GYRO_YOUT_H
 */
int16_t MPU6050_getRotationY(void) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_GYRO_YOUT_H, 2, mpu6050.buffer);
    return (((int16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}
/** Get Z-axis gyroscope reading.
 * @return Z-axis rotation measurement in 16-bit 2's complement format
 * @see getMotion6()
 * @see MPU6050_RA_GYRO_ZOUT_H
 */
int16_t MPU6050_getRotationZ(void) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_GYRO_ZOUT_H, 2, mpu6050.buffer);
    return (((int16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}

// EXT_SENS_DATA_* registers

/** Read single byte from external sensor data register.
 * These registers store data read from external sensors by the Slave 0, 1, 2,
 * and 3 on the auxiliary I2C interface. Data read by Slave 4 is stored in
 * I2C_SLV4_DI (Register 53).
 *
 * External sensor data is written to these registers at the Sample Rate as
 * defined in Register 25. This access rate can be reduced by using the Slave
 * Delay Enable registers (Register 103).
 *
 * External sensor data registers, along with the gyroscope measurement
 * registers, accelerometer measurement registers, and temperature measurement
 * registers, are composed of two sets of registers: an internal register set
 * and a user-facing read register set.
 *
 * The data within the external sensors' internal register set is always updated
 * at the Sample Rate (or the reduced access rate) whenever the serial interface
 * is idle. This guarantees that a burst read of sensor registers will read
 * measurements from the same sampling instant. Note that if burst reads are not
 * used, the user is responsible for ensuring a set of single byte reads
 * correspond to a single sampling instant by checking the Data Ready interrupt.
 *
 * Data is placed in these external sensor data registers according to
 * I2C_SLV0_CTRL, I2C_SLV1_CTRL, I2C_SLV2_CTRL, and I2C_SLV3_CTRL (Registers 39,
 * 42, 45, and 48). When more than zero bytes are read (I2C_SLVx_LEN > 0) from
 * an enabled slave (I2C_SLVx_EN = 1), the slave is read at the Sample Rate (as
 * defined in Register 25) or delayed rate (if specified in Register 52 and
 * 103). During each Sample cycle, slave reads are performed in order of Slave
 * number. If all slaves are enabled with more than zero bytes to be read, the
 * order will be Slave 0, followed by Slave 1, Slave 2, and Slave 3.
 *
 * Each enabled slave will have EXT_SENS_DATA registers associated with it by
 * number of bytes read (I2C_SLVx_LEN) in order of slave number, starting from
 * EXT_SENS_DATA_00. Note that this means enabling or disabling a slave may
 * change the higher numbered slaves' associated registers. Furthermore, if
 * fewer total bytes are being read from the external sensors as a result of
 * such a change, then the data remaining in the registers which no longer have
 * an associated slave device (i.e. high numbered registers) will remain in
 * these previously allocated registers unless reset.
 *
 * If the sum of the read lengths of all SLVx transactions exceed the number of
 * available EXT_SENS_DATA registers, the excess bytes will be dropped. There
 * are 24 EXT_SENS_DATA registers and hence the total read lengths between all
 * the slaves cannot be greater than 24 or some bytes will be lost.
 *
 * Note: Slave 4's behavior is distinct from that of Slaves 0-3. For further
 * information regarding the characteristics of Slave 4, please refer to
 * Registers 49 to 53.
 *
 * EXAMPLE:
 * Suppose that Slave 0 is enabled with 4 bytes to be read (I2C_SLV0_EN = 1 and
 * I2C_SLV0_LEN = 4) while Slave 1 is enabled with 2 bytes to be read so that
 * I2C_SLV1_EN = 1 and I2C_SLV1_LEN = 2. In such a situation, EXT_SENS_DATA _00
 * through _03 will be associated with Slave 0, while EXT_SENS_DATA _04 and 05
 * will be associated with Slave 1. If Slave 2 is enabled as well, registers
 * starting from EXT_SENS_DATA_06 will be allocated to Slave 2.
 *
 * If Slave 2 is disabled while Slave 3 is enabled in this same situation, then
 * registers starting from EXT_SENS_DATA_06 will be allocated to Slave 3
 * instead.
 *
 * REGISTER ALLOCATION FOR DYNAMIC DISABLE VS. NORMAL DISABLE:
 * If a slave is disabled at any time, the space initially allocated to the
 * slave in the EXT_SENS_DATA register, will remain associated with that slave.
 * This is to avoid dynamic adjustment of the register allocation.
 *
 * The allocation of the EXT_SENS_DATA registers is recomputed only when (1) all
 * slaves are disabled, or (2) the I2C_MST_RST bit is set (Register 106).
 *
 * This above is also true if one of the slaves gets NACKed and stops
 * functioning.
 *
 * @param position Starting position (0-23)
 * @return Byte read from register
 */
uint8_t MPU6050_getExternalSensorByte(int position) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_EXT_SENS_DATA_00 + position, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Read word (2 bytes) from external sensor data registers.
 * @param position Starting position (0-21)
 * @return Word read from register
 * @see getExternalSensorByte()
 */
uint16_t MPU6050_getExternalSensorWord(int position) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_EXT_SENS_DATA_00 + position, 2, mpu6050.buffer);
    return (((uint16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}
/** Read double word (4 bytes) from external sensor data registers.
 * @param position Starting position (0-20)
 * @return Double word read from registers
 * @see getExternalSensorByte()
 */
uint32_t MPU6050_getExternalSensorDWord(int position) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_EXT_SENS_DATA_00 + position, 4, mpu6050.buffer);
    return (((uint32_t)mpu6050.buffer[0]) << 24) | (((uint32_t)mpu6050.buffer[1]) << 16) | (((uint16_t)mpu6050.buffer[2]) << 8) | mpu6050.buffer[3];
}

// MOT_DETECT_STATUS register

/** Get X-axis negative motion detection interrupt status.
 * @return Motion detection status
 * @see MPU6050_RA_MOT_DETECT_STATUS
 * @see MPU6050_MOTION_MOT_XNEG_BIT
 */
bool MPU6050_getXNegMotionDetected(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_MOT_DETECT_STATUS, MPU6050_MOTION_MOT_XNEG_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get X-axis positive motion detection interrupt status.
 * @return Motion detection status
 * @see MPU6050_RA_MOT_DETECT_STATUS
 * @see MPU6050_MOTION_MOT_XPOS_BIT
 */
bool MPU6050_getXPosMotionDetected(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_MOT_DETECT_STATUS, MPU6050_MOTION_MOT_XPOS_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get Y-axis negative motion detection interrupt status.
 * @return Motion detection status
 * @see MPU6050_RA_MOT_DETECT_STATUS
 * @see MPU6050_MOTION_MOT_YNEG_BIT
 */
bool MPU6050_getYNegMotionDetected(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_MOT_DETECT_STATUS, MPU6050_MOTION_MOT_YNEG_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get Y-axis positive motion detection interrupt status.
 * @return Motion detection status
 * @see MPU6050_RA_MOT_DETECT_STATUS
 * @see MPU6050_MOTION_MOT_YPOS_BIT
 */
bool MPU6050_getYPosMotionDetected(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_MOT_DETECT_STATUS, MPU6050_MOTION_MOT_YPOS_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get Z-axis negative motion detection interrupt status.
 * @return Motion detection status
 * @see MPU6050_RA_MOT_DETECT_STATUS
 * @see MPU6050_MOTION_MOT_ZNEG_BIT
 */
bool MPU6050_getZNegMotionDetected(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_MOT_DETECT_STATUS, MPU6050_MOTION_MOT_ZNEG_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get Z-axis positive motion detection interrupt status.
 * @return Motion detection status
 * @see MPU6050_RA_MOT_DETECT_STATUS
 * @see MPU6050_MOTION_MOT_ZPOS_BIT
 */
bool MPU6050_getZPosMotionDetected(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_MOT_DETECT_STATUS, MPU6050_MOTION_MOT_ZPOS_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Get zero motion detection interrupt status.
 * @return Motion detection status
 * @see MPU6050_RA_MOT_DETECT_STATUS
 * @see MPU6050_MOTION_MOT_ZRMOT_BIT
 */
bool MPU6050_getZeroMotionDetected(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_MOT_DETECT_STATUS, MPU6050_MOTION_MOT_ZRMOT_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}

// I2C_SLV*_DO register

/** Write byte to Data Output container for specified slave.
 * This register holds the output data written into Slave when Slave is set to
 * write mode. For further information regarding Slave control, please
 * refer to Registers 37 to 39 and immediately following.
 * @param num Slave number (0-3)
 * @param data Byte to write
 * @see MPU6050_RA_I2C_SLV0_DO
 */
void MPU6050_setSlaveOutputByte(uint8_t num, uint8_t data) {
    if (num > 3) return;
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_I2C_SLV0_DO + num, data);
}

// I2C_MST_DELAY_CTRL register

/** Get external data shadow delay enabled status.
 * This register is used to specify the timing of external sensor data
 * shadowing. When DELAY_ES_SHADOW is set to 1, shadowing of external
 * sensor data is delayed until all data has been received.
 * @return Current external data shadow delay enabled status.
 * @see MPU6050_RA_I2C_MST_DELAY_CTRL
 * @see MPU6050_DELAYCTRL_DELAY_ES_SHADOW_BIT
 */
bool MPU6050_getExternalShadowDelayEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_DELAY_CTRL, MPU6050_DELAYCTRL_DELAY_ES_SHADOW_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set external data shadow delay enabled status.
 * @param enabled New external data shadow delay enabled status.
 * @see getExternalShadowDelayEnabled()
 * @see MPU6050_RA_I2C_MST_DELAY_CTRL
 * @see MPU6050_DELAYCTRL_DELAY_ES_SHADOW_BIT
 */
void MPU6050_setExternalShadowDelayEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_DELAY_CTRL, MPU6050_DELAYCTRL_DELAY_ES_SHADOW_BIT, enabled);
}
/** Get slave delay enabled status.
 * When a particular slave delay is enabled, the rate of access for the that
 * slave device is reduced. When a slave's access rate is decreased relative to
 * the Sample Rate, the slave is accessed every:
 *
 *     1 / (1 + I2C_MST_DLY) Samples
 *
 * This base Sample Rate in turn is determined by SMPLRT_DIV (register  * 25)
 * and DLPF_CFG (register 26).
 *
 * For further information regarding I2C_MST_DLY, please refer to register 52.
 * For further information regarding the Sample Rate, please refer to register 25.
 *
 * @param num Slave number (0-4)
 * @return Current slave delay enabled status.
 * @see MPU6050_RA_I2C_MST_DELAY_CTRL
 * @see MPU6050_DELAYCTRL_I2C_SLV0_DLY_EN_BIT
 */
bool MPU6050_getSlaveDelayEnabled(uint8_t num) {
    // MPU6050_DELAYCTRL_I2C_SLV4_DLY_EN_BIT is 4, SLV3 is 3, etc.
    if (num > 4) return 0;
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_DELAY_CTRL, num, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set slave delay enabled status.
 * @param num Slave number (0-4)
 * @param enabled New slave delay enabled status.
 * @see MPU6050_RA_I2C_MST_DELAY_CTRL
 * @see MPU6050_DELAYCTRL_I2C_SLV0_DLY_EN_BIT
 */
void MPU6050_setSlaveDelayEnabled(uint8_t num, bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_I2C_MST_DELAY_CTRL, num, enabled);
}

// SIGNAL_PATH_RESET register

/** Reset gyroscope signal path.
 * The reset will revert the signal path analog to digital converters and
 * filters to their power up configurations.
 * @see MPU6050_RA_SIGNAL_PATH_RESET
 * @see MPU6050_PATHRESET_GYRO_RESET_BIT
 */
void MPU6050_resetGyroscopePath(void) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_SIGNAL_PATH_RESET, MPU6050_PATHRESET_GYRO_RESET_BIT, true);
}
/** Reset accelerometer signal path.
 * The reset will revert the signal path analog to digital converters and
 * filters to their power up configurations.
 * @see MPU6050_RA_SIGNAL_PATH_RESET
 * @see MPU6050_PATHRESET_ACCEL_RESET_BIT
 */
void MPU6050_resetAccelerometerPath(void) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_SIGNAL_PATH_RESET, MPU6050_PATHRESET_ACCEL_RESET_BIT, true);
}
/** Reset temperature sensor signal path.
 * The reset will revert the signal path analog to digital converters and
 * filters to their power up configurations.
 * @see MPU6050_RA_SIGNAL_PATH_RESET
 * @see MPU6050_PATHRESET_TEMP_RESET_BIT
 */
void MPU6050_resetTemperaturePath(void) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_SIGNAL_PATH_RESET, MPU6050_PATHRESET_TEMP_RESET_BIT, true);
}

// MOT_DETECT_CTRL register

/** Get accelerometer power-on delay.
 * The accelerometer data path provides samples to the sensor registers, Motion
 * detection, Zero Motion detection, and Free Fall detection modules. The
 * signal path contains filters which must be flushed on wake-up with new
 * samples before the detection modules begin operations. The default wake-up
 * delay, of 4ms can be lengthened by up to 3ms. This additional delay is
 * specified in ACCEL_ON_DELAY in units of 1 LSB = 1 ms. The user may select
 * any value above zero unless instructed otherwise by InvenSense. Please refer
 * to Section 8 of the MPU-6000/MPU-6050 Product Specification document for
 * further information regarding the detection modules.
 * @return Current accelerometer power-on delay
 * @see MPU6050_RA_MOT_DETECT_CTRL
 * @see MPU6050_DETECT_ACCEL_ON_DELAY_BIT
 */
uint8_t MPU6050_getAccelerometerPowerOnDelay(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_MOT_DETECT_CTRL, MPU6050_DETECT_ACCEL_ON_DELAY_BIT, MPU6050_DETECT_ACCEL_ON_DELAY_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set accelerometer power-on delay.
 * @param delay New accelerometer power-on delay (0-3)
 * @see getAccelerometerPowerOnDelay()
 * @see MPU6050_RA_MOT_DETECT_CTRL
 * @see MPU6050_DETECT_ACCEL_ON_DELAY_BIT
 */
void MPU6050_setAccelerometerPowerOnDelay(uint8_t delay) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_MOT_DETECT_CTRL, MPU6050_DETECT_ACCEL_ON_DELAY_BIT, MPU6050_DETECT_ACCEL_ON_DELAY_LENGTH, delay);
}
/** Get Free Fall detection counter decrement configuration.
 * Detection is registered by the Free Fall detection module after accelerometer
 * measurements meet their respective threshold conditions over a specified
 * number of samples. When the threshold conditions are met, the corresponding
 * detection counter increments by 1. The user may control the rate at which the
 * detection counter decrements when the threshold condition is not met by
 * configuring FF_COUNT. The decrement rate can be set according to the
 * following table:
 *
 * <pre>
 * FF_COUNT | Counter Decrement
 * ---------+------------------
 * 0        | Reset
 * 1        | 1
 * 2        | 2
 * 3        | 4
 * </pre>
 *
 * When FF_COUNT is configured to 0 (reset), any non-qualifying sample will
 * reset the counter to 0. For further information on Free Fall detection,
 * please refer to Registers 29 to 32.
 *
 * @return Current decrement configuration
 * @see MPU6050_RA_MOT_DETECT_CTRL
 * @see MPU6050_DETECT_FF_COUNT_BIT
 */
uint8_t MPU6050_getFreefallDetectionCounterDecrement(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_MOT_DETECT_CTRL, MPU6050_DETECT_FF_COUNT_BIT, MPU6050_DETECT_FF_COUNT_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Free Fall detection counter decrement configuration.
 * @param decrement New decrement configuration value
 * @see getFreefallDetectionCounterDecrement()
 * @see MPU6050_RA_MOT_DETECT_CTRL
 * @see MPU6050_DETECT_FF_COUNT_BIT
 */
void MPU6050_setFreefallDetectionCounterDecrement(uint8_t decrement) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_MOT_DETECT_CTRL, MPU6050_DETECT_FF_COUNT_BIT, MPU6050_DETECT_FF_COUNT_LENGTH, decrement);
}
/** Get Motion detection counter decrement configuration.
 * Detection is registered by the Motion detection module after accelerometer
 * measurements meet their respective threshold conditions over a specified
 * number of samples. When the threshold conditions are met, the corresponding
 * detection counter increments by 1. The user may control the rate at which the
 * detection counter decrements when the threshold condition is not met by
 * configuring MOT_COUNT. The decrement rate can be set according to the
 * following table:
 *
 * <pre>
 * MOT_COUNT | Counter Decrement
 * ----------+------------------
 * 0         | Reset
 * 1         | 1
 * 2         | 2
 * 3         | 4
 * </pre>
 *
 * When MOT_COUNT is configured to 0 (reset), any non-qualifying sample will
 * reset the counter to 0. For further information on Motion detection,
 * please refer to Registers 29 to 32.
 *
 */
uint8_t MPU6050_getMotionDetectionCounterDecrement(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_MOT_DETECT_CTRL, MPU6050_DETECT_MOT_COUNT_BIT, MPU6050_DETECT_MOT_COUNT_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Motion detection counter decrement configuration.
 * @param decrement New decrement configuration value
 * @see getMotionDetectionCounterDecrement()
 * @see MPU6050_RA_MOT_DETECT_CTRL
 * @see MPU6050_DETECT_MOT_COUNT_BIT
 */
void MPU6050_setMotionDetectionCounterDecrement(uint8_t decrement) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_MOT_DETECT_CTRL, MPU6050_DETECT_MOT_COUNT_BIT, MPU6050_DETECT_MOT_COUNT_LENGTH, decrement);
}

// USER_CTRL register

/** Get FIFO enabled status.
 * When this bit is set to 0, the FIFO mpu6050.buffer is disabled. The FIFO mpu6050.buffer
 * cannot be written to or read from while disabled. The FIFO mpu6050.buffer's state
 * does not change unless the MPU-60X0 is power cycled.
 * @return Current FIFO enabled status
 * @see MPU6050_RA_USER_CTRL
 * @see MPU6050_USERCTRL_FIFO_EN_BIT
 */
bool MPU6050_getFIFOEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set FIFO enabled status.
 * @param enabled New FIFO enabled status
 * @see getFIFOEnabled()
 * @see MPU6050_RA_USER_CTRL
 * @see MPU6050_USERCTRL_FIFO_EN_BIT
 */
void MPU6050_setFIFOEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_EN_BIT, enabled);
}
/** Get I2C Master Mode enabled status.
 * When this mode is enabled, the MPU-60X0 acts as the I2C Master to the
 * external sensor slave devices on the auxiliary I2C bus. When this bit is
 * cleared to 0, the auxiliary I2C bus lines (AUX_DA and AUX_CL) are logically
 * driven by the primary I2C bus (SDA and SCL). This is a precondition to
 * enabling Bypass Mode. For further information regarding Bypass Mode, please
 * refer to Register 55.
 * @return Current I2C Master Mode enabled status
 * @see MPU6050_RA_USER_CTRL
 * @see MPU6050_USERCTRL_I2C_MST_EN_BIT
 */
bool MPU6050_getI2CMasterModeEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set I2C Master Mode enabled status.
 * @param enabled New I2C Master Mode enabled status
 * @see getI2CMasterModeEnabled()
 * @see MPU6050_RA_USER_CTRL
 * @see MPU6050_USERCTRL_I2C_MST_EN_BIT
 */
void MPU6050_setI2CMasterModeEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_EN_BIT, enabled);
}
/** Switch from I2C to SPI mode (MPU-6000 only)
 * If this is set, the primary SPI interface will be enabled in place of the
 * disabled primary I2C interface.
 */
void MPU6050_switchSPIEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_IF_DIS_BIT, enabled);
}
/** Reset the FIFO.
 * This bit resets the FIFO mpu6050.buffer when set to 1 while FIFO_EN equals 0. This
 * bit automatically clears to 0 after the reset has been triggered.
 * @see MPU6050_RA_USER_CTRL
 * @see MPU6050_USERCTRL_FIFO_RESET_BIT
 */
void MPU6050_resetFIFO(void) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_RESET_BIT, true);
}
/** Reset the I2C Master.
 * This bit resets the I2C Master when set to 1 while I2C_MST_EN equals 0.
 * This bit automatically clears to 0 after the reset has been triggered.
 * @see MPU6050_RA_USER_CTRL
 * @see MPU6050_USERCTRL_I2C_MST_RESET_BIT
 */
void MPU6050_resetI2CMaster(void) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_RESET_BIT, true);
}
/** Reset all sensor registers and signal paths.
 * When set to 1, this bit resets the signal paths for all sensors (gyroscopes,
 * accelerometers, and temperature sensor). This operation will also clear the
 * sensor registers. This bit automatically clears to 0 after the reset has been
 * triggered.
 *
 * When resetting only the signal path (and not the sensor registers), please
 * use Register 104, SIGNAL_PATH_RESET.
 *
 * @see MPU6050_RA_USER_CTRL
 * @see MPU6050_USERCTRL_SIG_COND_RESET_BIT
 */
void MPU6050_resetSensors(void) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_SIG_COND_RESET_BIT, true);
}

// PWR_MGMT_1 register

/** Trigger a full device reset.
 * A small delay of ~50ms may be desirable after triggering a reset.
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_DEVICE_RESET_BIT
 */
void MPU6050_reset(void) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_DEVICE_RESET_BIT, true);
}
/** Get sleep mode status.
 * Setting the SLEEP bit in the register puts the device into very low power
 * sleep mode. In this mode, only the serial interface and internal registers
 * remain active, allowing for a very low standby current. Clearing this bit
 * puts the device back into normal mode. To save power, the individual standby
 * selections for each of the gyros should be used if any gyro axis is not used
 * by the application.
 * @return Current sleep mode enabled status
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_SLEEP_BIT
 */
bool MPU6050_getSleepEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set sleep mode status.
 * @param enabled New sleep mode enabled status
 * @see getSleepEnabled()
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_SLEEP_BIT
 */
void MPU6050_setSleepEnabled(uint8_t enabled) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_1, enabled);
}
/** Get wake cycle enabled status.
 * When this bit is set to 1 and SLEEP is disabled, the MPU-60X0 will cycle
 * between sleep mode and waking up to take a single sample of data from active
 * sensors at a rate determined by LP_WAKE_CTRL (register 108).
 * @return Current sleep mode enabled status
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_CYCLE_BIT
 */
bool MPU6050_getWakeCycleEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CYCLE_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set wake cycle enabled status.
 * @param enabled New sleep mode enabled status
 * @see getWakeCycleEnabled()
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_CYCLE_BIT
 */
void MPU6050_setWakeCycleEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CYCLE_BIT, enabled);
}
/** Get temperature sensor enabled status.
 * Control the usage of the internal temperature sensor.
 *
 * Note: this register stores the *disabled* value, but for consistency with the
 * rest of the code, the function is named and used with standard true/false
 * values to indicate whether the sensor is enabled or disabled, respectively.
 *
 * @return Current temperature sensor enabled status
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_TEMP_DIS_BIT
 */
bool MPU6050_getTempSensorEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_TEMP_DIS_BIT, mpu6050.buffer);
    return mpu6050.buffer[0] == 0; // 1 is actually disabled here
}
/** Set temperature sensor enabled status.
 * Note: this register stores the *disabled* value, but for consistency with the
 * rest of the code, the function is named and used with standard true/false
 * values to indicate whether the sensor is enabled or disabled, respectively.
 *
 * @param enabled New temperature sensor enabled status
 * @see getTempSensorEnabled()
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_TEMP_DIS_BIT
 */
void MPU6050_setTempSensorEnabled(bool enabled) {
    // 1 is actually disabled here
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_TEMP_DIS_BIT, !enabled);
}
/** Get clock source setting.
 * @return Current clock source setting
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_CLKSEL_BIT
 * @see MPU6050_PWR1_CLKSEL_LENGTH
 */
uint8_t MPU6050_getClockSource(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set clock source setting.
 * An internal 8MHz oscillator, gyroscope based clock, or external sources can
 * be selected as the MPU-60X0 clock source. When the internal 8 MHz oscillator
 * or an external source is chosen as the clock source, the MPU-60X0 can operate
 * in low power modes with the gyroscopes disabled.
 *
 * Upon power up, the MPU-60X0 clock source defaults to the internal oscillator.
 * However, it is highly recommended that the device be configured to use one of
 * the gyroscopes (or an external clock source) as the clock reference for
 * improved stability. The clock source can be selected according to the following table:
 *
 * <pre>
 * CLK_SEL | Clock Source
 * --------+--------------------------------------
 * 0       | Internal oscillator
 * 1       | PLL with X Gyro reference
 * 2       | PLL with Y Gyro reference
 * 3       | PLL with Z Gyro reference
 * 4       | PLL with external 32.768kHz reference
 * 5       | PLL with external 19.2MHz reference
 * 6       | Reserved
 * 7       | Stops the clock and keeps the timing generator in reset
 * </pre>
 *
 * @param source New clock source setting
 * @see getClockSource()
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_CLKSEL_BIT
 * @see MPU6050_PWR1_CLKSEL_LENGTH
 */
void MPU6050_setClockSource(uint8_t source) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, source);
}

// PWR_MGMT_2 register

/** Get wake frequency in Accel-Only Low Power Mode.
 * The MPU-60X0 can be put into Accerlerometer Only Low Power Mode by setting
 * PWRSEL to 1 in the Power Management 1 register (Register 107). In this mode,
 * the device will power off all devices except for the primary I2C interface,
 * waking only the accelerometer at fixed intervals to take a single
 * measurement. The frequency of wake-ups can be configured with LP_WAKE_CTRL
 * as shown below:
 *
 * <pre>
 * LP_WAKE_CTRL | Wake-up Frequency
 * -------------+------------------
 * 0            | 1.25 Hz
 * 1            | 2.5 Hz
 * 2            | 5 Hz
 * 3            | 10 Hz
 * <pre>
 *
 * For further information regarding the MPU-60X0's power modes, please refer to
 * Register 107.
 *
 * @return Current wake frequency
 * @see MPU6050_RA_PWR_MGMT_2
 */
uint8_t MPU6050_getWakeFrequency(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_LP_WAKE_CTRL_BIT, MPU6050_PWR2_LP_WAKE_CTRL_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set wake frequency in Accel-Only Low Power Mode.
 * @param frequency New wake frequency
 * @see MPU6050_RA_PWR_MGMT_2
 */
void MPU6050_setWakeFrequency(uint8_t frequency) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_LP_WAKE_CTRL_BIT, MPU6050_PWR2_LP_WAKE_CTRL_LENGTH, frequency);
}

/** Get X-axis accelerometer standby enabled status.
 * If enabled, the X-axis will not gather or report data (or use power).
 * @return Current X-axis standby enabled status
 * @see MPU6050_RA_PWR_MGMT_2
 * @see MPU6050_PWR2_STBY_XA_BIT
 */
bool MPU6050_getStandbyXAccelEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_XA_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set X-axis accelerometer standby enabled status.
 * @param New X-axis standby enabled status
 * @see getStandbyXAccelEnabled()
 * @see MPU6050_RA_PWR_MGMT_2
 * @see MPU6050_PWR2_STBY_XA_BIT
 */
void MPU6050_setStandbyXAccelEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_XA_BIT, enabled);
}
/** Get Y-axis accelerometer standby enabled status.
 * If enabled, the Y-axis will not gather or report data (or use power).
 * @return Current Y-axis standby enabled status
 * @see MPU6050_RA_PWR_MGMT_2
 * @see MPU6050_PWR2_STBY_YA_BIT
 */
bool MPU6050_getStandbyYAccelEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_YA_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Y-axis accelerometer standby enabled status.
 * @param New Y-axis standby enabled status
 * @see getStandbyYAccelEnabled()
 * @see MPU6050_RA_PWR_MGMT_2
 * @see MPU6050_PWR2_STBY_YA_BIT
 */
void MPU6050_setStandbyYAccelEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_YA_BIT, enabled);
}
/** Get Z-axis accelerometer standby enabled status.
 * If enabled, the Z-axis will not gather or report data (or use power).
 * @return Current Z-axis standby enabled status
 * @see MPU6050_RA_PWR_MGMT_2
 * @see MPU6050_PWR2_STBY_ZA_BIT
 */
bool MPU6050_getStandbyZAccelEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_ZA_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Z-axis accelerometer standby enabled status.
 * @param New Z-axis standby enabled status
 * @see getStandbyZAccelEnabled()
 * @see MPU6050_RA_PWR_MGMT_2
 * @see MPU6050_PWR2_STBY_ZA_BIT
 */
void MPU6050_setStandbyZAccelEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_ZA_BIT, enabled);
}
/** Get X-axis gyroscope standby enabled status.
 * If enabled, the X-axis will not gather or report data (or use power).
 * @return Current X-axis standby enabled status
 * @see MPU6050_RA_PWR_MGMT_2
 * @see MPU6050_PWR2_STBY_XG_BIT
 */
bool MPU6050_getStandbyXGyroEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_XG_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set X-axis gyroscope standby enabled status.
 * @param New X-axis standby enabled status
 * @see getStandbyXGyroEnabled()
 * @see MPU6050_RA_PWR_MGMT_2
 * @see MPU6050_PWR2_STBY_XG_BIT
 */
void MPU6050_setStandbyXGyroEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_XG_BIT, enabled);
}
/** Get Y-axis gyroscope standby enabled status.
 * If enabled, the Y-axis will not gather or report data (or use power).
 * @return Current Y-axis standby enabled status
 * @see MPU6050_RA_PWR_MGMT_2
 * @see MPU6050_PWR2_STBY_YG_BIT
 */
bool MPU6050_getStandbyYGyroEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_YG_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Y-axis gyroscope standby enabled status.
 * @param New Y-axis standby enabled status
 * @see getStandbyYGyroEnabled()
 * @see MPU6050_RA_PWR_MGMT_2
 * @see MPU6050_PWR2_STBY_YG_BIT
 */
void MPU6050_setStandbyYGyroEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_YG_BIT, enabled);
}
/** Get Z-axis gyroscope standby enabled status.
 * If enabled, the Z-axis will not gather or report data (or use power).
 * @return Current Z-axis standby enabled status
 * @see MPU6050_RA_PWR_MGMT_2
 * @see MPU6050_PWR2_STBY_ZG_BIT
 */
bool MPU6050_getStandbyZGyroEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_ZG_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Z-axis gyroscope standby enabled status.
 * @param New Z-axis standby enabled status
 * @see getStandbyZGyroEnabled()
 * @see MPU6050_RA_PWR_MGMT_2
 * @see MPU6050_PWR2_STBY_ZG_BIT
 */
void MPU6050_setStandbyZGyroEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_PWR_MGMT_2, MPU6050_PWR2_STBY_ZG_BIT, enabled);
}

// FIFO_COUNT* registers

/** Get current FIFO mpu6050.buffer size.
 * This value indicates the number of bytes stored in the FIFO mpu6050.buffer. This
 * number is in turn the number of bytes that can be read from the FIFO mpu6050.buffer
 * and it is directly proportional to the number of samples available given the
 * set of sensor data bound to be stored in the FIFO (register 35 and 36).
 * @return Current FIFO mpu6050.buffer size
 */
uint16_t MPU6050_getFIFOCount(void) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_FIFO_COUNTH, 2, mpu6050.buffer);
    return (((uint16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}

// FIFO_R_W register

/** Get byte from FIFO mpu6050.buffer.
 * This register is used to read and write data from the FIFO mpu6050.buffer. Data is
 * written to the FIFO in order of register number (from lowest to highest). If
 * all the FIFO enable flags (see below) are enabled and all External Sensor
 * Data registers (Registers 73 to 96) are associated with a Slave device, the
 * contents of registers 59 through 96 will be written in order at the Sample
 * Rate.
 *
 * The contents of the sensor data registers (Registers 59 to 96) are written
 * into the FIFO mpu6050.buffer when their corresponding FIFO enable flags are set to 1
 * in FIFO_EN (Register 35). An additional flag for the sensor data registers
 * associated with I2C Slave 3 can be found in I2C_MST_CTRL (Register 36).
 *
 * If the FIFO mpu6050.buffer has overflowed, the status bit FIFO_OFLOW_INT is
 * automatically set to 1. This bit is located in INT_STATUS (Register 58).
 * When the FIFO mpu6050.buffer has overflowed, the oldest data will be lost and new
 * data will be written to the FIFO.
 *
 * If the FIFO mpu6050.buffer is empty, reading this register will return the last byte
 * that was previously read from the FIFO until new data is available. The user
 * should check FIFO_COUNT to ensure that the FIFO mpu6050.buffer is not read when
 * empty.
 *
 * @return Byte from FIFO mpu6050.buffer
 */
uint8_t MPU6050_getFIFOByte(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_FIFO_R_W, mpu6050.buffer);
    return mpu6050.buffer[0];
}
void MPU6050_getFIFOBytes(uint8_t *data, uint8_t length) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_FIFO_R_W, length, data);
}
/** Write byte to FIFO mpu6050.buffer.
 * @see getFIFOByte()
 * @see MPU6050_RA_FIFO_R_W
 */
void MPU6050_setFIFOByte(uint8_t data) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_FIFO_R_W, data);
}

// WHO_AM_I register

/** Get Device ID.
 * This register is used to verify the identity of the device (0b110100, 0x34).
 * @return Device ID (6 bits only! should be 0x34)
 * @see MPU6050_RA_WHO_AM_I
 * @see MPU6050_WHO_AM_I_BIT
 * @see MPU6050_WHO_AM_I_LENGTH
 */
uint8_t MPU6050_getDeviceID(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
/** Set Device ID.
 * Write a new ID into the WHO_AM_I register (no idea why this should ever be
 * necessary though).
 * @param id New device ID to set.
 * @see getDeviceID()
 * @see MPU6050_RA_WHO_AM_I
 * @see MPU6050_WHO_AM_I_BIT
 * @see MPU6050_WHO_AM_I_LENGTH
 */
void MPU6050_setDeviceID(uint8_t id) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, id);
}

// ======== UNDOCUMENTED/DMP REGISTERS/METHODS ========

// XG_OFFS_TC register

uint8_t MPU6050_getOTPBankValid(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OTP_BNK_VLD_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
void MPU6050_setOTPBankValid(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OTP_BNK_VLD_BIT, enabled);
}
int8_t MPU6050_getXGyroOffsetTC(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
void MPU6050_setXGyroOffsetTC(int8_t offset) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}

// YG_OFFS_TC register

int8_t MPU6050_getYGyroOffsetTC(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_YG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
void MPU6050_setYGyroOffsetTC(int8_t offset) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_YG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}

// ZG_OFFS_TC register

int8_t MPU6050_getZGyroOffsetTC(void) {
    MPU6050_readBits(mpu6050.devAddr, MPU6050_RA_ZG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, mpu6050.buffer);
    return mpu6050.buffer[0];
}
void MPU6050_setZGyroOffsetTC(int8_t offset) {
    MPU6050_writeBits(mpu6050.devAddr, MPU6050_RA_ZG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}

// X_FINE_GAIN register

int8_t MPU6050_getXFineGain(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_X_FINE_GAIN, mpu6050.buffer);
    return mpu6050.buffer[0];
}
void MPU6050_setXFineGain(int8_t gain) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_X_FINE_GAIN, gain);
}

// Y_FINE_GAIN register

int8_t MPU6050_getYFineGain(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_Y_FINE_GAIN, mpu6050.buffer);
    return mpu6050.buffer[0];
}
void MPU6050_setYFineGain(int8_t gain) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_Y_FINE_GAIN, gain);
}

// Z_FINE_GAIN register

int8_t MPU6050_getZFineGain(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_Z_FINE_GAIN, mpu6050.buffer);
    return mpu6050.buffer[0];
}
void MPU6050_setZFineGain(int8_t gain) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_Z_FINE_GAIN, gain);
}

// XA_OFFS_* registers

int16_t MPU6050_getXAccelOffset(void) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_XA_OFFS_H, 2, mpu6050.buffer);
    return (((int16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}
void MPU6050_setXAccelOffset(int16_t offset) {
    MPU6050_writeWord(mpu6050.devAddr, MPU6050_RA_XA_OFFS_H, offset);
}

// YA_OFFS_* register

int16_t MPU6050_getYAccelOffset(void) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_YA_OFFS_H, 2, mpu6050.buffer);
    return (((int16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}
void MPU6050_setYAccelOffset(int16_t offset) {
    MPU6050_writeWord(mpu6050.devAddr, MPU6050_RA_YA_OFFS_H, offset);
}

// ZA_OFFS_* register

int16_t MPU6050_getZAccelOffset(void) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_ZA_OFFS_H, 2, mpu6050.buffer);
    return (((int16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}
void MPU6050_setZAccelOffset(int16_t offset) {
    MPU6050_writeWord(mpu6050.devAddr, MPU6050_RA_ZA_OFFS_H, offset);
}

// XG_OFFS_USR* registers

int16_t MPU6050_getXGyroOffset(void) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_XG_OFFS_USRH, 2, mpu6050.buffer);
    return (((int16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}
void MPU6050_setXGyroOffset(int16_t offset) {
    MPU6050_writeWord(mpu6050.devAddr, MPU6050_RA_XG_OFFS_USRH, offset);
}

// YG_OFFS_USR* register

int16_t MPU6050_getYGyroOffset(void) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_YG_OFFS_USRH, 2, mpu6050.buffer);
    return (((int16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}
void MPU6050_setYGyroOffset(int16_t offset) {
    MPU6050_writeWord(mpu6050.devAddr, MPU6050_RA_YG_OFFS_USRH, offset);
}

// ZG_OFFS_USR* register

int16_t MPU6050_getZGyroOffset(void) {
    MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_ZG_OFFS_USRH, 2, mpu6050.buffer);
    return (((int16_t)mpu6050.buffer[0]) << 8) | mpu6050.buffer[1];
}
void MPU6050_setZGyroOffset(int16_t offset) {
    MPU6050_writeWord(mpu6050.devAddr, MPU6050_RA_ZG_OFFS_USRH, offset);
}

// INT_ENABLE register (DMP functions)

bool MPU6050_getIntPLLReadyEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_PLL_RDY_INT_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
void MPU6050_setIntPLLReadyEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_PLL_RDY_INT_BIT, enabled);
}
bool MPU6050_getIntDMPEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_DMP_INT_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
void MPU6050_setIntDMPEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_DMP_INT_BIT, enabled);
}

// DMP_INT_STATUS

bool MPU6050_getDMPInt5Status(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_DMP_INT_STATUS, MPU6050_DMPINT_5_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
bool MPU6050_getDMPInt4Status(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_DMP_INT_STATUS, MPU6050_DMPINT_4_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
bool MPU6050_getDMPInt3Status(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_DMP_INT_STATUS, MPU6050_DMPINT_3_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
bool MPU6050_getDMPInt2Status(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_DMP_INT_STATUS, MPU6050_DMPINT_2_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
bool MPU6050_getDMPInt1Status(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_DMP_INT_STATUS, MPU6050_DMPINT_1_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
bool MPU6050_getDMPInt0Status(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_DMP_INT_STATUS, MPU6050_DMPINT_0_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}

// INT_STATUS register (DMP functions)

bool MPU6050_getIntPLLReadyStatus(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_PLL_RDY_INT_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
bool MPU6050_getIntDMPStatus(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_INT_STATUS, MPU6050_INTERRUPT_DMP_INT_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}

// USER_CTRL register (DMP functions)

bool MPU6050_getDMPEnabled(void) {
    MPU6050_readBit(mpu6050.devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_EN_BIT, mpu6050.buffer);
    return mpu6050.buffer[0];
}
void MPU6050_setDMPEnabled(bool enabled) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_EN_BIT, enabled);
}
void MPU6050_resetDMP(void) {
    MPU6050_writeBit(mpu6050.devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_RESET_BIT, true);
}

// BANK_SEL register

void MPU6050_setMemoryBank(uint8_t bank, bool prefetchEnabled, bool userBank) {
    bank &= 0x1F;
    if (userBank) bank |= 0x20;
    if (prefetchEnabled) bank |= 0x40;
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_BANK_SEL, bank);
}

// MEM_START_ADDR register

void MPU6050_setMemoryStartAddress(uint8_t address) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_MEM_START_ADDR, address);
}

// MEM_R_W register

uint8_t MPU6050_readMemoryByte(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_MEM_R_W, mpu6050.buffer);
    return mpu6050.buffer[0];
}
void MPU6050_writeMemoryByte(uint8_t data) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_MEM_R_W, data);
}
void MPU6050_readMemoryBlock(uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address) {
    uint8_t chunkSize;
    unsigned int i;
    MPU6050_setMemoryBank(bank, false, false);
    MPU6050_setMemoryStartAddress(address);
    for (i = 0; i < dataSize;) {
        // determine correct chunk size according to bank position and data size
        chunkSize = MPU6050_DMP_MEMORY_CHUNK_SIZE;

        // make sure we don't go past the data size
        if (i + chunkSize > dataSize) chunkSize = dataSize - i;

        // make sure this chunk doesn't go past the bank boundary (256 bytes)
        if (chunkSize > 256 - address) chunkSize = 256 - address;

        // read the chunk of data as specified
        MPU6050_readBytes(mpu6050.devAddr, MPU6050_RA_MEM_R_W, chunkSize, data + i);
        
        // increase byte index by [chunkSize]
        i += chunkSize;

        // uint8_t automatically wraps to 0 at 256
        address += chunkSize;

        // if we aren't done, update bank (if necessary) and address
        if (i < dataSize) {
            if (address == 0) bank++;
            MPU6050_setMemoryBank(bank, false, false);
            MPU6050_setMemoryStartAddress(address);
        }
    }
}

// DMP_CFG_1 register

uint8_t MPU6050_getDMPConfig1(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_DMP_CFG_1, mpu6050.buffer);
    return mpu6050.buffer[0];
}
void MPU6050_setDMPConfig1(uint8_t config) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_DMP_CFG_1, config);
}

// DMP_CFG_2 register

uint8_t MPU6050_getDMPConfig2(void) {
    MPU6050_readByte(mpu6050.devAddr, MPU6050_RA_DMP_CFG_2, mpu6050.buffer);
    return mpu6050.buffer[0];
}
void MPU6050_setDMPConfig2(uint8_t config) {
    MPU6050_writeByte(mpu6050.devAddr, MPU6050_RA_DMP_CFG_2, config);
}