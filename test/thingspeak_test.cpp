//
// Created by zhiyo on 2025/10/4.
//

#include "FreeRTOS.h"
#include <cstdio>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "task.h"
#include "network/entry/thing_speak.h"
#include "network/service/thing_speak_service.h"
#include "pico/cyw43_arch.h"

#define NETWORK_SET_CO2 (1<<4) //co2 setting from network
#define WIFI_INIT (1 << 5) // wifi_init_success
#define WIFI_SCAN_DONE (1 << 6) // wifi_scan_done

extern "C" {
uint32_t read_runtime_ctr(void) {
    return timer_hw->timerawl;
}
}

typedef struct {
    thing_speak *ts;
    thing_speak_service *ts_service;
} TestStruct;


void wifi_init(void *param) {
    auto *ts_struct = static_cast<TestStruct *>(param);
    auto *ts = ts_struct->ts;
    auto *ts_service = ts_struct->ts_service;
    printf("wifi_init start\n");
    ts_service->network_init(ts);
    vTaskDelete(nullptr); // delete self task
}

// controller scan wifi ssid task example
void controller_part_scan_wifi(void *param) {
    auto *ts_struct = static_cast<TestStruct *>(param);
    auto *ts = ts_struct->ts;
    auto *ts_service = ts_struct->ts_service;
    printf("controller scan wifi start\n");
    EventBits_t wifi_init = xEventGroupWaitBits(ts->get_co2_wifi_scan_event_group(), WIFI_INIT, pdFALSE, pdTRUE,
                                                portMAX_DELAY); // wait for wifi init success
    if (wifi_init & WIFI_INIT) {
        ts_service->scan_wifi_ssid_arr(ts);

        EventBits_t wifi_scan_done = xEventGroupWaitBits(ts->get_co2_wifi_scan_event_group(), WIFI_SCAN_DONE,
                                                         pdTRUE, pdTRUE,
                                                         portMAX_DELAY);
        if (wifi_scan_done & WIFI_SCAN_DONE) {
            // UI get ssid array
            auto ssids = ts->get_wifi_scan_result(); // get wifi ssid array
            // just for test
            for (int i = 0; i < 9; i++) {
                if (ssids[i][0] != '\0') {
                    printf("%s\n", ssids[i]);
                }
            }
        }
        printf("controller scan wifi ssid end\n");
    }
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}

// test set ssid and password task example
void set_ssid_pass(void *param) {
    auto ts_struct = static_cast<TestStruct *>(param);
    auto ts = ts_struct->ts;
    printf("set ssid and password start, waiting 50s\n");
    ts->set_ssid("Redmi_138D");
    ts->set_pwd("zzyzmy20272025888");
    printf("set ssid and password end222\n");
    vTaskResume(*ts->get_wifi_connect_handle_ptr());
    printf("break connecting wifi 30s later\n");
    EventBits_t t =  xEventGroupWaitBits(ts->get_co2_wifi_scan_event_group(),
                                        WIFI_CONNECTED, pdTRUE,
                                        pdTRUE, portMAX_DELAY);
    if(t & WIFI_CONNECTED) {
        printf("WiFi connected000\n");
        vTaskDelay(pdMS_TO_TICKS(50000));
    }
    printf("input error pass to reconnect\n");
    ts->set_ssid("Redmi_138D");
    ts->set_pwd("zzyzmy2");
    printf("Leave WiFi...\n");

    thing_speak_service::wifi_disconnect();
    printf("second try connect WiFi with wrong pass!!!!!!!!!!!!!!\n");
    vTaskResume(*ts->get_wifi_connect_handle_ptr());
    t =  xEventGroupWaitBits(ts->get_co2_wifi_scan_event_group(),
                                        WIFI_CONNECTED, pdTRUE,
                                        pdTRUE, pdMS_TO_TICKS(30000));
    if(t & WIFI_CONNECTED) {
        printf("WiFi connected111\n");
    }else {
        printf("Failed to connect WiFi with wrong pass\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    printf("finally try connect Wifi with correct pass !!!!!!!!!!!!!!!!\n");
    printf("input correct pass to reconnect\n");
    ts->set_ssid("Redmi_138D");
    ts->set_pwd("zzyzmy20272025888");
    vTaskResume(*ts->get_wifi_connect_handle_ptr());
    t =  xEventGroupWaitBits(ts->get_co2_wifi_scan_event_group(),
                                        WIFI_CONNECTED, pdTRUE,
                                        pdTRUE, portMAX_DELAY);
    if(t & WIFI_CONNECTED) {
        printf("WiFi connected222\n");
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}

void wifi_connect_task(void *param) {
    auto *ts = static_cast<thing_speak *>(param);
    thing_speak_service::wifi_connect(ts);
}


int main() {
    stdio_init_all();

    static thing_speak ts;
    static EventGroupHandle_t wifi_init_success_group = xEventGroupCreate();
    static SemaphoreHandle_t net_mutex = xSemaphoreCreateMutex();
    ts.set_co2_wifi_scan_event_group(wifi_init_success_group);
    ts.set_net_mutex(net_mutex);
    static thing_speak_service ts_service;
    // static TaskHandle_t wifi_connect_handle;
    static TestStruct ts_struct{&ts, &ts_service};


    // set event group

    xTaskCreate(wifi_init, "wifi_init", 512, &ts_struct, tskIDLE_PRIORITY + 2, nullptr); // create wifi init task

    xTaskCreate(wifi_connect_task, "wifi_connect", 2048, &ts, tskIDLE_PRIORITY + 1, ts.get_wifi_connect_handle_ptr());

    xTaskCreate(controller_part_scan_wifi, "controller_part_scan_wifi", 1024, &ts_struct, tskIDLE_PRIORITY + 2, nullptr);

    xTaskCreate(set_ssid_pass, "set_ssid_pass", 2048, &ts_struct, tskIDLE_PRIORITY + 1, nullptr);
    // ts.set_ssid("Redmi_138D");
    // ts.set_pwd("zzyzmy20272025888");
    // controller part scan wifi ssid task example
    ts.set_ssid("Redmi");
    ts.set_pwd("zzyzmy");
    xTaskCreate(thing_speak_service::start, "timer_start", 1024, &ts, tskIDLE_PRIORITY + 1, nullptr);


    vTaskStartScheduler();
    return 0;
}
