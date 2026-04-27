idf_component_register(
    SRCS "server_handler.c"
    INCLUDE_DIRS "."
    REQUIRES esp_event esp_netif nvs_flash esp_http_server LED log
)
