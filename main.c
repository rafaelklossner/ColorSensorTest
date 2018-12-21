#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sensor.h>

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




