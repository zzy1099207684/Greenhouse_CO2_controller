//
// Created by Riina on 25/09/2025.
//

#include "ui.h"

#include <task.h>

#define BTN1 9
#define BTN2 8
#define BTN3 7
#define ROT_A 10
#define ROT_B 11
#define ROT_SW 12

UI_control* UI_control::instance_ptr = nullptr;

UI_control::UI_control(const std::shared_ptr<PicoI2C> &i2cbus): input_queue(nullptr),task_handle(nullptr), display(i2cbus){
  init();

}

void UI_control::gpio_callback(uint gpio, uint32_t events) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  gpioEvent event{};

  if(gpio == ROT_A && (events & GPIO_IRQ_EDGE_RISE)){
    event.type = gpioType::ROT_ENCODER;
    event.direction = gpio_get(ROT_B) ? -1 : +1;
  } else if(gpio == ROT_SW && (events & GPIO_IRQ_EDGE_FALL)){
      event.type = gpioType::ROT_SWITCH;
  }else if(gpio == BTN1 && (events & GPIO_IRQ_EDGE_FALL)){
    event.type = gpioType::BUTTON1;
  }else if(gpio == BTN2 && (events & GPIO_IRQ_EDGE_FALL)){
    event.type = gpioType::BUTTON2;
  }else if(gpio == BTN3 && (events & GPIO_IRQ_EDGE_FALL)){
    event.type = gpioType::BUTTON3;
  }
  event.timestamp = xTaskGetTickCountFromISR();
  xQueueSendFromISR(instance_ptr->input_queue, &event, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}

