/************************************************************************************************************************************
@title T2602_PicoTFT28_AIO
@git  	https://github.com/infrapale/T2602_PicoTFT28_AIO.git
************************************************************************************************************************************
@HW Raspberry Pi Pico W on a pico TFT classic board (2.8" TFT display + 4 Cherry key buttons)

************************************************************************************************************************************
https://github.com/adafruit/Adafruit_MQTT_Library
https://circuitdigest.com/microcontroller-projects/arduino-freertos-tutorial1-creating-freertos-task-to-blink-led-in-arduino-uno
https://circuitdigest.com/microcontroller-projects/arduino-freertos-tutorial-using-semaphore-and-mutex-in-freertos-with-arduino
https://learn.adafruit.com/dvi-io/code-the-dashboard
***********************************************************************************************************************************/


#include "main.h"
// #define BOARD_PICO_TFT_4KEYS
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>
#include "io.h"
#include "menu.h"
#include "time_func.h"
#include "atask.h"
#include "aio_mqtt.h"
#include "dashboard.h"
#include "io.h"
#include "box.h"

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

void print_debug_task(void)
{
  atask_print_status(true);
  dashboard_debug_print();
}

//                                  123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st debug_task_handle    =   {"Debug Task     ", 5000,    0,     0,  255,    0,  1,  print_debug_task };


uint32_t  targetTime = 0; 

void run_tft_pin_check(void);

void setup(void) {
  delay(3000);
  Serial.begin(115200); // For debug
  while (!Serial);
  Serial.println(F(APP_NAME));
  Serial.printf(" Compiled: %s %s\n",__DATE__, __TIME__);
  run_tft_pin_check();
  Wire.setSDA(PIN_WIRE_SDA);
  Wire.setSCL(PIN_WIRE_SCL);
  Wire.begin();
  time_begin();
  
  atask_initialize();
  atask_add_new(&debug_task_handle);
  
  //dashboard_initialize();   // start dashboard task
  menu_initialize();        // starting scan and read tasks
  aio_mqtt_initialize();    // task is stopped - for debug purpose only

  box_initialize();
  box_structure_print();
  //box_show_all();
  //box_scroll_test(BOX_GROUP_MENU);    //BOX_GROUP_8);
}

void setup1()
{
  //Watchdog.reset();
  aio_mqtt_initialize();
  targetTime = millis() + 100;
}

// Fast running loop
void loop() 
{
  atask_run();
}

// Slow running loop: WiFI an MQTT
void loop1()
{
  if (millis() > targetTime)
  {
    aio_mqtt_stm();
    targetTime = millis() + 100;
  }
}

tft_pin_check_st tft_pin_check[8] =
{
    { "BL  ",  PIN_TFT_LED,  TFT_BL},
    { "CS  ",  PIN_TFT_CS,   TFT_CS}, 
    { "RST ",  PIN_TFT_RST,  TFT_RST},
    { "DC  ",  PIN_TFT_DC,   TFT_DC},
    { "MISO",  PIN_TFT_MISO, TFT_MISO},
    { "CLK ",  PIN_TFT_CLK,  TFT_SCLK},
    { "MOSI",  PIN_TFT_MOSI, TFT_MOSI},
    { "T_CS",  PIN_TOUCH_CS, TOUCH_CS},  
};

void run_tft_pin_check(void)
{
    bool tft_setup_is_ok = true;
    #if TFT_TARGET_BOARD == BOARD_PICO_TFT_4KEYS
        Serial.println("BOARD_PICO_TFT_4KEYS was defined - OK");
    #else
        Serial.println("BOARD_PICO_TFT_4KEYS was NOT defined - ERROR");
        tft_setup_is_ok = false
    #endif    
    #ifdef ILI9341_DRIVER
        Serial.println("ILI9341_DRIVER was defined - OK");
    #else
        Serial.println("ILI9341_DRIVER was not defined - ERROR");
        tft_setup_is_ok = false
    #endif
    Serial.println("Pin  Design Library");
    for (uint8_t i = 0; i<8;i++)
    {
        Serial.print(tft_pin_check[i].label);
        Serial.print(": ");
        Serial.print(tft_pin_check[i].design_pin);
        Serial.print(" --> ");
        Serial.print(tft_pin_check[i].library_pin);
        if(tft_pin_check[i].design_pin == tft_pin_check[i].library_pin)
            Serial.println(" OK");
        else {
            tft_setup_is_ok = false;
            Serial.println(" ERROR");
        }  
    }

}