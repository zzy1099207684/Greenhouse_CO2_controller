//
// Created by zhiyo on 2025/9/21.
//

#ifndef THINK_SPEAK_SERVICE_H
#define THINK_SPEAK_SERVICE_H
#include "IPStack.h"
#define TWO_TASK 0

class thing_speak_service {
public:
    // get SETTING CO2 data from thing speak
    # if TWO_TASK
    static void get_SETTING_CO2_data(void *param);
    static void upload_data_to_thing_speak(void *param);
    # endif
    static void deal_SETTING_CO2_data(void *param);
    // upload sensor data to thing speak
    static void get_setting_co2_val_or_upload(void *param);



    static void wifi_connect(void *param);
    static void request_HTTPS(void *param);
    static void start(void *param);
    void scan_wifi_ssid_arr(void *param);
    static void network_init(void *param);
    static void wifi_disconnect();

};


#endif //THINK_SPEAK_SERVICE_H
