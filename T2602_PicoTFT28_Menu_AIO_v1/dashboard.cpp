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
#include <SPI.h>


#define NBR_BOXES           7





typedef enum
{
  BOX_UPPER_LARGE = 0,
  BOX_MID_LARGE,
  BOX_ROW_1,
  BOX_ROW_2,
  BOX_ROW_3,
  BOX_ROW_4,
  BOX_LOW_AREA,
  BOX_NBR_OF
} boxes_et;

typedef struct
{
    uint16_t x_pos;
    uint16_t y_pos;
    uint16_t width;
    uint16_t height;
    char     txt[TXT_LEN];
    uint8_t  font_indx;
    uint8_t  font_size;
    uint16_t fill_color;
    uint16_t border_color;
    uint16_t text_color;
} disp_box_st;

typedef struct
{
    char txt[BASIC_ROW_LEN];
    uint16_t text_color;
    uint16_t background_color;

} basic_row_st;

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

// extern value_st subs_data[];
dashboard_backlight_st backlight = {0};

extern TFT_eSPI tft;
extern value_st subs_data[AIO_SUBS_NBR_OF];
// extern main_ctrl_st main_ctrl;

disp_box_st db_box[BOX_NBR_OF] =
{
  // x    y    w    h   label  fon f  fill color  border color  text color
  {  0,   0, 319, 172, "Box 0", 4, 1, TFT_BLACK, TFT_LIGHTGREY, TFT_LIGHTGREY },
  {  0,  72, 319,  80, "Box 1", 8, 1, TFT_BLACK, TFT_GOLD, TFT_GOLD },
  {  0,   0, 319,  32, "Box 2", 4, 1, TFT_BLACK, TFT_GOLD, TFT_WHITE },
  {  0,  32, 319,  32, "Box 3", 4, 1, TFT_BLACK, TFT_GOLD, TFT_WHITE },
  {  0,  64, 319,  32, "Box 4", 4, 1, TFT_BLACK, TFT_GOLD, TFT_WHITE},
  {  0,  96, 319,  32, "Box 5", 4, 1, TFT_BLACK, TFT_GOLD, TFT_WHITE},
  {  0,  90, 319,  90, "Box 6", 8, 1, TFT_BLACK, TFT_VIOLET, TFT_GOLD },
};

basic_row_st basic_row[NBR_BASIC_ROWS] = {0};


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

//                                        123456789012345   ival  next  state  prev  cntr flag  call backup
atask_st dashboard_task_handle      =   {"Dashboard SM   ", 1000,   0,     0,  255,    0,   1,  dashboard_update_task };
atask_st bl_task                    =   {"Backlight task ", 1000,   0,     0,  255,    0,   1,  dashboard_backlight_task };

void dashboard_clear(void)
{
    // tft.setRotation(3);
    // tft.setTextSize(1);
    // tft.fillScreen(TFT_BLACK);
    if (box_not_reserved()){
        uint8_t bindx = box_get_indx(BOX_GROUP_1, 0);
        box_paint(bindx, 0);
        // box_print_text(bindx, menu[menu_ctrl.active].row_label);
        box_show_one(bindx);
    } 
}

void dashboard_initialize(void)
{
    atask_add_new(&dashboard_task_handle);
    atask_add_new(&bl_task);

    // pinMode(PIN_TFT_LED, OUTPUT);
    // pinMode(PIN_PIR_INP,INPUT);
    // pinMode(PIN_LDR_ANALOG_INP,INPUT);
    // analogReadResolution(12);
    // analogWrite(PIN_TFT_LED,200);
    
    // //SPI.beginTransaction(mySPISettings);
    // tft.init();
    dashboard_clear();
    // tft.fillScreen(TFT_MAROON);
}




// void dashboard_draw_box(uint8_t bindx)
// {
//     tft.setTextSize(db_box[bindx].font_size);
//     tft.setTextColor(db_box[bindx].text_color, db_box[bindx].fill_color, false);
//     tft.fillRect(db_box[bindx].x_pos, db_box[bindx].y_pos, db_box[bindx].width, db_box[bindx].height, db_box[bindx].fill_color);
//     tft.drawString( db_box[bindx].txt , db_box[bindx].x_pos+4, db_box[bindx].y_pos+2, db_box[bindx].font_indx);
//     //Serial.print("Box: "); Serial.print(bindx); Serial.print(" = ");Serial.println(db_box[bindx].txt);
// }

