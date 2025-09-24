//
// Created by zhiyo on 2025/9/21.
//

#ifndef THING_SPEAK_H
#define THING_SPEAK_H

#include "FreeRTOS.h"
#include <string.h>
#include "queue.h"
#include "limits.h"
#include "semphr.h"
#include "pico/util/datetime.h"
#include "lwip/apps/sntp.h"

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
    int get_request_type() const { return request_type; } // 0: read, 1: write

    // data setters
    void set_CO2_level(int v) { CO2_level = v; }
    void set_Relative_humidity(int v) { Relative_humidity = v; }
    void set_Temperature(int v) { Temperature = v; }
    void set_fan_speed(int v) { fan_speed = v; }
    void set_request_type(int v) { request_type = v; }

    // string getters
    char *get_response() { return response; }
    char *get_request() { return request; }
    char *get_api_server() { return api_server; }
    char *get_ssid() { return ssid; }
    char *get_pwd() { return pwd; }
    char *get_write_api_key() { return write_api_key; }
    char *get_read_api_key() { return read_api_key; }

    // queue getters
    QueueHandle_t get_set_queue() const { return set_queue; }
    SemaphoreHandle_t get_upload_Sem() const { return upload_Sem; }

    // string setters
    void set_response(const char *s) { strncpy(response, s, sizeof(response) - 1); }
    void set_request(const char *s) { strncpy(request, s, sizeof(request) - 1); }
    void set_api_server(const char *s) { strncpy(api_server, s, sizeof(api_server) - 1); }
    void set_ssid(const char *s) { strncpy(ssid, s, sizeof(ssid) - 1); }
    void set_pwd(const char *s) { strncpy(pwd, s, sizeof(pwd) - 1); }
    void set_write_api_key(const char *s) { strncpy(write_api_key, s, sizeof(write_api_key) - 1); }
    void set_read_api_key(const char *s) { strncpy(read_api_key, s, sizeof(read_api_key) - 1); }

    void init() {
    }

    thing_speak() {
        set_queue = xQueueCreate(50, sizeof(int));
        upload_Sem = xSemaphoreCreateBinary();
        set_ssid("Redmi_138D");
        set_pwd("zzyzmy20272025888");
        set_api_server("api.thingspeak.com");
        set_write_api_key(WRITE_API_KEY);
        set_read_api_key(READ_API_KEY);
        configASSERT(set_queue);
        configASSERT(upload_Sem);
    }

private:
    char response[1500]{};
    char request[200]{};
    char api_server[64]{};
    char ssid[64]{};
    char pwd[64]{};
    char write_api_key[64]{};
    char read_api_key[64]{};
    int request_type{0}; // 0: read, 1: write
    int CO2_level{INT_MIN};
    int Relative_humidity{INT_MIN};
    int Temperature{INT_MIN};
    int fan_speed{INT_MIN};
    QueueHandle_t set_queue;
    SemaphoreHandle_t upload_Sem;
};


#endif //THING_SPEAK_H
