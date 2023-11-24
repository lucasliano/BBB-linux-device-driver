#include <linux/module.h>           // Core header for loading LKMs into the kernel
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>  // platform_device
#include <linux/of.h>               // of_match_ptr
#include <linux/mod_devicetable.h>

#define DEVICE_NAME    "i2c_lliano"
#define COMPATIBLE	   "lliano,i2c"



/* ------ Platform Driver ------*/
static int i2c_probe(struct platform_device * i2c_plat_dev)
{
    int status = 0;
    pr_info("%s: Entre al PROBE. Dev_name = %s\n", DEVICE_NAME, i2c_plat_dev->name);
    return status;
}

static int i2c_remove(struct platform_device * i2c_plat_dev)
{
    int status = 0;
    pr_info("%s: Entre al REMOVE. Dev_name = %s\n", DEVICE_NAME, i2c_plat_dev->name);
    return status;
}

static struct of_device_id i2c_of_device_ids[] = {
    {
        .compatible = "lliano,i2c",
    }, {}
};

MODULE_DEVICE_TABLE(of, i2c_of_device_ids);

static struct platform_driver i2c_plat_driver = {
    .probe = i2c_probe,
    .remove = i2c_remove,
    .driver = {
        .name = "lliano,i2c",
        // .owner = THIS_MODULE,
        .of_match_table = i2c_of_device_ids, //of_match_ptr(i2c_of_device_ids)
    },
};



/* ------ Kernel Module ------*/
static int __init i2c_init(void)
{
    int status = 0;
    
    pr_info("Hola. Entro en el init\n");
    
    if(platform_driver_register(&i2c_plat_driver) != 0) {
        pr_info("%s: No se pudo registrar el platformDevice\n", DEVICE_NAME);
        return -1;
    }

    return 0;
}

static void __exit i2c_exit(void)
{
    pr_info("Chau. Entre en el exit\n");
    platform_driver_unregister(&i2c_plat_driver);
}


module_init(i2c_init);
module_exit(i2c_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Lucas Liaño");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("TD3 LKM");