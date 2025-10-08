//
// Created by zhiyo on 2025/9/21.
//
#include "FreeRTOS.h"
#include "../entry/thing_speak.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mdns.h"
#include "task.h"
#include <cstring>
#include "Tools/json/json_handler.h"
#include <timers.h>
#include "thing_speak_service.h"

#include <cerrno>


Json_handler handler;


void print_waiting_log(const char *fmt, int times = 1, int each_ms = 100);

static char *extract_json(char *http_response, size_t max_len);

extern "C" {
bool run_tls_client(const uint8_t *cert, size_t cert_len, const char *server, const char *request, int timeout,
                    void *tsp);
}

extern "C" {
void get_data(char *param, void *tsp) {
    auto *ts = static_cast<thing_speak *>(tsp);
    if (strstr(param, "SAMEORIGIN") != nullptr) {
        if (strstr(param, "feeds") != nullptr) {
            char *json = extract_json(param, strlen(param));
            ts->set_response(json);
            ts->set_request(nullptr);
            thing_speak_service::deal_SETTING_CO2_data(ts);
        }
    } else {
        int back_res = atoi(param);
        ts->set_CO2_level(INT_MIN);
        ts->set_Relative_humidity(FLT_MIN);
        ts->set_Temperature(FLT_MIN);
        ts->set_fan_speed(INT_MIN);
        if (back_res > 0) {
            printf("request success\n");
        }
    }
}
}

