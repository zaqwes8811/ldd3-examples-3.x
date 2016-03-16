 
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/io.h> 
#include <linux/ioport.h>
#include <linux/gpio.h>                 // Required for the GPIO functions
#include <linux/interrupt.h>            // Required for the IRQ code

#define BCM2708_PERI_BASE       0x20000000
#define GPIO_BASE              (BCM2708_PERI_BASE + 0x200000) // GPIO controller
#define PERI_MAX_OFFSET         0xB0
#define PULL_UD_ENA_OFFSET  (0x94/4)
#define PULL_UD_CLK0_OFFSET (0x98/4)
#define PULL_UD_CLK1_OFFSET (0x9C/4)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zaqwes8811");
MODULE_DESCRIPTION("A Button/LED test driver for the RPI");
MODULE_VERSION("0.1");

static const u32 c_gpio_button = 18;
static unsigned int irqNumber;

// fixme: никчему это
static struct bcm2835_peripheral 
{
    unsigned long addr_p;
    int mem_fd;
    void *map;
    volatile u32 *addr;  // 4 байта?
} gpio;

// enable sysfs 
//   http://raspberrypi.stackexchange.com/questions/24092/kernel-config-necessary-options

static irq_handler_t gpio_irq_handler(
	unsigned int irq, void *dev_id, struct pt_regs *regs) 
{ 
   // the actions that the interrupt should perform
	printk(KERN_INFO "gpio_test: Interrupt! (button state is %d)\n", 
		gpio_get_value(c_gpio_button));
	return (irq_handler_t) IRQ_HANDLED;
}

inline void pull_up( u8 pin )
{
    *(gpio.addr + PULL_UD_ENA_OFFSET) =  0x01 << 1;
    udelay(200);
	*(gpio.addr + PULL_UD_CLK0_OFFSET) = 0x01 << pin;
    udelay(200);
    *(gpio.addr + PULL_UD_ENA_OFFSET) = 0x00;
    *(gpio.addr + PULL_UD_CLK0_OFFSET) = 0x00;
}

static struct resource * mem;
static int __init ebbgpio_init(void)
{
	mem = request_mem_region(GPIO_BASE, PERI_MAX_OFFSET, "mygpio");
	if( !mem ){
		pr_err("request reg failed\n");
        return -EBUSY;
	}

	/* Set up gpio pointer for direct register access */
	gpio.map = ioremap_nocache(GPIO_BASE, PERI_MAX_OFFSET);
    if( gpio.map == NULL ){
        pr_err("io remap failed\n");
        return -EBUSY;
    }

    gpio.addr = (volatile u32*)gpio.map;

    pull_up( c_gpio_button );

	if( !gpio_is_valid( c_gpio_button ) ){
		pr_err("gpio is non valid\n");
        return -EBUSY;
	}

	int status = gpio_request( c_gpio_button, "sysfs" );
	if( status ){
		return status;
	}

	status = gpio_direction_input( c_gpio_button );
	if( status ){
		return status;
	}

	irqNumber = gpio_to_irq(c_gpio_button);
	printk(KERN_INFO "gpio_test: The button is mapped to IRQ: %d\n", 
		irqNumber);

   // This next call requests an interrupt line
   status = request_irq(irqNumber,             // The interrupt number requested
                        (irq_handler_t) gpio_irq_handler, // The pointer to the handler function below
                        IRQF_TRIGGER_RISING,   // Interrupt on rising edge (button press, not release)
                        "ebb_gpio_handler",    // Used in /proc/interrupts to identify the owner
                        NULL);

  	if( status ){
  		pr_err("request_irq failed");
  		return status;
  	}

	// fixme: not working
	// gpio_set_debounce(c_gpio_button, 200);
	// int status = gpio_export(c_gpio_button, false);
	// if( status ){
	// 	return status;
	// }

	// printk(KERN_INFO "gpio_test: The button state is currently: %x\n", 
	// 	gpio_get_value(c_gpio_button) );
	
	printk(KERN_INFO "gpio_test: init done\n");
	return 0;
}

static void __exit ebbgpio_exit(void) 
{
	// free_irq(irqNumber, NULL);
	// gpio_unexport( c_gpio_button );
	gpio_free( c_gpio_button);

	iounmap(gpio.map);
	release_mem_region(GPIO_BASE, PERI_MAX_OFFSET);
}

module_init(ebbgpio_init);
module_exit(ebbgpio_exit);