TARGET = gpiolcd
obj-m	:= $(TARGET).o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
#KERNELDIR :=/usr/src/linux-3.8.0/
PWD       := $(shell pwd)
CC = gcc
all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions modules.order Module.symvers

