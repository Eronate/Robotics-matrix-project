#include "Arduino.h"
#include "LedControl.h" 
#include "LiquidCrystal.h"
#include "EEPROM.h"
#include "Macros.h"


void GlobalMenuHandler();
void MainMenuHandler();
const short bootingUpMessageTimer = 5000;
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;

byte gameState = GAME_STATE_MENUS;

unsigned long checkpointWallsStart=0, playerTimerStart=0, keyStart=0, doorStart=0,
              moveStart = 0, endingStart = 0;
byte playerLEDON = 0;
// pin 12 is connected to the MAX7219 pin 1
// pin 11 is connected to the CLK pin 13
// pin 10 is connected to LOAD pin 12
// 1 as we are only using 1 MAX7219
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte horizontalJoystickPin = A1;
const byte verticalJoystickPin = A0;
const byte clickJoystickPin = 2;
const byte LCDBacklightPin = 3;
//custom characters
extern const byte leftArrow[8], rightArrow[8], upArrow[8], skull[8];

extern short hasDisplayed;
extern bool hasFinishedBooting;

//joystick vars
short horizontalInput, verticalInput;
bool clickInput = 0;

byte matrixBrightness = 2;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); //DIN, CLK, LOAD, No. DRIVER

byte lcdBrightness = 127;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
char playerName[5];


byte playerPosRow = PLAYER_START_ROW, playerPosCol = PLAYER_START_COL;
//byte playerPosRow = 7, playerPosCol = 8;
//variables for room
byte rowStart, colStart;

bool hasMoved = 0;
byte needsToUpdateLCD = 0;

extern bool lastClickInput;
extern unsigned long lastClickReadTime;
char myCharArray[30];
String s;

byte matrix[MATRIX_SIZE][MATRIX_SIZE] = {
  //Room 3                  //Room 4
  {4, 0, 0, 0, 6, 0, 0, 0,  0, 1, 6, 6, 2, 2, 0, 0},
  {1, 0, 2, 2, 0, 1, 1, 0,  0, 1, 0, 0, 1, 1, 0, 0},
  {1, 0, 2, 2, 0, 1, 1, 0,  0, 1, 0, 0, 0, 1, 6, 0},
  {1, 0, 0, 0, 0, 0, 0, 6,  0, 1, 0, 0, 1, 1, 0, 0},
  {1, 0, 0, 0, 6, 0, 0, 0,  0, 1, 0, 0, 0, 1, 6, 0},
  {1, 0, 1, 1, 0, 1, 1, 0,  0, 1, 0, 0, 1, 1, 0, 0},
  {2, 0, 1, 1, 0, 1, 1, 0,  0, 1, 0, 0, 0, 1, 6, 0},
  {0, 0, 0, 0, 0, 0, 0, 6,  0, 3, 6, 6, 1, 1, 2, 2},
  //Room 1                  //Room 2
  {3, 1, 1, 1, 1, 1, 1, 1,  1, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0,  2, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 6, 0, 0, 0, 1, 0,  2, 0, 0, 6, 0, 6, 0, 0},
  {0, 1, 0, 0, 0, 6, 1, 0,  2, 0, 0, 6, 6, 0, 0, 0},
  {0, 1, 6, 0, 0, 0, 1, 0,  2, 0, 6, 6, 6, 0, 0, 4},
  {2, 1, 0, 0, 0, 6, 1, 0,  2, 0, 0, 0, 6, 0, 0, 0},
  {2, 1, 6, 0, 0, 0, 1, 0,  2, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 1,  1, 0, 0, 0, 0, 0, 0, 0}

};

short deathCounterCurr = 0;
byte lastCheckpointCol = PLAYER_START_COL, lastCheckpointRow = PLAYER_START_ROW;
byte currentRoom = 1;

byte maxMatVal;

byte currentEvenCols = 2, currentOddCols = 5;
unsigned long anomalySwapStart = 0, anomalyPullStart = 0, room1ECD = 0, room3EnemiesStart =0;
bool anomalyIndex = 0;
bool evenOnesGoingRight = 1, oddOnesGoingLeft = 1;
byte firstEnemyPosR3Col = 4, firstEnemyPosR3Row = 4, secondEnemyPosR3Col = 7, secondEnemyPosR3Row = 7;

