#include "FreeRTOS.h"
#include <cstdio>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "task.h"
#include "entry/thing_speak.h"
#include "service/thing_speak_service.h"

extern "C" {
uint32_t read_runtime_ctr(void) {
    return timer_hw->timerawl;
}
}


void wifi_init(void *param) {
    printf("wifi_init start\n");
    auto *ts = static_cast<thing_speak *>(param);
    thing_speak_service::wifi_init();
    xEventGroupSetBits(ts->get_co2_wifi_scan_event_group(), WIFI_INIT_BIT ); // set wifi init success bit
    vTaskDelete(nullptr); // delete self task
}

// controller scan wifi ssid task example
void controller_scan_wifi(void *param) {
    printf("controller scan wifi start\n");
    auto *ts = static_cast<thing_speak *>(param);
    EventBits_t b = xEventGroupWaitBits(ts->get_co2_wifi_scan_event_group(), WIFI_INIT_BIT, pdFALSE, pdTRUE, portMAX_DELAY); // wait for wifi init success
    if (b & WIFI_INIT_BIT) {
        // controller scan wifi ssid
        thing_speak_service::scan_wifi_ssid_arr(param);
        auto ssids = ts->get_wifi_scan_result(); // wifi scan
        if (ts->get_wifi_ssid_index() == -1) {
            for (int i = 0; i < 9; i++) {
                if (ssids[i][0] != '\0') {
                    printf("%s\n", ssids[i]);
                }
            }
        }
        printf("controller scan wifi ssid end\n");
    }
}


int main() {
    stdio_init_all();

    thing_speak ts;
    // create a event group for co2 level change notification
    EventGroupHandle_t wifi_init_success_group = xEventGroupCreate();
    ts.set_co2_wifi_scan_event_group(wifi_init_success_group); // set event group

    xTaskCreate(wifi_init, "wifi_init", 256, &ts, 1, nullptr); // create wifi init task

    xTaskCreate(controller_scan_wifi, "controller_scan_wifi", 256, &ts, 1, nullptr); // controller scan wifi ssid task

    ts.set_ssid("Redmi_138D");
    ts.set_pwd("zzyzmy20272025888");

    // xTaskCreate(thing_speak_service::start, "wifi_init", 256, &ts, 1, nullptr); // create wifi init task

    vTaskStartScheduler();


    // xTaskCreate(thing_speak_service::start, "thing_speak_service::start", 256, &ts, 2, nullptr); // create thing speak service task

    for (;;) {}
}
