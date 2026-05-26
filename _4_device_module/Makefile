obj-m += device1_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	  make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

update: sudo rmmod device1_module.ko && make && sudo insmod device1_module.ko