byte firstSliderRow = 0, secondSliderRow = 7;
bool firstSliderDescending = 1, secondSliderAscending = 1;
unsigned long moveSliderStart = 0, squareMoveStart = 0;
byte square1Row = 2, square1Col = 14, 
      square2Row = 4, square2Col = 14, 
      square3Row = 6, square3Col = 14;
byte counter = 1;

byte rankOnLadder = 0;
extern short currentPosInString;
extern bool hasHandledClick;

//for the 2 pairs of synchronised enemies
void IncrementEnemyPosition(byte &enemyPosR3Row, byte &enemyPosR3Col, String what, short step)
{
  matrix[enemyPosR3Row - 4][enemyPosR3Col] = 0;
  matrix[enemyPosR3Row][enemyPosR3Col] = 0;
  if(what == "col")
    enemyPosR3Col += step;
  else if(what == "row")
    enemyPosR3Row += step;
  else Serial.println("something went wrong");     
  matrix[enemyPosR3Row - 4][enemyPosR3Col] = ENEMY;
  matrix[enemyPosR3Row][enemyPosR3Col] = ENEMY;
}

void Room3EnemiesHandler(byte &enemyPosR3Row, byte &enemyPosR3Col, short batch)
{
  byte middle, bottom, left, right;
  if(batch == 1)
  {
    middle = R3_FIRST_BATCH_MIDDLE_LINE;
    bottom = R3_FIRST_BATCH_BOTTOM_LINE;
    left = R3_FIRST_BATCH_LEFT_LINE;
    right = R3_FIRST_BATCH_RIGHT_LINE;
  }
  else if(batch == 2)
  {
    middle = R3_SECOND_BATCH_MIDDLE_LINE;
    bottom = R3_SECOND_BATCH_BOTTOM_LINE;
    left = R3_SECOND_BATCH_LEFT_LINE;
    right = R3_SECOND_BATCH_RIGHT_LINE;
  }
  else 
    Serial.println("Batch must be 1 or 2");
  if(enemyPosR3Row == middle && enemyPosR3Col > left)
    IncrementEnemyPosition(enemyPosR3Row, enemyPosR3Col, "col", -1);
  else if(enemyPosR3Col == left && enemyPosR3Row < bottom)
    IncrementEnemyPosition(enemyPosR3Row, enemyPosR3Col, "row", 1);
  
  else if(enemyPosR3Row == bottom && enemyPosR3Col < right)
    IncrementEnemyPosition(enemyPosR3Row, enemyPosR3Col, "col", 1);
  
  else if(enemyPosR3Col == right && enemyPosR3Row > middle)
    IncrementEnemyPosition(enemyPosR3Row, enemyPosR3Col, "row", -1);
}

