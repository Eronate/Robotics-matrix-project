//MATRIXPROJECTROBOTICS.INO


#define checkpointWallsTimer 1250
#define doorTimer 1000
#define keyTimer 500
#define playerTimer 100
#define moveInterval 333
#define ANOMALY_SWAP_INTERVAL 1000
#define ANOMALY_PULL_INTERVAL 2000
#define R1_ENEMIES_MOVE_CD 250

#define DEATH_WALL 1
#define CHECKPOINT_WALL 2
#define DOOR 3
#define KEY 4
#define PLAYER_VALUE 5
#define ENEMY 6
#define MAX_BYTE 255
#define MATRIX_SIZE 16

//A_MENU.INO
#define  MAIN_MENU 0 //must be first in the list for navigating to work

#define  MM_ENTRIES_START 1 //logic for main menu navigation for less memory usage, 2 indexes to iterate through
#define  START_GAME 1
#define  HOW_TO_PLAY 2
#define  ABOUT 3
#define  HIGHSCORES 4
#define  SETTINGS 5
#define MM_ENTRIES_END 5 //logic for main menu navigation for less memory usage, must modify if adding more mm entries

#define  ENDGAME_LOG 6

#define  SETTINGS_ENTRIES_START 7 //logic for settings menu navigation for less memory usage, 2 indexes to iterate through
#define  ENTER_NAME 7
#define  SOUNDS_CONTROL 8
#define  LCD_BRIGHTNESS 9
#define  MATRIX_BRIGHTNESS 10 
#define  RESET_HIGHSCORES 11
#define  SETTINGS_ENTRIES_END 11 //logic for settings menu navigation for less memory usage
//--------------------------------------------------------------------------------------------------

#define  MIN_JOYSTICK_DEFAULT 400
#define  MAX_JOYSTICK_DEFAULT 650
#define  AUTOSCROLL_SPEED 255

#define NO_OF_HIGHSCORES 5

#define LEFT_ARROW 0
#define UP_ARROW 1
#define RIGHT_ARROW 2
#define SKULL 3

#define PER_STROKE_BRIGHTNESS_UP_INATION 10

#define DEBOUNCE_INTERVAL 25
#define MENU_SCROLL_INTERVAL 250
#define INITIAL_DELAY 1500

//---------------------------------------------------------------------------------------------------

//macros for bounding the matrix

#define R1_UP_BORDER    8
#define R1_DOWN_BORDER  15
#define R1_LEFT_BORDER  0
#define R1_RIGHT_BORDER 7

#define R2_UP_BORDER    8
#define R2_DOWN_BORDER  15
#define R2_LEFT_BORDER  8
#define R2_RIGHT_BORDER 15

#define R3_UP_BORDER    0
#define R3_DOWN_BORDER  7
#define R3_LEFT_BORDER  0
#define R3_RIGHT_BORDER 7

#define R4_UP_BORDER    0
#define R4_DOWN_BORDER  7
#define R4_LEFT_BORDER  8
#define R4_RIGHT_BORDER 15

#define PLAYER_START_COL 0
#define PLAYER_START_ROW 9

#define FIRST_DOOR_POSITION_COL 0
#define FIRST_DOOR_POSITION_ROW 8
#define FIRST_KEY_POSITION_COL 15
#define FIRST_KEY_POSITION_ROW 12

#define SECOND_KEY_POSITION_ROW 0 
#define SECOND_KEY_POSITION_COL 0
#define SECOND_DOOR_POSITION_COL 9
#define SECOND_DOOR_POSITION_ROW 7

#define ROOM1_NAME "1. Sliders"
#define ROOM2_NAME "2. Black Hole"
#define ROOM3_NAME "3. Satellites"
#define ROOM4_NAME "4. Escalatorz"

#define GAME_STATE_PLAYING 0 
#define GAME_STATE_MENUS 1
#define GAME_STATE_ENDING 2

#define ANOMALY_CENTER_COL 12
#define ANOMALY_CENTER_ROW 11

#define R1_LEFT_LINE 2
#define R1_RIGHT_LINE 5

#define R3_FIRST_BATCH_MIDDLE_LINE 4
#define R3_FIRST_BATCH_BOTTOM_LINE 7
#define R3_FIRST_BATCH_LEFT_LINE 1
#define R3_FIRST_BATCH_RIGHT_LINE 4

#define R3_SECOND_BATCH_MIDDLE_LINE 4
#define R3_SECOND_BATCH_BOTTOM_LINE 7
#define R3_SECOND_BATCH_LEFT_LINE 4
#define R3_SECOND_BATCH_RIGHT_LINE 7

#define ROOM3_ENEMIES_MOVE_INTERVAL 200

#define SLIDER_INTERVAL 700
#define SQUARE_MOVE_INTERVAL 500

#define ENDING_SQUARE1_COL 14
#define ENDING_SQUARE1_ROW 7

#define ENDING_SQUARE2_COL 15
#define ENDING_SQUARE2_ROW 7
