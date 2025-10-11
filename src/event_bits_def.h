//
// Event bits definition
//

#ifndef GREENHOUSE_CO2_EVENT_BITS_DEF_H
#define GREENHOUSE_CO2_EVENT_BITS_DEF_H

// UI related events
#define UI_SET_CO2 (1<<0)           // CO2 setting ready from UI
#define UI_GET_NETWORK (1<<1)       // UI needs SSID list
#define UI_SSID_READY (1<<2)        // SSID list ready, set by controller
#define UI_CONNECT_NETWORK (1<<3)   // Password ready from UI

// System warnings
#define CO2_WARNING (1<<4)          // Warning from CO2 controller, critical high level
#define FAN_WARNING (1<<5)         // Warning from fan controller, fan is not working

// Sensor events
#define ENV_SENSOR_TIMER_REACHED (1<<6)  // Environment sensor timer reached

// Network related events
#define NETWORK_SET_CO2 (1<<7)      // CO2 setting from network
#define WIFI_INIT (1<<8)            // WiFi initialization success
#define WIFI_SCAN_DONE (1<<9)       // WiFi scan completed
#define WIFI_CONNECTED (1<<10)       // WiFi connected
#define WIFI_CONNECTED_GET_SETTING_CO2_DATA (1<<11)         // WiFi connected, ready to get CO2 settings
#define WIFI_CONNECTED_UPLOAD_DATA_TO_THING_SPEAK (1<<12)   // WiFi connected, ready to upload data

#endif //GREENHOUSE_CO2_EVENT_BITS_DEF_H

