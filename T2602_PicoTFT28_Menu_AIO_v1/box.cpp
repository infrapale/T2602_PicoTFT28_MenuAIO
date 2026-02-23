/************************************************************************
    BOX_GROUP_1   BOX_GROUP_8   BOX_DIM4    BOX_GROUP_2   BOX_GROUP_3
    ---------   ---------   --------    ---------   ---------
    |       |   |       |   |       |   |       |   |       |  
    |       |   ---------   |       |   |       |   |       |  
    |       |   |       |   |       |   |       |   |       |  
    |       |   ---------   ---------   |       |   |       |  
    |       |   |       |   |       |   |       |   ---------  
    |       |   ---------   |       |   |       |   |       |  
    |       |   |       |   |       |   |       |   |       |  
    |       |   ---------   ---------   ---------   |       | 
    |       |   |       |   |       |   |       |   |       |  
    |       |   ---------   |       |   |       |   |       |
    |       |   |       |   |       |   |       |   ---------  
    |       |   ---------   ---------   |       |   |       |  
    |       |   |       |   |       |   |       |   |       |  
    |       |   ---------   |       |   |       |   |       |  
    |       |   |       |   |       |   |       |   |       |  
    ---------   ---------   --------    ---------   ---------


***************************************************************************/


#include "main.h"
#include "box.h"
//#include "dashboard.h"
//#include "aio_mqtt.h"
//#include "time_func.h"
//#include "menu.h"
#include "atask.h"
#include <SPI.h>

#define BOX_MAX_NUMBER      32
#define BOX_DEFAULT_GROUP   0

typedef struct
{
    uint8_t  group;
    bool     visible;
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
} box_st;

typedef struct
{
    uint16_t fill_color;
    uint16_t border_color;
    uint16_t text_color;
} box_color_st;

typedef struct 
{
    uint8_t nbr;
    uint8_t reserve;
} box_ctrl_st;

SPISettings mySPISettings(4000000, MSBFIRST, SPI_MODE0); // 4 MHz clock
extern TFT_eSPI tft;

box_group_st boxgr[BOX_GROUP_NBR_OF] =
{
    [BOX_GROUP_1]           = {.height=TFT_LS_HIGHT,                    .width=TFT_LS_WIDTH,    .round = 0, .nbr=1,             .index = 0, .font_indx = BOX_FONT_XXL_75,       .font_size = 2 },
    [BOX_GROUP_8]           = {.height=TFT_LS_HIGHT / TFT_LS_ROWS,      .width=TFT_LS_WIDTH,    .round = 0, .nbr=TFT_LS_ROWS,   .index = 0, .font_indx = BOX_FONT_SMALL_16,     .font_size = 1 },
    [BOX_GROUP_4]           = {.height=TFT_LS_HIGHT / TFT_LS_ROWS * 2,  .width=TFT_LS_WIDTH,    .round = 0, .nbr=TFT_LS_ROWS/2, .index = 0, .font_indx = BOX_FONT_LARGE_48,     .font_size = 1 },
    [BOX_GROUP_3]           = {.height=TFT_LS_HIGHT / 3,                .width=TFT_LS_WIDTH,    .round = 0, .nbr=3,             .index = 0, .font_indx = BOX_FONT_XXL_75,       .font_size = 1 },
    [BOX_GROUP_2]           = {.height=TFT_LS_HIGHT / TFT_LS_ROWS * 4,  .width=TFT_LS_WIDTH,    .round = 0, .nbr=TFT_LS_ROWS/4, .index = 0, .font_indx = BOX_FONT_7_SEGM_48,    .font_size = 2 },
    [BOX_GROUP_MENU]        = {.height=TFT_LS_HIGHT / TFT_LS_ROWS ,     .width=TFT_LS_WIDTH/3,  .round = 0, .nbr=3,             .index = 0, .font_indx = BOX_FONT_MEDIUM_26,    .font_size = 1 },
    [BOX_GROUP_HEAD_ROOM]   = {.height=TFT_LS_HIGHT / TFT_LS_ROWS * 7,  .width=TFT_LS_WIDTH,    .round = 0, .nbr=1,             .index = 0, .font_indx = BOX_FONT_MEDIUM_26,     .font_size = 1 },
};

