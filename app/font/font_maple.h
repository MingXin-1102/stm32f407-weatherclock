#ifndef __FONT_MAPLE_H
#define __FONT_MAPLE_H

// !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~

#include <stdint.h>

typedef struct {
    const uint8_t *table;  // 字模起始地址
    uint8_t width;         // 字符宽度
    uint8_t height;        // 字符高度
    uint16_t char_count;   // 字符数量
} FONT_INFO;

extern const FONT_INFO Font8x16;
extern const FONT_INFO Font16x24;
extern const FONT_INFO Font16x32;
extern const FONT_INFO Font20x40;
extern const FONT_INFO Font20x50;
extern const FONT_INFO Font32x64;



#endif /* __FONT_MAPLE_H */
