#ifndef _HARDWARE_H_
#define _HARDWARE_H_
struct weather {
    float temperature, humidity;
};

static struct weather current_weather;

struct weather getCurrentWeather();

#endif
