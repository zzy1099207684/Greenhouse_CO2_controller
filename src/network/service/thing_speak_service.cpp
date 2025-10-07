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


Json_handler handler;

static char *extract_json(char *http_response);

extern "C" {
bool run_tls_client(const uint8_t *cert, size_t cert_len, const char *server, const char *request, int timeout,
                    void *tsp);
}

extern "C" {
void get_data(char *param, void *tsp) {
    auto *ts = static_cast<thing_speak *>(tsp);
    if (strstr(param, "SAMEORIGIN") != nullptr) {
        if (strstr(param, "feeds") != nullptr) {
            char *json = extract_json(param);
            ts->set_response(json);
            thing_speak_service::deal_SETTING_CO2_data(ts);
        }
    } else {
        int back_res = atoi(param);
        ts->set_CO2_level(INT_MIN);
        ts->set_Relative_humidity(FLT_MIN);
        ts->set_Temperature(FLT_MIN);
        ts->set_fan_speed(INT_MIN);
        if (back_res > 0) {
            printf("upload success\n");
        }
    }
}
}

// timer
void thing_speak_service::get_SETTING_CO2_data(void *param) {
    int count = 0;
    for (;;) {
        auto *ts = static_cast<thing_speak *>(param);
        xEventGroupWaitBits(ts->get_co2_wifi_scan_event_group(),
                            WIFI_CONNECTED_GET_SETTING_CO2_DATA, pdFALSE, pdTRUE,
                            portMAX_DELAY); // wait for wifi connected
        printf("get_SETTING_CO2_data timer start !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        // "GET /update?api_key=91TLCMJCMTU4K9Z0&field1=0 HTTP/1.1\r\nHost: api.thingspeak.com\r\nConnection: close\r\n\r\n"
        char request[300] = {};
        sprintf(request,
                "GET /channels/3083662/feeds.json?api_key=%s&results=1 HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
                ts->get_read_api_key(), ts->get_api_server());
        ts->set_request(request);
        // if co2 setting data is from hardware, do not get data from thing speak 2 times
        if (!ts->get_is_co2_setting_data_from_hardware()) { // from network
            request_HTTPS(ts);
        } else { // from hardware
            if(count < 2) {
                request_HTTPS(ts);
            }else {
                ts->set_is_co2_setting_data_from_hardware(false);
            }
            count++;
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void thing_speak_service::deal_SETTING_CO2_data(void *param) {
    auto *ts = static_cast<thing_speak *>(param);
    handler.get_value_from_key(ts->get_response(), "feeds");
    char *value = handler.get_final_result();
    handler.get_value_from_key(value, "field5");
    value = handler.get_final_result();
    if (strcmp(value, "") != 0) {
        const int field_5 = atoi(value);
        if ((field_5 != ts->get_co2_level_from_network()) && (field_5 != 0)) {
            printf("SETTING CO2 level from thing speak: %d\n", field_5);
            ts->set_co2_level_from_network(field_5);
            xEventGroupSetBits(ts->get_co2_wifi_scan_event_group(), NETWORK_SET_CO2); // set co2 level change bit
        }
    }
}

// task
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
        sprintf(params, "field1=%d&", field_1);
        if (field_2 == FLT_MIN) field_2 = 0.0f;
        sprintf(params + strlen(params), "field2=%f&", field_2);
        if (field_3 == FLT_MIN) field_3 = 0.0f;
        sprintf(params + strlen(params), "field3=%f&", field_3);
        if (field_4 == INT_MIN) field_4 = 0;
        sprintf(params + strlen(params), "field4=%d&", field_4);
        if (field_5 == INT_MIN) field_5 = 0;
        sprintf(params + strlen(params), "field5=%d&", field_5);
        params[strlen(params) - 1] = '\0';
        char request[200] = {};
        sprintf(request, "GET /update?api_key=%s&%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
                ts->get_write_api_key(), params, ts->get_api_server());
        ts->set_request(request);
        printf("request %s", request);
        request_HTTPS(ts); //upload data to thing speak
        vTaskDelay(pdMS_TO_TICKS(15000));
    }
}

void thing_speak_service::request_HTTPS(void *param) {
    auto *ts = static_cast<thing_speak *>(param);
    constexpr uint8_t things_cert[] = THINGSPEAK_CERT;
    const char *server = ts->get_api_server();
    const char *request = ts->get_request();
    xSemaphoreTake(ts->get_net_mutex(), portMAX_DELAY);
    bool pass = run_tls_client(things_cert, sizeof(things_cert), server, request,
                               TLS_CLIENT_TIMEOUT_SECS, ts);
    xSemaphoreGive(ts->get_net_mutex());

    if (pass) {
        printf("Test passed\n");
    } else {
        printf("Test failed\n");
    }
    /* sleep a bit to let usb stdio write out any buffer to host */
    vTaskDelay(pdMS_TO_TICKS(100));
}

void thing_speak_service::wifi_connect(void *param) {
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
                xEventGroupSetBits(ts->get_co2_wifi_scan_event_group(),
                                   WIFI_CONNECTED | WIFI_CONNECTED_UPLOAD_DATA_TO_THING_SPEAK |
                                   WIFI_CONNECTED_GET_SETTING_CO2_DATA);
                vTaskSuspend(ts->get_wifi_connect_handle());
                fail_count = 0;
            } else {
                printf("connect status: no ip\n");
                fail_count++;
            }
        } else {
            xEventGroupClearBits(ts->get_co2_wifi_scan_event_group(),
                                 WIFI_CONNECTED | WIFI_CONNECTED_UPLOAD_DATA_TO_THING_SPEAK |
                                 WIFI_CONNECTED_GET_SETTING_CO2_DATA);
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


static char *extract_json(char *http_response) {
    char *body = strstr(http_response, "\r\n{");
    if (!body) {
        return nullptr;
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

static int scan_cb(void *param, const cyw43_ev_scan_result_t *res) {
    if (!res) {
        // 扫描结束
        return 0;
    }
    auto *ts = static_cast<thing_speak *>(param);
    auto wifi_scan_result = ts->get_wifi_scan_result();
    const int index = ts->get_wifi_ssid_index();
    for (int i = 0; i < 10; ++i) {
        if (wifi_scan_result[i][0] != '\0' &&
            strcmp(wifi_scan_result[i], reinterpret_cast<const char *>(res->ssid)) == 0) {
            return 0;
        }
    }
    if (index < 10 && res->ssid_len > 0) {
        char buf[64];
        int len = res->ssid_len;
        if (len > 63) len = 63;
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
    cyw43_arch_lwip_begin();
    vTaskDelay(pdMS_TO_TICKS(3000));
    cyw43_arch_lwip_end();
    xEventGroupSetBits(ts->get_co2_wifi_scan_event_group(), WIFI_SCAN_DONE);
    printf("Scan done.\n");
}

void thing_speak_service::network_init(void *param) {
    auto *ts = static_cast<thing_speak *>(param);
    printf("WiFi init start\n");
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_WORLDWIDE) != 0) {
        printf("WiFi init failed\n");
        return;
    }
    cyw43_arch_enable_sta_mode();
    xEventGroupSetBits(ts->get_co2_wifi_scan_event_group(), WIFI_INIT); // set wifi init success bit
    printf("WiFi init success\n");
}


void thing_speak_service::start(void *param) {
    printf("start start start\n");
    auto *ts = static_cast<thing_speak *>(param);
    xEventGroupWaitBits(ts->get_co2_wifi_scan_event_group(),
                        WIFI_CONNECTED, pdFALSE,
                        pdTRUE, portMAX_DELAY); // wait for wifi connected
    xTaskCreate(get_SETTING_CO2_data, "get_SETTING_CO2_data", 4096, param,
                tskIDLE_PRIORITY + 1, nullptr);

    xTaskCreate(upload_data_to_thing_speak, "upload_data_to_thing_speak", 4096, param,
                tskIDLE_PRIORITY + 1, nullptr);
    vTaskDelete(nullptr); // delete self task
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
