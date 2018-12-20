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
static int ready = 0;

/**
 * @brief read8
 * @param reg
 * @return
 */
uint8_t read8(uint8_t reg){
    uint8_t receive = 0;
    reg = reg | TCS34725_COMMAND_BIT;
    if (write(i2c_fd, &reg, 1) != 1){
        perror("write error!");
    }
    if (read(i2c_fd, &receive, 1) != 1){
        perror("read error!");
    }
    return receive;
}

/**
 * @brief readTest
 * @param reg
 */
void readTest(uint8_t reg){
    uint8_t receive = 0;
    reg = reg | TCS34725_COMMAND_BIT;
    if (write(i2c_fd, &reg, 1) != 1){
        perror("write error!");
    }
    if (read(i2c_fd, &receive, 1) != 1){
        perror("read error!");
    }
    printf("Received value is: %d\n", receive);
}

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
    uint8_t id;

    printf("Starting Application\n");
    initSensor();
    readTest(TCS34725_ID);
    id = read8(TCS34725_ID);
    printf("ID is: %d\n", id);
    deinitSensor();
    return 0;
}