void UI_control::init(){
  current_state=UIState::MAIN;
  input_mode=InputMode::ScrollSSID;
  char_set = CharSet::Lowercase;

  menu_index = 0;
  target_CO2 = 0;
  CO2_level=0;
  Relative_humidity=0.0;
  Temperature=0.0;
  fan_status=0;

  memset(ssid,0,sizeof(ssid));
  memset(password,0,sizeof(password));

  strcpy(alphabet_lower,"abcdefghijklmnopqrstuvwxyz");
  strcpy(alphabet_upper,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  strcpy(alphabet_symbols,"!?+-_ .@#$%^&*()=[]{}/,;:");
  strcpy(alphabet_digits,"0123456789");

  network_cursor=0;
  editing_ssid=false;
  ssid_list_index=0;

  input_queue = xQueueCreate(16,sizeof(gpioEvent));
  instance_ptr = this;

  gpio_init(ROT_A);
  gpio_set_dir(ROT_A, GPIO_IN);
  gpio_init(ROT_B);
  gpio_set_dir(ROT_B, GPIO_IN);
  gpio_init(ROT_SW);
  gpio_set_dir(ROT_SW, GPIO_IN);
  gpio_pull_up(ROT_SW);
  gpio_init(BTN1);
  gpio_set_dir(BTN1, GPIO_IN);
  gpio_pull_up(BTN1);
  gpio_init(BTN2);
  gpio_set_dir(BTN2, GPIO_IN);
  gpio_pull_up(BTN2);
  gpio_init(BTN3);
  gpio_set_dir(BTN3, GPIO_IN);
  gpio_pull_up(BTN3);

  gpio_set_irq_enabled_with_callback(ROT_A, GPIO_IRQ_EDGE_RISE, true,&UI_control::gpio_callback);
  gpio_set_irq_enabled(ROT_SW, GPIO_IRQ_EDGE_FALL,true);
  gpio_set_irq_enabled(BTN1, GPIO_IRQ_EDGE_FALL,true);
  gpio_set_irq_enabled(BTN2, GPIO_IRQ_EDGE_FALL,true);
  gpio_set_irq_enabled(BTN3, GPIO_IRQ_EDGE_FALL,true);
  xTaskCreate(runner, "UI control", 2048, (void*) this, tskIDLE_PRIORITY + 2, &task_handle);
}

int UI_control::get_CO2_level(){ return target_CO2;}
char* UI_control::get_ssid(){ return ssid;}
char* UI_control::get_password(){ return password;}
void UI_control::set_CO2_level(uint16_t new_level){ CO2_level = new_level;}
void UI_control::set_Relative_humidity(float new_humidity){ Relative_humidity = new_humidity;}
void UI_control::set_Temperature(float new_temperature){ Temperature = new_temperature;}
void UI_control::set_fan_speed(int new_status){ fan_status= new_status;}

void UI_control::display_main(){
  display.fill(0);
  display.text("CO2:" + std::to_string(CO2_level), 0, 0);
  display.text("Humidity:" + std::to_string(Relative_humidity), 0, 10);
  display.text("Temperature:" + std::to_string(Temperature), 0, 20);
  if(fan_status > 0){
    display.text("Fan on !!ALARM!!", 0, 30);
  } else {
    display.text("Fan off", 0, 30);
  }
  display.text("Button for Menu", 0, 50);
  display.show();
}

void UI_control::display_menu(){
  const char* menu_items[] = {"Set CO2", "Network settings", "Go back"};
  display.fill(0);
  for (int i = 0; i < 3; i++) {
    if(i == menu_index){
      display.rect(0, i*10, 128, 8, 1, true);
      display.text(menu_items[i], 0, i*10, 0);
    } else {
      display.text(menu_items[i], 0, i*10, 1);
    }
  }
  display.show();
}

void UI_control::display_set_co2(){
  display.fill(0);
  display.text("Set CO2 level:", 0, 0);
  display.text(std::to_string(target_CO2), 0, 10);
  display.text("Rotate to change.", 0, 30);
  display.text("Press to set.",0,40);
  display.show();
}

void UI_control::display_network() {
  display.fill(0);
  display.text("Network settings:", 0, 0);

  display.text("SSID: ",0,10);
  display.text(ssid,0,20);

  display.text("Password: ",0,30);
  display.text(password,0,40);

  display.text("Turn change, Press set.",0,50);
  display.show();
}

void UI_control::handle_menu_event(const gpioEvent &event) {
  if(event.type == gpioType::ROT_ENCODER){
    menu_index += event.direction;
    if(menu_index > 2) menu_index = 2;
    if(menu_index < 0) menu_index = 0;
  }
  if(event.type == gpioType::ROT_SWITCH) {
    switch(menu_index){
      case 0:
        current_state = UIState::SET_CO2;
        break;
      case 1:
        current_state= UIState::NETWORK_SETTINGS;
        input_mode = InputMode::ScrollSSID;
        network_cursor = 0;
        editing_ssid=true;
        break;
      case 2:
        current_state = UIState::MAIN;
        break;
    }
  }
}

void UI_control::handle_set_co2_event(const gpioEvent &event) {
  if(event.type == gpioType::ROT_ENCODER){
    target_CO2 += event.direction;
    if(target_CO2 < 0) target_CO2 = 0;
    if(target_CO2 > 1500) target_CO2 = 1500;
  }

  if(event.type == gpioType::ROT_SWITCH){
    current_state = UIState::SETTING_MENU;
    //TODO set bit to notify controller of change.
  }
}

void UI_control::handle_network_scroll(const gpioEvent &event) {
  if(event.type == gpioType::ROT_ENCODER){  // TODO display SSID from list
    ssid_list_index += event.direction;
    if(ssid_list_index < 0) ssid_list_index = 0;
    if(ssid_list_index > 10) ssid_list_index = 10;
  }
  if(event.type == gpioType::BUTTON2) {
    input_mode = InputMode::ManualSSID;
    network_cursor = 0;
    if(ssid[0] == '\0') ssid[0] = alphabet_lower[0];
  }

  if(event.type == gpioType::ROT_SWITCH) {
    input_mode = InputMode::Password;
    network_cursor = 0;
    if(password[0] == '\0') password[0] = alphabet_lower[0];
  }
}

void UI_control::handle_network_manual(const gpioEvent &event, char* buffer) {
  const int max_length = 63;
  if(event.type == gpioType::ROT_ENCODER) {
    char* alphabet = nullptr;
    switch(char_set) {
      case CharSet::Lowercase: alphabet = alphabet_lower; break;
      case CharSet::Uppercase: alphabet = alphabet_upper; break;
      case CharSet::Symbols: alphabet = alphabet_symbols; break;
      case CharSet::Numbers: alphabet = alphabet_digits; break;
    }
    char current = buffer[network_cursor];
    char* pos = strchr(alphabet, current);
    int index = pos ? (pos - alphabet) : 0;
    index += event.direction;
    int max_index = strlen(alphabet)-1;
    if(index < 0) index = max_index;
    if(index > max_index) index = 0;
    buffer[network_cursor] = alphabet[index];
  }

  if(event.type == gpioType::BUTTON1 && network_cursor > 0) {
    buffer[network_cursor] = '\0';
    network_cursor--;
  }

  if(event.type == gpioType::BUTTON2) {
    switch(char_set) {
      case CharSet::Lowercase: char_set = CharSet::Uppercase; break;
      case CharSet::Uppercase: char_set = CharSet::Symbols; break;
      case CharSet::Symbols: char_set = CharSet::Numbers; break;
      case CharSet::Numbers: char_set = CharSet::Lowercase; break;
    }
  }

  if(event.type == gpioType::BUTTON3 && network_cursor < max_length -1) {
    network_cursor++;
    buffer[network_cursor] = alphabet_lower[0];
  }

  if(event.type == gpioType::ROT_SWITCH) {
    if(input_mode == InputMode::ManualSSID) {
      input_mode = InputMode::Password;
      network_cursor = 0;
      password[0] = alphabet_lower[0];
    } else {
      current_state = UIState::SETTING_MENU;
      //TODO notify controller that  ssid and password are set
    }
  }
}

void UI_control::run() {
  gpioEvent event;
  uint32_t last_press[5] = {0}; // ROT_ENCODER, ROT_SW, BTN1, BTN2 & BTN3
  const uint32_t debounce_ms[5] = {50, 250, 250, 250, 250};

  display_main();

  while(true){
    if(xQueueReceive(input_queue, &event, portMAX_DELAY)==pdTRUE){
      uint32_t now = xTaskGetTickCount();
      int index = -1;
      switch(event.type) {
        case gpioType::ROT_ENCODER: index = 0; break;
        case gpioType::ROT_SWITCH: index = 1; break;
        case gpioType::BUTTON1: index = 2; break;
        case gpioType::BUTTON2: index = 3; break;
        case gpioType::BUTTON3: index = 4; break;
      }
      if(index >= 0 && now - last_press[index] < pdMS_TO_TICKS(debounce_ms[index])) continue;
      last_press[index] = now;

      switch(current_state){
        case UIState::MAIN:
          if(event.type == gpioType::ROT_SWITCH) {
            current_state = UIState::SETTING_MENU;
            menu_index = 0;
            }
          break;

        case UIState::SETTING_MENU:
          handle_menu_event(event);
          break;

        case UIState::SET_CO2:
          handle_set_co2_event(event);
          break;

        case UIState::NETWORK_SETTINGS:
          switch(input_mode) {
            case InputMode::ScrollSSID:
              handle_network_scroll(event);
            break;
            case InputMode::ManualSSID:
              handle_network_manual(event,ssid);
            break;
            case InputMode::Password:
              handle_network_manual(event,password);
            break;
          }
          break;
      }

      switch(current_state){
        case UIState::MAIN: display_main(); break;
        case UIState::SETTING_MENU: display_menu(); break;
        case UIState::SET_CO2: display_set_co2(); break;
        case UIState::NETWORK_SETTINGS: display_network(); break;
      }

    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}


void UI_control::runner(void *params){
  auto *instance = static_cast<UI_control *>(params);
  instance->run();
}