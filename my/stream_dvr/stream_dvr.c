// interaction with poll/select
// fixme: how connect with read/write
//
// "Целью вызовов poll и select является определить заранее,
// будет ли операция ввода/вывода блокирована."
//
// wait something
// http://stackoverflow.com/questions/12117227/linux-kernel-wait-for-other-threads-to-complete
// not work in x64
//interruptible_sleep_on( &s_q_wait );  // very old! never use it
//
// http://www.ibm.com/developerworks/ru/library/l-linux_kernel_22/

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/ioctl.h>
#include <linux/fcntl.h>
#include <linux/wait.h>  // не хватает хедеров
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/sched.h>

#include <asm/uaccess.h>
#include <asm/atomic.h>

MODULE_LICENSE("Dual BSD/GPL");

#define DEVICE_NAME "stream_dvr0"
#define CLASS_NAME "stream0"
#define LEN_MSG 256
#define EOF 0

//static int dev_open(struct  inode* inodep, struct file* filep);
//static int dev_release(struct inode *inodep, struct file *filep);
static ssize_t read( struct file *file, char *buf,
		size_t count, loff_t *ppos );
static ssize_t write( struct file *file,
		const char *buf, size_t count, loff_t *ppos );
static unsigned int poll( struct file *file,
		struct poll_table_struct *poll_table_ptr );

////////////////////////////////////////////////

static int major_n_;
static struct class* sdvr_cls_ = NULL;
static struct device* sdvr_dev_ = NULL;

static struct file_operations fops_ =
{
//   .open = dev_open,
   .read = read,
   .write = write,
   .poll = poll,
//   .release = dev_release,
};

struct private
{
	atomic_t read_offset;
	char buf[ LEN_MSG + 2 ];
};

static struct private dev_blk_ =
{
   .read_offset = ATOMIC_INIT( 0 ),
   .buf = "",
};

static struct private *dev_ptr_ = &dev_blk_;

// http://stackoverflow.com/questions/13787726/how-to-use-wake-up-interruptible
static DECLARE_WAIT_QUEUE_HEAD( q_wait_ );

////////////////////////////////////////////////

static ssize_t read( struct file *file, char *buf,
		size_t count, loff_t *ppos )
{
	// fixme: it's thread-safe? I think races here
	int rd_len = 0;
	int offset = atomic_read( &dev_ptr_->read_offset );
	if( offset > strlen( dev_ptr_->buf ) ){
		if( file->f_flags & O_NONBLOCK ) {
			return -EAGAIN;
		}
	}
	// it's strange...
	offset = atomic_read( &dev_ptr_->read_offset );
	if( offset == strlen( dev_ptr_->buf ) ){
		atomic_set( &dev_ptr_->read_offset, offset+1 );
		return -EOF;
	}

	rd_len = strlen( dev_ptr_->buf )-offset;
	if( count < rd_len ){
		rd_len = count;
	}
	if( copy_to_user( buf, dev_ptr_->buf+offset, rd_len ) ){
		return -EFAULT;
	}

	atomic_set( &dev_ptr_->read_offset, offset+rd_len );
	return rd_len;
}

static ssize_t write( struct file *file,
		const char *buf, size_t count, loff_t *ppos )
{
   int res = 0;
   int len = LEN_MSG;
   if( count < LEN_MSG ){
	   len = count;
   }

   res = copy_from_user( dev_ptr_->buf, (void*)buf, len );

   dev_ptr_->buf[ len ] = '\0';
   if( '\n' != dev_ptr_->buf[ len-1 ] ){
      strcat( dev_ptr_->buf, "\n" );
   }

   // read enable
   // set offset to start
   atomic_set( &dev_ptr_->read_offset, 0 );

   wake_up_interruptible( &q_wait_ );
   return len;
}

static unsigned int poll( struct file *file,
		struct poll_table_struct *poll_table_ptr )
{
   int flag = POLLOUT | POLLWRNORM;

   // call for on wait queue
   // fixme: как возвращается в залоченное состояние?
   // http://stackoverflow.com/questions/30234496/why-do-we-need-to-call-poll-wait-in-poll
   // fixme: я не понимаю как это работает
   //
   // https://blackfin.uclinux.org/doku.php?id=linux-kernel:polling
   //
   // https://groups.yahoo.com/neo/groups/linux-bangalore-programming/conversations/topics/7527
   // http://tali.admingilde.org/dhwk/vorlesung/ar01s08.html
   // https://www.quora.com/How-often-does-poll_wait-return-if-there-is-data-available-to-read
   //
   // fixme: как происходит чтение?
   poll_wait( file, &q_wait_, poll_table_ptr );

   // flags for read
   if( atomic_read( &dev_ptr_->read_offset ) <= strlen( dev_ptr_->buf ) ) {
      flag |= ( POLLIN | POLLRDNORM );
   }

   return flag;
}

////////////////////////////////////////////////

static int stream_dvr_init(void)
{
	major_n_ = register_chrdev(0, DEVICE_NAME, &fops_);
	if( major_n_ < 0 ){
		return major_n_;
	}

	sdvr_cls_ = class_create( THIS_MODULE, CLASS_NAME );
	if( IS_ERR( sdvr_cls_ ) ){
		class_destroy( sdvr_cls_ );
		unregister_chrdev( major_n_, DEVICE_NAME );
		return PTR_ERR( sdvr_cls_ );
	}

	sdvr_dev_ = device_create( sdvr_cls_, NULL,
			MKDEV(major_n_, 0), NULL, DEVICE_NAME );
	if( IS_ERR( sdvr_dev_ ) ){
		class_destroy( sdvr_cls_ );
		unregister_chrdev( major_n_, DEVICE_NAME );
		return PTR_ERR( sdvr_dev_ );
	}

	printk( KERN_INFO "StreamDvr: loaded" );

	return 0;
}

static void stream_dvr_exit(void)
{
	device_destroy( sdvr_cls_, MKDEV( major_n_, 0 ) );
	class_unregister( sdvr_cls_ );
	class_destroy( sdvr_cls_ );
	unregister_chrdev( major_n_, DEVICE_NAME );
	printk(KERN_INFO "StreamDvr: unloaded\n");
}

////////////////////////////////////////////////

module_init(stream_dvr_init);
module_exit(stream_dvr_exit);
