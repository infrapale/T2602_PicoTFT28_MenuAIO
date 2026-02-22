# T2602_PicoTFT28_Menu_AIO_v1
TFT ILI9341 consol
- 4 keys
- SD-card
- PicoW


Setup Bodmer TFT_eSPI library
  Install TFT_eSPI Library in Arduino IDE
  Open console
  Change directory to the active libraries folder containing the TFT_eSPI librarie
  Clone https://github.com/infrapale/T2602_My_TFT_Setups.git
  The libraries folder should lookk like this
    libraries
      T2602_My_TFT_Setups
      TFT_eSPI


  Modify User_Setup_Select. at the root of the TFT_eSPI library:

    . . .
    ///////////////////////////////////////////////////////
    //   User configuration selection lines are below    //
    ///////////////////////////////////////////////////////

    // Only ONE line below should be uncommented to define your setup.  Add extra lines and files as needed.

    // This modification must be done in the TFT_eSPI library / User_Setup_Select.h file:

    // Defined in Application / main.h:
    // 	#define BOARD_PICO_TFT_4KEYS 		1
    // 	#define BOARD_TFT_4_QUADCORE_PICO	2
    // 	#define TFT_TARGET_BOARD            BOARD_PICO_TFT_4KEYS  

    //#pragma message("In: /T2602_My_TFT_Setups/")
    #if TFT_TARGET_BOARD == BOARD_PICO_TFT_4KEYS
      #include "../T2602_My_TFT_Setups/Setup_PicoConsol_ILI9341.h"
      //#pragma message("Including: Setup_PicoConsol_ILI9341.h")
    #elif TFT_TARGET_BOARD == BOARD_TFT_4_QUADCORE_PICO
      #include "../T2602_My_TFT_Setups/Setup_PicoQuadCore_ILI9488.h"
      //#pragma message("Including: Setup_PicoConsol_ILI9488.h")
    #else
      #pragma message("ERROR!!! No board selected")
      #include <User_Setup.h>           // Default setup is root library folder
    #endif

    . .


#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
#define TFT_PINK        0xFE19      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F
#define TFT_BROWN       0x9A60      /* 150,  75,   0 */
#define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
#define TFT_SILVER      0xC618      /* 192, 192, 192 */
#define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
#define TFT_VIOLET      0x915C      /* 180,  46, 226 */

