#ifndef __BOX_H__
#define __BOX_H__
#include "main.h"
#include <TFT_eSPI.h> 

//#define NBR_BASIC_DIM      12
// #define TFT_HIGHT           240
// #define TFT_WIDTH           320
#define TFT_LS_WIDTH        320
#define TFT_LS_HIGHT        240
#define TFT_LS_ROWS         8
//#define BASIC_ROW_HEIGHT    (TFT_LS_HEIGHT / NBR_BASIC_DIM)
#define BASIC_ROW_LEN       40

typedef enum 
{
    BOX_GROUP_1 = 0,
    BOX_GROUP_8,
    BOX_GROUP_4,
    BOX_GROUP_3,
    BOX_GROUP_2,
    BOX_GROUP_MENU,
    BOX_GROUP_HEAD_ROOM,
    BOX_GROUP_NBR_OF
} box_group_et;

typedef enum
{
    BOX_FONT_GLCD   = 1,
    BOX_FONT_SMALL_16  = 2,
    BOX_FONT_MEDIUM_26 = 4,
    BOX_FONT_LARGE_48  = 6,            
    BOX_FONT_7_SEGM_48  = 7,
    BOX_FONT_XXL_75     = 8,
} box_font_et;

typedef enum 
{
    BOX_RESERVE_MENU    = 0b00000001,
    BOX_RESERVE_NONE    = 0b00000000,
} box_reserve_et;


typedef struct 
{
    uint16_t height;
    uint16_t width; 
    uint16_t round;
    uint16_t nbr;
    uint8_t  index;
    uint8_t  font_indx;
    uint8_t  font_size;
} box_group_st;


void box_initialize(void);

void box_reserve(uint8_t res_bm);

void box_release(uint8_t res_bm);

bool box_is_not_reserved(void);


uint8_t box_get_indx(uint8_t box_group, uint8_t bindx);

void box_set_visible(uint8_t box_group, uint8_t bindx, boolean visible );

void box_print_text(uint8_t bindx, char *txt);

void box_show_one( uint8_t bindx);

void box_show_group(uint8_t box_group);

void box_show_all(void);

void box_hide_all(void);

void box_clone(uint8_t target, uint8_t source);

void box_clear(uint8_t bindx);

void box_clear_group(uint8_t box_group);

void box_paint(uint8_t bindx, uint8_t color_sch);

void box_scroll_down(uint8_t box_group);

void box_scroll_test(uint8_t group);

void box_structure_print(void);

#endif