void DescendOrAscend(bool ascending, byte &sliderRow)
{
  matrix[sliderRow][10] = 0;
  matrix[sliderRow][11] = 0;
  if(ascending)
    sliderRow--;
  else
    sliderRow++;
}
void MoveInSquareShape(byte &row, byte &col, byte counter)
{
  matrix[row][col] = 0;
  switch(counter)
  {
    case 1:
      row-=1;
      break;
    case 2:
      col+=1;
      break;
    case 3:
      row+=1;
      break;
    case 4:
      col-=1;
      break;
  }
  matrix[row][col] = ENEMY;
}
void EnemyMovementAndRoomEffectsHandler()
{
  switch(currentRoom)
  {
    case 1:
    {
      //take samples of just 2 enemies to get the behavior of the rest
      if(millis() - room1ECD > R1_ENEMIES_MOVE_CD)
      {
        if(matrix[10][5] == ENEMY)
          evenOnesGoingRight = 0;
        else if(matrix[10][2] == ENEMY)
          evenOnesGoingRight = 1;
        if(matrix[11][2] == ENEMY)
          oddOnesGoingLeft = 0;
        else if(matrix[11][5] == ENEMY)
          oddOnesGoingLeft = 1;

        if(evenOnesGoingRight && currentEvenCols < R1_RIGHT_LINE)
        {
          for(int i = 10; i<= 14; i+=2)
          {
            matrix[i][currentEvenCols] = 0;
            matrix[i][currentEvenCols + 1] = ENEMY;
          }
          currentEvenCols++;
        }
        else if(!evenOnesGoingRight && currentEvenCols > R1_LEFT_LINE)
        {
          for(int i = 10; i<= 14; i+=2)
          {
            matrix[i][currentEvenCols] = 0;
            matrix[i][currentEvenCols - 1] = ENEMY;
          }
          currentEvenCols--;
        }
        if(oddOnesGoingLeft && currentOddCols > R1_LEFT_LINE)
        {
          for(int i = 11; i<= 13; i+=2)
          {
            matrix[i][currentOddCols] = 0;
            matrix[i][currentOddCols - 1] = ENEMY;
          }
          currentOddCols--;
        }
        
        else if(!oddOnesGoingLeft && currentOddCols < R1_RIGHT_LINE)
        {
          for(int i = 11; i<= 13; i+=2)
          {
            matrix[i][currentOddCols] = 0;
            matrix[i][currentOddCols + 1] = ENEMY;
          }
          currentOddCols++;
        }
        room1ECD = millis();
      }
      break;
    }
    case 2:
    {
      if(millis() - anomalySwapStart > ANOMALY_SWAP_INTERVAL)
      {
        //shapeshift anomaly
        if(anomalyIndex)
        {
          matrix[10][12] = 0;
          matrix[10][11] = 6;
          matrix[10][13] = 6;
          matrix[11][10] = 0;
          matrix[11][13] = 0;
          matrix[12][10] = 6;
          matrix[13][11] = 0;
          matrix[13][13] = 6;
        }
        else
        {
          matrix[10][12] = 6;
          matrix[10][11] = 0;
          matrix[10][13] = 0;
          matrix[11][10] = 6;
          matrix[11][13] = 6;
          matrix[12][10] = 0;
          matrix[13][11] = 6;
          matrix[13][13] = 0;
        }
        anomalyIndex = !anomalyIndex;
        anomalySwapStart = millis();
      }
      if(millis() - anomalyPullStart > ANOMALY_PULL_INTERVAL)
      {
        if(playerPosCol > ANOMALY_CENTER_COL)
          playerPosCol = playerPosCol - 1;

        else if(playerPosCol < ANOMALY_CENTER_COL)
          playerPosCol = playerPosCol + 1;

        if(playerPosRow > ANOMALY_CENTER_ROW)
          playerPosRow = playerPosRow - 1;
        
        else if(playerPosRow < ANOMALY_CENTER_ROW)
          playerPosRow = playerPosRow + 1;

        anomalyPullStart = millis();
      }
      break;
    }
    case 3:
    {
      if(millis() - room3EnemiesStart > ROOM3_ENEMIES_MOVE_INTERVAL )
      {
        Room3EnemiesHandler(firstEnemyPosR3Row, firstEnemyPosR3Col, 1);
        Room3EnemiesHandler(secondEnemyPosR3Row, secondEnemyPosR3Col, 2);
        room3EnemiesStart = millis();
      }
      break;
    }
    case 4: 
    {
      if(millis() - moveSliderStart > SLIDER_INTERVAL)
      {
        if(firstSliderRow == 7)
          firstSliderDescending = 0;
        else if(firstSliderRow == 0)
          firstSliderDescending = 1;

        if(secondSliderRow == 7)
          secondSliderAscending = 1;
        else if(secondSliderRow == 0)
          secondSliderAscending = 0;

        DescendOrAscend(secondSliderAscending, secondSliderRow);
        DescendOrAscend(!firstSliderDescending, firstSliderRow);
        //gets overwritten by the other elevator
        matrix[firstSliderRow][10] = ENEMY; 
        matrix[firstSliderRow][11] = ENEMY;
        matrix[secondSliderRow][10] = ENEMY; 
        matrix[secondSliderRow][11] = ENEMY;
        moveSliderStart = millis();
      }
      if(millis() - squareMoveStart > SQUARE_MOVE_INTERVAL)
      {
        MoveInSquareShape(square1Row, square1Col, counter);
        MoveInSquareShape(square2Row, square2Col, counter);
        MoveInSquareShape(square3Row, square3Col, counter);
        
        if(counter == 4)
          counter = 1;
        else 
          counter++;
        squareMoveStart = millis();
      }
      break;
    }
    default:
      break;
  }
}

