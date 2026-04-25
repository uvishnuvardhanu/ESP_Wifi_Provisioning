#include "esp_http_server.h"
#include "led.h"
#include "esp_log.h"

static const char *TAG = "server_handler";

/* ===================== ROOT HANDLER ===================== */
esp_err_t root_handler(httpd_req_t *req)
{
    const char *resp =
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<title>LED Control</title>"
        "<style>"
        "body {"
        "font-family: Arial, sans-serif;"
        "margin: 0; padding: 0;"
        "background: linear-gradient(135deg,#1e3c72,#2a5298);"
        "color: white; text-align: center;"
        "}"
        ".container {"
        "display: flex;"
        "flex-direction: column;"
        "justify-content: center;"
        "align-items: center;"
        "height: 100vh;"
        "}"
        "h1 { margin-bottom: 30px; font-size: 32px; }"
        ".btn {"
        "display: inline-block;"
        "padding: 15px 30px;"
        "margin: 10px;"
        "font-size: 18px;"
        "font-weight: bold;"
        "border: none;"
        "border-radius: 8px;"
        "cursor: pointer;"
        "transition: 0.3s;"
        "}"
        ".btn-on { background: #4CAF50; color: white; }"
        ".btn-on:hover { background: #45a049; }"
        ".btn-off { background: #f44336; color: white; }"
        ".btn-off:hover { background: #da190b; }"
        ".bg-text {"
        "position: fixed;"
        "top: 50%; left: 50%;"
        "transform: translate(-50%, -50%);"
        "font-size: 80px;"
        "color: rgba(255,255,255,0.1);"
        "font-weight: bold;"
        "pointer-events: none;"
        "}"
        "</style>"
        "</head>"

        "<body>"
        "<div class='bg-text'>Elecbits</div>"
        "<div class='container'>"
        "<h1>LED Control Panel</h1>"
        "<a href='/on'><button class='btn btn-on'>Turn ON</button></a>"
        "<a href='/off'><button class='btn btn-off'>Turn OFF</button></a>"
        "</div>"
        "</body>"
        "</html>";

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* ===================== LED ON HANDLER ===================== */
esp_err_t on_handler(httpd_req_t *req)
{
    led_on();

    const char *resp =
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<title>LED ON</title>"
        "</head>"
        "<body style='background:linear-gradient(135deg,#1e3c72,#2a5298);"
        "color:white;text-align:center;'>"
        "<h1>LED is ON</h1>"
        "<a href='/' style='color:white;'>Back to Control</a>"
        "</body>"
        "</html>";

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* ===================== LED OFF HANDLER ===================== */
esp_err_t off_handler(httpd_req_t *req)
{
    led_off();

    const char *resp =
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<title>LED OFF</title>"
        "</head>"
        "<body style='background:linear-gradient(135deg,#1e3c72,#2a5298);"
        "color:white;text-align:center;'>"
        "<h1>LED is OFF</h1>"
        "<a href='/' style='color:white;'>Back to Control</a>"
        "</body>"
        "</html>";

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* ===================== SERVER INIT ===================== */
void server_init(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t root_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_handler,
            .user_ctx = NULL};

        httpd_uri_t on_uri = {
            .uri = "/on",
            .method = HTTP_GET,
            .handler = on_handler,
            .user_ctx = NULL};

        httpd_uri_t off_uri = {
            .uri = "/off",
            .method = HTTP_GET,
            .handler = off_handler,
            .user_ctx = NULL};

        httpd_register_uri_handler(server, &root_uri);
        httpd_register_uri_handler(server, &on_uri);
        httpd_register_uri_handler(server, &off_uri);

        ESP_LOGI(TAG, "HTTP server started");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to start HTTP server");
    }
}