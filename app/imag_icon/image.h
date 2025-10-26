#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdint.h>

typedef struct
{
    uint16_t width;
    uint16_t height;
    const uint8_t *data;
} image_t;

extern const image_t sunny_0;
extern const image_t clear_1;
extern const image_t Fair_2;
extern const image_t Fair_3;
extern const image_t Cloudy_4;
extern const image_t Partly_Cloudy_5;
extern const image_t Partly_Cloudy_6;
extern const image_t Mostly_Cloudy_7;
extern const image_t Mostly_Cloudy_8;
extern const image_t Overcast_9;
extern const image_t Shower_10;
extern const image_t Thundershower_11;
extern const image_t Thundershower_with_Hail_12;
extern const image_t Light_Rain_13;
extern const image_t Moderate_Rain_14;
extern const image_t Heavy_Rain_15;
extern const image_t Storm_16;
extern const image_t Heavy_Storm_17;
extern const image_t Severe_Storm_18;
extern const image_t Ice_Rain_19;
extern const image_t Sleet_20;
extern const image_t Snow_Flurry_21;
extern const image_t Light_Snow_22;
extern const image_t Moderate_Snow_23;
extern const image_t Heavy_Snow_24;
extern const image_t Snowstorm_25;
extern const image_t Dust_26;
extern const image_t Sand_27;
extern const image_t Duststorm_28;
extern const image_t Sandstorm_29;
extern const image_t Foggy_30;
extern const image_t Haze_31;
extern const image_t Windy_32;
extern const image_t Blustery_33;
extern const image_t Hurricane_34;
extern const image_t Tropical_Storm_35;
extern const image_t Tornado_36;
extern const image_t Cold_37;
extern const image_t Hot_38;
extern const image_t rabbit;
extern const image_t error;
extern const image_t humi;
extern const image_t temp;
extern const image_t wifi;
extern const image_t wifi_mini;

#endif /* __IMAGE_H__ */
