/******************************************************************************
    dashboard.cpp  Show Time and measurements on the TFT display
*******************************************************************************
    BOX_H0      BOX_H1      BOX_H2      BOX_H4
    ---------   ---------   --------    ---------
    |       |   |       |   |       |   |       |  
    |       |   ---------   |       |   |       |  
    |       |   |       |   |       |   |       |  
    |       |   ---------   ---------   |       |  
    |       |   |       |   |       |   |       |  
    |       |   ---------   |       |   |       |  
    |       |   |       |   |       |   |       |  
    |       |   ---------   ---------   ---------  
    |       |   |       |   |       |   |       |  
    |       |   ---------   |       |   |       |
    |       |   |       |   |       |   |       |  
    |       |   ---------   ---------   |       |  
    |       |   |       |   |       |   |       |  
    |       |   ---------   |       |   |       |  
    |       |   |       |   |       |   |       |  
    ---------   ---------   --------    ---------


******************************************************************************/

//  This sketch uses the GLCD (font 1) and fonts 2, 4, 6, 7, 8

#include "main.h"
#include "dashboard.h"
#include "aio_mqtt.h"
#include "time_func.h"
#include "menu.h"
#include "atask.h"
#include "box.h"

typedef struct
{
    dashboard_mode_et mode;
    bool show_sensor_value;
    bool force_show_big_time;
    bool fast_forward;
    //bool show_basic_rows;
    uint8_t sensor_indx;
    uint8_t menu_sensor_indx;
    uint8_t basic_row_indx;
    bool    basic_row_updated;
} dashboard_ctrl_st;

typedef struct
{
    //uint16_t    state;
    uint16_t    bl_pwm;
    uint32_t    timeout;
    uint16_t    light_state;
    uint16_t    ldr_value;
    uint8_t     pir_value;
} dashboard_backlight_st;

dashboard_ctrl_st dashboard_ctrl    = {DASHBOARD_TIME_SENSOR, false, true, false, AIO_SUBS_LA_ID_TEMP, 0, 0, false};

dashboard_backlight_st backlight = {0};

extern value_st subs_data[AIO_SUBS_NBR_OF];

char unit_label[UNIT_NBR_OF][UNIT_LABEL_LEN] =
{
  // 012345678
    "Celsius ",
    "%       ",
    "kPa     ",
    "Light   ",
    "LDR     ",
    "V       ",
    "Time    ",
    "CO2     ",
    "LUX     "
};

char measure_label[UNIT_NBR_OF][MEASURE_LABEL_LEN] =
{
  // 0123456789012345
    "Temperature    ",
    "Humidity       ",
    "Air Pressure   ",
    "Light          ",
    "LDR Value      ",
    "Voltage        "
};

void dashboard_backlight_task(void);

//                      123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st dbh      =   {"Dashboard SM   ", 1000,   0,     0,  255,    0,   1,  dashboard_update_task };
atask_st blh      =   {"Backlight task ", 1000,   0,     0,  255,    0,   1,  dashboard_backlight_task };

void dashboard_clear(void)
{
    if (box_is_not_reserved()){
        uint8_t bindx = box_get_indx(BOX_GROUP_1, 0);
        box_paint(bindx, 0);
        box_show_one(bindx);
    } 
}

void dashboard_initialize(void)
{
    atask_add_new(&dbh);
    atask_add_new(&blh);
    dashboard_clear();
}



void dashboard_set_mode(dashboard_mode_et new_mode)
{
   dashboard_ctrl.mode = new_mode; 
   dbh.state = 0;
}

void dashboard_big_time(void)
{
    static uint8_t prev_minute = 99;
    char buff[8];
    uint8_t bindx = box_get_indx(BOX_GROUP_3, 0);

    DateTime *now = time_get_time_now();
    if (now->minute() != prev_minute)
    {
        sprintf(buff," %02d:%02d", now->hour(), now->minute());
        prev_minute = now->minute();
        box_paint(bindx, BOX_SCHEME_TIME);
        box_print_text(bindx, buff);
    }
}

void dashboard_show_app_info(void)
{
    // String Str_info = APP_NAME;
    // Str_info += "\n";
    // Str_info += __DATE__;
    // Str_info += __TIME__;

    /// TODO

}
void dashboard_show_info(uint8_t sindx)
{
    char            buff[40];
    uint8_t         bindx = box_get_indx(BOX_GROUP_6, 4);

    time_to_string(buff);
    box_paint(bindx,BOX_SCHEME_TIME);
    box_print_text(bindx, buff);

    bindx++;
    sprintf(buff,"%s %s",
        subs_data[sindx].location,
        unit_label[subs_data[sindx].unit_index]
    );
    box_paint(bindx, BOX_SCHEME_SENSOR);
    box_print_text(bindx, buff);

}

