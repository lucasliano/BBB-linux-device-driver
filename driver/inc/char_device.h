#ifndef CHAR_DEVICE_H
#define CHAR_DEVICE_H

#include <linux/init.h> 
#include <linux/kernel.h> 
#include <linux/kdev_t.h>          //agregar en /dev
#include <linux/device.h>          //agregar en /dev
#include <linux/cdev.h>            // Char device: File operation struct,
#include <linux/fs.h>              // Header for the Linux file system support (alloc_chrdev_region y unregister_chrdev_region) 
#include <linux/module.h>          // Core header for loading LKMs into the kernel 
#include <linux/of_address.h>      // of ionap
#include <linux/platform_device.h> // platform_device
#include <linux/of.h>              // of_match_ptr
#include <linux/io.h>              // foremap
#include <linux/interrupt.h>       // request_irq
#include <linux/delay.h>           // msleep, udelay y delay
#include <linux/uaccess.h>         // copy_to_user - copy_from_user 
#include <linux/types.h>           // typedefs varios
#include <linux/slab.h>            // kmalloc
#include <linux/ioctl.h>
#include "MPU6050.h"

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

// POSIX
#define PACKET_NUMBER 14

#endif // CHAR_DEVICE_H