void BlinkComponent(unsigned long &start, unsigned long timer, const short value, byte rowStart, byte colStart)
{
  if(millis() - start > timer)
  {
    for (byte row = rowStart; row < rowStart + 8; row++) 
      for (byte col = colStart; col < colStart + 8; col++) 
      {
        if(matrix[row][col] == value)
          matrix[row][col] = MAX_BYTE - value;
        else if(matrix[row][col] == MAX_BYTE - value)
          matrix[row][col] = value;
      }
    start = millis();
  }
}
void BlinkMatrixOnTimers(byte rowStart, byte colStart)
{
  //Handling checkpoint walls special blinking. 

  BlinkComponent(checkpointWallsStart, checkpointWallsTimer, CHECKPOINT_WALL, rowStart, colStart);
  //BlinkComponent(checkpointWallsStart, checkpointWallsTimer + 225, CHECKPOINT_WALL);
  //checkpointWallsStart = millis();

  BlinkComponent(keyStart, keyTimer, KEY, rowStart, colStart);
  //keyStart = millis();

  BlinkComponent(doorStart, doorTimer, DOOR, rowStart, colStart);
  //doorStart = millis();
  if(millis() - playerTimerStart > playerTimer)
  {
    playerLEDON = playerLEDON == 0 ? 1 : 0;
    playerTimerStart = millis();
  }
}

bool CheckIfAbleToMoveThere(byte previousRowVal, byte previousColVal, byte rowVal, byte colVal)
{
  if(rowVal < 0 || colVal < 0 || colVal >= MATRIX_SIZE || rowVal >= MATRIX_SIZE)
    return 0;
  //the following should not happen: go from R2 to R4 through R2UP
  //go from R3 to R1 through R3DOWN
  //go from R4 to R2 through R4DOWN
  bool R2toR4 = previousRowVal == R2_UP_BORDER && rowVal == R4_DOWN_BORDER && previousColVal >= R2_LEFT_BORDER && previousColVal <= R2_RIGHT_BORDER,
       R3toR1 = previousRowVal == R3_DOWN_BORDER && rowVal == R1_UP_BORDER && previousColVal >= R3_LEFT_BORDER && previousColVal <= R3_RIGHT_BORDER,
       R4toR2 = previousRowVal == R4_DOWN_BORDER && rowVal == R2_UP_BORDER && previousColVal >= R4_LEFT_BORDER && previousColVal <= R4_RIGHT_BORDER;
  if(R2toR4 || R3toR1 || R4toR2)
    return 0;
  else if(matrix[rowVal][colVal] == DOOR || matrix[rowVal][colVal] == MAX_BYTE - DOOR)
    return 0;
  return 1;
}

bool CheckIfWouldDie(byte rowVal, byte colVal)
{
  bool condition1 = matrix[rowVal][colVal] == DEATH_WALL || matrix[rowVal][colVal] == MAX_BYTE - DEATH_WALL,
       condition2 = matrix[rowVal][colVal] == ENEMY || matrix[rowVal][colVal] == MAX_BYTE - ENEMY;
  if(condition1 || condition2)
    return 1;
  return 0;
}

bool CheckIfWouldBeKey(byte rowVal, byte colVal)
{
  if(matrix[rowVal][colVal] == KEY || matrix[rowVal][colVal] == MAX_BYTE - KEY)
    return 1;
  return 0;
}

bool CheckIfWouldBeCheckpointWall(byte rowVal, byte colVal)
{
  if(matrix[rowVal][colVal] == CHECKPOINT_WALL || matrix[rowVal][colVal] == MAX_BYTE - CHECKPOINT_WALL)
    return 1;
  return 0;
}