void dashboard_draw_basic_rows(void)
{
    uint16_t y0;
    for (uint8_t row_indx = 0; row_indx < NBR_BASIC_ROWS; row_indx++ )
    {
        // Box 4", 4, 1, TFT_BLACK, TFT_GOLD, TFT_WHITE},
        tft.setTextSize(1);
        y0 = row_indx*BASIC_ROW_HEIGHT;
        tft.setTextColor(basic_row[row_indx].text_color, basic_row[row_indx].background_color, false);
        tft.fillRect(0, y0, TFT_HEIGHT, BASIC_ROW_HEIGHT, basic_row[row_indx].background_color);
        tft.drawString( basic_row[row_indx].txt , 4, y0+2, 2);
        //Serial.print("Box: "); Serial.print(bindx); Serial.print(" = ");Serial.println(db_box[bindx].txt);
    }
}

void dashboard_set_mode(dashboard_mode_et new_mode)
{
   dashboard_ctrl.mode = new_mode; 
   dashboard_task_handle.state = 0;

}

void dashboard_print_row(int8_t rindx, char *txtp, uint8_t color_scheme)
{
    uint8_t bindx = box_get_indx(BOX_GROUP_8, 0);
    box_scroll_down( BOX_GROUP_8);
    box_paint(bindx, color_scheme);
    box_print_text(bindx, txtp);
    box_show_one(bindx);
}

void dashboard_update_all(void)
{
    for (uint8_t i = 0; i < NBR_BOXES; i++)
    {
        dashboard_draw_box(i);
    }
}

void dashboard_set_text(uint8_t box_indx, char *txt_ptr)
{
    strcpy(db_box[box_indx].txt, txt_ptr);
}

void dashboard_big_time(void)
{
    static uint8_t p
    rev_minute = 99;
    char big_txt[8];
    uint8_t bindx = box_get_indx(BOX_GROUP_3, 1);

    DateTime *now = time_get_time_now();
    if ((now->minute() != prev_minute) || dashboard_ctrl.force_show_big_time)
    {
        prev_minute = now->minute();
        char s1[4];
        
        sprintf(s1,"%02d",now->hour());
        String time_str = s1;
        time_str += ":";
        sprintf(s1,"%02d",now->minute());
        time_str += s1;
        time_str.toCharArray(big_txt, 8);
        box_paint(bindx, 2);
        box_print_text(bindx, big_txt);
        box_show_one(bindx);

    }
}

void dashboard_show_info(void)
{
    String Str_info = APP_NAME;
    Str_info += "\n";
    Str_info += __DATE__;
    Str_info += __TIME__;

    // strcpy(db_box[BOX_UPPER_LARGE].txt, " ");
    // dashboard_draw_box(BOX_UPPER_LARGE);
 
    // strcpy(db_box[BOX_ROW_1].txt, APP_NAME);
    // dashboard_draw_box(BOX_ROW_1);

    // strcpy(db_box[BOX_ROW_2].txt, __DATE__);
    // dashboard_draw_box(BOX_ROW_2);

    // strcpy(db_box[BOX_ROW_3].txt, __TIME__);
    // dashboard_draw_box(BOX_ROW_3);

    // Str_info.toCharArray(db_box[0].txt, TXT_LEN);

}

void dashboard_show_common(void)
{
    String time_str;
    if (!dashboard_ctrl.show_sensor_value)  
    {
        strcpy(db_box[BOX_ROW_1].txt, MAIN_TITLE);
        dashboard_draw_box(BOX_ROW_1);
        time_to_string(&time_str);
        time_str.toCharArray(db_box[BOX_ROW_2].txt, TXT_LEN);
        dashboard_draw_box(BOX_ROW_2);
    }
}

