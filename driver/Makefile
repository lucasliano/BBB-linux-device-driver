KERNEL_SOURCE = /lib/modules/$(shell uname -r)/build
DRIVER_NAME = i2c_lliano
MOD_NAME = i2c_lucas

# Converts a module object in LKM
obj-m += $(MOD_NAME).o
EXTRA_CFLAGS := -I$(src)/inc

$(MOD_NAME)-objs := src/lucas_lkm.o src/i2c.o src/char_device.o src/MPU6050.o



# Compila el modulo haciendo una llamada al Makefile que esta en '/lib/modules/$(shell uname -r)/build'
all: softclean build insmod

build:
	make -C ${KERNEL_SOURCE} M=${PWD} modules

# Limpia todos los archivos objetos
softclean:
	make -C ${KERNEL_SOURCE} M=${PWD} clean

clean: rmmod softclean

# Revisa si el modulo esta instalado
cat:
	cat /proc/modules | grep $(MOD_NAME)

# Instala el modulo
insmod:
	sudo insmod $(MOD_NAME).ko

# Desinstala el modulo
rmmod:
	sudo rmmod $(MOD_NAME).ko

# Muestra los mensajes (dmesg), en el ejemplo todos los printk imprimen el nombre del archivo
dmesg:
	dmesg | grep $(DRIVER_NAME) 

awk:
	cat /proc/devices | grep $(MOD_NAME)

tail:
	tail -f /var/log/syslog

info:
	modinfo $(MOD_NAME).ko

dev:
	ls -l /dev/	| grep $(MOD_NAME)