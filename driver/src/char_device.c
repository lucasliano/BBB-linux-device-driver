#include "char_device.h"

/******************************************************************************
 * Static functions' prototypes
******************************************************************************/

static int char_device_open(struct inode *device_file, struct file *instance);
static int char_device_release(struct inode *device_file, struct file *instance);
static ssize_t char_device_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs);
static ssize_t char_device_read(struct file *file, char *user_buffer, size_t count, loff_t *offs);
static long int char_device_ioctl(struct file *file, unsigned cmd, unsigned long arg);

/******************************************************************************
 * Static variables
******************************************************************************/

static dev_t device_number;
static struct class *device_class;
static struct cdev my_device;
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = char_device_open,
    .release = char_device_release,
    .write = char_device_write,
    .read = char_device_read,
    .unlocked_ioctl = char_device_ioctl,
};

/******************************************************************************
 * Char device control
******************************************************************************/

/// @brief Creates the char device
/// @return "0"on success, non zero error code on error.
int char_device_create() {
    int retval = -1;

    // Allocate device number (MAJOR and MINOR)
    if ((retval = alloc_chrdev_region(&device_number, MINOR_NUMBER, NUMBER_OF_DEVICES, DEVICE_NAME)) != 0) {
        pr_err("Couldn't allocate device number.\n");
        goto chrdev_error;
    }

    // Create device class (This name will be seen as SUBSYSTEM=="DEVICE_CLASS_NAME"
    // for udev, and a folder with the same will be created inside "/sys/class/DEVICE_CLASS_NAME" )
    if ((device_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME)) == NULL) {
        pr_err("Device class couldn't be created.\n");
        retval = -1;
        goto class_error;
    }

    // Create device file (/sys/class/<DEVICE_CLASS_NAME>/<DEVICE_NAME>)
    if (device_create(device_class, NULL, device_number, NULL, DEVICE_NAME) == NULL) {
        pr_err("Couldn't create device file.\n");
        retval = -1;
        goto device_error;
    }

    // Initializing and registering device file
    cdev_init(&my_device, &fops);
    if ((retval = cdev_add(&my_device, device_number, NUMBER_OF_DEVICES)) != 0 ) {
        pr_err("Registering of device to kernel failed.\n");
        goto cdev_add_error;
    }

    pr_info("Device /dev/%s created successfully. Major: %d, Minor: %d.\n",
        DEVICE_NAME, MAJOR(device_number), MINOR(device_number));
    return 0;

    cdev_add_error: device_destroy(device_class, device_number);
    device_error: class_destroy(device_class);
    class_error: unregister_chrdev(device_number, DEVICE_NAME);
    chrdev_error: return retval;
}

/// @brief Remove previously created device.
void char_device_remove(void) {
    device_destroy(device_class, device_number);
    class_destroy(device_class);
    unregister_chrdev(device_number, DEVICE_NAME);
}

/******************************************************************************
 * File operations
******************************************************************************/

/// @brief This function is called when the device is opened
static int char_device_open(struct inode *device_file, struct file *instance) {
    // if(!bmp280_is_connected()) {
    //     pr_err("Couldn't open device.\n");
    //     return -1;
    // }
    pr_info("OPEN!");
    return 0;
}

/// @brief This function is called when the device is closed
static int char_device_release(struct inode *device_file, struct file *instance) {
    pr_info("RELEASE!");
    return 0;
}

/// @brief Does nothing. Upon successful completion, these functions shall return the number of bytes actually written to the file associated with fildes. This number shall never be greater than nbyte. Otherwise, -1 shall be returned and errno set to indicate the error
static ssize_t char_device_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offs) {
    pr_info("WRITE!");
    return count;
}

/// @brief Read temperature. Returns a string like "50.80".
/// @return Amount of bytes read, or "-1" on error.
static ssize_t char_device_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offs) {
    // int to_copy, not_copied;
    // char out_string[10];
    // s32 temperature;

    // // Get temperature
    // if ((temperature = bmp280_read_temperature()) == BMP280_ERROR) {
    //     pr_err("Couldn't read temperature.\n");
    //     return -1;
    // } else {
    //     snprintf(out_string, sizeof(out_string), "%d.%02d\n", temperature/100, temperature%100);
    // }

    // // Get amount of data to copy
    // to_copy = min(count, strlen(out_string) + 1);

    // // Copy data to user, return bytes that hasn't copied
    // not_copied = copy_to_user(user_buffer, out_string, to_copy);

    // return to_copy - not_copied;    // Return bytes copied

    pr_info("READ!");
    return count;
}

static long int char_device_ioctl(struct file *file, unsigned cmd, unsigned long __user arg) {
    // int operation;
    // if (copy_from_user(&operation, (int *) arg, sizeof(operation)) != 0) {
    //     pr_err("Couldn't copy data from user.\n");
    //     return -1;
    // }
    // switch(cmd) {
    //     case IOCTL_CMD_SET_FREQ:
    //         if (bmp280_set_frequency((bmp280_freq) operation) != 0) {
    //             pr_err("Couldn't change frequency.\n");
    //             return -1;
    //         }
    //         pr_info("Frequency changed to %d.\n", operation);
    //     break;
    //     case IOCTL_CMD_SET_MODE:
    //         if (bmp280_set_mode((bmp280_mode) operation) != 0) {
    //             pr_err("Couldn't change mode.\n");
    //             return -1;
    //         }
    //         pr_info("Mode changed to %d.\n", operation);
    //     break;
    //     case IOCTL_CMD_SET_UNIT:
    //         bmp280_set_unit((bmp280_unit) operation);
    //         pr_info("Unit changed to %d.\n", operation);
    //     break;
    //     default:
    //         pr_err("Unknown ioctl command.\n");
    //         return -1;
    //     break;
    // }
    pr_info("IOCTL!");
    return 0;
}