short ReadHighscoresAndUpdate(byte deathCounter)
{
  byte nrHighscores = EEPROM.read(0);
  Serial.println("Nr highscores: " + String(nrHighscores));
  byte highscoresRead = 0, k = 1, currHs = 0, 
      highscoreIndexToUpdate = 0;
  bool needsToUpdate = 0;

  int i;
  for(i = 0; i< nrHighscores; i++)
  {
    currHs = EEPROM.read(i*5+1);
    if(deathCounter < currHs)
    {
      highscoreIndexToUpdate = i;
      needsToUpdate = 1;
      break;
    }
  }
  if(i == nrHighscores)
    highscoreIndexToUpdate = nrHighscores;
  
  if(nrHighscores < 5)
  {
    nrHighscores++;
    needsToUpdate = 1;
  }
  
  if(needsToUpdate)
  {
    char oldName[5];
    byte indexOverwriting, indexOverwritten;

    EEPROM.put(0, nrHighscores);
    byte otherHs;
    for(int i = nrHighscores - 1; i > highscoreIndexToUpdate; i--)
    {
      //Serial.println("Overwritten index: " + String(i-1) + "overriding index: " + String(i));

      indexOverwritten = 1 + (i - 1) * 5;
      otherHs = EEPROM.read(indexOverwritten);
      oldName[0] = char(EEPROM.read(indexOverwritten + 1));
      oldName[1] = char(EEPROM.read(indexOverwritten + 2));
      oldName[2] = char(EEPROM.read(indexOverwritten + 3));
      oldName[3] = char(EEPROM.read(indexOverwritten + 4));
      oldName[4] = '\0';
      Serial.println(oldName);
      indexOverwriting = 1 + i * 5;
      EEPROM.put(indexOverwriting, otherHs);
      EEPROM.put(indexOverwriting + 1, oldName[0]);
      EEPROM.put(indexOverwriting + 2, oldName[1]);
      EEPROM.put(indexOverwriting + 3, oldName[2]);
      EEPROM.put(indexOverwriting + 4, oldName[3]);
    }
    EEPROM.put(1 + highscoreIndexToUpdate * 5, deathCounter);
    EEPROM.put(1 + highscoreIndexToUpdate * 5 + 1, playerName[0]);
    EEPROM.put(1 + highscoreIndexToUpdate * 5 + 2, playerName[1]);
    EEPROM.put(1 + highscoreIndexToUpdate * 5 + 3, playerName[2]);
    EEPROM.put(1 + highscoreIndexToUpdate * 5 + 4, playerName[3]);
    Serial.println(playerName);
    needsToUpdate = 0;
    return highscoreIndexToUpdate + 1;
  }
  else return 0;
}

void CheckIfGameEnded()
{
  //ending squares
  bool condition1 = (playerPosCol == ENDING_SQUARE1_COL) && (playerPosRow == ENDING_SQUARE1_ROW);
  bool condition2 = (playerPosCol == ENDING_SQUARE2_COL) && (playerPosRow == ENDING_SQUARE2_ROW);
  if(condition1 || condition2)
  {
    gameState = GAME_STATE_ENDING;
    endingStart = millis();
    currentPosInString = 0;
    rankOnLadder = ReadHighscoresAndUpdate(deathCounterCurr);
  }
  return;
}

byte GetRoomNumber(byte rowVal, byte colVal)
{
    if (rowVal >= R1_UP_BORDER && rowVal <= R1_DOWN_BORDER && colVal >= R1_LEFT_BORDER && colVal <= R1_RIGHT_BORDER) 
        return 1;
    
    else if (rowVal >= R2_UP_BORDER && rowVal <= R2_DOWN_BORDER && colVal >= R2_LEFT_BORDER && colVal <= R2_RIGHT_BORDER) 
        return 2;
    
    else if (rowVal >= R3_UP_BORDER && rowVal <= R3_DOWN_BORDER && colVal >= R3_LEFT_BORDER && colVal <= R3_RIGHT_BORDER) 
        return 3;
    
    else if (rowVal >= R4_UP_BORDER && rowVal <= R4_DOWN_BORDER && colVal >= R4_LEFT_BORDER && colVal <= R4_RIGHT_BORDER) 
        return 4;
    else 
    {
      Serial.println("Something bad happened");
      return 0;
    }
}

void PrintDetailsDuringGametime()
{
  if(needsToUpdateLCD)
  {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.write(byte(SKULL));
    lcd.print(": ");
    lcd.print(String(deathCounterCurr));
    lcd.setCursor(0, 0);
    switch(currentRoom)
    {
      case 1:
        lcd.print(ROOM1_NAME);
        break;
      case 2:
        lcd.print(ROOM2_NAME);
        break;
      case 3:
        lcd.print(ROOM3_NAME);
        break;
      case 4:
        lcd.print(ROOM4_NAME);
        break;
    }
    needsToUpdateLCD = 0;
  }
}

