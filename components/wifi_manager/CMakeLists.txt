idf_component_register(
    SRCS "wifi_manager.c" "wifi_prov.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi esp_event esp_netif nvs_flash esp_http_server server_handler
)
