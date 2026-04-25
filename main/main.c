#include "nvs_flash.h"
#include "wifi_prov.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

static const char *TAG = "MAIN";
static int retry_count = 0;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (retry_count < 10)
        {
            retry_count++;
            ESP_LOGW(TAG, "Disconnected, retrying... (%d/10)", retry_count);
            esp_wifi_connect();
        }
        else
        {
            ESP_LOGW(TAG, "Failed to connect after 10 retries, starting provisioning...");
            // Stop STA mode before switching
            esp_wifi_stop();
            // Launch SoftAP provisioning
            start_softap_provisioning();
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        retry_count = 0; // reset on success
    }
}

void wifi_init_sta(char *ssid, char *pass)
{
    esp_netif_create_default_wifi_sta();

    wifi_config_t wifi_config = {0};
    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, pass);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Connecting to %s...", ssid);
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    nvs_handle_t nvs;
    char ssid[32], pass[64];
    size_t ssid_len = sizeof(ssid);
    size_t pass_len = sizeof(pass);

    if (nvs_open("wifi", NVS_READONLY, &nvs) == ESP_OK &&
        nvs_get_str(nvs, "ssid", ssid, &ssid_len) == ESP_OK &&
        nvs_get_str(nvs, "pass", pass, &pass_len) == ESP_OK)
    {

        ESP_LOGI(TAG, "Found saved WiFi, SSID:%s", ssid);
        wifi_init_sta(ssid, pass);
    }
    else
    {
        ESP_LOGI(TAG, "No saved WiFi, starting provisioning...");
        start_softap_provisioning();
    }

    nvs_close(nvs);
}
