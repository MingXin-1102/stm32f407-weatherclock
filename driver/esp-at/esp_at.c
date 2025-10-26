#include "esp_at.h"

char rx_buffer[RX_BUFFER_SIZE];
volatile uint16_t rx_index = 0;
volatile bool frame_flag = false; // 标记一帧数据接收完成
volatile uint8_t rx_complete = 0; // 标记一帧接收完成

volatile uint32_t rxlen = 0 ;
volatile bool rxready = false ;
volatile uint8_t rxresult ;

static const char *weather_uri = "https://api.seniverse.com/v3/weather/now.json?key=SACM4ELUFLi5atgdN&location=Nanjing&language=en&unit=c";
weather_info_t weather_info;
rtc_date_time_t sntp_get_time;
static void on_usart_received(uint8_t rxstring)
{
    if (!rxready) return;

    if (rx_index < RX_BUFFER_SIZE - 1) {
        rx_buffer[rx_index++] = rxstring;
        rx_buffer[rx_index] = '\0'; // 封尾
    } else {
        rxresult = RX_RESULT_FAIL;
        rxready = false;
        return;
    }

    // 判断是否接收完成（以OK或ERROR作为一帧结束）
    if (strstr((char *)rx_buffer, "\r\nOK\r\n")) {
        rxresult = RX_RESULT_OK;
        rxready = false;
        frame_flag = true;
    } else if (strstr((char *)rx_buffer, "\r\nERROR\r\n")) {
        rxresult = RX_RESULT_ERROR;
        rxready = false;
    }
}

bool esp_send_at(const char *cmd, const char **rsp, uint32_t *length, uint32_t timeout)
{
    if (!cmd) return false;

    rx_index = 0;
    memset((char *)rx_buffer, 0, sizeof(rx_buffer)); // 缓冲区清空
    rxready = true;            // 准备接收数据
    rxresult = RX_RESULT_FAIL; // 初始化接收结果标志位  失败

    char at_cmd[128];
    snprintf(at_cmd, sizeof(at_cmd), "%s\r\n", cmd);
    // 发送 AT 命令
    USART2_SendString(at_cmd);
    // 等待接收完成或超时
    if (rxready)
    {
        for (int i = 0; i < timeout; i++)
        {
            Delay_Ms(5);
        }
        rxready = false; // 超时后关闭接收标志位
    }

    if (rsp)
    {
        *rsp = (const char *)rx_buffer;
    }
    if (length)
    {
        *length = rx_index;
    }

    /*超时和正常都在这里收尾*/
    if(rxresult == RX_RESULT_OK){
        return true;
    }else if(rxresult == RX_RESULT_ERROR){
        return false;
    }else{
        return false;
    }
}

bool esp_at_init(void)
{
   rxready = false;
   esp_uart_init(115200);
   esp_uart_recv_callback_register(on_usart_received);
   return esp_at_reset();
}

bool esp_at_reset(void)
{
    if (!esp_send_at("AT+RESTORE",NULL,NULL,500))
    {
        return false;
    }
    // Delay_Ms(50);
    if (!esp_send_at("ATE0", NULL,NULL,500) )
    {
        return false;
    }

    if (!esp_send_at("AT+SYSSTORE=0", NULL,NULL,500) )
    {
        return false;
    }

    // if (!esp_send_at("AT+CIPSNTPCFG=1,8,\"cn.ntp.org.cn\",\"ntp.sjtu.edu.cn\"", NULL, NULL, 1000) )
    // {
    //     return false;
    // }
    return true;
}

bool esp_at_wifi_init(void)
{
    if (esp_send_at("AT+CWMODE=1",NULL,NULL,500) != 0)
    {
        return false;
    }
    return true;
}

bool esp_at_wifi_connect(const char *ssid, const char *pwd)
{
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);
    if (esp_send_at(cmd, NULL,NULL,500) != false)
    {
        return false;
    }
    return true;
}

bool esp_at_get_http(const char *url, uint32_t timeout)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+HTTPCGET=\"%s\"", url);
    if (esp_send_at(cmd, NULL,NULL,timeout) != false)
    {
        return false;
    }
    return true;
}
bool esp_at_get_weather(void)
{
    if ( !esp_at_get_http(weather_uri,500) ){
        if (frame_flag)
        {
            frame_flag = false;
            // printf("rx buffer :%s\r\n",rx_buffer);
            if (parse_seniverse_response((char *)rx_buffer, &weather_info))
            {
                log_printf("[INFO] ESP32 get weather ok!\r\n");
                return true;
            }
        }
    }
    log_printf("[ERROR] ESP32 get weather error!\r\n");
    return false;
}

