#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "entry/thing_speak.h"
#include "service/thing_speak_service.h"

extern "C" {
    uint32_t read_runtime_ctr(void) {
        return timer_hw->timerawl;
    }
}


#define LED_PIN 22  // GPIO 25 or change to your LED pin

// void blink_task(void *pvParameters) {
//     gpio_init(LED_PIN);
//     gpio_set_dir(LED_PIN, GPIO_OUT);
//
//     while (1) {
//         gpio_put(LED_PIN, 1);                    // LED on
//         vTaskDelay(pdMS_TO_TICKS(500));          // 500ms delay
//
//         gpio_put(LED_PIN, 0);                    // LED off
//         vTaskDelay(pdMS_TO_TICKS(500));          // 500ms delay
//     }
// }

int main() {
    stdio_init_all();

    // create a event group for co2 level change notification
    EventGroupHandle_t co2_event_group = xEventGroupCreate();

    thing_speak ts;
    ts.set_co2_event_group(co2_event_group); // set event group
    thing_speak_service::scan_wifi_ssid_arr(&ts); // scan wifi ssid array
    auto ssids = ts.get_wifi_scan_result();// get scan result

    ts.set_ssid(ssids[0]); // set ssid
    ts.set_pwd("WIFI_PASSWORD"); // set wifi password

    thing_speak_service::wifi_connect(&ts); // connect to wifi

    thing_speak_service::start(&ts); // start thing speak service tasks

    vTaskStartScheduler();


    
    return 0;
}