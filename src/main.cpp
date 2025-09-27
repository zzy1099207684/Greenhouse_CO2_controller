#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

extern "C" {
    uint32_t read_runtime_ctr(void) {
        return timer_hw->timerawl;
    }
}


#define LED_PIN 22  // GPIO 25 or change to your LED pin

void blink_task(void *pvParameters) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1) {
        gpio_put(LED_PIN, 1);                    // LED on
        vTaskDelay(pdMS_TO_TICKS(500));          // 500ms delay

        gpio_put(LED_PIN, 0);                    // LED off
        vTaskDelay(pdMS_TO_TICKS(500));          // 500ms delay
    }
}

int main() {
    stdio_init_all();

    printf("Starting GPIO blinker...\n");

    xTaskCreate(blink_task, "Blink", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    return 0;
}