bool dasboard_show_sensor(uint8_t sindx)
{
    static uint8_t  middle_big_box = box_get_indx(BOX_GROUP_3, 1);
    char            buff[40];
    bool            update_box = false;

    if (millis() > subs_data[sindx].show_next_ms)
    {
        if ( subs_data[sindx].updated)
        {
            dashboard_ctrl.show_sensor_value = true;
            Serial.print("aio index: "); Serial.print(sindx); 
            Serial.println(" = Updated ");
            subs_data[sindx].updated = false;

            sprintf(buff," %.1f", subs_data[sindx].value);
            if(strlen(buff) > 5) sprintf(buff,"%.1f", subs_data[sindx].value);
            else if(strlen(buff) > 6) sprintf(buff,"%.0f", subs_data[sindx].value); 
            Serial.println(buff);
            //Str.toCharArray(buff,6);
            box_paint(middle_big_box, BOX_SCHEME_SENSOR);
            box_print_text(middle_big_box, buff);
            update_box = true;

        }
        subs_data[sindx].show_next_ms = millis() + subs_data[sindx].show_interval_ms;
        if (millis() > subs_data[sindx].next_update_limit) subs_data[sindx].state = SENSOR_TIMEOUT;
    }
    return update_box;
}


void dashboard_update_task(void)
{
    static uint32_t next_step_ms;
    bool            update_box;

    switch (dbh.state)
    {
        case 0:
            box_clear_group(BOX_GROUP_1);
            dbh.state = 10;
            break;
        case 10: 
            dashboard_big_time();
            dbh.state = 20;
            break;
        case 20:
            update_box = dasboard_show_sensor((uint8_t)dashboard_ctrl.sensor_indx);

            if (update_box )
            {
                dbh.state = 30;
                dashboard_show_info((uint8_t)dashboard_ctrl.sensor_indx);
                next_step_ms = millis() + 10000;
            }
            else
            {
               dbh.state = 10;
            }
            if (dashboard_ctrl.sensor_indx < AIO_SUBS_NBR_OF - 1) dashboard_ctrl.sensor_indx++;    
            else dashboard_ctrl.sensor_indx = AIO_SUBS_FIRST;
            
            break;  
        case 30:

            if (millis() > next_step_ms)
            {
                dbh.state = 10;
                box_update_area();
            } 
            break;
    }
    //Serial.printf("db %d -> %d\n", dbh.prev_state, dbh.state);
}



void dashboard_show_sensor_print(void){
    dashboard_set_mode(DASHBOARD_BASIC_ROWS);
}
void dashboard_show_time_sensor(void){
    dashboard_set_mode(DASHBOARD_TIME_SENSOR);
}


void dashboard_next_sensor(void)
{
    dashboard_ctrl.menu_sensor_indx++;
    if(dashboard_ctrl.menu_sensor_indx >= AIO_SUBS_NBR_OF) dashboard_ctrl.menu_sensor_indx = AIO_SUBS_FIRST;
    subs_data[dashboard_ctrl.menu_sensor_indx].show_next_ms = 0              ;
    dashboard_ctrl.sensor_indx = dashboard_ctrl.menu_sensor_indx;
    Serial.printf("dashboard_ctrl.menu_sensor_indx=%d\n",dashboard_ctrl.menu_sensor_indx);
    dashboard_ctrl.fast_forward = true;
}

void dashboard_previous_sensor(void)
{
    if(dashboard_ctrl.menu_sensor_indx <= 1 ) dashboard_ctrl.menu_sensor_indx = AIO_SUBS_NBR_OF -1;
    else dashboard_ctrl.menu_sensor_indx--;
    subs_data[dashboard_ctrl.menu_sensor_indx].show_next_ms = 0;
    dashboard_ctrl.sensor_indx = dashboard_ctrl.menu_sensor_indx;
    Serial.printf("dashboard_ctrl.menu_sensor_indx=%d\n",dashboard_ctrl.menu_sensor_indx);
    dashboard_ctrl.fast_forward = true;
}

void dashboard_debug_print(void)
{
    Serial.printf("LDR: %d PIR %d PWM %d\n", backlight.ldr_value, backlight.pir_value, backlight.bl_pwm);
}

void dashboard_backlight_task(void)
{
    backlight.ldr_value = analogRead(PIN_LDR_ANALOG_INP);
    backlight.pir_value = digitalRead(PIN_PIR_INP);

    switch(blh.state)
    {
        case 0:
            blh.state = 10; //dark
            break;
        case 10:
            if(backlight.ldr_value > 3000 ) blh.state = 300;
            else if(backlight.ldr_value > 2000 ) blh.state = 200;
            else blh.state = 100;
            break;
        case 100: 
            if (backlight.pir_value){
                backlight.bl_pwm = 50;
                backlight.timeout = millis() + 10000; 
                blh.state = 105;
            } 
            else {
                backlight.bl_pwm = 30;           
                blh.state = 10;
            }
            break;
        case 105:
            if (millis() > backlight.timeout) blh.state = 10;
            break;
        case 200: 
            if (backlight.pir_value){
                backlight.bl_pwm = 120;
                backlight.timeout = millis() + 10000; 
                blh.state = 205;
            } 
            else {
                backlight.bl_pwm = 80;           
                blh.state = 10;
            }
            break;
        case 205:
            if (millis() > backlight.timeout) blh.state = 10;
            break;
        case 300: 
            if (backlight.pir_value){
                backlight.bl_pwm = 1023;
                backlight.timeout = millis() + 10000; 
                blh.state = 305;
            } 
            else {
                backlight.bl_pwm = 200;           
                blh.state = 10;
            }
            break;
        case 305:
            if (millis() > backlight.timeout) blh.state = 10;
            break;
    }
    analogWrite(PIN_TFT_LED, backlight.bl_pwm);
    //analogWrite(PIN_TFT_LED, 200);

}