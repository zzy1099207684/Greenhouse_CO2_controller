#include <memory>
#include <PicoI2C.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ui/ui.h"
#include "event_groups.h"

extern "C" {
    uint32_t read_runtime_ctr(void) {
        return timer_hw->timerawl;
    }
}


int main() {
    stdio_init_all();
    EventGroupHandle_t event_group = xEventGroupCreate();
    auto i2cbus{std::make_shared<PicoI2C>(1, 400000)};
    UI_control ui(i2cbus, event_group);

    vTaskStartScheduler();
    while(true){};
    return 0;
}
