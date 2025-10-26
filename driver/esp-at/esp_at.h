#ifndef __ESP_AT_H
#define __ESP_AT_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"
//#include "rtc.h"
#include "log.h"
#include "delay.h"
#include "esp_uart.h"
#include "weather.h"
#include "lcd.h"
#include "image.h"

#define RX_BUFFER_SIZE  1024

#define RX_RESULT_OK    0   /* AT命令返回OK */
#define RX_RESULT_ERROR 1   /* AT命令返回ERROR */
#define RX_RESULT_FAIL  2   /* 数据接收错误，缓存区满 */


bool esp_at_init(void);
bool esp_send_at(const char *cmd, const char **rsp, uint32_t *length, uint32_t timeout);


bool esp_at_reset(void);
bool esp_at_wifi_init(void);
bool esp_at_wifi_connect(const char *ssid ,const char *pwd);
bool esp_at_get_http(const char *url,uint32_t timeout);
bool esp_at_get_weather(void);
bool esp_at_sntp_init(void);
bool esp_at_sntp_get(void);


bool parse_sntp_time(const char *resp, rtc_date_time_t *t);
int sntp_update_rtc(const char *sntp_str, int timezone_offset);
bool esp_at_fetch_sntp_time(void);
bool parse_sntp_time(const char *resp, rtc_date_time_t *t);
bool sntp_process_time(const char *sntp_str, int timezone_offset, rtc_date_time_t *result);

bool sntp_perform_sync(int timezone_offset, int max_retries);
void esp_at_sntp_sync(void);
void esp_at_sntp_sync_with_timezone(int timezone_offset);
bool esp_at_sntp_sync_ex(int timezone_offset, int max_retries);
int sntp_update_rtc(const char *sntp_str, int timezone_offset);

bool esp_at_get_time(uint32_t *timestamp);
bool esp_at_wifi_get_ip(char ip[16]);
bool esp_at_wifi_get_mac(char mac[18]);


#endif /* __ESP_AT_H */