bool esp_at_sntp_init(void)
{
    /* 设置sntp服务器 */
    if (esp_send_at("AT+CIPSNTPCFG=1,0,\"cn.ntp.org.cn\",\"ntp.sjtu.edu.cn\"", NULL, NULL, 1000) != false)
    {
        return false;
    }
    return true;
}
bool esp_at_sntp_get(void)
{
    /* 从sntp服务器获取时间，此时会校准esp32的rtc时间 */
    if (esp_send_at("AT+CIPSNTPTIME?", NULL, NULL, 1000) != false)
    {
        return false;
    }
    return true;
}

// ==================== SNTP 数据获取层 ====================
/**
 * @brief 获取SNTP时间原始数据
 * @return true-成功 false-失败
 */
bool esp_at_fetch_sntp_time(void)
{
    if (esp_at_sntp_init()) {
        log_printf("[ERROR] ESP32 sntp init error!\r\n");
        return false;
    }

    if (esp_at_sntp_get()) {
        log_printf("[ERROR] ESP32 sntp get error!\r\n");
        return false;
    }

    if (!frame_flag) {
        log_printf("[ERROR] No SNTP response received\r\n");
        return false;
    }

    frame_flag = false;
    log_printf("[INFO] SNTP raw data: %s\r\n", rx_buffer);
    return true;
}

/**
 * @brief 获取SNTP响应数据
 * @return 指向SNTP响应数据的指针
 */
const char* esp_at_get_sntp_response(void)
{
    return (const char*)rx_buffer;
}

// ==================== SNTP 数据处理层 ====================
/**
 * @brief 解析SNTP时间字符串
 * @param resp SNTP响应字符串
 * @param t 输出的时间结构体
 * @return true-成功 false-失败
 */
bool parse_sntp_time(const char *resp, rtc_date_time_t *t)
{
    memset(t, 0, sizeof(rtc_date_time_t));

    const char *p = strstr(resp, "+CIPSNTPTIME:");
    if (!p) {
        log_printf("[parse_sntp_time] +CIPSNTPTIME: not found\r\n");
        return false;
    }
    p += strlen("+CIPSNTPTIME:");

    char week[4] = {0}, month[4] = {0};
    unsigned int day = 0, hour = 0, minute = 0, second = 0, year = 0;

    int matched = sscanf(p, "%3s %3s %u %u:%u:%u %u",
                        week, month, &day, &hour, &minute, &second, &year);

    if (matched != 7) {
        matched = sscanf(p, " %3s %3s %u %u:%u:%u %u",
                        week, month, &day, &hour, &minute, &second, &year);
        log_printf("[parse_sntp_time] Second try matched: %d\r\n", matched);
    }

    if (matched != 7) {
        log_printf("[parse_sntp_time] All parsing attempts failed\r\n");
        return false;
    }
    t->day = day;
    t->hour = hour;
    t->minute = minute;
    t->second = second;
    t->year = year;

    const char *weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    for (int i = 0; i < 7; i++) {
        if (strcmp(week, weekdays[i]) == 0) {
            t->weekday = i;
            break;
        }
    }

    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for (int i = 0; i < 12; i++) {
        if (strcmp(month, months[i]) == 0) {
            t->month = i + 1;
            break;
        }
    }

    log_printf("[parse_sntp_time] Parsed: %04d-%02d-%02d %02d:%02d:%02d weekday=%d\r\n",
               t->year, t->month, t->day, t->hour, t->minute, t->second, t->weekday);

    return true;
}

/**
 * @brief 判断是否为闰年
 * @param year 年份
 * @return 1-闰年 0-非闰年
 */