#if TWO_TASK
void thing_speak_service::get_SETTING_CO2_data(void *param) {
    int count = 0;
    for (;;) {
        auto *ts = static_cast<thing_speak *>(param);
        // If the hardware has new data, stop pulling and wait for the upload to succeed.
        if (!ts->get_is_co2_setting_data_from_hardware()) {
            xEventGroupWaitBits(ts->get_co2_wifi_scan_event_group(),
                                WIFI_CONNECTED_GET_SETTING_CO2_DATA, pdFALSE, pdTRUE,
                                portMAX_DELAY); // wait for wifi connected
            printf("get_SETTING_CO2_data timer start !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            // "GET /update?api_key=91TLCMJCMTU4K9Z0&field1=0 HTTP/1.1\r\nHost: api.thingspeak.com\r\nConnection: close\r\n\r\n"
            char request[300] = {};
            snprintf(request, sizeof(request),
                     "GET /channels/3083662/feeds.json?api_key=%s&results=1 HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
                     ts->get_read_api_key(), ts->get_api_server());
            ts->set_request(request);
            // from network
            request_HTTPS(ts);
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void thing_speak_service::upload_data_to_thing_speak(void *param) {
    for (;;) {
        auto *ts = static_cast<thing_speak *>(param);
        xEventGroupWaitBits(ts->get_co2_wifi_scan_event_group(),
                            WIFI_CONNECTED_UPLOAD_DATA_TO_THING_SPEAK, pdFALSE, pdTRUE,
                            portMAX_DELAY);
        printf("upload_data_to_thing_speak timer start !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        int field_1 = ts->get_CO2_level();
        float field_2 = ts->get_Relative_humidity();
        float field_3 = ts->get_Temperature();
        int field_4 = ts->get_fan_speed();
        int field_5 = ts->get_co2_level_from_network();

        char params[64] = {};
        if (field_1 == INT_MIN) field_1 = 0;
        snprintf(params, sizeof(params), "field1=%d&", field_1);
        if (field_2 == FLT_MIN) field_2 = 0.0f;
        snprintf(params + strlen(params), sizeof(params) - strlen(params), "field2=%f&", field_2);
        if (field_3 == FLT_MIN) field_3 = 0.0f;
        snprintf(params + strlen(params), sizeof(params) - strlen(params), "field3=%f&", field_3);
        if (field_4 == INT_MIN) field_4 = 0;
        snprintf(params + strlen(params), sizeof(params) - strlen(params), "field4=%d&", field_4);
        if (field_5 == INT_MIN) field_5 = 0;
        snprintf(params + strlen(params), sizeof(params) - strlen(params), "field5=%d&", field_5);

        if (strlen(params) > 0) params[strlen(params) - 1] = '\0';
        char request[200] = {};
        snprintf(request, sizeof(request),
                 "GET /update?api_key=%s&%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
                 ts->get_write_api_key(), params, ts->get_api_server());
        ts->set_request(request);
        printf("request %s", request);
        request_HTTPS(ts); //upload data to thing speak
        vTaskDelay(pdMS_TO_TICKS(20000));
    }
}
#endif

void thing_speak_service::deal_SETTING_CO2_data(void *param) {
    auto *ts = static_cast<thing_speak *>(param);
    handler.get_value_from_key(ts->get_response(), "feeds");
    char *value = handler.get_final_result();
    handler.get_value_from_key(value, "field5");
    value = handler.get_final_result();
    int field_5 = atoi(value);
    handler.get_value_from_key(ts->get_response(), "feeds");
    handler.get_value_from_key(value, "field6");
    if (value != nullptr) ts->set_async_network_to_hardware(true);
    if (strcmp(value, "") != 0 && (field_5 != ts->get_co2_level_from_network()) && (field_5 != 0) &&
        !ts->get_is_co2_setting_data_from_hardware() && ts->get_async_network_to_hardware()) {
        printf("SETTING CO2 level from thing speak: %d\n", field_5);
        ts->set_co2_level_from_network(field_5);
        xEventGroupSetBits(ts->get_co2_wifi_scan_event_group(), NETWORK_SET_CO2); // set co2 level change bit
        ts->set_async_network_to_hardware(false);
    }
}

void thing_speak_service::get_setting_co2_val_or_upload(void *param) {
    auto *ts = static_cast<thing_speak *>(param);
    static char request[300] = {};
    for (;;) {
        // If the hardware has new data, upload it to ThingSpeak.
        xEventGroupWaitBits(ts->get_co2_wifi_scan_event_group(),
                            WIFI_CONNECTED, pdFALSE, pdTRUE,
                            portMAX_DELAY); // wait for wifi connected
        if (!ts->get_is_co2_setting_data_from_hardware() && !ts->get_task_switch()) {
            printf("get_SETTING_CO2_data timer start !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            // "GET /update?api_key=91TLCMJCMTU4K9Z0&field1=0 HTTP/1.1\r\nHost: api.thingspeak.com\r\nConnection: close\r\n\r\n"
            snprintf(request, sizeof(request),
                     "GET /channels/3083662/feeds.json?api_key=%s&results=1 HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
                     ts->get_read_api_key(), ts->get_api_server());
            ts->set_request(request);
            // from network
            request_HTTPS(ts);
            ts->set_task_switch(true);
            print_waiting_log("Waiting for hardware to upload data to ThingSpeak", 15, 1000);
        } else {
            auto *ts = static_cast<thing_speak *>(param);
            printf("upload_data_to_thing_speak timer start !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            int field_1 = ts->get_CO2_level();
            float field_2 = ts->get_Relative_humidity();
            float field_3 = ts->get_Temperature();
            int field_4 = ts->get_fan_speed();
            int field_5 = ts->get_co2_level_from_network();

            char params[150] = {};

            if (field_1 == INT_MIN) field_1 = 0;
            snprintf(params, sizeof(params), "field1=%d&", field_1);
            if (field_2 == FLT_MIN) field_2 = 0.0f;
            snprintf(params + strlen(params), sizeof(params) - strlen(params), "field2=%f&", field_2);
            if (field_3 == FLT_MIN) field_3 = 0.0f;
            snprintf(params + strlen(params), sizeof(params) - strlen(params), "field3=%f&", field_3);
            if (field_4 == INT_MIN) field_4 = 0;
            snprintf(params + strlen(params), sizeof(params) - strlen(params), "field4=%d&", field_4);
            if (field_5 == INT_MIN) field_5 = 0;
            snprintf(params + strlen(params), sizeof(params) - strlen(params), "field5=%d&", field_5);

            if (strlen(params) > 0) params[strlen(params) - 1] = '\0';
            snprintf(request, sizeof(request),
                     "GET /update?api_key=%s&%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
                     ts->get_write_api_key(), params, ts->get_api_server());
            ts->set_request(request);
            printf("request %s", request);
            request_HTTPS(ts); //upload data to thing speak
            ts->set_task_switch(false);
            if (!ts->get_is_co2_setting_data_from_hardware()) {
                print_waiting_log("Waiting for ThingSpeak to provide new CO2 setting", 5, 1000);
            } else {
                print_waiting_log("Waiting for hardware to upload data to ThingSpeak", 15, 1000);
            }
        }
    }
}


void thing_speak_service::request_HTTPS(void *param) {
    auto *ts = static_cast<thing_speak *>(param);
    constexpr uint8_t things_cert[] = THINGSPEAK_CERT;
    const char *server = ts->get_api_server();
    const char *request = ts->get_request();

    // two tasks use it, need mutex
    // xSemaphoreTake(ts->get_net_mutex(), portMAX_DELAY);
    bool pass = run_tls_client(things_cert, sizeof(things_cert), server, request,
                               TLS_CLIENT_TIMEOUT_SECS, ts);
    // xSemaphoreGive(ts->get_net_mutex());

    if (pass) {
        printf("request passed\n");
        ts->set_is_co2_setting_data_from_hardware(false);
    } else {
        printf("request failed\n");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}

void thing_speak_service::wifi_connect(void *param) {
    printf("WiFi connect task started\n");
    auto *ts = static_cast<thing_speak *>(param);
    xEventGroupWaitBits(ts->get_co2_wifi_scan_event_group(), WIFI_INIT, pdFALSE, pdTRUE, portMAX_DELAY);
    int fail_count = 0;
    for (;;) {
        int ret = cyw43_arch_wifi_connect_timeout_ms(ts->get_ssid(), ts->get_pwd(),
                                                     CYW43_AUTH_WPA2_AES_PSK, 15000);
        cyw43_arch_lwip_begin();
        netif_set_up(&cyw43_state.netif[CYW43_ITF_STA]);
        dhcp_start(&cyw43_state.netif[CYW43_ITF_STA]);
        cyw43_arch_lwip_end();

        if (ret == 0) {
            cyw43_arch_lwip_begin();
            if (dhcp_supplied_address(&cyw43_state.netif[CYW43_ITF_STA]) == 0) {
                dhcp_start(&cyw43_state.netif[CYW43_ITF_STA]);
            }
            cyw43_arch_lwip_end();

            vTaskDelay(pdMS_TO_TICKS(2000));
            cyw43_arch_lwip_begin();
            bool has_ip = dhcp_supplied_address(&cyw43_state.netif[CYW43_ITF_STA]) != 0;
            cyw43_arch_lwip_end();

            if (has_ip) {
                printf("WiFi connected\n");
                xEventGroupSetBits(ts->get_co2_wifi_scan_event_group(),WIFI_CONNECTED);
                ts->set_is_connected(true);
                fail_count = 0;
                vTaskSuspend(ts->get_wifi_connect_handle());
            } else {
                printf("connect status: no ip\n");
                fail_count++;
            }
        } else {
            xEventGroupClearBits(ts->get_co2_wifi_scan_event_group(),WIFI_CONNECTED);
            printf("WiFi connect failed (ret=%d)\n", ret);
            fail_count++;
        }

        if (fail_count >= 3) {
            printf("WiFi connect failed 3 times, rescan ssid\n");
            fail_count = 0;
            wifi_disconnect();
        }
        vTaskDelay(pdMS_TO_TICKS(15000));
    }
}

static char *extract_json(char *http_response, size_t max_len) {
    if (!http_response) return nullptr;

    char *body = strstr(http_response, "\r\n{");
    if (!body) return nullptr;
    body += 2;

    char *r = body;
    char *w = body;
    size_t count = 0;
    while (*r && count < max_len - 1) {
        char c = *r++;
        if (c == '\r' || c == '\n' || c == '\\' || c == '[' || c == ']')
            continue;
        *w++ = c;
        count++;
    }
    *w = '\0';
    return body;
}

static int scan_cb(void *param, const cyw43_ev_scan_result_t *res) {
    if (!res || res->ssid_len <= 0 || !res->ssid) return 0;

    auto *ts = static_cast<thing_speak *>(param);
    auto wifi_scan_result = ts->get_wifi_scan_result();
    int index = ts->get_wifi_ssid_index();

    for (int i = 0; i < 10; ++i) {
        if (wifi_scan_result[i][0] != '\0' &&
            strcmp(wifi_scan_result[i], reinterpret_cast<const char *>(res->ssid)) == 0) {
            return 0;
        }
    }

    if (index < 10) {
        char buf[64];
        int len = (res->ssid_len < 63) ? res->ssid_len : 63;
        memcpy(buf, res->ssid, len);
        buf[len] = '\0';
        ts->set_wifi_scan_result(index, buf);
        ts->set_wifi_ssid_index(index + 1);
    }
    return 0;
}


void thing_speak_service::scan_wifi_ssid_arr(void *param) {
    printf("Start Wi-Fi scan...\n");
    auto *ts = static_cast<thing_speak *>(param);
    ts->set_wifi_ssid_index(0);
    ts->init_wifi_scan_result();
    cyw43_wifi_scan_options_t opts = {0};
    int rc = cyw43_wifi_scan(&cyw43_state, &opts, ts, scan_cb);
    if (rc != 0) {
        printf("scan start failed: %d\n", rc);
    }
    vTaskDelay(pdMS_TO_TICKS(6000));
    xEventGroupSetBits(ts->get_co2_wifi_scan_event_group(), WIFI_SCAN_DONE);
    printf("Scan success.\n");
}

void thing_speak_service::network_init(void *param) {
    auto *ts = static_cast<thing_speak *>(param);
    printf("network init start\n");
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_WORLDWIDE) != 0) {
        printf("network init failed\n");
        return;
    }
    cyw43_arch_enable_sta_mode();
    xEventGroupSetBits(ts->get_co2_wifi_scan_event_group(), WIFI_INIT); // set wifi init success bit
    printf("network init success\n");
    vTaskDelete(nullptr); // delete self task
}


void thing_speak_service::start(void *param) {
    auto *ts = static_cast<thing_speak *>(param);
    xTaskCreate(network_init, "network_init_task", 256, param, tskIDLE_PRIORITY + 2, nullptr);
    xTaskCreate(wifi_connect, "wifi_connect", 512, param, tskIDLE_PRIORITY + 2, ts->get_wifi_connect_handle_ptr());
    xTaskCreate(get_setting_co2_val_or_upload, "get_setting_co2_val_or_upload", 1536, param,
                tskIDLE_PRIORITY + 1, nullptr);
}

void thing_speak_service::wifi_disconnect() {
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
    vTaskDelay(pdMS_TO_TICKS(200));
    cyw43_arch_lwip_begin();
    dhcp_stop(&cyw43_state.netif[CYW43_ITF_STA]);
    netif_set_down(&cyw43_state.netif[CYW43_ITF_STA]);
    cyw43_arch_lwip_end();
    vTaskDelay(pdMS_TO_TICKS(300));
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
    vTaskDelay(pdMS_TO_TICKS(500));
}

void print_waiting_log(const char *fmt, int times, int each_ms) {
    for (int i = 0; i < times; i++) {
        printf("%s, rest of %d\n", fmt, times - i);
        vTaskDelay(pdMS_TO_TICKS(each_ms));
    }
}
