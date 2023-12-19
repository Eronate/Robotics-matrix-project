#include "Arduino.h"
#include "EEPROM.h"
#include "LiquidCrystal.h"
#include "string.h"
#include "Macros.h"

bool soundsONBoolean;
extern char playerName[];
byte menusState = MAIN_MENU;
extern byte gameState;
extern byte lcdBrightness;
extern byte needsToUpdateLCD;
const byte leftArrow[8] = {
  B00000,
  B00000,
  B00010,
  B00110,
  B01110,
  B00110,
  B00010,
  B00000
};

const byte rightArrow[8] = {
  B00000,
  B00000,
  B01000,
  B01100,
  B01110,
  B01100,
  B01000,
  B00000
};

const byte upArrow[8] = {
  B00000,
  B00000,
  B00100,
  B01110,
  B11111,
  B00000,
  B00000,
  B00000
};

const byte skull[] = {
  B00000,
  B00000,
  B01110,
  B11111,
  B10101,
  B11111,
  B01010,
  B01010
};

extern LiquidCrystal lcd;
extern const short bootingUpMessageTimer;

//for joystick
extern const byte horizontalJoystickPin, verticalJoystickPin, clickJoystickPin;
extern short horizontalInput, verticalInput;
extern bool clickInput;

//menu variables
short submenusIndex = 0, 
      currentHoveringMenuPos = 1, 
      hasDisplayed = 0, 
      settingsMenuIndex = SETTINGS_ENTRIES_START, 
      currentNamingCursorPos = 0;
bool lastClickInput = 0, hasHandledClick = 0;
unsigned long printingStart = 0, lastMenuScrollReadingTime = 0, lastClickReadTime = 0, initialDelayStarter = 0;

byte hasReadEEPROM = 0, highscores[NO_OF_HIGHSCORES] = {13,13,15,16,18}, currentHighscoreIndex = 0;
char highscoreNames[NO_OF_HIGHSCORES][5] = {
                                            {"MOCK"}, {"JACK"}, {"MACK"}, {"RACK"}, {"HACK"}
                                          };

bool Debouncer(bool buttonState, bool &lastButtonState, unsigned long &startClickButton, bool &hasHandledClick)
{
  if(buttonState == 1 && lastButtonState == 0)
    {
      startClickButton = millis();
      hasHandledClick = 0;
    }
  else if (buttonState == 1 && millis() - startClickButton > DEBOUNCE_INTERVAL && hasHandledClick == 0)
  {
    startClickButton = millis();
    return 1;
  }

  else 
  {
    return 0;
  }
}

void HandleHorizontallyMovingBetweenMenus(short &menuPos, const short start, const short end)
{
  if(millis() - lastMenuScrollReadingTime > MENU_SCROLL_INTERVAL)
  {
    if(horizontalInput > MAX_JOYSTICK_DEFAULT) //moved right
        //index based iteration for less memory usage
    {
      menuPos = menuPos + 1 <= end ? menuPos + 1 : start;
      hasDisplayed = 0;
    }
    else if(horizontalInput < MIN_JOYSTICK_DEFAULT) //moved left
    {
      menuPos = menuPos - 1 >= start ? menuPos - 1 : end;
      hasDisplayed = 0;
    }
    lastMenuScrollReadingTime = millis();
  }
}

void rereadJoystickInputs()
{
  horizontalInput = analogRead(horizontalJoystickPin);
  verticalInput   = analogRead(verticalJoystickPin);
}

short currentPosInString = 0; //bound to the function below, can only have 1 autoscroll at a time
void PrintStringWithAutoscrollOnLineX(char string[], byte line)
{
  if(millis() - initialDelayStarter < INITIAL_DELAY)
  {
    if(!hasDisplayed)
      {
        lcd.clear();
        for(int i = currentPosInString; i <= currentPosInString + 15; i++)
          lcd.print(string[i]);
        hasDisplayed = 1;
      }
  }
  else
    if(millis() - printingStart > AUTOSCROLL_SPEED)
    {
      //lcd.clear(); commented for now
      lcd.clear();
      lcd.setCursor(0, line);
      //check if accessing out of bounds memory.
      if(string[currentPosInString + 15] != NULL)
      {
        for(int i = currentPosInString; i <= currentPosInString + 15; i++)
          lcd.print(string[i]);
        printingStart = millis();
        currentPosInString++;
      }
      else 
      {
        initialDelayStarter = millis();
        hasDisplayed = 0;
        currentPosInString = 0; 
      }
    }
}