box_color_st box_color_scheme[10] PROGMEM =
{
    [0] = {.fill_color = TFT_BLACK,         .border_color = TFT_LIGHTGREY,      .text_color = TFT_WHITE },
    [1] = {.fill_color = TFT_BLUE,          .border_color = TFT_YELLOW,         .text_color = TFT_YELLOW },
    [2] = {.fill_color = TFT_GREENYELLOW,   .border_color = TFT_DARKGREEN,      .text_color = TFT_NAVY },
    [3] = {.fill_color = TFT_BROWN,         .border_color = TFT_ORANGE,         .text_color = TFT_WHITE },
    [4] = {.fill_color = TFT_SKYBLUE,       .border_color = TFT_DARKCYAN,       .text_color = TFT_BLACK },
    [5] = {.fill_color = TFT_SILVER,        .border_color = TFT_GOLD,           .text_color = TFT_BLACK },
    [6] = {.fill_color = TFT_RED,           .border_color = TFT_YELLOW,         .text_color = TFT_YELLOW },
    [7] = {.fill_color = TFT_WHITE,         .border_color = TFT_OLIVE,          .text_color = TFT_MAROON },
    [8] = {.fill_color = TFT_MAROON,        .border_color = TFT_SILVER,         .text_color = TFT_WHITE },
    [9] = {.fill_color = TFT_DARKCYAN,      .border_color = TFT_WHITE,          .text_color = TFT_WHITE },
    // [9] = {.fill_color = TFT_CYAN,          .border_color = TFT_MAGENTA,        .text_color = TFT_MAGENTA },
};

box_ctrl_st box_ctrl = {0};

// #define TFT_BLACK       0x0000      /*   0,   0,   0 */
// #define TFT_NAVY        0x000F      /*   0,   0, 128 */
// #define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
// #define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
// #define TFT_MAROON      0x7800      /* 128,   0,   0 */
// #define TFT_PURPLE      0x780F      /* 128,   0, 128 */
// #define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
// #define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
// #define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
// #define TFT_BLUE        0x001F      /*   0,   0, 255 */
// #define TFT_GREEN       0x07E0      /*   0, 255,   0 */
// #define TFT_CYAN        0x07FF      /*   0, 255, 255 */
// #define TFT_RED         0xF800      /* 255,   0,   0 */
// #define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
// #define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
// #define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
// #define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
// #define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
// #define TFT_PINK        0xFE19      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F
// #define TFT_BROWN       0x9A60      /* 150,  75,   0 */
// #define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
// #define TFT_SILVER      0xC618      /* 192, 192, 192 */
// #define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
// #define TFT_VIOLET      0x915C      /* 180,  46, 226 */


box_st  box[BOX_MAX_NUMBER];

  // x    y    w    h   label  fon f  fill color  border color  text color
//   {  0,   0, 319, 172, "Box 0", 4, 1, TFT_BLACK, TFT_LIGHTGREY, TFT_LIGHTGREY },
//   {  0,  72, 319,  80, "Box 1", 8, 1, TFT_BLACK, TFT_GOLD, TFT_GOLD },
//   {  0,   0, 319,  32, "Box 2", 4, 1, TFT_BLACK, TFT_GOLD, TFT_WHITE },
//   {  0,  32, 319,  32, "Box 3", 4, 1, TFT_BLACK, TFT_GOLD, TFT_WHITE },
//   {  0,  64, 319,  32, "Box 4", 4, 1, TFT_BLACK, TFT_GOLD, TFT_WHITE},
//   {  0,  96, 319,  32, "Box 5", 4, 1, TFT_BLACK, TFT_GOLD, TFT_WHITE},
//   {  0,  90, 319,  90, "Box 6", 8, 1, TFT_BLACK, TFT_VIOLET, TFT_GOLD },

void box_clear(void)
{
    tft.setRotation(3);
    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);
    //dashboard_draw_box(0);  // clear dashboard
}

