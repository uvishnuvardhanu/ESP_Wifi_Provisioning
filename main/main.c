#include "esp_log.h"
#include "led.h"
#include "wifi_manager.h"
#include "spiffs_init.h"

/* ===================== MAIN ===================== */
void app_main(void)
{
    init_spiffs();

    wifi_init();
    led_init();

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Main loop delay
    }
}