#include "main.h"

// extern char rx_buffer[RX_BUFFER_SIZE];
extern volatile bool frame_flag; // 标记一帧数据接收完成

extern volatile bool flag_weather ;
extern volatile bool flag_sntp    ;
extern volatile bool flag_aht20   ;
extern volatile bool flag_time    ;
// float tempe,humi;

char show_buffer[100];
extern void board_init(void);
bool wifi_status;
void esp_start(void)
{

    if (!esp_at_init()){
        log_printf("[ERROR] ESP32 init failed!\r\n");
        error_page_display();
    }else{
        log_printf("[INFO] ESP32 init ok!\r\n");
    }
    Delay_Us(10);

    /* WIFI名称 */
    static const char *wifi_ssid = "Minglan_739";
    /* WIFI密码 */
    static const char *wifi_password = "Sml991102";
    if (!esp_at_wifi_init()){
        log_printf("[INFO] ESP32 wifi init ok!\r\n");
        wifi_status = true;
        wifi_page_display();
    }else{
        log_printf("[ERROR]ESP32 wifi init error!\r\n");
        wifi_status = false;
        error_page_display();
    }
    Delay_Us(10);

    if (!esp_at_wifi_connect(wifi_ssid ,wifi_password)){
        log_printf("[INFO] ESP32 wifi connect ok!\r\n");
    }else
        log_printf("[ERROR]ESP32 wifi connect error!\r\n");
        error_page_display();
    // Delay_Us(10);
    // if (!esp_at_sntp_init()){
    //     log_printf("[ERROR]ESP32 sntp init error!\r\n");
    //     error_page_display();
    // }
    main_page_display();
}
int main(void)
{
	board_init();
    welcome_page_display();
    esp_start();
    esp_at_sntp_sync();
    update_wifi_ui(wifi_status);
    update_time_ui();
    update_aht20_ui();
    update_outside_ui();

    while (1)
    {
        if (flag_time) {
            flag_time = false;
            update_time_ui();
        }
        if (flag_weather) {
            flag_weather = false;
            update_outside_ui();
            update_wifi_ui(wifi_status);
        }

        if (flag_sntp) {
            flag_sntp = false;
            esp_at_sntp_sync();
        }

        if (flag_aht20) {
            flag_aht20 = false;
            update_aht20_ui();
            // update_inner_ui();
        }
        //MCU 进入低功耗模式，等待中断唤醒
        __WFI();
    }
}


