#include "i2c.h"

/******************************************************************************
 * Static variables
******************************************************************************/

// Pointers to memory mapped registers of the CPU
static void __iomem *i2c_ptr = NULL;
static void __iomem *clk_ptr = NULL;
static void __iomem *control_module_ptr = NULL;

// Use to send to sleep processes using the driver.
DECLARE_WAIT_QUEUE_HEAD(waiting_queue);

// Used to enable only one process to read or write to the bus
DEFINE_MUTEX(lock_bus);


// I2C Data structure
static struct i2c_buffers{
    u8 * buff_rx;  // Pointer to user data in kernel space
    u8 pos_rx;     // Data to be received. This will be updated by the ISR.
    u8 buff_rx_len;

    u8 * buff_tx;
    u8 pos_tx;
    u8 buff_tx_len;
} data_i2c;

int sleeping_condition;  // Condition to handle the state of the processes.
static int g_irq;       // IRQ number, saved for deinitialization

/*****buff_tx*************************************************************************
 * Static functions' prototypes
******************************************************************************/

/// @brief Sets the target slave address
static inline void __set_slave_address(u8 addr)
{
    iowrite32(addr, i2c_ptr + I2C_REG_SA);
}

/// @brief Wakeup the I2C2 clock. The OS might put the I2C clock to sleep, so
///  re-enable the clock just in case.
static void __wakeup(void)
{
    int aux;

    aux = (unsigned int)clk_ptr + (unsigned int)(IDCM_PER_I2C2_CLKCTRL); 
    aux = (unsigned int)ioread32((unsigned int *) aux); 
    aux |= 0x02;
    iowrite32(aux, clk_ptr + IDCM_PER_I2C2_CLKCTRL);
    pr_info("%s: Waking UP I2C2..", DRIVER_NAME);
    while(ioread32(clk_ptr + IDCM_PER_I2C2_CLKCTRL) != CM_PER_I2C2_CLKCTRL_ENABLE);
}

/// @brief Wait until the bus is freed.
/// @return "0" if the bus is free. "-1" on timeout.
static int __wait_for_bus_busy(void) 
{
    u8 i = 0;

    while(mutex_is_locked(&lock_bus)) {
        msleep(1);
        if (i++ == 100) {
            pr_warn("%s: TIMEOUT ERROR: I2C bus is locked.\n", DRIVER_NAME);
            return -1;
        }
    }
    return 0;
}

/// @brief Clears the i2c_buffers global structure.
/// @return None.
static void __clean_data_i2c(void)
{
    memset(data_i2c.buff_rx, 0, 4096);
    data_i2c.pos_rx = 0;
    data_i2c.buff_rx_len = 0;

    memset(data_i2c.buff_tx, 0, 4096);
    data_i2c.pos_tx = 0;
    data_i2c.buff_tx_len = 0;
}

/******************************************************************************
 * I2C private operations
******************************************************************************/


