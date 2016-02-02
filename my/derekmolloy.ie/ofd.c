#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/fs.h>

// fixme: make as in video but without direct calls HW
// fixme: how use IRQ
//
// fixme: try with poll() interface. only one can open

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anil Kumar Pugalia <email_at_sarika-pugs_dot_com>");
MODULE_DESCRIPTION("Our First Driver");

static int majorNumber;
static char message[256];
static short size_of_message;
static int numberOpens = 0;
static struct class* ebbcharClass = NULL;

static int dev_open(struct inode*, struct file *);

static struct file_operations fops =
{
		.open = dev_open
};

static int __init ebbchar_int(void)
{
//	majorNu
    printk(KERN_INFO "Namaskar: ofd registered");
    return 0;
}

static void __exit ofd_exit(void)
{
    printk(KERN_INFO "Alvida: ofd unregistered");
} 

module_init(ebbchar_int);
module_exit(ofd_exit);

