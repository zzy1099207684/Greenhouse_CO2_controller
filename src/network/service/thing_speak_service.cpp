//
// Created by zhiyo on 2025/9/21.
//

#include "thing_speak_service.h"
#include <cstring>
#include <timers.h>
#include "Tools/json/json_handler.h"

thing_speak *tss = nullptr;
Json_handler handler;

static char *extract_json(char *http_response);

extern "C" {
bool run_tls_client_test(const uint8_t *cert, size_t cert_len, const char *server, const char *request, int timeout,
                         void *tsp);
}

extern "C" {
void get_data(char *param, void *tsp) {
    thing_speak &ts = *(thing_speak *) tsp;
    if (strstr(param, "SAMEORIGIN") != nullptr) {
        if (strstr(param, "feeds") != nullptr) {
            char *json = extract_json(param);
            ts.set_response(json);
            thing_speak_service::deal_SETTING_CO2_data(&ts);
        }
    } else {
        int back_res = atoi(param);
        ts.set_CO2_level(INT_MIN);
        ts.set_Relative_humidity(INT_MIN);
        ts.set_Temperature(INT_MIN);
        ts.set_fan_speed(INT_MIN);
        if (back_res > 0) {
            printf("upload success\n");
        }
        printf("upload failed\n");
    }
}
}

// timer
void thing_speak_service::get_SETTING_CO2_data(TimerHandle_t xTimer) {
    thing_speak &ts = *(thing_speak *) pvTimerGetTimerID(xTimer);
    // "GET /update?api_key=91TLCMJCMTU4K9Z0&field1=0 HTTP/1.1\r\nHost: api.thingspeak.com\r\nConnection: close\r\n\r\n"
    char request[300] = {};
    sprintf(request,
            "GET /channels/3083662/feeds.json?api_key=%s&results=1 HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
            ts.get_read_api_key(), ts.get_api_server());
    ts.set_request(request);
    request_HTTPS(&ts); //get data from thing speak
}

void thing_speak_service::deal_SETTING_CO2_data(void *param) {
    thing_speak &ts = *(thing_speak *) param;

    int field_5 = 0;
    handler.get_value_from_key(ts.get_response(), "feeds");
    char *value = handler.get_final_result();
    handler.get_value_from_key(value, "field1");
    value = handler.get_final_result();
    if (strcmp(value, "") != 0) {
        field_5 = atoi(value);
        xQueueSend(ts.get_set_queue(), &field_5, 0);
    }
}


// task
void thing_speak_service::upload_data_to_thing_speak(TimerHandle_t xTimer) {
    thing_speak &ts = *(thing_speak *) pvTimerGetTimerID(xTimer);

    int field_1 = ts.get_CO2_level();
    int field_2 = ts.get_Relative_humidity();
    int field_3 = ts.get_Temperature();
    int field_4 = ts.get_fan_speed();

    char params[64] = {};
    if (field_1 != INT_MIN) {
        sprintf(params, "field1=%d&", field_1);
    }
    if (field_2 != INT_MIN) {
        sprintf(params + strlen(params), "field2=%d&", field_2);
    }
    if (field_3 != INT_MIN) {
        sprintf(params + strlen(params), "field3=%d&", field_3);
    }
    if (field_4 != INT_MIN) {
        sprintf(params + strlen(params), "field4=%d", field_4);
    }
    if (strlen(params) > 0 && params[strlen(params) - 1] == '&') {
        params[strlen(params) - 1] = '\0';
        char request[200] = {};
        sprintf(request, "GET /update?api_key=%s&%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
                ts.get_write_api_key(), params, ts.get_api_server());
        ts.set_request(request);
        printf("request %s", request);
        request_HTTPS(&ts); //upload data to thing speak
    }
}

void thing_speak_service::wifi_init_once(void *param) {
    thing_speak &ts = *(thing_speak *) param;
    const char *ssid = ts.get_ssid();
    const char *pwd = ts.get_pwd();

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return;
    }
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(ssid, pwd, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect\n");
        return;
    }
    printf("WiFi connected\n");
}


void thing_speak_service::request_HTTPS(void *param) {
    thing_speak &ts = *(thing_speak *) param;
    const uint8_t things_cert[] = THINGSPEAK_CERT;
    char *server = ts.get_api_server();
    char *request = ts.get_request();

    bool pass = run_tls_client_test(things_cert, sizeof(things_cert), server, request,
                                    TLS_CLIENT_TIMEOUT_SECS, &ts);
    if (pass) {
        printf("Test passed\n");
    } else {
        printf("Test failed\n");
    }
    /* sleep a bit to let usb stdio write out any buffer to host */
    vTaskDelay(pdMS_TO_TICKS(100));
}


static char *extract_json(char *http_response) {
    char *body = strstr(http_response, "\r\n{");
    if (!body) {
        return NULL;
    }
    body += 2;

    char *r = body;
    char *w = body;
    while (*r) {
        char c = *r++;
        if (c == '\r' || c == '\n' || c == '\\' || c == '[' || c == ']') {
            continue;
        }
        *w++ = c;
    }
    *w = '\0';
    return body;
}
