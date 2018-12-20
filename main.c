#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <sensor.h>

/* private variables */
static int i2c_fd;

/**
 * @brief initializes sensor
 */
void initSensor(void){
    printf("init sensor\n");

    /* Open the Linux i2c device */
    i2c_fd = open("/dev/i2c-4", O_RDWR);
    if (i2c_fd < 0){
        perror("i2cOpen");
        exit(1);
    }

    /* Set the I2C slave address for all subsequent I2C transfers */
    if (ioctl(i2c_fd, I2C_SLAVE, TCS34725_ADDRESS) < 0){
        perror("i2cSetAddress");
        exit(1);
    }
}

/**
 * @brief deinitializes sensor
 */
void deinitSensor(void){
   printf("deinit sensor\n");
   close(i2c_fd);
}

/**
 * @brief this is the main function
 * @return 0
 */
int main(int argc, char** argv){
    (void) argc;
    (void) argv;

    printf("Starting Application\n");
    initSensor();
    deinitSensor();
    return 0;
}