static int is_leap_year(int year)
{
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

/**
 * @brief 每个月的天数（非闰年）
 */
static const int days_in_month[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/**
 * @brief 时区调整
 * @param time 时间结构体
 * @param offset_hours 时区偏移（小时）
 */
static void adjust_timezone(rtc_date_time_t *time, int offset_hours)
{
    time->hour += offset_hours;
    // 处理小时进位
    while (time->hour >= 24) {
        time->hour -= 24;
        time->day += 1;
        //星期增加1，循环处理
        time->weekday = (time->weekday + 1) % 7;

        int dim = days_in_month[time->month - 1];
        if (time->month == 2 && is_leap_year(time->year)) {
            dim = 29;
        }
        if (time->day > dim) {
            time->day = 1;
            time->month += 1;
            if (time->month > 12) {
                time->month = 1;
                time->year += 1;
            }
        }
    }

    // 处理小时借位
//    while (time->hour < 0) {
//        time->hour += 24;
//        time->day -= 1;
//        //星期减少1，循环处理
//        time->weekday = (time->weekday + 6) % 7;

//        if (time->day < 1) {
//            time->month -= 1;
//            if (time->month < 1) {
//                time->month = 12;
//                time->year -= 1;
//            }
//            int dim = days_in_month[time->month - 1];
//            if (time->month == 2 && is_leap_year(time->year)) {
//                dim = 29;
//            }
//            time->day = dim;
//        }
//    }
}

/**
 * @brief 处理SNTP时间并转换为本地时间
 * @param sntp_str SNTP响应字符串
 * @param timezone_offset 时区偏移
 * @param result 输出的本地时间结构体
 * @return true-成功 false-失败
 */
bool sntp_process_time(const char *sntp_str, int timezone_offset, rtc_date_time_t *result)
{
    rtc_date_time_t utc_time;

    if (!parse_sntp_time(sntp_str, &utc_time)) {
        log_printf("[ERROR] SNTP parse failed\r\n");
        return false;
    }

    *result = utc_time;

    // 时区调整
    log_printf("[DEBUG] Before timezone adjustment: %04d-%02d-%02d %02d:%02d:%02d weekday=%d\r\n",
               result->year, result->month, result->day,
               result->hour, result->minute, result->second, result->weekday);
    adjust_timezone(result, timezone_offset);

    log_printf("[DEBUG] After timezone adjustment: %04d-%02d-%02d %02d:%02d:%02d weekday=%d\r\n",
               result->year, result->month, result->day,
               result->hour, result->minute, result->second, result->weekday);

    log_printf("[INFO] UTC time: %04d-%02d-%02d %02d:%02d:%02d\r\n",
               utc_time.year, utc_time.month, utc_time.day,
               utc_time.hour, utc_time.minute, utc_time.second);

    log_printf("[INFO] Local time: %04d-%02d-%02d %02d:%02d:%02d\r\n",
               result->year, result->month, result->day,
               result->hour, result->minute, result->second);

    return true;
}


// ==================== SNTP 同步服务层 ====================
/**
 * @brief 执行完整的SNTP时间同步流程
 * @param timezone_offset 时区偏移量
 * @param max_retries 最大重试次数
 * @return true-同步成功 false-同步失败
 */
bool sntp_perform_sync(int timezone_offset, int max_retries)
{
    for (int attempt = 0; attempt < max_retries; attempt++) {
        if (attempt > 0) {
            log_printf("[INFO] SNTP sync retry %d/%d\r\n", attempt + 1, max_retries);
            Delay_Ms(500); // 重试间隔
        }

        if (!esp_at_fetch_sntp_time()) {
            continue; // 获取失败，重试
        }

        const char *sntp_response = esp_at_get_sntp_response();
        rtc_date_time_t local_time;

        if (sntp_process_time(sntp_response, timezone_offset, &local_time)) {
            if (rtc_validate_time(&local_time)) {
                if (rtc_safe_set_time(&local_time)) {
                    log_printf("[INFO] SNTP time sync completed successfully\r\n");
                    return true;
                }
            } else {
                log_printf("[ERROR] SNTP processed time validation failed\r\n");
            }
        }
    }
    log_printf("[ERROR] SNTP time sync failed after %d attempts\r\n", max_retries);
    return false;
}

// ==================== 高层接口（兼容原有代码） ====================
/**
 * @brief SNTP同步并更新RTC（简单接口，使用默认参数）
 * 默认：北京时间UTC+8，重试3次
 */
void esp_at_sntp_sync(void)
{
    sntp_perform_sync(8, 3);
}

/**
 * @brief SNTP同步并更新RTC（带时区参数）
 * @param timezone_offset 时区偏移
 */
void esp_at_sntp_sync_with_timezone(int timezone_offset)
{
    sntp_perform_sync(timezone_offset, 3);
}

/**
 * @brief SNTP同步并更新RTC（完整参数）
 * @param timezone_offset 时区偏移
 * @param max_retries 最大重试次数
 * @return true-成功 false-失败
 */
bool esp_at_sntp_sync_ex(int timezone_offset, int max_retries)
{
    return sntp_perform_sync(timezone_offset, max_retries);
}


/**
 * @brief SNTP更新RTC（原有函数接口）
 * @param sntp_str SNTP字符串
 * @param timezone_offset 时区偏移
 * @return 1-成功 0-失败
 */
int sntp_update_rtc(const char *sntp_str, int timezone_offset)
{
    rtc_date_time_t local_time;

    if (sntp_process_time(sntp_str, timezone_offset, &local_time)) {
        return rtc_safe_set_time(&local_time) ? 1 : 0;
    }
    return 0;
}

// void esp_at_sntp_sync(void)
// {
//     if (esp_at_sntp_init()){
//         log_printf("[ERROR]ESP32 sntp init error!\r\n");
//         return ;
//     }
//     if ( !esp_at_sntp_get()){
//            if (frame_flag)
//            {
//                 frame_flag = false;
//                 log_printf("%s\r\n",rx_buffer);
//                 // sntp_update_rtc((char*)rx_buffer, 8); // 北京时间 UTC+8
//                 sntp_update_rtc((char*)rx_buffer, 0); // 北京时间 UTC+8
//            }
//            log_printf("[INFO] ESP32 sntp time ok!\r\n");
//        }else
//            log_printf("[ERROR]ESP32 sntp time error!\r\n");
// }
// bool parse_sntp_time(const char *resp, rtc_date_time_t *t)
// {
//     // 找到 "+CIPSNTPTIME:"  +CIPSNTPTIME:Sat Sep 27 10:43:10 2025
//     memset(t, 0, sizeof(rtc_date_time_t));

//     const char *p = strstr(resp, "+CIPSNTPTIME:");
//     if (!p) {
//         log_printf("[parse_sntp_time] +CIPSNTPTIME: not found\r\n");
//         return false;
//     }
//     p += strlen("+CIPSNTPTIME:");

//     char week[4] = {0}, month[4] = {0};
//     unsigned int day = 0, hour = 0, minute = 0, second = 0, year = 0;

//     // 尝试不同的格式字符串
//     int matched = sscanf(p, "%3s %3s %u %u:%u:%u %u",
//                         week, month, &day,
//                         &hour, &minute, &second,
//                         &year);

//     if (matched != 7) {
//         // 尝试另一种格式：可能有额外的空格或不可见字符
//         matched = sscanf(p, " %3s %3s %u %u:%u:%u %u",
//                         week, month, &day,
//                         &hour, &minute, &second,
//                         &year);
//         log_printf("[parse_sntp_time] Second try matched: %d\r\n", matched);
//     }

//     if (matched != 7) {
//         log_printf("[parse_sntp_time] All parsing attempts failed\r\n");
//         return false;
//     }

//     // 将解析的值赋给结构体
//     t->day = day;
//     t->hour = hour;
//     t->minute = minute;
//     t->second = second;
//     t->year = year;
//     log_printf("[parse_sntp_time] : %s \r\n",week);
//     // 星期转换
//     if      (strcmp(week, "Sun") == 0) t->weekday = 0;
//     else if (strcmp(week, "Mon") == 0) t->weekday = 1;
//     else if (strcmp(week, "Tue") == 0) t->weekday = 2;
//     else if (strcmp(week, "Wed") == 0) t->weekday = 3;
//     else if (strcmp(week, "Thu") == 0) t->weekday = 4;
//     else if (strcmp(week, "Fri") == 0) t->weekday = 5;
//     else if (strcmp(week, "Sat") == 0) t->weekday = 6;

//     // 月份转换
//     if      (strcmp(month, "Jan") == 0) t->month = 1;
//     else if (strcmp(month, "Feb") == 0) t->month = 2;
//     else if (strcmp(month, "Mar") == 0) t->month = 3;
//     else if (strcmp(month, "Apr") == 0) t->month = 4;
//     else if (strcmp(month, "May") == 0) t->month = 5;
//     else if (strcmp(month, "Jun") == 0) t->month = 6;
//     else if (strcmp(month, "Jul") == 0) t->month = 7;
//     else if (strcmp(month, "Aug") == 0) t->month = 8;
//     else if (strcmp(month, "Sep") == 0) t->month = 9;
//     else if (strcmp(month, "Oct") == 0) t->month = 10;
//     else if (strcmp(month, "Nov") == 0) t->month = 11;
//     else if (strcmp(month, "Dec") == 0) t->month = 12;

//     return true;
// }
// // 判断闰年
// static int is_leap_year(int year)
// {
//     return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
// }

// // 每个月的天数（非闰年）
// static const int days_in_month[12] = {
//     31, 28, 31, 30, 31, 30,
//     31, 31, 30, 31, 30, 31
// };
// /*处理从stnp获取的时间，并转换为rtc_date_time_t结构体*/
// static void adjust_timezone(rtc_date_time_t *time, int offset_hours)
// {
//     time->hour += offset_hours;

//     while (time->hour >= 24) {
//         time->hour -= 24;
//         time->day += 1;
//         // TODO: 建议加上月份、闰年处理

//         // 判断当前月天数
//         int dim = days_in_month[time->month - 1];
//         if (time->month == 2 && is_leap_year(time->year)) {
//             dim = 29;
//         }
//         if (time->day > dim) {
//             time->day = 1;
//             time->month += 1;
//             if (time->month > 12) {
//                 time->month = 1;
//                 time->year += 1;
//             }
//         }
//     }

//     // 处理小时借位（负向）
//     while (time->hour < 0) {
//         time->hour += 24;
//         time->day -= 1;

//         if (time->day < 1) {
//             time->month -= 1;
//             if (time->month < 1) {
//                 time->month = 12;
//                 time->year -= 1;
//             }

//             int dim = days_in_month[time->month - 1];
//             if (time->month == 2 && is_leap_year(time->year)) {
//                 dim = 29;
//             }
//             time->day = dim;
//         }
//     }
// }

// int sntp_update_rtc(const char *sntp_str, int timezone_offset)
// {
//     rtc_date_time_t utc_time;

//     if (!parse_sntp_time(sntp_str, &utc_time)) {
//         log_printf("[ERROR] SNTP parse failed\r\n");
//         return 0;
//     }

//     // 时区调整
//     adjust_timezone(&utc_time, timezone_offset);

//     // 更新 RTC
//     rtc_set_time(&utc_time);

//     // 打印日志
//     log_printf("[INFO] RTC updated by SNTP\r\n");

//     return 1;
// }
bool esp_at_get_time(uint32_t *timestamp)
{
    const char *rsp;
    uint32_t length;
    /*+SYSTIMESTAMP:1758940999
    OK*/
    if (esp_send_at("AT+SYSTIMESTAMP?", &rsp, &length, 1000) != false)
    {
        return false;
    }
    char *sts = strstr(rsp, "+SYSTIMESTAMP:") + strlen("+SYSTIMESTAMP:");

    *timestamp = atoi(sts);

    return true;
}

bool esp_at_wifi_get_ip(char ip[16])
{
    const char *rsp;
    /*+CIPSTA:ip:"192.168.1.7"
    +CIPSTA:gateway:"192.168.1.1"
    +CIPSTA:netmask:"255.255.255.0"

    OK*/
    if (esp_send_at("AT+CIPSTA?", &rsp, NULL, 1000))
    {
        return false;
    }

    /* 解析ip地址，响应示例：+CIPSTA:ip:192.168.1.1 */
    const char *pip = strstr(rsp, "+CIPSTA:ip:") + strlen("+CIPSTA:ip:");

    /* 如果响应中有ip地址，则拷贝到ip中 */
    if (pip)
    {
        for (int i = 0; i < 16; i++)
        {
            if (pip[i] == '\r')
            {
                ip[i] = '\0';
                break;
            }
            ip[i] = pip[i];
        }
        return true;
    }

    return true;
}


bool esp_at_wifi_get_mac(char mac[18])
{
    const char *rsp;
    /*+CIPSTAMAC:"9c:9e:6e:ee:49:28"

    OK*/
    if (esp_send_at("AT+CIPSTAMAC?", &rsp, NULL, 1000))
    {
        return false;
    }

    /* 解析mac地址，响应示例：+CIPSTAMAC:mac:00:11:22:33:44:55 */
    const char *pmac = strstr(rsp, "+CIPSTAMAC:mac:") + strlen("+CIPSTAMAC:mac:");
    /* 如果响应中有mac地址，则拷贝到mac中 */
    if (pmac)
    {
        strncpy(mac, pmac, 18);
        return true;
    }

    return true;
}