void PrintMenuChoice(String s)
{
  if(!hasDisplayed)
  {
    lcd.clear();
  //rewrite the title
    lcd.setCursor(0,0);
    lcd.write("Platformer menus");

  //write menu indicators for which joystick input to push
    lcd.setCursor(0,1);
    lcd.write(byte(LEFT_ARROW));
    lcd.print(s);
    lcd.write(byte(RIGHT_ARROW));
    hasDisplayed = 1;
  }
}

//HANDLER FOR MAIN_MENU STATE
void MainMenuHandler()
{
    //read joystick inputs and 
    rereadJoystickInputs();

    const short start = MM_ENTRIES_START, end = MM_ENTRIES_END;
    HandleHorizontallyMovingBetweenMenus(currentHoveringMenuPos, start, end); //handle moving between menus with a delay between inputs to not scroll many menus at once

    switch(currentHoveringMenuPos)
    {
      case START_GAME:
        PrintMenuChoice("START GAME");
        break;
      case HOW_TO_PLAY:
        PrintMenuChoice("HOW_TO_PLAY");
        break;
      case ABOUT:
        PrintMenuChoice("ABOUT");
        break;
      case HIGHSCORES:
        PrintMenuChoice("HIGHSCORES");
        break;
      case SETTINGS:
        PrintMenuChoice("SETTINGS");
        break;
      default:  
        Serial.println("Something went wrong.");
        break;
    }
    lastClickInput = clickInput;
    clickInput = !digitalRead(clickJoystickPin);

    if(Debouncer(clickInput, lastClickInput, lastClickReadTime, hasHandledClick)) //if the
    {
       //Serial.println(clickInput);
        //set the global menu state to the current hovering main menu choice
      menusState = currentHoveringMenuPos;
      hasDisplayed = 0;
      hasHandledClick = 1;
      currentHighscoreIndex = 0;
      hasReadEEPROM = 0;
      return;
    }
}

bool hasFinishedBooting = 0;
//display while game is booting up
void DisplayOnStart()
{
  if(hasDisplayed == 0)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome to Platformer!");
    lcd.setCursor(0, 1);
    lcd.print("Buckle up");
    hasDisplayed = 1;
    hasFinishedBooting = 1;
  }
}