//if player can move to the suggested coordinates, set his coordinates to those. if there are objects there that interact with him in some way, handle it
void HandleImpactOnGameBasedOnMovement(byte rowVal, byte colVal)
{
  if(CheckIfAbleToMoveThere(playerPosRow, playerPosCol, rowVal, colVal))
  { 
    //if the room is different, then surely the player just moved between rooms, no need for further tests
    byte room = GetRoomNumber(rowVal, colVal);

    if(room != currentRoom)
    { 
      currentRoom = room;
      lcd.clear();
      lcd.setCursor(0,0);
      needsToUpdateLCD = 1;
    }

    playerPosCol = colVal;
    playerPosRow = rowVal;
    hasMoved = 1;
    if(CheckIfWouldDie(playerPosRow, playerPosCol))
    {
      //increase death counter
      deathCounterCurr++;
      playerPosCol = lastCheckpointCol; //eset to last checkpoint
      playerPosRow = lastCheckpointRow;
      currentRoom = GetRoomNumber(playerPosRow, playerPosCol);
      needsToUpdateLCD = 1;
    }
    else if(CheckIfWouldBeCheckpointWall(playerPosRow, playerPosCol))
    {
      //set last checkpoint as current position
      lastCheckpointCol = playerPosCol;
      lastCheckpointRow = playerPosRow;
    }
    else if(CheckIfWouldBeKey(playerPosRow, playerPosCol))
    {
      if(playerPosRow == FIRST_KEY_POSITION_ROW && playerPosCol == FIRST_KEY_POSITION_COL)
      {
        matrix[FIRST_DOOR_POSITION_ROW][FIRST_DOOR_POSITION_COL] = 0; 
        matrix[FIRST_KEY_POSITION_ROW][FIRST_KEY_POSITION_COL] = 0;
      }
      else if(playerPosRow == SECOND_KEY_POSITION_ROW && playerPosCol == SECOND_KEY_POSITION_COL)
      {
        matrix[SECOND_DOOR_POSITION_ROW][SECOND_DOOR_POSITION_COL] = 0; 
        matrix[SECOND_KEY_POSITION_ROW][SECOND_KEY_POSITION_COL] = 0;
      }
      else
      Serial.println("Something bad happened Key");
    }
  }
}
extern bool soundsONBoolean;
void setup() 
{
  for(byte i = 0; i < 4; i++)
    playerName[0 + i] = EEPROM.read(5 * NO_OF_HIGHSCORES + 1 + i);
  playerName[4] = '\0';
  
  lcdBrightness = EEPROM.read(5 * NO_OF_HIGHSCORES + 1 + 4);
  matrixBrightness = EEPROM.read(5 * NO_OF_HIGHSCORES + 1 + 4 + 1);
  soundsONBoolean = EEPROM.read(5 * NO_OF_HIGHSCORES + 1 + 4 + 2);

  // the zero refers to the MAX7219 number, it is zero for 1 chip
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.setIntensity(0, matrixBrightness); // sets brightness (0~15 possible values)
  lc.clearDisplay(0);// clear screen

  pinMode(horizontalJoystickPin, INPUT);
  pinMode(verticalJoystickPin, INPUT);

  for (byte row = 0; row < MATRIX_SIZE; row++) 
    for (byte col = 0; col < MATRIX_SIZE; col++) 
      maxMatVal = max(maxMatVal, matrix[row][col]);
  maxMatVal = MAX_BYTE - maxMatVal; // this is used as a maximum value for blinking purposes. If a current matrix value is > maxMatVal, it's set on OFF for blinking purposes. 
                                    // basically works as a threshold that if something is over it, it will be off.
  //LCD setup
  lcd.createChar(LEFT_ARROW, leftArrow);
  lcd.createChar(UP_ARROW, upArrow);
  lcd.createChar(RIGHT_ARROW, rightArrow);
  lcd.createChar(SKULL, skull);

  pinMode(clickJoystickPin, INPUT_PULLUP);
  pinMode(LCDBacklightPin, OUTPUT);
  analogWrite(LCDBacklightPin, lcdBrightness);
  lcd.begin(16, 2);
  lcd.setCursor(0, 1);
  
  Serial.begin(9600);
}

