# Compila el modulo haciendo una llamada al Makefile que esta en '/lib/modules/$(shell uname -r)/build'
all: build

build:
	gcc -g -Wall cdev_test.c -o cdev_test.o

clean:
	rm cdev_test.o

run:
	sudo ./cdev_test.o