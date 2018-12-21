/*TODO
 * Extract functions to sensor.c
 * Add declarations of local an global functions
 * Solve i2c-4 problem --> must touch display to continue (display also on i2c-4 bus)
 */

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
 * @brief write a byte to color sensor
 * @param reg register to write to
 * @param value value to write in register
 */
void write8(uint8_t reg, uint8_t value){
    /* set command bit to specify command */
    reg = reg | TCS34725_COMMAND_BIT;
    /* copy in buffer and send it */
    uint8_t buffer[2] = {reg, value};
    if (write(i2c_fd, buffer, 2) != 2){
        perror("write error!");
    }
    usleep(100);
}

/**
 * @brief read a byte from color sensor
 * @param reg register to read from
 * @return reveived contempt of register
 */
uint8_t read8(uint8_t reg){
    uint8_t receive = 0;
    /* set command bit to specify command */
    reg = reg | TCS34725_COMMAND_BIT;
    /* write register adress */
    if (write(i2c_fd, &reg, 1) != 1){
        perror("write error!");
    }
    /* receive register value */
    if (read(i2c_fd, &receive, 1) != 1){
        perror("read error!");
    }
    usleep(100);
    return receive;
}

/**
 * @brief test the ability to read a value from register
 * @param reg register to read from
 */
void readTest(uint8_t reg){
    uint8_t receive = 0;
    /* set command bit to specify command */
    reg = reg | TCS34725_COMMAND_BIT;
    /* set command bit to specify command */
    if (write(i2c_fd, &reg, 1) != 1){
        perror("write error!");
    }
    /* receive register value */
    if (read(i2c_fd, &receive, 1) != 1){
        perror("read error!");
    }
    printf("Received value is: %d\n", receive);
}

/**
 * @brief get data from all registers
 * @param r red pointer
 * @param g green pointer
 * @param b blue pointer
 * @param c clear pointer
 */
void getData (uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c){
    /* check if sensor is running */
    if (sensorRunning == 1){
        uint8_t cBuffer[2] = {0};
        uint8_t rBuffer[2] = {0};
        uint8_t gBuffer[2] = {0};
        uint8_t bBuffer[2] = {0};

        /* receive and process data */
        cBuffer[0] = read8(TCS34725_CDATAL);
        cBuffer[1] = read8(TCS34725_CDATAH);
        *c = ( (cBuffer[1] << 8) | cBuffer[0] );

        rBuffer[0] = read8(TCS34725_RDATAL);
        rBuffer[1] = read8(TCS34725_RDATAH);
        *r = ( (rBuffer[1] << 8) | rBuffer[0] );

        gBuffer[0] = read8(TCS34725_GDATAL);
        gBuffer[1] = read8(TCS34725_GDATAH);
        *g = ( (gBuffer[1] << 8) | gBuffer[0] );

        bBuffer[0] = read8(TCS34725_BDATAL);
        bBuffer[1] = read8(TCS34725_BDATAH);
        *b = ( (bBuffer[1] << 8) | bBuffer[0] );
    }else{ /* if not print a message to console */
        printf("Sensor not running --> call sensorInit() and startSensor() first\n");
    }
}

/**
 * @brief power on sensor and start messurement
 */
void startSensor(void){
    /* check if sensor is connected */
    if (sensorConnected == 1){
        /* power on */
        write8(TCS34725_ENABLE, TCS34725_ENABLE_PON);
        /* start messure */
        write8(TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
        /* set status on running */
        sensorRunning = 1;
    }else{ /* if not print a message to console */
        printf("Sensor not connected --> call sensorInit() first\n");
    }
}

/**
 * @brief stop messurement and let the sensor sleep
 */
void stopSensor(void){
    /* Turn the device off to save power */
    uint8_t reg = 0;
    reg = read8(TCS34725_ENABLE);
    write8(TCS34725_ENABLE, reg & ~(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN));
    /* set status on stop */
    sensorRunning = 0;
}

/**
 * @brief configurate integration time and gain for messurement
 */
void configSensor(void){
    /* set integration time */
    write8(TCS34725_ATIME, TCS34725_INTEGRATIONTIME_101MS);
    /* set gain */
    write8(TCS34725_CONTROL, TCS34725_GAIN_1X);
}

/**
 * @brief initialize sensor
 * @return status -1=error (do not start program), 1=okay
 */
int8_t initSensor(void){
    int8_t status = 0;
    /* check if sensor is not already connected */
    if (sensorConnected == 0){
        printf("init sensor\n");

        /* Open the Linux i2c device */
        i2c_fd = open("/dev/i2c-4", O_RDWR);
        if (i2c_fd < 0){
            perror("i2cOpen");
            status = -1;
        }

        /* Set the I2C slave address for all subsequent I2C transfers */
        if (ioctl(i2c_fd, I2C_SLAVE, TCS34725_ADDRESS) < 0){
            perror("i2cSetAddress");
            status = -1;
        }

        /* check if connection ready */
        uint8_t checkValue = 0x77;
        uint8_t oldValue = read8(TCS34725_WTIME);
        write8(TCS34725_WTIME, checkValue);
        uint8_t newValue = read8(TCS34725_WTIME);
        write8(TCS34725_WTIME, oldValue);
        if(oldValue != newValue && checkValue == newValue){
            /* let the program continue */
            sensorConnected = 1;
            printf("sensor is now connected\n");
            status = 1;
        }else{
            /* stop the program here */
            printf("no connection aviable\n");
            status = -1;
        }
    }else{ /* if it is print message to console */
        printf("sensor is already connected, proceed with startSensor()\n");
        status = 1;
    }
    return status;
}

/**
 * @brief deinitializes sensor
 */
void deinitSensor(void){
    if(sensorRunning == 0){
        printf("deinit sensor\n");
        close(i2c_fd);
        sensorConnected = 0;
    }else{
        printf("sensor is still running, call stopSensor() before deinit\n");
    }
}

/**
 * @brief this is the main function
 * @return 0
 */
int main(int argc, char** argv){
    (void) argc;
    (void) argv;
    int8_t status;
    uint16_t r,g,b,c;

    printf("Starting Application Color Sensing\n");
    status = initSensor();
    /* check if initialization was successful */
    if(status == 1){
        configSensor();
        startSensor();

        /* test while */
        int i = 100;
        while (i > 0) {
            usleep(200000); /* do not forget to sleep during integration time */
            getData(&r, &g, &b, &c);
            printf("r: %d, g: %d, b: %d, c: %d\n", r, g, b, c);
            i--;
        }

        stopSensor();
        deinitSensor();
    }
    return 0;
}




