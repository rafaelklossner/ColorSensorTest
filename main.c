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
static int sensorConnected = 0;
static int sensorRunning = 0;

/**
 * @brief write8
 * @param reg
 * @param value
 */
void write8(uint8_t reg, uint8_t value){
    reg = reg | TCS34725_COMMAND_BIT;
    uint8_t buffer[2] = {reg, value};
    if (write(i2c_fd, buffer, 2) != 2){
        perror("write error!");
    }
}

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

void getData (uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c){
    if (sensorRunning == 1){
        printf("2.1\n");
        uint8_t cBuffer[2] = {0};
        uint8_t rBuffer[2] = {0};
        uint8_t gBuffer[2] = {0};
        uint8_t bBuffer[2] = {0};
        printf("2.2\n");
        cBuffer[0] = read8(TCS34725_CDATAL);
        printf("2.3\n");
        usleep(100);
        cBuffer[1] = read8(TCS34725_CDATAH);
        printf("2.4\n");
        *c = ( (cBuffer[1] << 8) | cBuffer[0] );
        printf("2.5\n");

        rBuffer[0] = read8(TCS34725_RDATAL);
        usleep(100);
        rBuffer[1] = read8(TCS34725_RDATAH);
        *r = ( (rBuffer[1] << 8) | rBuffer[0] );

        gBuffer[0] = read8(TCS34725_GDATAL);
        usleep(100);
        gBuffer[1] = read8(TCS34725_GDATAH);
        *g = ( (gBuffer[1] << 8) | gBuffer[0] );

        bBuffer[0] = read8(TCS34725_BDATAL);
        usleep(100);
        bBuffer[1] = read8(TCS34725_BDATAH);
        *b = ( (bBuffer[1] << 8) | bBuffer[0] );
    }else{
        printf("Sensor not ready --> call sensorInit() and startSensor() first\n");
    }
}

void startSensor(void){
    if (sensorConnected == 1){
        /* power on */
        write8(TCS34725_ENABLE, TCS34725_ENABLE_PON);
        usleep(500);
        /* start messure */
        write8(TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
        /* set status on running */
        sensorRunning = 1;
    }
}

void stopSensor(void){
    if (sensorConnected == 1){
        /* Turn the device off to save power */
        uint8_t reg = 0;
        reg = read8(TCS34725_ENABLE);
        write8(TCS34725_ENABLE, reg & ~(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN));
        /* set status on stop */
        sensorRunning = 0;
    }
}

/**
 * @brief configSensor
 */
void configSensor(void){
    /* set integration time */
    write8(TCS34725_ATIME, TCS34725_INTEGRATIONTIME_101MS);
    /* set gain */
    write8(TCS34725_CONTROL, TCS34725_GAIN_1X);
}

/**
 * @brief initializes sensor
 */
uint8_t initSensor(void){
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

    /* check if connection ready */
    uint8_t checkValue = 0x77;
    printf("init 1\n");
    uint8_t oldValue = read8(TCS34725_WTIME);
    printf("init 2\n");
    usleep(100);
    write8(TCS34725_WTIME, checkValue);
    printf("init 3\n");
    usleep(100);
    uint8_t newValue = read8(TCS34725_WTIME);
    printf("init 4\n");
    usleep(100);
    write8(TCS34725_WTIME, oldValue);
    printf("init 5\n");
    if(oldValue != newValue && checkValue == newValue){
        sensorConnected = 1;
        printf("sensor is now connected\n");
        return 0;
    }else{
        printf("no connection\n");
        return 1;
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
    int i = 100;
    uint16_t r,g,b,c;

    printf("Starting Application\n");
    if(initSensor() == 0){
        configSensor();
        startSensor();

        while (i > 0) {
            printf("1\n");
            usleep(200000);
            printf("2\n");
            getData(&r, &g, &b, &c);
            printf("3\n");
            printf("r: %d, g: %d, b: %d, c: %d\n", r, g, b, c);
            printf("4\n");
            i--;
            printf("5\n");
        }

        stopSensor();
        deinitSensor();
    }
    return 0;
}




