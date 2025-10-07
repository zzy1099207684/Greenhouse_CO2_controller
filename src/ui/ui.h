//
// Created by Riina on 25/09/2025.
//
#ifndef UI_H
#define UI_H
#include "FreeRTOS.h"
#include <memory>
#include <string.h>
#include <cstring>
#include <event_groups.h>
#include "task.h"
#include "queue.h"
#include "ssd1306os.h"
#include "pico/stdlib.h"

enum class UIState{
  MAIN,
  SETTING_MENU,
  SET_CO2,
  NETWORK_SETTINGS
};

enum class InputMode{
  ScrollSSID,
  ManualSSID,
  Password
};

enum class CharSet{
  Lowercase,
  Uppercase,
  Symbols,
  Numbers
};
enum class gpioType{
  BUTTON1,
  BUTTON2,
  BUTTON3,
  ROT_SWITCH,
  ROT_ENCODER
};

struct gpioEvent{
  gpioType type;
  int direction;
  TickType_t timestamp;
};

class UI_control {
public:
  UI_control(const std::shared_ptr<PicoI2C> &i2cbus, EventGroupHandle_t group);
  static void gpio_callback(uint gpio, uint32_t events);
  void init();

  int get_CO2_level();
  char* get_ssid();
  char* get_password();

  void set_CO2_level(int CO2_level);
  void set_Relative_humidity(float Relative_humidity);
  void set_Temperature(float Temperature);
  void set_fan_speed(int Fan_speed);
  void set_ssid_list(const char* list[]);
  void set_network_status(bool status);
  void set_CO2_alarm(bool is_Emergency);

  void display_main();
  void display_menu();
  void display_set_co2();
  void display_successfull_set_co2();
  void display_network();
  void display_successfull_set_network();

  void handle_menu_event(const gpioEvent &event);
  void handle_set_co2_event(const gpioEvent &event);
  void handle_network_scroll(const gpioEvent &event);
  void handle_network_manual(const gpioEvent &event, char *buffer);


  void run();
  static void runner(void *params);


private:
  UIState current_state;
  InputMode input_mode;
  CharSet char_set;
  gpioEvent gpio_event;

  int menu_index=0;
  int co2SetPoint=700;

  //Sensor values
  int co2_level=0;
  float Relative_humidity=0.0;
  float Temperature=0.0;
  int fan_speed=0;
  bool co2_alarm=false;

  //Network info
  char ssid[64];
  char password[64];
  int network_cursor=0;
  bool editing_ssid=true;
  int ssid_list_index=0;
  bool connected_to_network=false;

  char alphabet_lower[27];
  char alphabet_upper[27];
  char alphabet_symbols[26];
  char alphabet_digits[11];
  char ssid_list[10][64];
  int ssid_list_count;
  bool needs_update = true;
  TaskHandle_t task_handle;
  std::shared_ptr<PicoI2C> i2c_bus;
  ssd1306os* display;
  QueueHandle_t input_queue;
  EventGroupHandle_t event_group;
  static UI_control *instance_ptr;
};

#endif //UI_H