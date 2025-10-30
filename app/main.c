#include "main.h"

extern void board_init(void);
extern void board_lowlevel_init(void);
static void main_init(void *param)
{
    board_lowlevel_init();
    board_init();
    welcome_page_display();

    wifi_init();
    wifi_page_display();
    wifi_wait_connect();

    main_page_display();
    app_init();

    vTaskDelete(NULL);
}

int main(void)
{
    workqueue_init();

    xTaskCreate(main_init, "init", 1024, NULL, 9, NULL);

    vTaskStartScheduler();

    while (1)
    {
        ;
    }
}