void box_initialize(void)
{
    pinMode(PIN_TFT_LED, OUTPUT);
    pinMode(PIN_PIR_INP,INPUT);
    pinMode(PIN_LDR_ANALOG_INP,INPUT);
    analogReadResolution(12);
    analogWrite(PIN_TFT_LED,200);
    
    SPI.beginTransaction(mySPISettings);
    tft.init();
    box_clear();
    tft.fillScreen(TFT_MAROON);


    uint8_t bindx = 0;
    for (bindx = 0; bindx < box_ctrl.nbr; bindx++)
    {
        box[bindx].visible = true;
        box[bindx].font_indx = 4;
        box[bindx].font_size = 1;
        box[bindx].fill_color = TFT_BLUE;
        box[bindx].border_color = TFT_YELLOW;
        box[bindx].text_color = TFT_WHITE;

    }
    bindx = 0;
    uint8_t box_group = (uint8_t) BOX_GROUP_1;


    for (uint8_t box_group = (uint8_t) BOX_GROUP_1; box_group < BOX_GROUP_NBR_OF; box_group++)
    {
        uint16_t xpos = 0;
        uint16_t ypos = 0;
        
        if (box_group == BOX_GROUP_MENU) ypos = TFT_LS_HIGHT / TFT_LS_ROWS * 7;

        uint8_t nbr_boxes = boxgr[box_group].nbr;
        
        boxgr[box_group].index = bindx;
        for(uint8_t i = 0; i < nbr_boxes; i++)
        {
            box[bindx].x_pos        = xpos;
            box[bindx].y_pos        = ypos;
            box[bindx].width        = boxgr[box_group].width;
            box[bindx].height       = boxgr[box_group].height;
            box[bindx].font_indx    = boxgr[box_group].font_indx;
            box[bindx].font_size    = boxgr[box_group].font_size;
            box[bindx].group        = box_group;
            sprintf(box[bindx].txt, "%d-%d-%d",
                box_group,
                bindx,
                boxgr[box_group].nbr
            );
            box[bindx].visible = true;
            bindx++;
            if(bindx >= BOX_MAX_NUMBER) {
                Serial.println("!!!! ERROR MAX BOX NUMBER EXCEEDED !!!!");
                box_group =99;
                break;
            }
            if (box_group == BOX_GROUP_MENU)
                xpos += boxgr[box_group].width;
            else
                ypos += boxgr[box_group].height;
        }
    } 
    box_ctrl.nbr = bindx - 1;
}

void box_reserve(uint8_t res_bm)
{
    box_ctrl.reserve  |= res_bm;
}
void box_release(uint8_t res_bm)
{
    box_ctrl.reserve  &= ~res_bm;
}

bool box_is_not_reserved(void)
{
    return (box_ctrl.reserve == 0);
}


uint8_t box_get_indx(uint8_t box_group, uint8_t bindx)
{
    if( bindx < boxgr[box_group].nbr)
        return boxgr[box_group].index + bindx;
    else 
        return boxgr[box_group].index;
}
void box_set_visible(uint8_t box_group, uint8_t bindx, boolean visible )
{
    uint8_t indx = box_get_indx(box_group, bindx);
    box[bindx].visible = visible;
}

void box_print_text(uint8_t bindx, char *txt)
{
    box[bindx].visible = true;
    strncpy(box[bindx].txt, txt, BASIC_ROW_LEN);
}

void box_show_one( uint8_t bindx)
{
    uint8_t group = box[bindx].group;
    tft.setTextSize(box[bindx].font_size);
    tft.setTextFont(box[bindx].font_indx);
    tft.setTextColor(box[bindx].text_color, box[bindx].fill_color, false);
    if(boxgr[group].round == 0) {
        tft.fillRect(box[bindx].x_pos, box[bindx].y_pos, box[bindx].width, box[bindx].height, box[bindx].fill_color);
        tft.drawRect(box[bindx].x_pos, box[bindx].y_pos, box[bindx].width, box[bindx].height, box[bindx].border_color);
    }
    else {
        tft.fillRoundRect(box[bindx].x_pos, box[bindx].y_pos, box[bindx].width, box[bindx].height, boxgr[group].round, box[bindx].fill_color);
        tft.drawRoundRect(box[bindx].x_pos, box[bindx].y_pos, box[bindx].width, box[bindx].height, boxgr[group].round, box[bindx].border_color);
    }
    tft.drawString( box[bindx].txt , box[bindx].x_pos+4, box[bindx].y_pos+2, box[bindx].font_indx);
}

