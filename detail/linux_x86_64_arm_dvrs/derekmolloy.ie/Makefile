# 1. crosscompile eclipse
#проблемы с путями
#
#/home/zaqwes/emb-rpi/buildroot-2014.11/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-
#
#что-то скомпилировал но на плате получил секфаулит - не в этом дело
#  скомп. на прямую запускается

obj-m += ofd.o ebbchar.o


RPI_ROOT=/home/zaqwes/emb-rpi/buildroot-2014.11/output
CCPREFIX=${RPI_ROOT}/host/usr/bin/arm-buildroot-linux-uclibcgnueabi-
KERNEL_SRC=${RPI_ROOT}/build/linux-c256eb9968c8997dce47350d2075e42f1b3991d3
CC=${CCPREFIX}gcc

default:
	make ARCH=arm CROSS_COMPILE=${CCPREFIX} -C ${KERNEL_SRC} M=$(PWD) modules
	$(CC) testebbchar.c -o test
clean: 
	rm *.ko *.o *.mod.c *.order *.symvers test