//maybe could be rewritten with direction vectors
void CheckForMoves()
{
  horizontalInput = analogRead(horizontalJoystickPin);
  verticalInput   = analogRead(verticalJoystickPin);

  if (verticalInput > MAX_JOYSTICK_DEFAULT && hasMoved == 0)
  {
    if (horizontalInput > MAX_JOYSTICK_DEFAULT) 
      HandleImpactOnGameBasedOnMovement(playerPosRow - 1, playerPosCol + 1);
      
    else if (horizontalInput < MIN_JOYSTICK_DEFAULT)
      HandleImpactOnGameBasedOnMovement(playerPosRow - 1, playerPosCol - 1);
    
    else
      HandleImpactOnGameBasedOnMovement(playerPosRow - 1, playerPosCol);
    moveStart = millis();
  }
  
  else if (verticalInput < MIN_JOYSTICK_DEFAULT && hasMoved == 0)
  {
    if (horizontalInput > MAX_JOYSTICK_DEFAULT)
      HandleImpactOnGameBasedOnMovement(playerPosRow + 1, playerPosCol + 1);

    else if (horizontalInput < MIN_JOYSTICK_DEFAULT)
      HandleImpactOnGameBasedOnMovement(playerPosRow + 1, playerPosCol - 1);
    
    else
      HandleImpactOnGameBasedOnMovement(playerPosRow + 1, playerPosCol);
    moveStart = millis();
  }

  else if(horizontalInput > MAX_JOYSTICK_DEFAULT && hasMoved == 0)
  {
    if(verticalInput > MAX_JOYSTICK_DEFAULT)
      HandleImpactOnGameBasedOnMovement(playerPosRow - 1, playerPosCol + 1);
    
    else if(verticalInput < MIN_JOYSTICK_DEFAULT)
      HandleImpactOnGameBasedOnMovement(playerPosRow + 1, playerPosCol + 1);
    
    else
      HandleImpactOnGameBasedOnMovement(playerPosRow, playerPosCol + 1);
    moveStart = millis();
  }

  else if(horizontalInput < MIN_JOYSTICK_DEFAULT && hasMoved == 0)
  {
    if(verticalInput > MAX_JOYSTICK_DEFAULT)
      HandleImpactOnGameBasedOnMovement(playerPosRow - 1, playerPosCol - 1);
    
    else if(verticalInput < MIN_JOYSTICK_DEFAULT)
      HandleImpactOnGameBasedOnMovement(playerPosRow + 1, playerPosCol - 1);
    
    else 
      HandleImpactOnGameBasedOnMovement(playerPosRow, playerPosCol - 1);
    moveStart = millis();
  }

  if(millis() - moveStart > moveInterval && hasMoved == 1)
  {
    hasMoved = 0;
    moveStart = millis();
  }
  
}

void ResetEnemyData()
{
  currentEvenCols = 2;
  currentOddCols = 5;
  room1ECD = 0;
  room3EnemiesStart = 0;
  anomalyIndex = 0;
  evenOnesGoingRight = 1;
  oddOnesGoingLeft = 1;
  firstEnemyPosR3Col = 4;
  firstEnemyPosR3Row = 4;
  secondEnemyPosR3Col = 7;
  secondEnemyPosR3Row = 7;
  firstSliderRow = 0;
  secondSliderRow = 7;
  firstSliderDescending = 1;
  secondSliderAscending = 1;
  square1Row = 2;
  square1Col = 14;
  square2Row = 4;
  square2Col = 14; 
  square3Row = 6;
  square3Col = 14;
  counter = 1;
  return;
}

void ResetGameToInitialState() 
{
  byte matrixInitial[MATRIX_SIZE][MATRIX_SIZE] =  {
  {4, 0, 0, 0, 6, 0, 0, 0,  0, 1, 6, 6, 2, 2, 0, 0},
  {1, 0, 2, 2, 0, 1, 1, 0,  0, 1, 0, 0, 1, 1, 0, 0},
  {1, 0, 2, 2, 0, 1, 1, 0,  0, 1, 0, 0, 0, 1, 6, 0},
  {1, 0, 0, 0, 0, 0, 0, 6,  0, 1, 0, 0, 1, 1, 0, 0},
  {1, 0, 0, 0, 6, 0, 0, 0,  0, 1, 0, 0, 0, 1, 6, 0},
  {1, 0, 1, 1, 0, 1, 1, 0,  0, 1, 0, 0, 1, 1, 0, 0},
  {2, 0, 1, 1, 0, 1, 1, 0,  0, 1, 0, 0, 0, 1, 6, 0},
  {0, 0, 0, 0, 0, 0, 0, 6,  0, 3, 6, 6, 1, 1, 2, 2},
  //Room 1                  //Room 2
  {3, 1, 1, 1, 1, 1, 1, 1,  1, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0,  2, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 6, 0, 0, 0, 1, 0,  2, 0, 0, 6, 0, 6, 0, 0},
  {0, 1, 0, 0, 0, 6, 1, 0,  2, 0, 0, 6, 6, 0, 0, 0},
  {0, 1, 6, 0, 0, 0, 1, 0,  2, 0, 6, 6, 6, 0, 0, 4},
  {2, 1, 0, 0, 0, 6, 1, 0,  2, 0, 0, 0, 6, 0, 0, 0},
  {2, 1, 6, 0, 0, 0, 1, 0,  2, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 1,  1, 0, 0, 0, 0, 0, 0, 0}
  };
  playerPosRow = PLAYER_START_ROW;
  playerPosCol = PLAYER_START_COL;
  lastCheckpointCol = PLAYER_START_COL;
  lastCheckpointRow = PLAYER_START_ROW;
  deathCounterCurr = 0;
  currentRoom = 1;
  for(int i = 0; i<MATRIX_SIZE; i++)
    for(int j = 0; j<MATRIX_SIZE; j++)
      matrix[i][j] = matrixInitial[i][j];
  ResetEnemyData();
  return;
}

