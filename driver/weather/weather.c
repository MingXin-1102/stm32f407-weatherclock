#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "weather.h"

bool parse_seniverse_response(const char *response, weather_info_t *info)
{
	response = strstr(response, "\"results\":");
	if (response == NULL)
		return false;

	const char *location_response = strstr(response, "\"location\":");
	if (location_response == NULL)
		return false;

	const char *location_name_response = strstr(location_response, "\"name\":");
	if (location_name_response)
	{
		sscanf(location_name_response, "\"name\": \"%31[^\"]\"", info->city);//最多读取 31 个非 " 的字符
	}

	const char *location_path_response = strstr(location_response, "\"path\":");
	if (location_path_response)
	{
		sscanf(location_path_response, "\"path\": \"%128[^\"]\"", info->location);
	}

	const char *now_response = strstr(response, "\"now\":");
	if (now_response == NULL)
		return false;

	const char *now_text_response = strstr(now_response, "\"text\":");
	if (now_text_response)
	{
		sscanf(now_text_response, "\"text\": \"%15[^\"]\"", info->weather);
	}

	const char *now_code_response = strstr(now_response, "\"code\":");
	if (now_code_response)
	{
        if (!sscanf(now_code_response, "\"code\": \"%d\"", &info->weather_code) == 1)
            info->weather_code = -1; // 解析失败
	}

	char temperature_str[16] = { 0 };
	const char *now_temperature_response = strstr(now_response, "\"temperature\":");
	if (now_temperature_response)
	{
		if (sscanf(now_temperature_response, "\"temperature\": \"%15[^\"]\"", temperature_str) == 1)
			info->temperature = atof(temperature_str);
	}

	return true;
}