/// @brief Handler for the I2C IRQ.
static irqreturn_t i2c_isr(int irq_number, void *dev_id)
{
    int irq = ioread32(i2c_ptr + I2C_REG_IRQSTATUS);

    if (irq & I2C_IRQ_XRDY) // TX
    { 
        // Loads data register
        pr_info("XRDY HANDLER.\n");
        iowrite32(data_i2c.buff_tx[data_i2c.pos_tx++], i2c_ptr + I2C_REG_DATA);
    
        if(data_i2c.buff_tx_len == data_i2c.pos_tx)
        { 
            iowrite32(I2C_IRQENABLE_CLR_TX, i2c_ptr + I2C_REG_IRQENABLE_CLR);
            sleeping_condition = 1;
            wake_up_interruptible(&waiting_queue);
        }
        iowrite32(I2C_IRQSTATUS_CLR_ALL, i2c_ptr + I2C_REG_IRQSTATUS);
    }

    if (irq & I2C_IRQ_RRDY)  // RX
    {
        pr_info("RRDY HANDLER.\n");

        // Saves received data in buffer
        data_i2c.buff_rx[data_i2c.pos_rx++] = ioread32(i2c_ptr + I2C_REG_DATA);

        if(data_i2c.buff_rx_len == data_i2c.pos_rx)
        { 
            iowrite32(I2C_IRQENABLE_CLR_RX, i2c_ptr + I2C_REG_IRQENABLE_CLR);
            sleeping_condition = 1;
            wake_up_interruptible(&waiting_queue);
        }
        iowrite32(I2C_IRQSTATUS_CLR_ALL, i2c_ptr + I2C_REG_IRQSTATUS);

    }  

    if (irq & I2C_IRQ_ARDY) { // ACCESS READY
        pr_info("ARDY HANDLER.\n");

        // Clears ACK irq enable
        iowrite32(I2C_IRQENABLE_CLR_ACK, i2c_ptr + I2C_REG_IRQENABLE_CLR);
        
        // We switch to rx mode
        iowrite32(data_i2c.buff_rx_len, i2c_ptr + I2C_REG_CNT);
        iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_STOP | I2C_BIT_START, 
            i2c_ptr + I2C_REG_CON); // (RX is enable with 0 at I2C_BIT_TX)
            

        // enables RX irq
        iowrite32(I2C_IRQ_RRDY, i2c_ptr + I2C_REG_IRQENABLE_SET);

        iowrite32(I2C_IRQSTATUS_CLR_ALL, i2c_ptr + I2C_REG_IRQSTATUS);
    } 
    
    if (irq & I2C_IRQ_NACK) { // NACK NOTE: Not implemented!
        pr_warn("IRQ I2C NACK: Not acknowledge was received.\n");
    }

    return IRQ_HANDLED;
}

/******************************************************************************
 * Functions
******************************************************************************/

/// @brief Initialize the I2C2 bus, with pins P9.21 and P9.22.
int i2c_init(struct platform_device *pdev) {
    struct device *i2c_dev = &pdev->dev;
    u32 pins[4];
    int retval = -1;

    // -------------------------
    // Working w/ devtree
    // -------------------------

    // Check that parent device exists (should be target-module@9c000)
    if (i2c_dev->parent == NULL) {
        pr_err("I2C device doesn't have a parent.\n");
        goto pdev_error;
    }

    // Check for device properties
    if (!device_property_present(i2c_dev, DT_PROPERTY_PINS))
    {
        pr_err("Device properties for i2c device not found.\n");
        goto pdev_error;
    }

    // Read device properties
    if ((retval = device_property_read_u32_array(i2c_dev, DT_PROPERTY_PINS, pins, 4))!= 0) 
    {
        pr_err("Couldn't read properties.\n");
        goto pdev_error;
    }

    // -------------------------
    // Mapping Registers
    // -------------------------
    if((clk_ptr = ioremap(CM_PER, CM_PER_LEN)) == NULL){
        pr_alert("%s: Could not assign memory for clk_ptr (CM_PER).\n", DRIVER_NAME);
        goto pdev_error;
    }
    pr_info("%s: clk_ptr: 0x%X\n", DRIVER_NAME, (unsigned int)clk_ptr);

    if((control_module_ptr = ioremap(CTRL_MODULE_BASE, CTRL_MODULE_LEN)) == NULL){
        pr_alert("%s: Could not assign memory for control_module_ptr (CTRL_MODULE_BASE).\n", DRIVER_NAME);
        goto clk_ptr_error;
    }
    pr_info("%s: control_module_ptr: 0x%X\n", DRIVER_NAME, (unsigned int)control_module_ptr);

    if((i2c_ptr = ioremap(I2C2, I2C2_LEN)) == NULL){
        pr_alert("%s: Could not assign memory for i2c_ptr (I2C2).\n", DRIVER_NAME);
        goto control_module_ptr_error;
    }
    pr_info("%s: i2c_ptr: 0x%X\n", DRIVER_NAME, (unsigned int)i2c_ptr);


    // -------------------------
    // Pinmux configuration
    // -------------------------

    // Configure P9.21 and P9.22 pinmux as I2C
    iowrite32(pins[1], control_module_ptr + pins[0]);
    iowrite32(pins[3], control_module_ptr + pins[2]);


    // -------------------------
    // Configures I2C Connection
    // -------------------------
    
    // Turn ON I2C Clock
    __wakeup();

    // Disable I2C while configuring..
    iowrite32(0x0, i2c_ptr + I2C_REG_CON); 

    // Clock Configuration
    iowrite32(I2C_PSC_12MHZ, i2c_ptr + I2C_REG_PSC);
    iowrite32(I2C_SCLL_400K, i2c_ptr + I2C_REG_SCLL); 
    iowrite32(I2C_SCLH_400K, i2c_ptr + I2C_REG_SCLH); 

    // Force Idle
    iowrite32(0x00, i2c_ptr + I2C_REG_SYSC);   
    
    // Enable I2C device
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_TX, // 0x8600
        i2c_ptr + I2C_REG_CON);
    

    // -------------------------
    // Virtual IRQ request
    // -------------------------

    if ((g_irq = platform_get_irq(pdev, 0)) < 0) {
        pr_err("%s: Couldn't get I2C IRQ number.\n", DRIVER_NAME);
        goto i2c_ptr_error;
    }

    if ((request_irq(g_irq, (irq_handler_t) i2c_isr, IRQF_TRIGGER_RISING, "lliano,i2c", NULL) < 0)){//pdev->name, NULL)) < 0) {
        pr_err("%s: Couldn't request I2C IRQ.\n", DRIVER_NAME);
        goto i2c_ptr_error;
    }

    // -------------------------
    // Internal struct init.
    // -------------------------

    // We ask the kernel for memory for the buffers (4kB)
    if ((data_i2c.buff_rx = (char *) __get_free_page (GFP_KERNEL)) < 0){
        pr_alert("%s: Error while asking por a free page.\n", DRIVER_NAME);
        goto virq_error;
    }
    if ((data_i2c.buff_tx = (char *) __get_free_page (GFP_KERNEL)) < 0){
        pr_alert("%s: Error while asking por a free page.\n", DRIVER_NAME);
        goto virq_error;
    }

    pr_info("I2C successfully configured.\n");
    return 0;

    // -------------------------
    // Error Handling
    // -------------------------
    virq_error: free_irq(g_irq, NULL);
    i2c_ptr_error: iounmap(i2c_ptr);
    control_module_ptr_error: iounmap(control_module_ptr);
    clk_ptr_error: iounmap(clk_ptr);
    pdev_error: retval = -1; i2c_ptr = NULL; clk_ptr = NULL; control_module_ptr = NULL;
    return retval;
}