void GameEndingHandler()
{
  if(rankOnLadder)
  {
    s = "Highscore achieved! Rank: " + String(rankOnLadder);
    s.toCharArray(myCharArray, sizeof(myCharArray));
    PrintStringWithAutoscrollOnLineX(myCharArray, 0);
  }
  else
  {
    s = "Final score: " + String(deathCounterCurr);
    s.toCharArray(myCharArray, sizeof(myCharArray));
    PrintStringWithAutoscrollOnLineX(myCharArray, 0);
  }
  lcd.setCursor(0,1);
  lcd.print("Click to cont.");
  lastClickInput = clickInput;
  clickInput = !digitalRead(clickJoystickPin);
  
  if(Debouncer(clickInput, lastClickInput, lastClickReadTime, hasHandledClick))
  {
    //Serial.println("Clicked");
    ResetGameToInitialState();
    currentPosInString = 0;
    hasDisplayed = 0;
    hasHandledClick = 1;
    gameState = GAME_STATE_MENUS;
    return;
  }
}
void CurrentRoomSetCoordsForDisplay()
{
  switch(currentRoom)
    {
      case 1:
      {
        rowStart = R1_UP_BORDER;
        colStart = R1_LEFT_BORDER;
        break;
      }
      case 2:
      {
        rowStart = R2_UP_BORDER;
        colStart = R2_LEFT_BORDER;
        break;
      }
      case 3:
      {
        rowStart = R3_UP_BORDER;
        colStart = R3_LEFT_BORDER;
        break;
      }
      case 4:
      {
        rowStart = R4_UP_BORDER;
        colStart = R4_LEFT_BORDER;
        break;
      }
      default:
        break;
    }
}

void CheckForEnemyCollisions()
{
  if(matrix[playerPosRow][playerPosCol] == ENEMY)
  {
    playerPosRow = lastCheckpointRow;
    playerPosCol = lastCheckpointCol;
    currentRoom = GetRoomNumber(playerPosRow, playerPosCol);
    deathCounterCurr++;
    needsToUpdateLCD = 1;
  }
}

void PrintGameStateToMatrix()
{
  for (byte row = 0; row < 8; row++) 
  {
    for (byte col = 0; col <  8; col++) 
      if(! (row + rowStart == playerPosRow && col + colStart == playerPosCol) )
        lc.setLed(0, row, col, (matrix[row + rowStart][col + colStart] > 0 && matrix[row + rowStart][col + colStart] < maxMatVal)); 
      else
        lc.setLed(0, playerPosRow - rowStart, playerPosCol - colStart, playerLEDON > 0); 
  }
}

void loop() 
{
  if(gameState == GAME_STATE_PLAYING)
  {
    PrintDetailsDuringGametime();
    CurrentRoomSetCoordsForDisplay();
    EnemyMovementAndRoomEffectsHandler();
    CheckForMoves();
    CheckForEnemyCollisions();
    CheckIfGameEnded();
    PrintGameStateToMatrix();
    BlinkMatrixOnTimers(rowStart, colStart);
  }
  else if (gameState == GAME_STATE_ENDING)
  {
    GameEndingHandler();
  }
  else if (gameState == GAME_STATE_MENUS)
  {
    GlobalMenuHandler();
  }
}
