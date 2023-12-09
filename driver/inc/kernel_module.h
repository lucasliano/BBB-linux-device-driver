#ifndef MODULE_H
#define MODULE_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>


#include "i2c.h"
#include "MPU6050.h"
#include "char_device.h"



#define DRIVER_NAME    "i2c_lliano"
#define COMPATIBLE	   "lliano,i2c"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Lucas Liaño");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("This module is a kernel driver for the I2C2 bus.");

#endif // MODULE_H
