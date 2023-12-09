#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

/// @brief Test basic functionality of the ioctl() command
int main(void) {
    int fd;
    char buff[50];
    ssize_t retval;
    unsigned long tmp;

    printf("Opening cdev..\n");
    if ((fd = open("/dev/MPU6050", O_RDWR)) == -1) {
        perror("Error while opening.\n");
        return -1;
    }
    printf("File was successfully opened.\n");

    printf("Reading...\n");
    retval = read(fd, buff, sizeof(buff));
    printf("We read %d bytes.\n", retval);


    printf("Reading...\n");
    retval = write(fd, buff, 5);
    printf("We wrote %d bytes.\n", retval);

    printf("IOCTL...\n");
	ioctl(fd, 0, &tmp);
    printf("End IOCTL.\n");


    printf("Closing cdev..\n");
    close(fd);
    printf("File was successfully closed.\n");

	return 0;
}
