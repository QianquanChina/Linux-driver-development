ifneq ($(KERNELRELEASE),)
obj-m += Key_Workqu.o
else
       KERNELDIR=/opt/opt/EmbedSky/TQ210/kernel
all:
	PWD=$(shell pwd)
	$(MAKE) -C $(KERNELDIR) M=$(PWD)  
clean:	                
	rm -rf.*.cmd *.o*.mod.c *.ko.tmp_versions module* Module*
endif         
