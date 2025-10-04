//
// Created by zhiyo on 2025/9/21.
//

#ifndef THINK_SPEAK_SERVICE_H
#define THINK_SPEAK_SERVICE_H




class thing_speak_service {
public:
    // get SETTING CO2 data from thing speak
    static void get_SETTING_CO2_data(TimerHandle_t xTimer);
    void deal_SETTING_CO2_data(void *param);
    static void deal_SETTING_CO2_data_wrapper(void *param);
    // upload sensor data to thing speak
    static void upload_data_to_thing_speak(TimerHandle_t xTimer);
    static void wifi_connect(void *param);
    static void request_HTTPS(void *param);
    void start(void *param);
    void scan_wifi_ssid_arr(void *param);
    static void wifi_init(void *param);
};


#endif //THINK_SPEAK_SERVICE_H