/// @brief Deinitialize the I2C2 bus.
void i2c_deinit(void) {
    if (clk_ptr != NULL) {
        iounmap(clk_ptr);
    } if (control_module_ptr != NULL) {
        iounmap(control_module_ptr);
    } if (i2c_ptr != NULL) {
        iounmap(i2c_ptr);
    }
    free_irq(g_irq, NULL);
    free_page((unsigned long)data_i2c.buff_rx); 
    free_page((unsigned long)data_i2c.buff_tx);
}

/// @brief Write a value to the I2C bus.
/// @param slave_address Address of the I2C slave.
/// @param data Pointer to kernel space data buffer.
/// @param size Amount of data to be written.
/// @return "0" on success, "-1" on error.
int i2c_write(char slave_address, char* data, char size) 
{
    int retval = -1;
    int auxReg;

    if (size == 0)
    {
        pr_warn("%s: Write Error: Size should be greater than 0.\n", DRIVER_NAME);
        return retval;
    }

    // Wait until no other process is using it
    if(__wait_for_bus_busy() != 0) {
        return retval;
    }

    // Get control of the bus
    mutex_lock(&lock_bus);

    // Makes sure CLK is running
    __wakeup();
    
    // Set slave address
    __set_slave_address(slave_address);
    
    // Load the data structures and registers.
    pr_info("Configurando data structures\n");
    __clean_data_i2c();
    memcpy(data_i2c.buff_tx, data, size);
    data_i2c.buff_tx_len = size;

    // Load I2C DATA & CNT registers
    iowrite32(data_i2c.buff_tx_len,              i2c_ptr + I2C_REG_CNT);

    // Sets I2C CONFIG register w/ Master TX (=0x8600)
    pr_info("Seteo modo tx\n");
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_TX, i2c_ptr + I2C_REG_CON);

    // Clear IRQ Flags
    iowrite32(I2C_IRQSTATUS_CLR_ALL, i2c_ptr + I2C_REG_IRQENABLE_CLR); 
    iowrite32(I2C_IRQSTATUS_CLR_ALL, i2c_ptr + I2C_REG_IRQSTATUS);

    // Enables ACK interrupt
    pr_info("Habilito interrupcion de TX\n");
    iowrite32(I2C_IRQ_XRDY, i2c_ptr + I2C_REG_IRQENABLE_SET);
    // iowrite32(I2C_IRQ_XRDY, i2c_ptr + I2C_REG_IRQENABLE_SET);

    // Check irq status (occupied or free)
    pr_info("Espero a que el bus este libre\n");
    while (ioread32(i2c_ptr + I2C_REG_IRQSTATUS_RAW) & I2C_IRQ_BB) {msleep(1);}

    // Sends START
    sleeping_condition = 0;     // We need to ensure the condition before sending the start

    pr_info("Condicion de start\n");
    auxReg = ioread32(i2c_ptr + I2C_REG_CON); 
    auxReg |= I2C_BIT_START;
    iowrite32(auxReg, i2c_ptr + I2C_REG_CON);

    // Sends process to sleep
    pr_info("proceso a mimir\n");
    wait_event_interruptible (waiting_queue, sleeping_condition != 0);

    pr_info("%s: sigue bob\n", DRIVER_NAME);

    // We clear the start bit and sets the stop
    auxReg = ioread32(i2c_ptr + I2C_REG_CON);
    auxReg &= 0xFFFFFFFE;
    auxReg |= I2C_BIT_STOP;
    iowrite32(auxReg, i2c_ptr + I2C_REG_CON);

    // Waits for the core to send the stop and frees the mutex.
    fsleep(100); // 100 us
    mutex_unlock(&lock_bus);
    if (sleeping_condition > 0)
        retval = 0;
    
    pr_info("%s: Finaliza escritura\n", DRIVER_NAME);
    return retval;
}