//represents how many highscores are currently available. if the games not been played yet, it's going to be 0
byte highscoreCount; //will eventually have to be an export variable that is borrowed from the main game's state.
void HighscoresHandler()
{
  if(!hasReadEEPROM)
  {
    /*EEPROM:
     0                  ... (NO_OF_HIGHSCORES)        : BYTES REPRESENTING HIGHSCORE DEATH COUNTS
     (NO_OF_HIGHSCORES) ... (NO_OF_HIGHSCORES * 5 - 1): BYTES REPRESENTING THE TOP PLAYERS' NAMES*/
    highscoreCount = EEPROM.read(0);
    //Serial.println(highscoreCount);
    for(int i = 1; i< highscoreCount * 5 + 1; i+=5) //byte for each highscore
    {
      highscores[i/5] = EEPROM.read(i);
      highscoreNames[i/5][0] = char(EEPROM.read(i+1));
      highscoreNames[i/5][1] = char(EEPROM.read(i+2));
      highscoreNames[i/5][2] = char(EEPROM.read(i+3));
      highscoreNames[i/5][3] = char(EEPROM.read(i+4));
      highscoreNames[i/5][4] = '\0';
      Serial.println("Highscore name in function: ");
      Serial.println(highscoreNames[i/5]);
    }
    hasReadEEPROM = 1;
  }
  if(hasDisplayed == 0)
  {
    lcd.clear();
    int i;
    //print 2 hs' at once
    if( 
        (currentHighscoreIndex + 1) <= (highscoreCount - 1)
      )
    {
      lcd.setCursor(0, 0);
      //array is indexed from 0, highscores are printed on display starting from index 1
      lcd.print(currentHighscoreIndex+1);
      lcd.print(". ");
      lcd.print(highscores[currentHighscoreIndex]);
      lcd.print(" ");
      lcd.print(highscoreNames[currentHighscoreIndex]);

      lcd.setCursor(0, 1);
      //array is indexed from 0, highscores are printed on display starting from index 1
      lcd.print(currentHighscoreIndex+2);
      lcd.print(". ");
      lcd.print(highscores[currentHighscoreIndex+1]);
      lcd.print(" ");
      lcd.print(highscoreNames[currentHighscoreIndex + 1]);
    }

    //write the remainder scores to LCD
    if((currentHighscoreIndex) == (highscoreCount - 1)) 
    {
      lcd.setCursor(0, 0);
      lcd.print(currentHighscoreIndex+1);
      lcd.print(". ");
      lcd.print(highscores[currentHighscoreIndex]);
      lcd.print(" ");
      lcd.print(highscoreNames[currentHighscoreIndex]);
    }
    
    //conditionally render "BACK" visual indicator to go back to main menu
    if(currentHighscoreIndex == 0)
    {
      lcd.setCursor(14, 0);
      lcd.write(byte(UP_ARROW));
    }
    // lcd.setCursor(10, 1);
    // lcd.write("SCROLL");
    hasDisplayed = 1; // must be reset to 0 when scrolling down
  }

  rereadJoystickInputs();

  if(millis() - lastMenuScrollReadingTime > MENU_SCROLL_INTERVAL)
  {//go to main menus if pressed up and currently displaying first 2 highscores
    if(verticalInput > MAX_JOYSTICK_DEFAULT && currentHighscoreIndex == 0)
    {
      hasDisplayed = 0;
      //hasReadEEPROM = 0; will have to be decommented once the game actually works
      menusState = MAIN_MENU;
      return;
    }

    else if(verticalInput > MAX_JOYSTICK_DEFAULT && currentHighscoreIndex >= 2)
    {
      hasDisplayed = 0;
      currentHighscoreIndex -= 2;
    }
    
    else if(verticalInput < MIN_JOYSTICK_DEFAULT && currentHighscoreIndex + 2 < highscoreCount) //scrolling down
    {
      hasDisplayed = 0;
      currentHighscoreIndex+=2;
    }
    lastMenuScrollReadingTime = millis();
  }
}
bool doneSetLedsOff = 0;
void SettingsHandler()
{
  rereadJoystickInputs();
  const short start = SETTINGS_ENTRIES_START, end = SETTINGS_ENTRIES_END;
  short lastIndex = settingsMenuIndex;
  HandleHorizontallyMovingBetweenMenus(settingsMenuIndex, start, end);
  if(lastIndex == MATRIX_BRIGHTNESS && settingsMenuIndex!= lastIndex && doneSetLedsOff == 0)
  {
    for (byte row = 0; row < MATRIX_SIZE; row++) 
      for (byte col = 0; col < MATRIX_SIZE; col++) 
        lc.setLed(0, row, col, 0);
    doneSetLedsOff = 1;
  }
  switch(settingsMenuIndex)
  {
    case ENTER_NAME:
    {
      if (hasDisplayed == 0 )
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(byte(LEFT_ARROW));
        lcd.write("Enter name");
        lcd.write(byte(RIGHT_ARROW));
        lcd.setCursor(10, 1);
        lcd.write(byte(UP_ARROW));
        lcd.write("Back");
        hasDisplayed = 1;
      }
      lastClickInput = clickInput;
      clickInput = !digitalRead(clickJoystickPin);
      if(Debouncer(clickInput, lastClickInput, lastClickReadTime, hasHandledClick)) //if clicked, go to the menu in question
      {
        hasDisplayed = 0;
        hasHandledClick = 1;
        menusState = ENTER_NAME;
        return;
      }
      break;
    }
    case LCD_BRIGHTNESS:
    {
      if(hasDisplayed == 0)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(byte(LEFT_ARROW));
        lcd.write("LCD Brightness");
        lcd.write(byte(RIGHT_ARROW));
        lcd.setCursor(0, 1);
        lcd.write("Adjust with joystick");
        hasDisplayed = 1;
      }
      rereadJoystickInputs();
      if(millis() - lastMenuScrollReadingTime > MENU_SCROLL_INTERVAL)
      {
        if(verticalInput > MAX_JOYSTICK_DEFAULT)
        {
          lcdBrightness = min(255, lcdBrightness + PER_STROKE_BRIGHTNESS_UP_INATION); 
          analogWrite(LCDBacklightPin, lcdBrightness);
        }
        if(verticalInput < MIN_JOYSTICK_DEFAULT)
        {
          lcdBrightness = max(0, lcdBrightness - PER_STROKE_BRIGHTNESS_UP_INATION);
          analogWrite(LCDBacklightPin, lcdBrightness); 
        }
        lastMenuScrollReadingTime = millis();
      }
      break;
    }
    case SOUNDS_CONTROL:
    {
      if(hasDisplayed == 0)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(byte(LEFT_ARROW));
        soundsONBoolean = EEPROM.read(1 + 5 * NO_OF_HIGHSCORES + 4 + 1 + 1);
        soundsONBoolean == 1 ? lcd.write(" Sounds ON ") : lcd.write(" Sounds OFF ");
        lcd.write(byte(RIGHT_ARROW));
        lcd.setCursor(0, 1);
        lcd.write("Click.");
        lcd.setCursor(10,1);
        lcd.write(byte(UP_ARROW));
        lcd.write("Back");
        lcd.write(byte(RIGHT_ARROW));
        hasDisplayed = 1;
      }
      lastClickInput = clickInput;
      clickInput = !digitalRead(clickJoystickPin);
      if(Debouncer(clickInput, lastClickInput, lastClickReadTime, hasHandledClick)) //if clicked, go to the menu in question
      {
        soundsONBoolean = !soundsONBoolean;
        EEPROM.put(1 + 5 * NO_OF_HIGHSCORES + 4 + 1 + 1, soundsONBoolean);
        hasDisplayed = 0;
        hasHandledClick = 1;
        return;
      }
      break;
    }
    case MATRIX_BRIGHTNESS:
    {
      if(hasDisplayed == 0)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(byte(LEFT_ARROW));
        lcd.write("Matrix Bright.");
        lcd.write(byte(RIGHT_ARROW));
        lcd.setCursor(0, 1);
        lcd.write("Adjust with joystick");
        hasDisplayed = 1;
        for (byte row = 0; row < MATRIX_SIZE; row++) 
          for (byte col = 0; col < MATRIX_SIZE; col++) 
            lc.setLed(0, row, col, 1);
        doneSetLedsOff = 0;
      }
      rereadJoystickInputs();
      if(millis() - lastMenuScrollReadingTime > MENU_SCROLL_INTERVAL)
      {
        if(verticalInput > MAX_JOYSTICK_DEFAULT)
        {
          matrixBrightness = min(15, matrixBrightness + 1); 
          lc.setIntensity(0, matrixBrightness);
        }
        if(verticalInput < MIN_JOYSTICK_DEFAULT)
        {
          matrixBrightness = max(0, matrixBrightness - 1);
          lc.setIntensity(0, matrixBrightness);
        }
        lastMenuScrollReadingTime = millis();
      }
      break;
    }
    case RESET_HIGHSCORES:
    {
      if(hasDisplayed == 0)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(byte(LEFT_ARROW));
        lcd.write("Reset Highsc.");
        lcd.write(byte(RIGHT_ARROW));
        hasDisplayed = 1;
      }
      lastClickInput = clickInput;
      clickInput = !digitalRead(clickJoystickPin);
      if(Debouncer(clickInput, lastClickInput, lastClickReadTime, hasHandledClick)) //if clicked, go to the menu in question
      {
        EEPROM.put(0,0);
        for(int i = 1; i < 1 + 5* NO_OF_HIGHSCORES; i+=5)
          EEPROM.put(i, 255);
        lcd.setCursor(0,1);
        lcd.print("Reset.");
        hasHandledClick = 1;
        hasReadEEPROM = 0;
      }
      break;
    }
  }
    
  if(settingsMenuIndex != LCD_BRIGHTNESS && settingsMenuIndex != MATRIX_BRIGHTNESS)
  {
    rereadJoystickInputs();
    if(verticalInput > MAX_JOYSTICK_DEFAULT)
    {
      //if leaving settings menu, update eeprom values. this is to prevent eeprom updating at every step
      if(lcdBrightness != EEPROM.read(5 * NO_OF_HIGHSCORES + 1 + 4))
        EEPROM.put(5 * NO_OF_HIGHSCORES + 1 + 4, lcdBrightness);
      if(matrixBrightness != EEPROM.read(1 + 5 * NO_OF_HIGHSCORES + 5))
        EEPROM.put(1 + 5 * NO_OF_HIGHSCORES + 5, matrixBrightness);
      menusState = MAIN_MENU;
      hasDisplayed = 0;
      return;
    }
  }
}


