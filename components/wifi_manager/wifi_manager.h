#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi.h"

// Event group bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_DISCONNECTED_BIT BIT1

extern char device_mac_str[18];

void init_device_mac(void);

int get_wifi_rssi(void);

// Global event group handle
extern EventGroupHandle_t wifi_event_group;

// Initialize WiFi
void wifi_init(void);

// WiFi event handler
void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

#endif // WIFI_MANAGER_H
