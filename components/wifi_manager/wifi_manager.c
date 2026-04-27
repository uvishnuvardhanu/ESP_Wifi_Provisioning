#include "wifi_manager.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include <stdio.h>
#include "wifi_prov.h"
#include "server_handler.h"

// #define ESP_WIFI_SSID "Hotspot"
// #define ESP_WIFI_PASS "12341234"

static const char *TAG = "wifi_manager";

EventGroupHandle_t wifi_event_group;
char device_mac_str[18];

static int retry_count = 0;

/* ===================== MAC ADDRESS ===================== */
void init_device_mac(void)
{
    uint8_t mac[6];

    // Get base MAC from eFuse
    esp_efuse_mac_get_default(mac);

    snprintf(device_mac_str, sizeof(device_mac_str),
             "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2],
             mac[3], mac[4], mac[5]);
}

/* ===================== WIFI RSSI ===================== */
int get_wifi_rssi(void)
{
    wifi_ap_record_t ap_info;

    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK)
    {
        return ap_info.rssi; // in dBm
    }

    return -127; // invalid RSSI
}

/* ===================== WIFI EVENT HANDLER ===================== */
void wifi_event_handler(void *arg,
                        esp_event_base_t event_base,
                        int32_t event_id,
                        void *event_data)
{
    /* -------- WIFI EVENTS -------- */
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "Connected to AP");
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            if (retry_count < 10)
            {
                retry_count++;
                ESP_LOGW(TAG, "Disconnected, retrying... (%d/10)", retry_count);

                xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
                esp_wifi_connect();
            }
            else
            {
                ESP_LOGW(TAG, "Failed after 10 retries, starting provisioning...");

                // Stop STA mode before switching
                esp_wifi_stop();

                // Launch SoftAP provisioning
                start_softap_provisioning();
            }
            break;

        default:
            break;
        }
    }

    /* -------- IP EVENTS -------- */
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

        retry_count = 0; // reset retry count on success

        // Start HTTP server after connection
        server_init();
    }
}

/* ===================== WIFI INIT ===================== */
void wifi_init(void)
{
    esp_err_t ret;

    /* -------- NVS INIT -------- */
    ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    /* -------- NETWORK STACK -------- */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* -------- WIFI DRIVER -------- */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* -------- EVENT GROUP -------- */
    wifi_event_group = xEventGroupCreate();

    /* -------- EVENT HANDLERS -------- */
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            &instance_any_id));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL,
            &instance_got_ip));

    /* -------- CHECK SAVED WIFI -------- */
    nvs_handle_t nvs;
    char ssid[32];
    char pass[64];

    size_t ssid_len = sizeof(ssid);
    size_t pass_len = sizeof(pass);

    if (nvs_open("wifi", NVS_READONLY, &nvs) == ESP_OK &&
        nvs_get_str(nvs, "ssid", ssid, &ssid_len) == ESP_OK &&
        nvs_get_str(nvs, "pass", pass, &pass_len) == ESP_OK)
    {
        ESP_LOGI(TAG, "Found saved WiFi, SSID: %s", ssid);

        // Create STA interface
        esp_netif_create_default_wifi_sta();

        wifi_config_t wifi_config = {0};

        strcpy((char *)wifi_config.sta.ssid, ssid);
        strcpy((char *)wifi_config.sta.password, pass);

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
    }
    else
    {
        ESP_LOGI(TAG, "No saved WiFi, starting provisioning...");
        start_softap_provisioning();
    }

    nvs_close(nvs);
}