/// @brief Read a value from the I2C bus.
/// @param slave_address Address of the I2C slave.
/// @param data Pointer to kernel space data buffer.
/// @param size Amount of data to be read.
/// @return "0" on success, "-1" on error.
int i2c_read(char slave_address, char* read_buff, char size)
{
    int retval = -1;
    int auxReg;

    if (size == 0)
    {
        pr_warn("%s: Write Error: Size should be greater than 0.\n", DRIVER_NAME);
        return retval;
    }

    // Wait until no other process is using it
    if(__wait_for_bus_busy() != 0) {
        return retval;
    }

    // Get control of the bus
    mutex_lock(&lock_bus);

    // Makes sure CLK is running
    __wakeup();

    // Clear IRQ Flags
    iowrite32(I2C_IRQSTATUS_CLR_ALL, i2c_ptr + I2C_REG_IRQENABLE_CLR); 
    iowrite32(I2C_IRQSTATUS_CLR_ALL, i2c_ptr + I2C_REG_IRQSTATUS);
    
    // Set slave address
    __set_slave_address(slave_address);
    
    // Load the data structures and registers.
    pr_info("Configurando data structures\n");
    __clean_data_i2c();
    data_i2c.buff_rx_len = size;

    // Load I2C DATA & CNT registers
    pr_info("Caro cantidad de datos a recibir.\n");
    iowrite32(data_i2c.buff_rx_len,              i2c_ptr + I2C_REG_CNT);


    // Sets I2C CONFIG register w/ Master RX (=0x8400)
    pr_info("Seteo modo rx\n");
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE, i2c_ptr + I2C_REG_CON); // (RX is enable with 0 at I2C_BIT_TX)

    // Enables ACK interrupt
    pr_info("Habilito interrupcion de RX\n");
    iowrite32(I2C_IRQ_RRDY, i2c_ptr + I2C_REG_IRQENABLE_SET);

    // Check irq status (occupied or free)
    pr_info("Espero a que el bus este libre\n");
    while (ioread32(i2c_ptr + I2C_REG_IRQSTATUS_RAW) & I2C_IRQ_BB) {msleep(1);}

    // Sends START
    sleeping_condition = 0;     // We need to ensure the condition before sending the start

    pr_info("Condicion de start\n");
    auxReg = ioread32(i2c_ptr + I2C_REG_CON); 
    auxReg |= I2C_BIT_START;
    iowrite32(auxReg, i2c_ptr + I2C_REG_CON);

    // Sends process to sleep
    pr_info("Proceso a mimir\n");
    wait_event_interruptible (waiting_queue, sleeping_condition != 0);

    // We clear the start bit and sets the stop
    auxReg = ioread32(i2c_ptr + I2C_REG_CON);
    auxReg &= 0xFFFFFFFE;
    auxReg |= I2C_BIT_STOP;
    iowrite32(auxReg, i2c_ptr + I2C_REG_CON);

    // Waits for the core to send the stop and frees the mutex.
    fsleep(100); // 100 us
    mutex_unlock(&lock_bus);
    if (sleeping_condition > 0)
    {
        memcpy(read_buff, data_i2c.buff_rx, size);
        retval = 0;
    }
    
    pr_info("%s: Finaliza lectura\n", DRIVER_NAME);
    return retval;
}

