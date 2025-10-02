//
// Created by zhiyo on 2025/9/21.
//

#ifndef THING_SPEAK_H
#define THING_SPEAK_H

#include <event_groups.h>

#include "FreeRTOS.h"
#include <cstring>
#include "queue.h"
#include <climits>
#include "semphr.h"
#include "pico/util/datetime.h"
#include "lwip/apps/sntp.h"

#define WIFI_INIT_BIT (1 << 0) // wifi_init_success bit
#define WIFI_SCAN_DONE_BIT (1 << 1) // wifi_scan_done bit
#define NETWORK_SET_CO2_BIT (1<<3) // co2_event_group


#define WRITE_API_KEY "3O010WOMXCBHN887"
#define READ_API_KEY  "754RZS4U9IAOOGOE"

#define TLS_CLIENT_TIMEOUT_SECS  15
#define THINGSPEAK_CERT "-----BEGIN CERTIFICATE-----\n\
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n\
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n\
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n\
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n\
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n\
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n\
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n\
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n\
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n\
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n\
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n\
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n\
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n\
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n\
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n\
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n\
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n\
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n\
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n\
MrY=\n\
-----END CERTIFICATE-----\n"

class thing_speak {
public:
    // data getters
    int get_CO2_level() const { return CO2_level; }
    int get_Relative_humidity() const { return Relative_humidity; }
    int get_Temperature() const { return Temperature; }
    int get_fan_speed() const { return fan_speed; }

    // data setters
    void set_CO2_level(const int v) { CO2_level = v; }
    void set_Relative_humidity(const int v) { Relative_humidity = v; }
    void set_Temperature(const int v) { Temperature = v; }
    void set_fan_speed(const int v) { fan_speed = v; }
    void set_co2_level_from_network(const int v) { co2_level_from_network = v; }
    int get_co2_level_from_network() const { return co2_level_from_network; }

    // string getters
    char *get_response() { return response; }
    char *get_request() { return request; }
    char *get_api_server() { return api_server; }
    char *get_ssid() { return ssid; }
    char *get_pwd() { return pwd; }
    char *get_write_api_key() { return write_api_key; }
    char *get_read_api_key() { return read_api_key; }
    char (*get_wifi_scan_result())[64] { return wifi_scan_result; }
    int get_wifi_ssid_index() const { return wifi_ssid_index; }

    // Group getters and setters
    [[nodiscard]] EventGroupHandle_t get_co2_wifi_scan_event_group() const { return co2_wifi_scan_event_group; }
    void set_co2_wifi_scan_event_group(EventGroupHandle_t g) { co2_wifi_scan_event_group = g; }

    // string setters
    void set_response(const char *s) { strncpy(response, s, sizeof(response) - 1); }
    void set_request(const char *s) { strncpy(request, s, sizeof(request) - 1); }
    void set_api_server(const char *s) { strncpy(api_server, s, sizeof(api_server) - 1); }
    void set_ssid(const char *s) { strncpy(ssid, s, sizeof(ssid) - 1); }
    void set_pwd(const char *s) { strncpy(pwd, s, sizeof(pwd) - 1); }
    void set_write_api_key(const char *s) { strncpy(write_api_key, s, sizeof(write_api_key) - 1); }
    void set_read_api_key(const char *s) { strncpy(read_api_key, s, sizeof(read_api_key) - 1); }
    void set_wifi_scan_result(const int index, const char *value)  {
        if (index < 0 || index >= 10) return;
        strncpy(wifi_scan_result[index], value, sizeof(wifi_scan_result[index]) - 1);
        wifi_scan_result[index][sizeof(wifi_scan_result[index]) - 1] = '\0';
    }

    void set_wifi_ssid_index(const int index) { wifi_ssid_index = index; }

    void init_wifi_scan_result() {
        for (auto &p : wifi_scan_result) {
            p[0] = '\0';
        }
    }

    thing_speak() {
        set_api_server("api.thingspeak.com");
        set_write_api_key(WRITE_API_KEY);
        set_read_api_key(READ_API_KEY);
    }

private:
    char response[1500]{};
    char request[200]{};
    char api_server[64]{};
    char ssid[64]{};
    char pwd[64]{};
    char write_api_key[64]{};
    char read_api_key[64]{};
    int CO2_level{INT_MIN};
    int Relative_humidity{INT_MIN};
    int Temperature{INT_MIN};
    int fan_speed{INT_MIN};
    int co2_level_from_network{INT_MIN};
    EventGroupHandle_t co2_wifi_scan_event_group{};
    char wifi_scan_result[10][64]{};
    int wifi_ssid_index{0};
};

#endif //THING_SPEAK_H
