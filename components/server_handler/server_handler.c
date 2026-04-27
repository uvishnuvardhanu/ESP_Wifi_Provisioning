#include "esp_http_server.h"
#include "led.h"
#include "esp_log.h"

static const char *TAG = "server_handler";

/* ===================== ROOT HANDLER ===================== */
esp_err_t root_handler(httpd_req_t *req)
{
    FILE *f = fopen("/spiffs/led.html", "r");
    if (!f)
    {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char buffer[256];
    httpd_resp_set_type(req, "text/html");

    while (fgets(buffer, sizeof(buffer), f))
    {
        httpd_resp_sendstr_chunk(req, buffer);
    }
    fclose(f);

    httpd_resp_sendstr_chunk(req, NULL); // end response
    return ESP_OK;
}

/* ===================== LED ON HANDLER ===================== */
esp_err_t on_handler(httpd_req_t *req)
{
    led_on();

    // Send a redirect back to root
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);

    return ESP_OK;
}

esp_err_t off_handler(httpd_req_t *req)
{
    led_off();

    // Send a redirect back to root
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);

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