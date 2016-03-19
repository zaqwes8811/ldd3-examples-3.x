# /usr/src/linux-headers-$(uname -r)/include
#
# compile ub kernel
#
# Comment/uncomment the following line to disable/enable debugging
#DEBUG = y

#all:
#	echo


# Add your debugging flag (or not) to EXTRA_CFLAGS
ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DSBULL_DEBUG # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

EXTRA_CFLAGS += $(DEBFLAGS)
EXTRA_CFLAGS += -I..

ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m	:= stream_dvr.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

clean:
	rm -rf *.o *~ .tmp_versions Module.symvers modules.order .*.cmd *.ko *.mod.c

app:
	gcc main.c
	sudo ./a.out
	
reload_dvr:
	sudo rmmod stream_dvr
	sudo insmod stream_dvr.ko
	
	
	