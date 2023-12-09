#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/ioctl.h>

int char_device_create(void);
void char_device_remove(void);

// This value can be used by "udev" rules. Check for 'SUBSYSTEM=="DEVICE_CLASS_NAME"'.
#define DEVICE_CLASS_NAME "lliano"

// Name of the device. It'll be seen as "/dev/<DEVICE_NAME>"
#define DEVICE_NAME    "MPU6050"

// Minimum minor number that can be used.
#define MINOR_NUMBER 0

// Amount of devices that will be created
#define NUMBER_OF_DEVICES 1

#endif // CHAR_DEVICE_H
