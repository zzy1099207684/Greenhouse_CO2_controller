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

extern "C" {
uint32_t read_runtime_ctr(void) {
    return timer_hw->timerawl;
}
}


void wifi_init(void *param) {
    printf("wifi_init start\n");
    thing_speak_service::wifi_init(param);
    vTaskDelete(nullptr); // delete self task
}

// controller scan wifi ssid task example
void controller_part_scan_wifi(void *param) {
    while(1) {
        printf("controller scan wifi start\n");
        auto *ts = static_cast<thing_speak *>(param);
        EventBits_t wifi_init = xEventGroupWaitBits(ts->get_co2_wifi_scan_event_group(), WIFI_INIT_BIT, pdFALSE, pdTRUE,
                                                    portMAX_DELAY); // wait for wifi init success
        if (wifi_init & WIFI_INIT_BIT) {
            thing_speak_service::scan_wifi_ssid_arr(ts); // start scan wifi ssid

            EventBits_t wifi_scan_done = xEventGroupWaitBits(ts->get_co2_wifi_scan_event_group(), WIFI_SCAN_DONE_BIT,
                                                             pdTRUE, pdTRUE,
                                                             portMAX_DELAY);
            if (wifi_scan_done & WIFI_SCAN_DONE_BIT) {
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
        vTaskDelay(pdMS_TO_TICKS(30000)); // scan every 30s
    }
}


int main() {
    stdio_init_all();

    thing_speak ts;
    EventGroupHandle_t wifi_init_success_group = xEventGroupCreate();
    ts.set_co2_wifi_scan_event_group(wifi_init_success_group); // set event group

    xTaskCreate(wifi_init, "wifi_init", 256, &ts, 1, nullptr); // create wifi init task

    // controller part scan wifi ssid task example
    xTaskCreate(controller_part_scan_wifi, "controller_part_scan_wifi", 256, &ts, 1, nullptr);

    // set wifi ssid and pwd
    ts.set_ssid("Redmi_138D");
    ts.set_pwd("zzyzmy20272025888");

    xTaskCreate(thing_speak_service::start, "start", 256, &ts, 1, nullptr); // create wifi init task

    vTaskStartScheduler();
    return 0;
}