char currentLetter;
void EnterNameHandler()
{
  if(hasDisplayed == 0)
  {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.write(byte(RIGHT_ARROW));
    lcd.write(playerName);
    lcd.setCursor(5 + currentNamingCursorPos, 0);
    //lcd.blink();
    hasDisplayed = 1;

    lcd.setCursor(0, 1);
    lcd.write(byte(LEFT_ARROW));
    lcd.write(byte(RIGHT_ARROW));
    lcd.write(",");
    lcd.write(UP_ARROW);
    lcd.write("= Change");
  }
  lastClickInput = clickInput;
  clickInput = !digitalRead(clickJoystickPin);

  if(Debouncer(clickInput, lastClickInput, lastClickReadTime, hasHandledClick)) //if clicked, go to the menu in question
  {
    hasDisplayed = 0;
    hasHandledClick = 1;
    menusState = SETTINGS;
    //saves the player's name into EEPROM once the player presses "OK"
    EEPROM.put(1 + 5*NO_OF_HIGHSCORES, playerName[0]);
    EEPROM.put(1 + 5*NO_OF_HIGHSCORES + 1, playerName[1]);
    EEPROM.put(1 + 5*NO_OF_HIGHSCORES + 2, playerName[2]);
    EEPROM.put(1 + 5*NO_OF_HIGHSCORES + 3, playerName[3]);
    return;
  }
  rereadJoystickInputs();
  //handles changing the letter at the current position 
  if(millis() - lastMenuScrollReadingTime > MENU_SCROLL_INTERVAL)
  {
    currentLetter = playerName[currentNamingCursorPos];
    if(verticalInput < MIN_JOYSTICK_DEFAULT)
    {
      playerName[currentNamingCursorPos] = (currentLetter == 'Z') ? 'A' : char(int(currentLetter) + 1);
      hasDisplayed = 0;
      lastMenuScrollReadingTime = millis(); //update this variable in the ifs so it doesn't go on "cooldown" even if you've not pressed anything
    }
    else if(verticalInput > MAX_JOYSTICK_DEFAULT)
    {
      playerName[currentNamingCursorPos] = (currentLetter == 'A') ? 'Z' : char(int(currentLetter) - 1);
      hasDisplayed = 0;
      lastMenuScrollReadingTime = millis();
    }
  }
  HandleHorizontallyMovingBetweenMenus(currentNamingCursorPos, 0, 3); //moving in between the 4 letters of the names
}