/// @brief Read a register from a compatible I2C slave. We will perform a Write + Read operation w/ repeated start.
/// @param slave_address Address of the I2C slave.
/// @param reg_address Address of the register to be read.
/// @param data Pointer to kernel space data buffer.
/// @return "0" on success, "-1" on error.
int i2c_read_reg(char slave_address, char reg_address, char* read_buff) 
{
    int retval = -1;
    int auxReg;

    // Wait until no other process is using it
    if(__wait_for_bus_busy() != 0) {
        return retval;
    }

    // Get control of the bus
    mutex_lock(&lock_bus);

    // Makes sure CLK is running
    __wakeup();
    
    // Set slave address
    __set_slave_address(slave_address);
    
    // Load the data structures and registers.
    pr_info("Configurando data structures\n");
    __clean_data_i2c();
    data_i2c.buff_rx_len = 1;

    // Load I2C DATA & CNT registers
    iowrite32(reg_address, i2c_ptr + I2C_REG_DATA);
    iowrite32(1, i2c_ptr + I2C_REG_CNT);

    // Sets I2C CONFIG register w/ Master TX (=0x8600)
    pr_info("Seteo modo tx\n");
    iowrite32(I2C_BIT_ENABLE | I2C_BIT_MASTER_MODE | I2C_BIT_TX, i2c_ptr + I2C_REG_CON);

    // Clear IRQ Flags
    iowrite32(I2C_IRQSTATUS_CLR_ALL, i2c_ptr + I2C_REG_IRQENABLE_CLR); 
    iowrite32(I2C_IRQSTATUS_CLR_ALL, i2c_ptr + I2C_REG_IRQSTATUS);

    // Enables ACK interrupt
    pr_info("Habilito interrupcion de ACK\n");
    iowrite32(I2C_IRQ_ARDY, i2c_ptr + I2C_REG_IRQENABLE_SET);

    // Check irq status (occupied or free)
    pr_info("Espero a que el bus este libre\n");
    while (ioread32(i2c_ptr + I2C_REG_IRQSTATUS_RAW) & I2C_IRQ_BB) {msleep(1);}

    // Sends START
    sleeping_condition = 0;     // We need to ensure the condition before sending the start

    pr_info("Condicion de start\n");
    auxReg = ioread32(i2c_ptr + I2C_REG_CON); 
    auxReg |= I2C_BIT_START;
    iowrite32(auxReg, i2c_ptr + I2C_REG_CON);

    // Sends process to sleep
    pr_info("proceso a mimir\n");
    wait_event_interruptible (waiting_queue, sleeping_condition != 0);

    // We clear the start bit and sets the stop
    auxReg = ioread32(i2c_ptr + I2C_REG_CON);
    auxReg &= 0xFFFFFFFE;
    auxReg |= I2C_BIT_STOP;
    iowrite32(auxReg, i2c_ptr + I2C_REG_CON);

    // Waits for the core to send the stop and frees the mutex.
    fsleep(100); // 100 us
    mutex_unlock(&lock_bus);
    if (sleeping_condition > 0)
    {
        memcpy(read_buff, data_i2c.buff_rx, 1);
        retval = 0;
    }
    
    pr_info("%s: Finaliza lectura\n", DRIVER_NAME);
    return retval;
}