#include "kernel_module.h"

/******************************************************************************
 * Platform Driver - Methods
******************************************************************************/

/******************************************************************************
 * Device tree functions
******************************************************************************/




/// @brief This function is called when a device matches the "compatible"
///  property in the device tree.
/// @param pdev Reference to the device tree.
/// @return "0" on success, not "0" on error.
static int i2c_probe(struct platform_device * i2c_plat_dev)
{
    int status = 0;
    pr_info("%s: PROBE - Initializing driver.. i2c_plat_dev->name = %s\n", DRIVER_NAME, i2c_plat_dev->name);
    
    if ((status = i2c_init(i2c_plat_dev)) != 0) {
        pr_warn("%s: PROBE - Error while running i2c_init().\n", DRIVER_NAME);
        goto i2c_error;
    }
    if ((status = MPU6050_init(i2c_plat_dev)) != 0) {
        pr_warn("%s: PROBE - Error while running mpu6050_init().\n", DRIVER_NAME);
        goto mpu6050_error;
    }
    // if ((status = char_device_create()) != 0) {
    //     pr_warn("%s: PROBE - Error while running char_device_create().\n", DRIVER_NAME);
    //     goto char_device_error;
    // }
    return 0;

    char_device_error: MPU6050_deinit();
    mpu6050_error: i2c_deinit();
    i2c_error: return status;
}


/// @brief This function is called when the driver is removed, for every device
///  that matched the "compatible" property in the device tree.
/// @param pdev Reference to the device tree.
/// @return "0" on success, not "0" on error.
static int i2c_remove(struct platform_device * i2c_plat_dev)
{
    pr_info("%s: REMOVE - Removing driver.. i2c_plat_dev->name = %s\n", DRIVER_NAME, i2c_plat_dev->name);
    char_device_remove();
    MPU6050_deinit();
    i2c_deinit();
    return 0;
}

/******************************************************************************
 * Platform Driver - Configuration Structures
******************************************************************************/
// Used to identify a device in the device tree. Matches "compatible" property
static struct of_device_id i2c_of_device_ids[] = {
    {.compatible = COMPATIBLE}, { /*Don't remove*/ }
};

MODULE_DEVICE_TABLE(of, i2c_of_device_ids);

static struct platform_driver i2c_plat_driver = {
    .probe = i2c_probe,
    .remove = i2c_remove,
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(i2c_of_device_ids)
    },
};

/******************************************************************************
 * Linux Kernel Module
******************************************************************************/
static int __init lkm_init(void)
{
    int status = 0;
    
    pr_info("%s: INIT - Running init configuration..\n", DRIVER_NAME);

    if( (status = platform_driver_register(&i2c_plat_driver)) != 0) {
        pr_warn("%s: Platform Device could not be initialized.\n", DRIVER_NAME);
        return status;
    }

    pr_info("%s: INIT - LKM was initialized successfully.\n", DRIVER_NAME);
    return status;
}

static void __exit lkm_exit(void)
{
    pr_info("%s: EXIT - Doing cleanup...\n", DRIVER_NAME);
    platform_driver_unregister(&i2c_plat_driver);
    pr_info("%s: EXIT - LKM was removed successfully.\n", DRIVER_NAME);
}

// Set entrypoint and exit point for the kernel module.
module_init(lkm_init);
module_exit(lkm_exit);