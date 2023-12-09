#ifndef _I2C_H
#define _I2C_H

#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_clk.h>
#include <linux/clk.h>

#define DRIVER_NAME "i2c_lliano"

int i2c_init(struct platform_device *pdev);
void i2c_deinit(void);
int i2c_write(char slave_address, char* data, char size);
int i2c_read(char slave_address, char* read_buff, char size);
int i2c_read_reg(char slave_address, char reg_address, char* read_buff);

#define TIMEOUT_READ_WRITE 100  // msec

#define DT_PROPERTY_PINMUX_PHANDLE  "pinmux"
#define DT_PROPERTY_PINS            "pins"
#define DT_PROPERTY_CLK_PHANDLE     "clocks"
#define DT_PROPERTY_CLK_FREQ        "clock-frequency"
#define DT_PROPERTY_INT_CLK_FREQ    "int-clock-frequency"
#define DT_PROPERTY_BIT_RATE        "bit-rate"

// Clock Module Peripheral (CM_PER) (page 179) Length = 1K-> 0x400
#define CM_PER                  0x44E00000
#define CM_PER_LEN              0x00000400 // 1K

// I2C2 clocks manager (page 1270)
#define IDCM_PER_I2C2_CLKCTRL      0x00000044
#define CM_PER_I2C2_CLKCTRL_MASK   0x00030003
#define CM_PER_I2C2_CLKCTRL_ENABLE 0x00000002

// Config de Control Module (page180)
#define CTRL_MODULE_BASE        0x44E10000
#define CTRL_MODULE_LEN         0x00002000 // 8K la datasheet dice 128k pero el fin es en @x44E11FFF 0x00000978 //pin 20 = sda (page1461)
#define CTRL_MODULE_UART1_CTSN  0x00000978 //pin 20 = sda (page1461)  
#define CTRL_MODULE_UART1_RTSN  0x0000097C //pin 19 = scl (page1461) 
#define CTRL_MODULE_UART1_MASK  0x0000007F

// I2C2 Module (page183)
#define I2C2                    0x4819C000
#define I2C2_LEN                0x00001000

// Registers' offset address
#define I2C_REG_REVNB_LO        0x00
#define I2C_REG_REVNB_HI        0x04
#define I2C_REG_SYSC            0x10
#define I2C_REG_IRQSTATUS_RAW   0x28
#define I2C_REG_IRQSTATUS       0x28
#define I2C_REG_IRQENABLE_SET   0x2C
#define I2C_REG_IRQENABLE_CLR   0x30
#define I2C_REG_WE              0x34
#define I2C_REG_SYSS            0x90
#define I2C_REG_BUF             0x94
#define I2C_REG_CNT             0x98
#define I2C_REG_DATA            0x9C
#define I2C_REG_CON             0xA4
#define I2C_REG_OA              0xA8
#define I2C_REG_SA              0xAC
#define I2C_REG_PSC             0xB0
#define I2C_REG_SCLL            0xB4
#define I2C_REG_SCLH            0xB8

// CON
#define I2C_BIT_ENABLE          (1 << 15)
#define I2C_BIT_MASTER_MODE     (1 << 10)
#define I2C_BIT_TX              (1 << 9)
#define I2C_BIT_RX              (1 << 9)
#define I2C_BIT_STOP            (1 << 1)
#define I2C_BIT_START           (1 << 0)

// SYSC
#define I2C_BIT_AUTOIDLE        (1 << 0)
#define I2C_BIT_RESET           (1 << 1)
#define I2C_BIT_WAKEUP          (1 << 2)    // Enable own wakeup
#define I2C_BIT_NOIDLE          (1 << 3)    // No idle
#define I2C_BIT_CLKACTIVITY     (3 << 8)    // Both clocks active

// IRQSTATUS
#define I2C_IRQ_BB              (1 << 12)
#define I2C_IRQ_XRDY            (1 << 4)
#define I2C_IRQ_RRDY            (1 << 3)
#define I2C_IRQ_ARDY            (1 << 2)
#define I2C_IRQ_NACK            (1 << 1)
#define I2C_IRQ_AL              (1 << 0)

#define I2C_IRQSTATUS_CLR_ALL   0x00006FFF

#define I2C_IRQENABLE_CLR_MASK  0x00006FFF
#define I2C_IRQENABLE_CLR_ACK   0x00000004
#define I2C_IRQENABLE_CLR_RX    0x00000008
#define I2C_IRQENABLE_CLR_TX    0x00000010

// BUF
#define I2C_BIT_RXFIFO_CLR      (1 << 14)
#define I2C_BIT_TXFIFO_CLR      (1 << 6)

// PSC
#define I2C_PSC_MASK            0x000000FF
#define I2C_PSC_12MHZ           0x00000003 // divided by 3, 48MHz/4 = 12MHz (page4589)
#define I2C_PSC_24MHZ           0x00000001 // divided by 1, 48MHz/1 = 24MHz (page4589)

// SCLL
#define I2C_SCLL_MASK           0x000000FF
#define I2C_SCLL_400K           0x00000017 // tLOW = 1,25 US = (23+7)*(2/48MHz)
#define I2C_SCLL_100K           0x000000E9 // tLOW = 5 US = (233+7)+(1/48MHz)

// SCLH
#define I2C_SCLH_MASK           0x000000FF
#define I2C_SCLH_400K           0x00000019 // THIGH = 1,25 uS = (25+5)*(1/48MHz)
#define I2C_SCLH_100K           0x000000EB // THIGH = 5 us = (235+5)*(1/48MHz)

#endif // _I2C_H
