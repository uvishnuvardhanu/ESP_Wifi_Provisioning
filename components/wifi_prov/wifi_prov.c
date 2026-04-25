#include "wifi_prov.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <string.h>

#define WIFI_SSID "ESP32_SETUP"
#define WIFI_PASS "12345678"

static const char *TAG = "PROV";
static httpd_handle_t server = NULL;

static const char *html_form =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>Elecbits WiFi Setup</title>"
    "<style>"
    "* { box-sizing: border-box; font-family: Arial, sans-serif; }"
    "body { margin:0; height:100vh; display:flex; justify-content:center; align-items:center; background: linear-gradient(135deg, #1e3c72, #2a5298); }"
    ".container { width:90%; max-width:350px; }"
    ".brand { text-align:center; color:white; font-size:28px; font-weight:bold; margin-bottom:20px; }"
    ".card { background:white; padding:20px; border-radius:12px; box-shadow:0 6px 20px rgba(0,0,0,0.2); }"
    "h3 { margin-top:0; text-align:center; }"
    "input { width:100%; padding:12px; margin:10px 0; border-radius:6px; border:1px solid #ccc; font-size:14px; }"
    "button { width:100%; padding:12px; background:#1e3c72; color:white; border:none; border-radius:6px; font-size:16px; }"
    "button:active { background:#16325c; }"
    "</style>"
    "</head>"
    "<body>"
    "<div class='container'>"
    "<div class='brand'>Elecbits</div>"
    "<div class='card'>"
    "<h3>WiFi Setup</h3>"
    "<form method='POST' action='/connect'>"
    "<input type='text' name='ssid' placeholder='Enter SSID' required>"
    "<input type='password' name='pass' placeholder='Enter Password' required>"
    "<button type='submit'>Connect</button>"
    "</form>"
    "</div>"
    "</div>"
    "</body>"
    "</html>";

/* Serve HTML */
esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html"); // IMPORTANT for mobile
    httpd_resp_send(req, html_form, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* Handle credentials */
esp_err_t connect_post_handler(httpd_req_t *req)
{
    int buf_len = req->content_len;
    char *buf = malloc(buf_len + 1);
    if (!buf)
        return ESP_ERR_NO_MEM;

    int ret = httpd_req_recv(req, buf, buf_len);
    if (ret <= 0)
    {
        free(buf);
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    char ssid[32] = {0};
    char pass[64] = {0};

    if (httpd_query_key_value(buf, "ssid", ssid, sizeof(ssid)) == ESP_OK &&
        httpd_query_key_value(buf, "pass", pass, sizeof(pass)) == ESP_OK)
    {

        ESP_LOGI(TAG, "Received SSID: %s PASS: %s", ssid, pass);

        nvs_handle_t nvs;
        if (nvs_open("wifi", NVS_READWRITE, &nvs) == ESP_OK)
        {
            nvs_set_str(nvs, "ssid", ssid);
            nvs_set_str(nvs, "pass", pass);
            nvs_commit(nvs);
            nvs_close(nvs);
        }

        httpd_resp_set_type(req, "text/html");
        httpd_resp_sendstr(req, "<h3>Saved! Rebooting...</h3>");

        free(buf);
        vTaskDelay(pdMS_TO_TICKS(2000)); // give browser time to receive response
        esp_restart();
        return ESP_OK;
    }

    free(buf);
    httpd_resp_sendstr(req, "Invalid input");
    return ESP_FAIL;
}

/* Start Web Server */
static void start_webserver()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_start(&server, &config);

    httpd_uri_t root = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_get_handler};

    httpd_uri_t connect = {
        .uri = "/connect",
        .method = HTTP_POST,
        .handler = connect_post_handler};

    httpd_register_uri_handler(server, &root);
    httpd_register_uri_handler(server, &connect);
}

/* Start SoftAP */
void start_softap_provisioning(void)
{
    esp_netif_create_default_wifi_ap(); // <-- important

    wifi_config_t ap_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .password = WIFI_PASS,
            .channel = 6,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK}};

    if (strlen(WIFI_PASS) < 8)
    {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
        ap_config.ap.password[0] = '\0';
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "SoftAP started, SSID:%s PASS:%s", WIFI_SSID, WIFI_PASS);
    start_webserver();
}
