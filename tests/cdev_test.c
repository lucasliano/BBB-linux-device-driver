#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#define PACKET_NUMBER 14*2


/// @brief Test basic functionality of the ioctl() command
int main(void) {
    int fd;
    char buff[PACKET_NUMBER];
    ssize_t retval;
    unsigned long tmp;
    double raw_accel_xout, raw_accel_yout, raw_accel_zout, raw_gyro_xout, raw_gyro_yout, raw_gyro_zout, raw_temp_out, mva_accel_xout = 0, mva_accel_yout = 0, mva_accel_zout = 0, mva_gyro_xout = 0, mva_gyro_yout = 0, mva_gyro_zout = 0, mva_temp_out = 0;
    float acc_modifier, gyro_modifier;


    printf("Opening cdev..\n");
    if ((fd = open("/dev/MPU6050", O_RDWR)) == -1) {
        perror("Error while opening.\n");
        return -1;
    }
    printf("File was successfully opened.\n");

    printf("Reading...\n");
    retval = read(fd, buff, sizeof(buff));
    printf("We read %d bytes.\n", retval);



    if((acc_modifier = ioctl(fd,0)/1) == -1) 
    {
        perror("Error while getting acc_modifier "); 
        goto error;
    }
    
    if((gyro_modifier = ioctl(fd,1)/10) == -1) 
    {
        perror("Error while getting gyro_modifier ");
        goto error;
    }

    

    raw_accel_xout = (float) ((short int)((buff[0] << 8) | buff[1]) / acc_modifier);
    raw_accel_yout = (float) ((short int)((buff[2] << 8) | buff[3]) / acc_modifier);
    raw_accel_zout = (float) ((short int)((buff[4] << 8) | buff[5]) / acc_modifier);

    raw_temp_out = (float) (((short int)((buff[6] << 8) | buff[7]) / 340) + 36.53);

    raw_gyro_xout = (float) ((short int)((buff[8] << 8)  | buff[9])  / gyro_modifier);
    raw_gyro_yout = (float) ((short int)((buff[10] << 8) | buff[11]) / gyro_modifier);
    raw_gyro_zout = (float) ((short int)((buff[12] << 8) | buff[13]) / gyro_modifier);


    printf("ACC X: %f\n", raw_accel_xout);
    printf("ACC Y: %f\n", raw_accel_yout);
    printf("ACC Z: %f\n", raw_accel_zout);
    printf("\n");
    printf("TEMP: %f\n", raw_temp_out);
    printf("\n");
    printf("GYRO X: %f\n", raw_gyro_xout);
    printf("GYRO Y: %f\n", raw_gyro_yout);
    printf("GYRO Z: %f\n", raw_gyro_zout);

    printf("Writting...\n");
    retval = write(fd, buff, 5);
    printf("We wrote %d bytes.\n", retval);

    error:
    printf("Closing cdev..\n");
    close(fd);
    printf("File was successfully closed.\n");

    return 0;
}
