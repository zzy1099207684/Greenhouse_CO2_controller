//
// Created by zhiyo on 2025/9/21.
//

#ifndef THINK_SPEAK_SERVICE_H
#define THINK_SPEAK_SERVICE_H

#
#include "../entry/thing_speak.h"
#include <mbedtls/debug.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "task.h"
#include <timers.h>


class thing_speak_service {
public:
    // get SETTING CO2 data from thing speak
    static void get_SETTING_CO2_data(TimerHandle_t xTimer);
    static void deal_SETTING_CO2_data(void *param);
    // upload sensor data to thing speak
    static void upload_data_to_thing_speak(TimerHandle_t xTimer);
    static void wifi_init_once(void *param);
    static void request_HTTPS(void *param);

};


#endif //THINK_SPEAK_SERVICE_H
