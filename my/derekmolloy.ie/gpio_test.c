/**
 * @file   gpio_test.c
 * @author Derek Molloy
 * @date   19 April 2015
 * @brief  A kernel module for controlling a GPIO LED/button pair. The device mounts devices via
 * sysfs /sys/class/gpio/gpio115 and gpio49. Therefore, this test LKM circuit assumes that an LED
 * is attached to GPIO 49 which is on P9_23 and the button is attached to GPIO 115 on P9_27. There
 * is no requirement for a custom overlay, as the pins are in their default mux mode states.
 * @see http://www.derekmolloy.ie/
*/
 
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h> 
#include <linux/ioport.h>
#include <linux/gpio.h>                 // Required for the GPIO functions
#include <linux/interrupt.h>            // Required for the IRQ code

#define BCM2708_PERI_BASE       0x20000000
#define GPIO_BASE              (BCM2708_PERI_BASE + 0x200000) // GPIO controller
#define PERI_MAX_OFFSET         0xB0

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Derek Molloy");
MODULE_DESCRIPTION("A Button/LED test driver for the BBB");
MODULE_VERSION("0.1");

static const int gpioButton = 18;

#define DCHECK( success ) ;

// IO Access
static struct bcm2835_peripheral 
{
    unsigned long addr_p;
    int mem_fd;
    void *map;
    volatile u32 *addr;  // 4 байта?
} gpio;

#define PULL_ENA  0x94
#define PULL_CLK0 0x98
#define PULL_CLK1 0x9C

// 	int s = (success); \
// 	if( s ){ \
// 		printk( KERN_INFO "gpio_test: %i\n", __LINE__); \
// 		return s; \
// 	} \
// } while( 0 );

// Need more
// https://github.com/recalbox/mk_arcade_joystick_rpi/blob/master/mk_arcade_joystick_rpi.c
// Нужно подтянуть!
// https://communities.intel.com/thread/87791?start=0&tstart=0

// enable sysfs 
//   http://raspberrypi.stackexchange.com/questions/24092/kernel-config-necessary-options

struct resource * mem;
static int __init ebbgpio_init(void)
{
	// !!! request_mem_region
	mem = request_mem_region(GPIO_BASE, PERI_MAX_OFFSET, "mygpio");
	if( !mem ){
		pr_err("request reg failed\n");
        return -EBUSY;
	}
	// https://e2e.ti.com/support/embedded/linux/f/354/t/390011

	/* Set up gpio pointer for direct register access */
	gpio.map = ioremap_nocache(GPIO_BASE, PERI_MAX_OFFSET);
    if( gpio.map == NULL ){
        pr_err("io remap failed\n");
        return -EBUSY;
    }

    gpio.addr = (volatile u32*)gpio.map;

	int status = 0;
	DCHECK( !gpio_is_valid( gpioButton ) );  // fixme: wrong
	status = gpio_request( gpioButton, "sysfs" );
	if( status )
		return status;

	// status = gpio_direction_input( gpioButton );
	// if( status )
	// 	return status;

	// DCHECK( gpio_set_debounce(gpioButton, 200) );
	
	// int status = gpio_export(gpioButton, false);
	// if( status ){
	// 	return status;
	// }

	// printk(KERN_INFO "GPIO_TEST: The button state is currently: %d\n", gpio_get_value(gpioButton));
	
	printk(KERN_INFO "gpio_test: init done\n");
	return 0;
}

static void __exit ebbgpio_exit(void) 
{
	// gpio_unexport( gpioButton );
	gpio_free( gpioButton);

	iounmap(gpio.map);

	release_mem_region(GPIO_BASE, PERI_MAX_OFFSET);
}

module_init(ebbgpio_init);
module_exit(ebbgpio_exit);