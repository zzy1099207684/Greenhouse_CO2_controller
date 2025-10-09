//
// Created by Riina on 25/09/2025.
//

#include "ui.h"
#include <task.h>
#include <cstdio>
#include <list>
#include "event_bits_def.h"

#define BTN1 9
#define BTN2 8
#define BTN3 7
#define ROT_A 10
#define ROT_B 11
#define ROT_SW 12
UI_control* UI_control::instance_ptr = nullptr; // Setting static instance pointer, what later will be used with ISR callback function.

UI_control::UI_control(const std::shared_ptr<PicoI2C> &i2c_bus,EventGroupHandle_t group): i2c_bus(i2c_bus), display(nullptr), input_queue(nullptr),event_group(group), task_handle(nullptr) {
  init();
}

void UI_control::gpio_callback(uint gpio, uint32_t events) { // Interrupt callback function for reading input from buttons.
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  gpioEvent event{}; // Set up struct to what assign information based on event what happened.

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
  xQueueSendFromISR(instance_ptr->input_queue, &event, &xHigherPriorityTaskWoken); // Send event info to queue to be handles elsewhere.
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}

void UI_control::init(){ //Initialize required variables.
  current_state=UIState::MAIN;          //Set current state as main screen.
  input_mode=InputMode::ScrollSSID;     //Set Input mode to scrollSSID this is not used unless going to network settings.
  char_set = CharSet::Lowercase;        //Set chars for manual input to start with Lowercase.

  memset(ssid,0,sizeof(ssid));          //Initialize ssid char buffer empty.
  memset(password,0,sizeof(password));  //Initialize password char buffer empty.

  strcpy(alphabet_lower,"abcdefghijklmnopqrstuvwxyz");  // Set up all character buffers needed in manual input.
  strcpy(alphabet_upper,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  strcpy(alphabet_symbols,"!?+-_ .@#$%^&*()=[]{}/,;:");
  strcpy(alphabet_digits,"0123456789");

  input_queue = xQueueCreate(16,sizeof(gpioEvent)); // Create event input queue.
  instance_ptr = this; // Set instance pointer to this instance.

  gpio_init(ROT_A);                   // Initialize all the buttons needed with direction and pull up.
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
  gpio_set_irq_enabled(ROT_SW, GPIO_IRQ_EDGE_FALL,true);  // Enable interrupts for buttons and callback for them.
  gpio_set_irq_enabled(BTN1, GPIO_IRQ_EDGE_FALL,true);
  gpio_set_irq_enabled(BTN2, GPIO_IRQ_EDGE_FALL,true);
  gpio_set_irq_enabled(BTN3, GPIO_IRQ_EDGE_FALL,true);

  // Create task.
  xTaskCreate(runner, "UI control", 512, (void*) this, tskIDLE_PRIORITY + 2, &task_handle);
}

// Functions for getting variable values.
int UI_control::get_CO2_Target(){ return co2SetPoint;}
char* UI_control::get_ssid(){ return ssid;}
char* UI_control::get_password(){ return password;}

// Functions to set variable values. Always needs update, so display will be updated when new info is set.
void UI_control::set_CO2_level(int new_level){ co2_level = new_level;needs_update=true; }
void UI_control::set_CO2_Target(int new_target){co2SetPoint = new_target; needs_update=true;}
void UI_control::set_Relative_humidity(float new_humidity){ Relative_humidity = new_humidity; needs_update=true; }
void UI_control::set_Temperature(float new_temperature){ Temperature = new_temperature; needs_update=true; }
void UI_control::set_fan_speed(int new_status){ fan_speed= new_status; needs_update=true; }

// Function used to set ssid list to variable, what is gotten from network.
void UI_control::set_ssid_list(const char *list[]) {
  ssid_list_count=0;
  memset(ssid_list,0,sizeof(ssid_list));
  for(int i=0; i < 10; i++) {
    if(list[i] != nullptr && strlen(list[i]) > 0) {
      printf("SSID %d: %s\n", i, list[i]);
      strncpy(ssid_list[i],list[i],63);
      ssid_list[i][63] = '\0';
      ssid_list_count += 1;
    }
  }
  strcpy(ssid, ssid_list[ssid_list_index]); // Copy one ssid from list to ssid variable.
  needs_update=true; // Display need update, so when we reach here ssids will be visible in network settings.
}
void  UI_control::set_network_status(bool status) { // Used to know if connected to network or not.
  connected_to_network = status;
  needs_update=true;
}

void UI_control::set_CO2_alarm(bool is_Emergency) { // Used when needing to display high CO2 levels.
  co2_alarm = is_Emergency;
  needs_update=true;
}

void UI_control::set_fan_error(bool is_error) { // If fan is broken this variable can be set, so info can be displayed.
  fan_error = is_error;
  needs_update=true;
}

void UI_control::display_main(){ // Used to display on main screen sensor readings.
  char buff[32];
  sprintf(buff,"%dppm",co2_level);
  display->text("CO2:" , 0, 0);
  display->text(buff, 70, 0);
  sprintf(buff,"%dppm" ,co2SetPoint);
  display->text("Target:" , 0, 8);
  display->text(buff, 70, 8);
  sprintf(buff,"%.1f%%",Relative_humidity);
  display->text("Rh:", 0, 16);
  display->text(buff, 70, 16);
  sprintf(buff,"%.1fC",Temperature);
  display->text("Temp:", 0, 24);
  display->text(buff, 70, 24);
  
  // if co2 alarm or fan error, use inverted colors
  bool needs_invert = co2_alarm || fan_error;
  
  if (needs_invert) {
    display->rect(0,33,128,8,1,true);
  }
  
  // Fan label: show (ERR) if fan has error
  if (fan_error) {
    display->text("Fan(ERR):", 0, 33, needs_invert ? 0 : 1);
  } else {
    display->text("Fan:", 0, 33, needs_invert ? 0 : 1);
  }
  
  // Fan status: show ALARM, ERROR, ON, or OFF
  if (co2_alarm) {
    display->text("ALARM", 70, 33, needs_invert ? 0 : 1);
  } else if (fan_error) {
    display->text("ERROR", 70, 33, needs_invert ? 0 : 1);
  } else if (fan_speed > 0) {
    display->text("ON", 70, 33, needs_invert ? 0 : 1);
  } else {
    display->text("OFF", 70, 33, needs_invert ? 0 : 1);
  }
  // Network status either connected or not connected.
  display->text("Network:", 0, 42);
  if(connected_to_network) {
    display->text("Online", 70, 42);
  } else {
    display->text("Offline", 70, 42);
  }
  // Highlighting text that button for menu to make it more obvious with all the info on the screen.
  display->rect(0,54,128,8,1,true);
  display->text("Button for Menu", 0, 54, 0);
}

void UI_control::display_menu(){ // Used to display menu options on the screen.
  const char* menu_items[] = {"Set CO2", "Network settings", "Go back"};
  for (int i = 0; i < 3; i++) {
    if(i == menu_index){
      display->rect(0, i*10, 128, 8, 1, true); // Highlight at which option cursor is at.
      display->text(menu_items[i], 0, i*10, 0);
    } else {
      display->text(menu_items[i], 0, i*10, 1);
    }
  }
}

void UI_control::display_set_co2(){ // Used to display setting new CO2 target.
  char buff[32];
  display->text("Set CO2 level:", 0, 0);
  sprintf(buff, "%d", co2SetPoint);
  display->text(buff, 0, 10);
  display->text("Rot to change.", 0, 30);
  display->text("Press to set.",0,40);
}

void UI_control::display_successfull_set_co2() { // Used as transition screen from setting CO2.
  display->fill(0);
  display->text("New target CO2",0,20);
  display->text("set successfully.",0,30);
  display->show();
}

void UI_control::display_network() { // Used to display setting Network settings.
  display->text("Network settings:", 0, 0);

  display->text("SSID: ",0,8);
  display->text(ssid,0,16);

  display->text("Password: ",0,26);
  display->text(password,0,34);

  display->text("Rot to change.",0,46);
  display->text("Press to save.",0,54);
}
void UI_control::display_successfull_set_network() { // Transition screen from network settings.
  display->fill(0);
  display->text("Network info set.",0,30);
  EventBits_t bits = xEventGroupGetBits(event_group);
  if(bits & WIFI_CONNECTED) { // If connected when transition screen is called, will display connection.  If not connected by then successful connection is displayed on main screen "Network:Online"
    display->text("WiFi connected.",0,40);
    connected_to_network = true;
  } else {
    display->text("Connecting...",0,40);
  }
  display->show();
}

void UI_control::handle_menu_event(const gpioEvent &event) { // Used to navigate menu items using event info from ISR.
  if(event.type == gpioType::ROT_ENCODER){ // Scroll between menu items, index used in display to highlight selection.
    menu_index += event.direction;
    if(menu_index > 2) menu_index = 2;
    if(menu_index < 0) menu_index = 0;
    needs_update = true;
  }
  if(event.type == gpioType::ROT_SWITCH) { // Update menu choice as current state, based on selection sets variables needed in next state.
    switch(menu_index){
      case 0:
        current_state = UIState::SET_CO2;
        break;
      case 1:
        current_state= UIState::NETWORK_SETTINGS;
        memset(ssid,0,sizeof(ssid)); // Make sure ssid and password are empty before going to input them.
        memset(password,0,sizeof(password));
        xEventGroupSetBits(event_group,UI_GET_NETWORK); // Let Network know I need ssid list.
        ssid_list_index = 0;
        input_mode = InputMode::ScrollSSID;
        network_cursor = 0;
        break;
      case 2:
        current_state = UIState::MAIN;
        break;
    }
    needs_update = true;
  }
}

void UI_control::handle_set_co2_event(const gpioEvent &event) { // Used to handle event input when current state is setting CO2 target.
  if(event.type == gpioType::ROT_ENCODER){ // rotating encoder increases/decreases target.
    co2SetPoint += event.direction * 10; // Scroll by 10 at time to adjust value faster as that accuracy is enough.
    if(co2SetPoint < 200) co2SetPoint = 200;
    if(co2SetPoint > 1500) co2SetPoint = 1500;
    needs_update = true;
  }

  if(event.type == gpioType::ROT_SWITCH){ // Confirms set CO2 target.
    display_successfull_set_co2(); // Call confirmation screen.
    vTaskDelay(pdMS_TO_TICKS(3000)); // Have delay so user has time to read confirmation screen, before going back to main.
    current_state = UIState::MAIN;
    xEventGroupSetBits(event_group, UI_SET_CO2); // Let controller know we have new CO2 target for them to take.
    needs_update = true;
  }
}

void UI_control::handle_network_scroll(const gpioEvent &event) { // Scroll SSID list or go to manual input.
  if(event.type == gpioType::ROT_ENCODER){ // Scroll SSID list options.
    EventBits_t bits = xEventGroupGetBits(event_group);
    if(bits & UI_SSID_READY) { // Make sure SSID list is ready before scrolling through them.
      ssid_list_index += event.direction;
      if(ssid_list_index < 0) ssid_list_index = ssid_list_count - 1;
      if(ssid_list_index > (ssid_list_count - 1))ssid_list_index = 0;
      strcpy(ssid, ssid_list[ssid_list_index]);
    }
  }
  if(event.type == gpioType::BUTTON2) { // If button 2 is pressed, then switch to manual SSID input.
    input_mode = InputMode::ManualSSID;
    memset(ssid,0,sizeof(ssid)); // Reset SSID to be empty and ready for input.
    network_cursor = 0;
    ssid[0] = alphabet_lower[0];
  }

  if(event.type == gpioType::ROT_SWITCH) { // If ROT is pressed, confirm scrolled SSID as SSID and go to password input.
    input_mode = InputMode::Password;
    network_cursor = 0;
    password[0] = alphabet_lower[0];
  }
  needs_update = true;
}

void UI_control::handle_network_manual(const gpioEvent &event, char* buffer) { // Handles manual SSID and password input.
  const int max_length = 63;
  if(event.type == gpioType::ROT_ENCODER) { // If rotary is turned scroll through chosen alphabet one char at time.
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
    if(index < 0) index = max_index; // Allow to rotate through alphabets to any direction in a loop.
    if(index > max_index) index = 0;
    buffer[network_cursor] = alphabet[index];
    needs_update = true;
  }

  if(event.type == gpioType::BUTTON1 && network_cursor > 0) { // Go back on input character and remove previously set char.
    buffer[network_cursor] = '\0';
    network_cursor--;
    needs_update = true;
  }

  if(event.type == gpioType::BUTTON2) { // Switch before different character sets.
    switch(char_set) {
      case CharSet::Lowercase: char_set = CharSet::Uppercase; break;
      case CharSet::Uppercase: char_set = CharSet::Symbols; break;
      case CharSet::Symbols: char_set = CharSet::Numbers; break;
      case CharSet::Numbers: char_set = CharSet::Lowercase; break;
    }
    needs_update = true;
  }

  if(event.type == gpioType::BUTTON3 && network_cursor < max_length -1) { // Move forward to next character to set.
    network_cursor++;
    buffer[network_cursor] = alphabet_lower[0];
    needs_update = true;
  }

  if(event.type == gpioType::ROT_SWITCH) {
    if(input_mode == InputMode::ManualSSID) { // If currently in manual SSID input go to password input.
      input_mode = InputMode::Password;
      network_cursor = 0;
      password[0] = alphabet_lower[0];
    } else {
      current_state = UIState::MAIN;
      xEventGroupSetBits(event_group,UI_CONNECT_NETWORK); // Let controller to know we have set SSID and password so they can connect to network.
      xEventGroupClearBits(event_group,UI_SSID_READY); // Clear SSID list ready bit, so if go network settings again then can set it again to get fresh list.
      vTaskDelay(pdMS_TO_TICKS(400)); // Have small delay to allow network to connect before transition screen.
      display_successfull_set_network();
      vTaskDelay(pdMS_TO_TICKS(3000)); // Have delay after transition screen so user has time to read the message, before moving to main screen.
    }
    needs_update = true;
  }
}

void UI_control::run() { // Function to handle ISR events and displaying on screen.
  display = new ssd1306os(i2c_bus); // Create display object.
  gpioEvent event;
  uint32_t last_press[5] = {0}; // ROT_ENCODER, ROT_SW, BTN1, BTN2 & BTN3. Last presses where store when each button was last pressed for debounce.
  const uint32_t debounce_ms[5] = {50, 250, 250, 250, 250}; // Debounce time list in same order as last_press list is.

  display->fill(0); // Display main screen, before events.
  display_main();
  display->show();
  needs_update=false;

  while(true){
    EventBits_t bits = xEventGroupGetBits(event_group); // Get event bits.
    if(xQueueReceive(input_queue, &event, pdMS_TO_TICKS(50))==pdTRUE) { // Wait 50 ticks for queue.
      int index = -1;
      switch(event.type) { // Based on event type, set index to check which debounce and last press to use.
        case gpioType::ROT_ENCODER: index = 0; break;
        case gpioType::ROT_SWITCH: index = 1; break;
        case gpioType::BUTTON1: index = 2; break;
        case gpioType::BUTTON2: index = 3; break;
        case gpioType::BUTTON3: index = 4; break;
      }
      if(index >= 0 && event.timestamp - last_press[index] < pdMS_TO_TICKS(debounce_ms[index])) continue; // If too much time since last press skip rest.
      last_press[index] = event.timestamp; // If enough time since last press, set new timestamp to last press.

      switch(current_state){ // Switch between different UI states.
        case UIState::MAIN:
          if(event.type == gpioType::ROT_SWITCH) {
            current_state = UIState::SETTING_MENU;
            menu_index = 0;
            needs_update = true;
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
    }
    if(needs_update) {
      if(bits & WIFI_CONNECTED) { // Check event group bits to see if WIFI connected and set status based on it.
        connected_to_network = true;
      } else {
        connected_to_network = false;
      }
      display->fill(0);
      switch(current_state){ // Display what state we are in.
        case UIState::MAIN: display_main(); break;
        case UIState::SETTING_MENU: display_menu(); break;
        case UIState::SET_CO2: display_set_co2(); break;
        case UIState::NETWORK_SETTINGS: display_network(); break;
      }
      display->show();
      needs_update = false;
    }
    vTaskDelay(pdMS_TO_TICKS(20)); // If nothing to do have small delay to allow other tasks to run.
  }
}


void UI_control::runner(void *params){
  auto *instance = static_cast<UI_control *>(params);
  instance->run();
}