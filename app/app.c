#include <stdint.h>
#include "font_maple.h"
#include "aht20.h"
bool aht20_read_data(float *temperature, float *humidity)
{
	if (!aht20_init())
	{
		log_printf("[ERROR]AHT20 init failed!\r\n");
        return false;
	}
	uint32_t failcount = 0;

	while (1)
	{
		if(failcount > 10)
		{
			log_printf("[ERROR]AHT20 error count %d try to reinit!\r\n",failcount);
			failcount = 0 ;
			// break;
			if (!aht20_init())
			{
				log_printf("[ERROR]AHT20 reinit failed!\r\n");
				Delay_Ms(5);
				continue;
			}
			else
			{
				log_printf("[WARING]AHT20 reinit success!\r\n");
				failcount = 0;
			}
		}
		if(!aht20_start_measure())
		{
			log_printf("[ERROR]AHT20 measure failed!\r\n");
			failcount++;
			continue;
		}
		if(!aht20_wait_for_measure()){
			log_printf("[ERROR]AHT20 wait for measure failed!\r\n");
			failcount++;
			continue;
		}
		if(!aht20_read_measurement(temperature,humidity))
		{
			log_printf("[ERROR]AHT20 read measure failed!\r\n");
			failcount++;
			continue;

		}
		failcount = 0;
        return true;
    }
}