void box_show_group(uint8_t box_group)
{
    uint8_t bindx = boxgr[box_group].index;
    uint8_t rows = boxgr[box_group].nbr;

    for (uint8_t i = 0; i< rows; i++)
    {
        box_show_one(bindx+i);
    }    
}

void box_show_all(void)
{
    for( uint8_t bindx = 0; bindx < box_ctrl.nbr; bindx++ )
    {
        if(  box[bindx].visible)
        {
            box_show_one( bindx);
            delay(100);
        }
    }
}

void box_hide_all(void)
{
    for( uint8_t bindx = 0; bindx < box_ctrl.nbr; bindx++ )
    {
        box[bindx].visible = false;
    }
}

void box_clone(uint8_t target, uint8_t source)
{
    box[target].visible      = box[source].visible;
    box[target].font_indx    = box[source].font_indx;
    box[target].font_size    = box[source].font_size;
    box[target].fill_color   = box[source].fill_color;
    box[target].border_color = box[source].border_color;
    box[target].text_color   = box[source].text_color;
    strncpy(box[target].txt,   box[source].txt, BASIC_ROW_LEN);
}
void box_clear(uint8_t bindx)
{
    uint8_t box_group =  box[bindx].group;
    memset(box[bindx].txt, 0x00, BASIC_ROW_LEN);
    box[bindx].font_indx    = boxgr[box_group].font_indx;
    box[bindx].font_size    = boxgr[box_group].font_size;
    box[bindx].fill_color   = box[BOX_DEFAULT_GROUP].fill_color;
    box[bindx].border_color = box[BOX_DEFAULT_GROUP].border_color;
    box[bindx].text_color   = box[BOX_DEFAULT_GROUP].text_color;
}

void box_clear_group(uint8_t box_group)
{
    uint8_t bindx = boxgr[box_group].index;
    uint8_t rows = boxgr[box_group].nbr;

    for (uint8_t i = 0; i< rows; i++)
    {
        box_clear(bindx+i);
    }
}
void box_paint(uint8_t bindx, uint8_t color_sch)
{
    box[bindx].fill_color   = box_color_scheme[color_sch].fill_color;
    box[bindx].border_color = box_color_scheme[color_sch].border_color;
    box[bindx].text_color   = box_color_scheme[color_sch].text_color;
}


void box_scroll_down(uint8_t box_group)
{
    uint8_t rows = boxgr[box_group].nbr;
    uint8_t bindx = boxgr[box_group].index + rows - 1;

    for (uint8_t i = 0; i < rows -1; i++)
    {
        box_clone(bindx, bindx - 1);
        bindx--;
    }
    bindx--;
    box_clear(bindx);
}

void box_scroll_test(uint8_t group)
{
    uint8_t color_sch = 0;
    uint8_t top_row = boxgr[group].index;

    box_clear_group(group);
    box_show_group(group);
    for (uint8_t n = 0; n < 20; n++)
    {
        box_scroll_down(group);
        sprintf(box[top_row].txt, "Group %d - Color %d", group, color_sch);
        box_paint(top_row,color_sch);

        box_show_group(group);
        color_sch++;
        if(color_sch >= 10) color_sch = 0;
        //delay(1000);
    }

}


void box_structure_print(void)
{
    Serial.println("Box Strucure:");

    for (uint8_t box_group = (uint8_t) BOX_GROUP_1; box_group < BOX_GROUP_NBR_OF; box_group++)
    {
        Serial.printf("BoxDim: %d: %d %d %d %d %d\n",
            box_group,        
            boxgr[box_group].index,        
            boxgr[box_group].nbr,        
            boxgr[box_group].height,
            boxgr[box_group].width,       
            boxgr[box_group].round        
        );
    }

    for (uint8_t i = 0; i < box_ctrl.nbr; i++ )
    {
        Serial.printf("%d: %d %d %d %d %d %d %d %d %d %d %d \n",
            i,
            box[i].group,
            box[i].visible,
            box[i].x_pos,
            box[i].y_pos,
            box[i].width,
            box[i].height,
            box[i].font_indx,
            box[i].font_size,
            box[i].fill_color,
            box[i].border_color,
            box[i].text_color
        );
    }
    
}