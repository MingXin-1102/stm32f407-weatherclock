#ifndef __WIFI_H__
#define __WIFI_H__

#include "log.h"
#include "esp_at.h"
#include "page.h"

#define WIFI_SSID       "Minglan_739"
#define WIFI_PASSWORD   "Sml991102"

void wifi_init (void);
void wifi_wait_connect(void);

#endif /*__WIFI_H__*/
