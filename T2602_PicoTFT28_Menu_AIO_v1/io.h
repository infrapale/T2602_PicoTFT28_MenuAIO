#ifndef __IO_H__ 
#define __IO_H__
#include "Arduino.h"

//#define BOARD_PICO_TFT_4KEYS


#define PIN_WIRE_SDA         (12u)
#define PIN_WIRE_SCL         (13u)

#define PIN_KEY1		          (28u)
#define PIN_KEY2		          (27u)
#define PIN_KEY3		          (17u)
#define PIN_KEY_STATUS        (2u)
#define PIN_LDR_ANALOG_INP    (26u)
#define PIN_PIR_INP           (5u)

// TFT PIN Definitions
#define PIN_TFT_LED           (10u)
#define PIN_TFT_CS            (9u)
#define PIN_TFT_RST           (14u)
#define PIN_TFT_DC            (15u)
#define PIN_TFT_MISO          (16u)
#define PIN_TFT_CLK           (18u)
#define PIN_TFT_MOSI          (19u)
#define PIN_TOUCH_CS          (21u)

// TFT Library Check
typedef struct 
{
    char label[5];
    uint8_t   design_pin;
    uint8_t   library_pin;
} tft_pin_check_st;


#endif
