//
// Created by zhiyo on 2025/9/21.
//

#ifndef THINK_SPEAK_SERVICE_H
#define THINK_SPEAK_SERVICE_H

#include "FreeRTOS.h"
#include "../entry/thing_speak.h"
#include <mbedtls/debug.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mdns.h"
#include "task.h"
#include <cstring>
#include "Tools/json/json_handler.h"
#include <timers.h>



class thing_speak_service {
public:
    // get SETTING CO2 data from thing speak
    static void get_SETTING_CO2_data(TimerHandle_t xTimer);
    static void deal_SETTING_CO2_data(void *param);
    // upload sensor data to thing speak
    static void upload_data_to_thing_speak(TimerHandle_t xTimer);
    static void wifi_connect(void *param);
    static void request_HTTPS(void *param);
    static void start(void *param);
    static void scan_wifi_ssid_arr(void *param);

};


#endif //THINK_SPEAK_SERVICE_H
