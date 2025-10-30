#ifndef __APP_H__
#define __APP_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "workqueue.h"

#include "rtc.h"
#include "aht20.h"
#include "esp_at.h"
#include "weather.h"
#include "page.h"
#include "log.h"

#define APP_VERSION "v2.0"

void app_init(void);

#endif /* __APP_H__ */

