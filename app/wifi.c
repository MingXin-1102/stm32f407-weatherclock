#include "wifi.h"
void wifi_init (void)
{
    if (!esp_at_init())
    {
        log_printf("[AT] init failed\n");
        goto err;
    }
    log_printf("[AT] inited\n");

    if (!esp_at_wifi_init()){
        log_printf("[WIFI] init error!\n");
        goto err;
    }
    log_printf("[WIFI] init ok!\n");

    if (!esp_at_sntp_init())
    {
        log_printf("[SNTP] init failed\n");
        goto err;
    }
    log_printf("[SNTP] init ok!\n");

    return ;
err:
    error_page_display("wifi failed");
    while (1)
    {
        ;
    }
}

void wifi_wait_connect(void)
{
    log_printf("[WIFI] connecting\n");

    esp_at_connect_wifi(WIFI_SSID, WIFI_PASSWORD, NULL);

    for (uint32_t t = 0; t < 10 * 1000; t += 100)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        esp_wifi_info_t wifi = { 0 };
        if (esp_at_get_wifi_info(&wifi) && wifi.connected)
        {
            log_printf("[WIFI] Connected\n");
            log_printf("[WIFI] SSID: %s, BSSID: %s, Channel: %d, RSSI: %d\n",
                wifi.ssid, wifi.bssid, wifi.channel, wifi.rssi);
            return;
        }
    }
    log_printf("[WIFI] Connection Timeout\n");
    error_page_display("wifi connect failed");
    while (1)
    {
        ;
    }
}

