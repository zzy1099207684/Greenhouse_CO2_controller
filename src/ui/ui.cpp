//
// Created by Riina on 25/09/2025.
//

#include "ui.h"

UI_control::UI_control(QueueHandle_t queue,std::shared_ptr<PicoI2C> i2cbus): controller_queue(queue), display(i2cbus){
  display.fill(0);
  display.show();
  CO2_level=NULL;
  Relative_humidity=NULL;
  Temperature=NULL;
  fan_speed=NULL;
}

int UI_control::get_CO2_level(){
  return CO2_level;
}

void UI_control::set_CO2_level(int new_level){
  CO2_level = new_level;
}

void UI_control::set_Relative_humidity(int new_humidity){
  Relative_humidity = new_humidity;
}

void UI_control::set_Temperature(int new_temperature){
  Temperature = new_temperature;
}

void UI_control::set_fan_speed(int new_speed){
 fan_speed = new_speed;
}

void UI_control::update_ui() {
  //TODO update OLED
}

void UI_control::read_input() {
  //TODO task to read rot
}

void ui_control::input_handle() {
  //TODO task to handle rot inputs
}