void GlobalMenuHandler()
{
    //If joystick is in up position, go back to main menu
    switch(menusState)
    {
    case MAIN_MENU:
      MainMenuHandler();
      break;
    case START_GAME:
    {
      gameState = GAME_STATE_PLAYING;
      needsToUpdateLCD = 1;
      menusState = MAIN_MENU;
      break;
    }
    case HOW_TO_PLAY:
      {
        //lcd.clear();
        if(initialDelayStarter == 0)
          initialDelayStarter = millis();
        PrintStringWithAutoscrollOnLineX("Avoid moving enemies that reset you to the last checkpoint zone if you touch them. Open doors with keys. Seek the way that checkpoints indicate.", 0);
        lcd.setCursor(10,1);
        lcd.write(byte(UP_ARROW)); //up arrow symbol
        lcd.print("Back");
        rereadJoystickInputs();
        if(verticalInput > MAX_JOYSTICK_DEFAULT)
        {
          currentPosInString = 0;
          initialDelayStarter = 0;
          hasDisplayed = 0;
          menusState = MAIN_MENU;
          return;
        }
        break;
      }
    case ABOUT:
      {
        //lcd.clear();
        if(initialDelayStarter == 0)
          initialDelayStarter = millis();
        PrintStringWithAutoscrollOnLineX("Github: https://github.com/Eronate, Dev: Duduman Cristian ", 0);  
        lcd.setCursor(10,1); 
        lcd.write(byte(UP_ARROW)); //up arrow symbol
        lcd.print("Back");
        rereadJoystickInputs();
        if(verticalInput > MAX_JOYSTICK_DEFAULT)
        {
          currentPosInString = 0;
          initialDelayStarter = 0;
          hasDisplayed = 0;
          menusState = MAIN_MENU;
          return;
        }
        break;
      }
    //top 5 will be saved into EEPROM
    case HIGHSCORES:
      HighscoresHandler();
      break;
    case SETTINGS:
      SettingsHandler();
      break;
    // case LCD_BRIGHTNESS:
    //   LCDBrightnessHandler();
    //   break;
    // case SOUNDS_CONTROL:
    //   SoundsControlHandler();
    //   break;
    case ENTER_NAME:
      EnterNameHandler();
      break;
    default:  
      Serial.println("Something went wrong.");
      break;
    }
}


