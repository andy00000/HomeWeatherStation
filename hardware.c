#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "hardware.h"
 
#define MAX_TIMINGS 85
#define DHT_PIN 7
int data[5] = { 0, 0, 0, 0, 0 };
 

static void setCurrentWeather(float temperature, float humidity) {
    pthread_mutex_lock(&my_lock);
    current_weather.temperature = temperature;
    current_weather.humidity = humidity;
    pthread_mutex_unlock(&my_lock);
}

static weather getCurrentWeather() {
    struct weather tempWeather;
    pthread_mutex_lock(&my_lock);
    tempWeather = current_weather;
    pthread_mutex_unlock(&my_lock);
    return tempWeather;   
}

static void read_dht_data() {
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;
 
    data[0] = data[1] = data[2] = data[3] = data[4] = 0;
    
    pthread_detach(pthread_self());
    
    /* pull pin down for 18 milliseconds */
    pinMode(DHT_PIN, OUTPUT);
    digitalWrite( DHT_PIN, LOW);
    delay(18);
 
    /* prepare to read the pin */
    pinMode( DHT_PIN, INPUT);
 
    /* detect change and read data */
    for ( i = 0; i < MAX_TIMINGS; i++) {
        counter = 0;
        while ( digitalRead( DHT_PIN ) == laststate) {
            counter++;
            delayMicroseconds(1);
            if ( counter == 255 ) {
                break;
            }
        }
        laststate = digitalRead( DHT_PIN);
 
        if ( counter == 255)
            break;
 
        /* ignore first 3 transitions */
        if ((i >= 4) && (i % 2 == 0)) {
            /* shove each bit into the storage bytes */
            data[j / 8] <<= 1;
            if ( counter > 16)
                data[j / 8] |= 1;
            j++;
        }
    }
 
    /*
     * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
     * print it out if data is good
     */
 
    if ((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))){
        float h = (float)((data[0] << 8) + data[1]) / 10;
        if ( h > 100 ) {
            h = data[0];    // for DHT11
        }
        float c = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
        if ( c > 125 ) {
            c = data[2];    // for DHT11
        }
        if ( data[2] & 0x80 ) {
            c = -c;
        }
        setCurrentWeather(c, h);
        // printf( "Humidity = %.1f %% Temperature = %.1f *C \n", h, c );

    }
}

static pthread_t thr;
static pthread_mutex_t my_lock;

static void* _workerThreadProc(void* rawArg) {
    while ( 1 ) {
        read_dht_data();
        delay(1000); /* wait 2 seconds before next read */
    }

    pthread_exit(NULL);
}
 

int setup() {
    if ( wiringPiSetup() == -1 )
        return 1;
 
    if(pthread_create(&thr, NULL, _workerThreadProc, NULL) != 0) {
        printf("%s\n", "error creating thread");
        return 1;
    }
}
