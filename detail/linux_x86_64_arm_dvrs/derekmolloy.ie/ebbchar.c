
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

// Note: модуль ядра, драйвер, device - модуль ядра не 
//   обязательно драйвер
//   http://unix.stackexchange.com/questions/47208/what-is-the-difference-between-kernel-drivers-and-kernel-modules

// Minor - получает как параметр
//   http://linuxdrivers.blogspot.ru/2010/10/13.html

#define DEVICE_NAME "ebbchar"
#define CLASS_NAME "ebb"

static int majorNumber;
static char message[256];
static short size_of_message;
static int numberOpens = 0;
static struct class* ebbcharClass = NULL;
static struct device* ebbcharDevice = NULL;

// prototypes
static int dev_open(struct inode*, struct file*);
static int dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);
 
/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

// ctor/dtor
static int __init ebbchar_init(void) 
{
	printk(KERN_INFO "EBBChar: Init the EBBChar LKM\n");

	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber<0){
		printk(KERN_ALERT "EBBChar: failed to reg a major number\n");
		return majorNumber;
	}

	// reg device class
	ebbcharClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(ebbcharDevice)) {
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to reg device class\n");
		return PTR_ERR(ebbcharClass);
	}
	printk(KERN_INFO "EBBChar: device class reg correctly\n");

	ebbcharDevice = device_create(
		ebbcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(ebbcharDevice)) {
		class_destroy(ebbcharClass);
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(ebbcharDevice);
	}
	printk(KERN_INFO "EBBChar: device class created correctly\n");
	return 0;
}

static void __exit ebbchar_exit(void) 
{
	device_destroy(ebbcharClass, MKDEV(majorNumber, 0));     // remove the device
	class_unregister(ebbcharClass);                          // unregister the device class
	class_destroy(ebbcharClass);                             // remove the device class
	unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
	printk(KERN_INFO "EBBChar: Goodbye from the LKM!\n");
}

static int dev_open(struct  inode* inodep, struct file* filep) 
{
	numberOpens++;
	printk(KERN_INFO 
		"EBBChar: Device has been opened %d tims(s)\n", numberOpens);	
	return 0;
}

static ssize_t dev_read(struct file* filep, char *buffer, size_t len,
	loff_t* offset) 
{
	const int err_count = 
		copy_to_user(buffer, message, size_of_message);

	if (err_count==0) {
		printk(KERN_INFO "EBBChar: Sent %d characters to the user\n", size_of_message);
		return (size_of_message=0);
	} 
	else {
		printk(KERN_INFO "EBBChar: Failed to send %d characters to the user\n", err_count);
		return -EFAULT;
	}
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   sprintf(message, "%s(%d letters)", buffer, len);   // appending received string with its length
   size_of_message = strlen(message);                 // store the length of the stored message
   printk(KERN_INFO "EBBChar: Received %d characters from the user\n", len);
   return len;
}
 
/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
	// fixme: --coun...
   printk(KERN_INFO "EBBChar: Device successfully closed\n");
   return 0;
}

module_init(ebbchar_init);
module_exit(ebbchar_exit);

// !!!
MODULE_LICENSE("GPL");