void dashboard_time_and_sensors(void)
{
    static uint32_t next_step_ms;
    bool            update_box;
    String          Str;
    uint8_t         i; 
    
    switch (dashboard_task_handle.state)
    {
        case 0:
            dashboard_clear();
            dashboard_show_info();
            menu_draw();
            dashboard_task_handle.state++;
            break;
        case 1: 
            dashboard_show_common();
            dashboard_big_time();
            dashboard_ctrl.force_show_big_time = false;
            dashboard_task_handle.state++;
            break;
        case 2:
            update_box = false;
            
            i = (uint8_t)dashboard_ctrl.sensor_indx;
            if (millis() > subs_data[i].show_next_ms)
            {
                if ( subs_data[i].updated)
                {
                    dashboard_ctrl.show_sensor_value = true;
                    Serial.print("aio index: "); Serial.print(i); 
                    Serial.println(" = Updated ");
                    subs_data[i].updated = false;
                    Str = subs_data[i].location;
                    Str += " ";
                    Str.toCharArray(db_box[BOX_ROW_1].txt,40);

                    Str = measure_label[subs_data[i].unit_index];
                    Str += " ";
                    Str += unit_label[subs_data[i].unit_index];
                    Str.toCharArray(db_box[BOX_ROW_2].txt, TXT_LEN);

                    Str = String(subs_data[i].value);
                    Serial.println(Str);
                    Str.toCharArray(db_box[BOX_MID_LARGE].txt,6);
                    update_box = true;
                    if (update_box)
                    {
                        dashboard_draw_box(BOX_UPPER_LARGE);
                        dashboard_draw_box(BOX_MID_LARGE);
                        dashboard_draw_box(BOX_ROW_1);
                        dashboard_draw_box(BOX_ROW_2);
                    }
                }
                subs_data[i].show_next_ms = millis() + subs_data[i].show_interval_ms;
                if (millis() > subs_data[i].next_update_limit) subs_data[i].state = SENSOR_TIMEOUT;
            }

            if (dashboard_ctrl.sensor_indx < AIO_SUBS_NBR_OF - 1) dashboard_ctrl.sensor_indx++;    
            else dashboard_ctrl.sensor_indx = AIO_SUBS_FIRST;
            
            if (update_box )
            {
                dashboard_task_handle.state = 3;
                next_step_ms = millis() + 10000;
            }
            else
            {
               dashboard_task_handle.state = 1;
            }
            break;  
        case 3:
            if ((millis() > next_step_ms) || dashboard_ctrl.fast_forward)
            {
                dashboard_ctrl.force_show_big_time = true;
                dashboard_task_handle.state = 1;
                dashboard_ctrl.show_sensor_value = false;
                dashboard_ctrl.fast_forward = false;
            } 
              
            break;
    }
    //Serial.printf("db %d -> %d\n", dashboard_task_handle.prev_state, dashboard_task_handle.state);
}


void dashboard_basic_rows(void)
{
    static uint32_t next_step_ms;
    String          Str;
    uint8_t         i; 
    
    switch (dashboard_task_handle.state)
    {
        case 0:
            dashboard_clear();
            dashboard_print_row(-1,(char*)"Data Printer",TFT_YELLOW, TFT_BLUE);
            dashboard_task_handle.state++;
            dashboard_ctrl.basic_row_updated = true;
            break;
        case 1: 
            if (dashboard_ctrl.basic_row_updated){
                dashboard_draw_basic_rows();
                dashboard_ctrl.basic_row_updated = false;
            }
            break;
    }
}
void dashboard_update_task()
{
    switch (dashboard_ctrl.mode)
    {
        case DASHBOARD_TIME_SENSOR:
            dashboard_time_and_sensors();
            break;
        case DASHBOARD_BASIC_ROWS:
            dashboard_basic_rows();
            break;
    }
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

    switch(bl_task.state)
    {
        case 0:
            bl_task.state = 10; //dark
            break;
        case 10:
            if(backlight.ldr_value > 3000 ) bl_task.state = 300;
            else if(backlight.ldr_value > 2000 ) bl_task.state = 200;
            else bl_task.state = 100;
            break;
        case 100: 
            if (backlight.pir_value){
                backlight.bl_pwm = 50;
                backlight.timeout = millis() + 10000; 
                bl_task.state = 105;
            } 
            else {
                backlight.bl_pwm = 30;           
                bl_task.state = 10;
            }
            break;
        case 105:
            if (millis() > backlight.timeout) bl_task.state = 10;
            break;
        case 200: 
            if (backlight.pir_value){
                backlight.bl_pwm = 120;
                backlight.timeout = millis() + 10000; 
                bl_task.state = 205;
            } 
            else {
                backlight.bl_pwm = 80;           
                bl_task.state = 10;
            }
            break;
        case 205:
            if (millis() > backlight.timeout) bl_task.state = 10;
            break;
        case 300: 
            if (backlight.pir_value){
                backlight.bl_pwm = 1023;
                backlight.timeout = millis() + 10000; 
                bl_task.state = 305;
            } 
            else {
                backlight.bl_pwm = 200;           
                bl_task.state = 10;
            }
            break;
        case 305:
            if (millis() > backlight.timeout) bl_task.state = 10;
            break;
    }
    analogWrite(PIN_TFT_LED, backlight.bl_pwm);
    //analogWrite(PIN_TFT_LED, 200);

}