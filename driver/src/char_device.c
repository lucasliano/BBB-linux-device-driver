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
static int char_device_open(struct inode *device_file, struct file *instance)
{
    if(!MPU6050_testConnection()) {
        pr_err("Couldn't open device.\n");
        return -1;
    }
    return 0;
}

/// @brief This function is called when the device is closed
static int char_device_release(struct inode *device_file, struct file *instance)
{
    // pr_info("%s: RELEASE was handled but there's nothing to do here!\n", DEVICE_NAME);
    return 0;
}

/// @brief Does nothing. Upon successful completion.
/// @return Amount of bytes written, or "-1" on error. Here it always return count.
static ssize_t char_device_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offs)
{
    pr_info("%s: WRITE was handled but there's nothing to do here!\n", DEVICE_NAME);
    return count;
}

/// @brief Reads all acceleration, angular velocity and temperature from a char[] buffer.
/// @return Amount of bytes read, or "-1" on error.
static ssize_t char_device_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offs)
{
    // Kernel space buffer
    char bufferaux [PACKET_NUMBER];

    // Raw data
    unsigned short int a_x, a_y, a_z;
    unsigned short int g_x, g_y, g_z;
    unsigned short int raw_temp;

    if (count < PACKET_NUMBER)
    {
        pr_alert("%s: You must performa full read of %d bytes.", DEVICE_NAME, PACKET_NUMBER);
        return -1;
    }
    
    // Data read
    MPU6050_getMotion6(&a_x, &a_y, &a_z, &g_x, &g_y, &g_z);
    raw_temp = MPU6050_getTemperature();

    // Debug print
    pr_info("MPU6050: a_x = %d\n", a_x);
    pr_info("MPU6050: a_y = %d\n", a_y);
    pr_info("MPU6050: a_z = %d\n", a_z);
    pr_info("MPU6050: g_x = %d\n", g_x);
    pr_info("MPU6050: g_y = %d\n", g_y);
    pr_info("MPU6050: g_z = %d\n", g_z);
    pr_info("\n");

    // Output buffer formatting
    pr_info("MPU6050: a_x = %x\n", a_x);
    pr_info("MPU6050: (a_x >> 8) & 0xFF = %x\n", (a_x >> 8) & 0xFF);
    pr_info("MPU6050: a_x & 0xFF= %x\n", a_x & 0xFF);

    bufferaux[0] = (a_x >> 8) & 0xFF;
    bufferaux[1] = a_x & 0xFF;
    bufferaux[2] = (a_y >> 8) & 0xFF;
    bufferaux[3] = a_y & 0xFF;
    bufferaux[4] = (a_z >> 8) & 0xFF;
    bufferaux[5] = a_z & 0xFF;

    bufferaux[6] = (raw_temp >> 8) & 0xFF;
    bufferaux[7] = raw_temp & 0xFF;

    bufferaux[8] = (g_x >> 8) & 0xFF;
    bufferaux[9] = g_x & 0xFF;
    bufferaux[10] = (g_y >> 8) & 0xFF;
    bufferaux[11] = g_y & 0xFF;
    bufferaux[12] = (g_z >> 8) & 0xFF;
    bufferaux[13] = g_z & 0xFF;

    // Copy to a user level buffer
    if(copy_to_user(user_buffer, (char*) bufferaux, PACKET_NUMBER) != 0)
    {
        pr_alert("%s: Error in copy_to_user().", DEVICE_NAME);
        return -1;
    }

    msleep(1);
    return PACKET_NUMBER;
}

static long int char_device_ioctl(struct file *file, unsigned cmd, unsigned long __user arg)
{
    int retVal = -1;
    int acc_modifier, gyro_modifier;
    int acc_range, gyro_range;

    switch(cmd) {
        case 0:
            acc_range = MPU6050_getFullScaleAccelRange();
            
            switch (acc_range)
            {
                case ACCEL_RANGE_2G:
                    acc_modifier = ACCEL_SCALE_MODIFIER_2G;
                    break;

                case ACCEL_RANGE_4G:
                    acc_modifier = ACCEL_SCALE_MODIFIER_4G;
                    break;
                
                case ACCEL_RANGE_8G:
                    acc_modifier = ACCEL_SCALE_MODIFIER_8G;
                    break;
                
                case ACCEL_RANGE_16G:
                    acc_modifier = ACCEL_SCALE_MODIFIER_16G;
                    break;
                
                default:
                    pr_warn("Unkown accelerometer range - acc_modifier set to ACCEL_SCALE_MODIFIER_2G\n");
                    acc_modifier = ACCEL_SCALE_MODIFIER_2G;
                    break;
            }
            return acc_modifier;
        break;

        case 1:
            gyro_range = MPU6050_getFullScaleGyroRange();
            switch (gyro_range)
            {
                case GYRO_RANGE_250DEG:
                    gyro_modifier = GYRO_SCALE_MODIFIER_250DEG;
                    break;

                case GYRO_RANGE_500DEG:
                    gyro_modifier = GYRO_SCALE_MODIFIER_500DEG;
                    break;
                
                case GYRO_RANGE_1000DEG:
                    gyro_modifier = GYRO_SCALE_MODIFIER_1000DEG;
                    break;
                
                case GYRO_RANGE_2000DEG:
                    gyro_modifier = GYRO_SCALE_MODIFIER_2000DEG;
                    break;
                
                default:
                    pr_warn("Unkown gyro range - gyro_modifier set to GYRO_RANGE_250DEG\n");
                    gyro_modifier = GYRO_SCALE_MODIFIER_250DEG;
                    break;
            }
            return gyro_modifier;
        break;


        default:
            pr_info("%s: IOCTL was handled but there's nothing to do here!\n", DEVICE_NAME);
        break;
    }
    return retVal;
}
