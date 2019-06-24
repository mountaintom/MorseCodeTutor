/*
  This code is placed in Open Source according to the MIT license agreement. The entirity of this comment must be left in this file.

To do:
  SD card support
  QSO words
  Panic escape from some menu options via the encoder key.
  Improve feedback from all of the send options...need to think about this.
  Reduce the number of globals
  Change encoder from polling to interrupt (fast enough probably not necessary?)
  If polling remains, yank out and replace with function call
  
  April 20, 2019  Jack Purdum, W8TEE. Testing in case, modified menu structure, added flashcard
  March 23, 2019  Jack Purdum, W8TEE. Move from breadboard to protoboard
  March 21, 2019  Jack Purdum, W8TEE. Started work on Flashcard section
  March 15, 2019  Jack Purdum, W8TEE. Started work on CopyCat section
  March 14, 2019  Jack Purdum, W8TEE. Started work on Send sections
  March 11, 2019  Jack Purdum, W8TEE. Started work on Config sections, added EEPROM routines
  February 20, 2019  Jack Purdum, W8TEE. Continued work on Receive sections of tutor
  February 10, 2019  Jack Purdum, W8TEE. Began moving code to STM32F103 processor
  February 3,  2019  Jack Purdum, W8TEE. Began moving code to TFT color display and implementing the menuing system
  February 2,  2019  Jack Purdum, W8TEE. Modified the Tutor talk from Doug Hendrick's QRPp article

Modifications/Enhancments:
  ========
  Setup TFT module specifying all pins explicitly. Added pin for TFT display reset.
  20190620 WB6B
  ========
*/

#ifndef BEENHERE
#include "MorseTutor.h"
#endif

#define NUMBEROFMENULISTS  3     // Refers to the arrays below (e.g., Receive, Send, Config

char *menuLevel1[]  = {" Receive ", "  Send   ", "  Config "};        // Top level Menu

                                                                      // Submenus for top menu
char *menuReceive[] = {" Letters ", " Numbers ", " Punc    ", " Words   ", " Let-Num ", " Call Sign", " QSO     ", " Exit    "};
char *menuSend[]    = {" By two's", " Mix     ", " CopyCat ", "Flashcard", " Exit    "};
char *menuConfig[]  = {" Speed   ", " Encoding",  " Tone    "," Dit Pad "," Save    ", " Exit    "};
int defaultsList[]  = {3, 2, 0};         // Says: default is 3 (words) for menuReceive, 1 (Mix) for menuSend, 0 (Speed)-+ for menuConfig

// From The Art & Skill of Radiotelegraphy, Chapter 22
char hundredCommonWords[] = "a an the this these that some all any every who which what such other I me my we us our you your he him his she her it its they them their man men people time work well May will can one two great little first at by on upon over before to from with in into out for of about up when then now how so like as well very only no not more there than and or if but be am is are was were been has have had may can could will would shall should must say said like go come do made work";

char msg[MAXMSGLENGTH + 1];                                   // Don't forget the NULL
char spaces[SCREENWIDTH + 1];
char screen[SCREENWIDTH + 1];
char QSO[120];
char fake[10];

char singleCallSignPrefixes[] = {'K', 'N', 'W'};
char *doubleCallSignPrefixes[] = {"AL", "KN", "NZ", "WZ"};
char *states[] = {"AL", "AK", "AZ", "AR", "CA", "CO", "CT", "DE", "FL", "GA", "HI", "ID", "IL", "IN", "IA", "KS", "KY", "LA", "ME", "MD",
                  "MA", "MI", "MN", "MS", "MO", "MT", "NE", "NV", "NH", "NJ", "NM", "NY", "NC", "ND", "OH", "OK", "OR", "PA", "RI", "SC",
                  "SD", "TN", "TX", "UT", "VT", "VA", "WA", "WV", "WI", "WY", "GU", "PR", "VI"
                 };
char *sectionList[] = {"CT", "EMA", "ME", "NH", "RI", "VT", "WMA", "ENY", "NLI", "NNJ", "NNY", "SNJ", "WNY", "DE", "EPA", "MDC", "WPA", "AL",
                       "GA", "KY", "NC", "NFL", "SC", "EB", "LAX", "ORG", "SB", "SCV", "SDG", "SF", "SJV", "SV", "PAC", "AZ", "EWA", "ID", "MT", "NV",
                       "OR", "UT", "WWA", "WY", "AK", "MI", "OH", "WV", "SFL", "WCF", "TN", "VA", "PR", "VI", "AR", "LA", "MS", "NM", "NTX", "OK", "STX",
                       "WTX", "IL", "IN", "WI", "CO", "IA", "KS", "MN", "MO", "NE", "ND", "SD", "MAR", "NL", "QC", "ONN", "ONS", "ONE", "GTA", "MB",
                       "SK", "AB", "BC", "NT, DX"
                      };
char ssClasses[] = {'Q', 'A', 'B', 'U', 'M', 'S'};
char *words[] = {"DE", "TNX FER", "BT", "WX", "HR", "TEMP", "ES", "RIG", "ANT", "DIPOLE", "VERTICAL", // 0-10
                 "BEAM", "HW", "CPI", "WARM", "SUNNY", "CLOUDY", "COLD", "RAIN", "SNOW", "FOG",       // 11-20
                 "RPT", "NAME", "QTH", "CALL", "UR", "SLD", "FB", "RST"                               // 21-28
                };
char *names[] = {"FRED", "JOHN", "TERRY", "JANE", "SUE", "LEON", "RUMPLESTILLSKIN", "WM WALLACE", "ZEKE", "JOSH", "JILL", "LYNN"};
char *cities[] = {"MEDINA, OH", "BILLINGS, MT", "SF, CA", "WALLA WALLA, WA", "VERO BEACH, FL", "NASHVILLE, TN", "NYC", "CHICAGO", "LA, CA", // 0-8
                  "POSSUM TROT, MS", "ASPEN, CO", "AUSTIN, TX", "RALIEGH, NC"};

#define ICOMOFFSET        0             // Where in the array below do the mfg's numbers begin
#define KENWOODOFFSET     9
#define YAESUOFFSET      12
#define FLEXOFFSET       17

char *rigNumbers[] = {"7851", "7700", "7600", "7410", "7300", "7100", "718", "78", "9100",  // ICOM
                      "990", "480", "2000",                                                 // Kenwood
                      "991", "891", "1200", "3000", "2000",                                 // Yaesu
                      "1500", "6300", "6400", "6500", "6700"                                // Flex
                      };                                                                    // Elecraft is 1-3 with letters?
                      
char myReverseBinarySearchSet[] = "##TEMNAIOGKDWRUS##QZYCXBJP#L#FVH09#8###7#####/-61#######2###3#45"; // BInary search string

char letterTable[] = {                 // Morse coding: dit = 0, dah = 1
  0b101,              // A                first 1 is the sentinel marker
  0b11000,            // B
  0b11010,            // C
  0b1100,             // D
  0b10,               // E
  0b10010,            // F
  0b1110,             // G
  0b10000,            // H
  0b100,              // I
  0b10111,            // J
  0b1101,             // K
  0b10100,            // L
  0b111,              // M
  0b110,              // N
  0b1111,             // O
  0b10110,            // P
  0b11101,            // Q
  0b1010,             // R
  0b1000,             // S
  0b11,               // T
  0b1001,             // U
  0b10001,            // V
  0b1011,             // W
  0b11001,            // X
  0b11011,            // Y
  0b11100             // Z
};

char numberTable[] = {
  0b111111,           // 0
  0b101111,           // 1
  0b100111,           // 2
  0b100011,           // 3
  0b100001,           // 4
  0b100000,           // 5
  0b110000,           // 6
  0b111000,           // 7
  0b111100,           // 8
  0b111110            // 9
};

char punctuationTable[] = {
  0b01101011,         // exclamation mark 33
  0b01010010,         // double quote 34
  0b10001001,         // dollar sign 36
  0b00101000,         // ampersand 38
  0b01011110,         // apostrophe 39
  0b01011110,         // parentheses (L) 40, 41
  0b01110011,         // comma 44
  0b00100001,         // hyphen 45
  0b01010101,         // period  46
  0b00110010,         // slash 47
  0b01111000,         // colon 58             
  0b01101010,         // semi-colon 59
  0b01001100,         // question mark 63
  0b01001101,         // underline 95
  0b01101000,         // paragraph
  };
int ASCIIForPunctuation[] = {33, 34, 36, 39, 41, 44, 45, 46, 47, 58, 59, 63, 95};  // Indexes into code

byte WPM;
byte displayState;

int activeMenuIndex;               // Which menu item is currently selected
int activeMenuLevel;               // Which menu level is being used
int activeColumn;
int activeRow;
int aVal;
int col, row;

int ditlen;
int dir;
uint16 ditPaddle;
int elements;                       // Holds menu elements
uint16 encodingType;                   // Normal or Farnsowrth
uint16 farnsworthWPM;
int farnsworthDelay;
uint16 kochSpeed;
int menuState;                  // Which menu is active
uint16 sidetoneFrequency;        // Default
int spacing;
int targetWPM;
int tutorMode = RECEIVESTATE;
uint16 wordsPerMinute;

volatile int rotationDirection;     // + is CW, - is CCW

// Note: The tft display pins are defined in MorseTutor.h
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// As an alternative you may use this TFT setup that lets you specify all the pins. However the TFT library will
// switch to bit banging the SPI data and the display will be slower.
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);


//Adafruit_MCP4725 dac;
Rotary myEncoder = Rotary(ENCODER1PINA, ENCODER1PINB);            // sets the pins the rotary encoder uses.  Must be interrupt pins.
Menuing myMenu(DISPLAYWIDTH, DISPLAYHEIGHT, GREEN, BLACK, BLUE, WHITE, 1);

/*****
  Purpose: to cause a delay in program execution

  Paramter list:
  unsigned long millisWait    // the number of millseconds to wait

  Return value:
  void
*****/
void mydelay(unsigned long millisWait)
{
  unsigned long now = millis();

  while (millis() - now < millisWait)
    ;     // Twiddle thumbs...
}


/*****
  Purpose: To generate a random one or two letter call sign for the US

  Parameter list:
    char call[]     array to hold the resulting call

  Return value:
    void

  CAUTION: assumes call[] is large enough for up to 7 chars
*****/
void GenerateDomesticCall(char call[])
{
  char temp[3];
  int index = 0;
  int i;
  long randomValue;

  randomSeed(millis());
  randomValue = random(0, 2);   // Single letter or two letter prefix?

  switch (randomValue) {
    case ONELETTERPREFIX:
      randomValue = random(0, 3);     // Generate 0-2 inclusively for first letter
      call[index++] = singleCallSignPrefixes[randomValue];
      randomValue = random(0, 10);     // Generate 0-9 inclusively
      call[index++] = (char) randomValue + '0';
      randomValue = random(2, 4);     // Number of characters after call area
      for (int i = randomValue; i != 0; i--) {
        randomValue = random(0, 26);  // Alpha character
        call[index++] = 'A' + (char) randomValue;
      }
      call[index] = '\0';
      break;

    case TWOLETTERPREFIX:
      randomValue = random(0, 4);     // Generate 0-2 inclusively for first letter
      strcpy(temp, doubleCallSignPrefixes[randomValue]);      // Copy the double prefix
      randomValue = random((int) temp[0], (int) temp[1] + 1); // Set new limit to second char
      strcpy(call, temp);
      call[1] = (char) randomValue;   // The in-range second character
      randomValue = random(0, 10);    // Call area
      call[2] = (char) (randomValue + '0');
      index = (int) random(1, 3);     // One or two letters?
      for (i = 0; i < index; i++) {
        call[3 + i] = (char) random(LETTERA, LETTERZ + 1);
      }
      call[3 + i] = '\0';             // Make it a string
      break;

    default:
      break;
  }

}




/*****
  Purpose: to cause a delay in program execution

  Paramter list:
  unsigned long millisWait    // the number of millseconds to wait

  Return value:
  void
*****/
void MyDelay(unsigned long millisWait)
{
  unsigned long now = millis();

  while (millis() - now < millisWait)
    ;     // Twiddle thumbs...
}


/*****
  Purpose: To simulate a Field Day exchange in the format:

           <transmitters><class A-F><section list>

  Parameter list:
    void

  Return value:
    void
*****/
void FieldDayExcahange()
{
  byte rand, i, pass = 1;
  char exchange[15];
  char temp[5];
  int transmitters;

  rand = (byte)random(1, 10) + '0';           // Number of transmitters
  temp[0] = rand;;                            // Make it a string
  temp[1] = '\0';
  strcpy(exchange, temp);
  rand = random(0, 6) + 'A';                  // Station class, A - F
  temp[0] = rand;                             // Make it a string
  temp[1] = '\0';
  strcat(exchange, temp);
  strcat(exchange, " ");
  rand = random(0, ELEMENTS(sectionList));  // Section
  strcat(exchange, sectionList[rand]);
  strcat(exchange, " ");
  tft.setCursor(0, 1);
  ShowString(exchange);
  ShowString(exchange);
  ShowString("BK");
  tft.setCursor(0, 0);
  tft.print(spaces);
}



/*****
  Purpose: Build a string for a hypothetical rig

  Parameter list:
    char s[]        array of chars to hold rig string

  Return value:
    void
*****/
void CreateRig(char s[])
{
  byte rand;
  int number;
  char temp[6];

  rand = (byte) random(0, 6);

  switch (rand) {
    case 0:               // ICOM
      strcat(s, "ICOM ");
      rand = (byte) random(0, KENWOODOFFSET);             // Which model
      number = pgm_read_word_near(rigNumbers + rand);
      itoa(number, temp, 10);
      strcat(s, temp);
      break;

    case 1:               // Kenwood
      strcat(s, "KNWD ");
      rand = (byte) random(KENWOODOFFSET, YAESUOFFSET);
      number = pgm_read_word_near(rigNumbers + rand);
      itoa(number, temp, 10);
      strcat(s, temp);
      break;

    case 2:               // Yaesu
      strcat(s, "YAESU ");
      rand = (byte) random(YAESUOFFSET, FLEXOFFSET);
      number = pgm_read_word_near(rigNumbers + rand);
      itoa(number, temp, 10);
      strcat(s, temp);
      break;

    case 3:               // Flex
      strcat(s, "FLEX ");
      rand = (byte) random(FLEXOFFSET, ELEMENTS(rigNumbers));
      number = pgm_read_word_near(rigNumbers + rand);
      itoa(number, temp, 10);
      strcat(s, temp);
      break;

    case 4:               // Elecraft
      strcat(s, "ELCFT KX");
      rand = (byte) random(1, 4);     // Which model
      itoa(rand, temp, 10);
      strcat(s, temp);
      break;

    default:               // HB
      strcat(s, "HB ");
      break;
  }
}

/*****
  Purpose: Sign-on screen

  Paramter list:
    void

  Return value:
    void
*****/
void Splash()
{
  int row, col;
  tft.fillScreen(BLACK);

  row = 240 / 5;
  col = 320 / 4;
  tft.setTextSize(3);         // 18 characters per line with this size
  tft.setTextColor(MAGENTA, BLACK);
  col = 320 / 2;
  tft.setCursor(0, 25);
  tft.print(" Morse Code Tutor");
  
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(2);
  tft.setCursor(103, 60);
  tft.print("Version: ");
  tft.print(VERSION);
  
  tft.setCursor(88, 83);
  tft.print(SUBVERSION);
  
  tft.setCursor(145, 108);
  tft.print("by");

  tft.setTextSize(2);       // 26 characters per line
  tft.setCursor(0, 0);
  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(0, row + 100);
  tft.print("    Jack Purdum, W8TEE");
  tft.setCursor(0, row + 150);
  tft.print("      Al Peter, AC8GY");

  MyDelay(1500);
}

//=================================================================
void setup()
{
  char c;
  char temp[10];
  int items;

  Serial.begin(115200);

  pinMode(ENCODER1PINA,     INPUT_PULLUP);
  pinMode(ENCODER1PINB,     INPUT_PULLUP);
  pinMode(ENCODERSWITCH1,   INPUT_PULLUP);
  

  pinMode(DASHPADDLE,       INPUT_PULLUP);
  pinMode(DITPADDLE,        INPUT_PULLUP);
  
  digitalWrite(ENCODERSWITCH1,  HIGH);
  digitalWrite(DASHPADDLE,      HIGH);
  digitalWrite(DITPADDLE,       HIGH);

  pinMode(SPEAKERPIN, OUTPUT);


  // Do a tft display reset outside of Adafruit library.
  // This allows the display reset to be connected to a
  // 10k resistor or the hardware reset (pin PA3) without
  // changing the TFT setup (constructor) type.
  pinMode(TFT_RST, INPUT_PULLUP);
  digitalWrite(TFT_RST, LOW);
  delay(20);
  digitalWrite(TFT_RST, HIGH);
  delay(500);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  memset(spaces, ' ', sizeof(spaces));        // Fill array with space character

  Splash();

  DefineEEPROMPage();                         // Setup EEPROM pages
  ReadConfigData();                           // Read default data
  
#ifdef DEBUG  
  ShowConfigData();                           // Show what you read
#endif  

  SetDitLength(wordsPerMinute);

  myMenu.itemCount = ELEMENTS(menuLevel1);
  myMenu.spacing = myMenu.width / myMenu.itemCount;
  myMenu.eraseDisplay(tft);
  myMenu.showMenu(tft, menuLevel1);
  tft.drawLine(0, TOPDEADSPACE - 4, myMenu.width, TOPDEADSPACE - 4, YELLOW);
}

void loop()
{
  char c;
  char fake[10];
  char bogusCall[10];  
  int charsRead;
  int index;
  int pass;
  int selectedMenuOption;
  int statusFlag;
  
  long myRand;
  static int switchState = HIGH;
  int8_t encoder = 0;
  static int val = 0;

  if (digitalRead(DITPADDLE) == LOW) {
    dit();
  }
  if (digitalRead(DASHPADDLE) == LOW) {
    dah();
  }

  dir = ReadEncoder();            // Polling, not interrupts
 
  if (dir) {                                                      // All of this is a hack and pretty ugly...
    if (dir == 1)
      val++;                      // Clockwise          (CW)
    else
      val--;                      // Counter-clockwise  (CCW)
    if (val == 2 || val == -2) {  // It takes 2 pulses to make a rotation
      if (val > 0) {              // CW??
        rotationDirection = 1;
      } else {                    // Nope, CCW
        rotationDirection = -1;
      }
      if (activeMenuLevel == 0) {                                     // top-most menu, horizontal scroll
        myMenu.deselectMenuItem(tft, menuLevel1);
        myMenu.itemCount = ELEMENTS(menuLevel1);
        myMenu.scrollTopMenu(tft, menuLevel1, rotationDirection, defaultsList);
      } else {
        switch (activeMenuIndex) {
          case 0:                                                     // This is the first submenu, e.g.,  *menuReceive[]
            myMenu.deselectMenuItem(tft, menuReceive);
            myMenu.itemCount = ELEMENTS(menuReceive);
            myMenu.updateMenu(tft, menuReceive, rotationDirection);
            break;

          case 1:                                                     // This is the second submenu, e.g.,  *menuSend[]
            myMenu.deselectMenuItem(tft, menuSend);
            myMenu.itemCount = ELEMENTS(menuSend);
            myMenu.updateMenu(tft, menuSend, rotationDirection);
            break;

          case 2:                                                     // This is the third submenu, e.g.,  *menuConfig[]
            myMenu.deselectMenuItem(tft, menuConfig);
            myMenu.itemCount = ELEMENTS(menuConfig);
            myMenu.updateMenu(tft, menuConfig, rotationDirection);
            break;
          default:                                                    // Something went south...
            Serial.print("I shouldn't be here: activeMenuLevel = ");
            Serial.println(activeMenuLevel);
            break;
        }   // end switch
      }   // end else
      val = 0;
    }
  }
  if (digitalRead(ENCODERSWITCH1) == LOW) {
    myMenu.eraseMenus(tft);
    myMenu.deselectMenuItem(tft, menuLevel1);
    activeMenuLevel++;
    if (activeMenuLevel > NUMBEROFMENULISTS) {
      activeMenuLevel = 0;
    }
    tutorMode = activeMenuIndex + 1;

    switch (activeMenuIndex) {                                                        // Only applies to submenus
      case 0:                                                                         // This is the first submenu, e.g.,  *menuSend[]
        myMenu.itemCount = ELEMENTS(menuReceive);
        myMenu.showNewMenu(tft, menuReceive, rotationDirection, defaultsList);
        selectedMenuOption = myMenu.selectFromMenu(tft, menuReceive) + FIRSTOFFSET;
        break;
        
      case 1:                                                                         // This is the second submenu
        myMenu.itemCount = ELEMENTS(menuSend);
        myMenu.showNewMenu(tft, menuSend, rotationDirection, defaultsList);
        selectedMenuOption = myMenu.selectFromMenu(tft, menuSend) + SECONDOFFSET;
        break;
        
      case 2:                                                                         // This is the third submenu
        myMenu.itemCount = ELEMENTS(menuConfig);
        myMenu.showNewMenu(tft, menuConfig, rotationDirection, defaultsList);
        selectedMenuOption = myMenu.selectFromMenu(tft, menuConfig) + THIRDOFFSET;
        break;
        
      default:
        Serial.print("Shouldn't be here. activeMenuIndex = ");
        Serial.println(activeMenuIndex);
        break;
    }   
    MyDelay(500UL);                                                     // A hacky debounce...
#ifdef DEBUG
    Serial.print("Bottom Encoder switch: activeMenuLevel = ");
    Serial.print(activeMenuLevel);
    Serial.print("  activeMenuIndex = ");
    Serial.println(activeMenuIndex);
    Serial.print("   selectedMenuOption = ");
    Serial.print(selectedMenuOption);
    Serial.print("   activeColumn = ");
    Serial.print(activeColumn);
    Serial.print("   activeRow = ");
    Serial.println(activeRow);
#endif
    // ============================== Receive ====================================

    switch (selectedMenuOption) {
      case 0:                                 // Receive - Letters
        tft.setCursor(0, TOPDEADSPACE);
        SendLetters(myMenu);
        Erase();

        break;

      case 1:                                 // Receive - Numbers
        tft.setCursor(0, TOPDEADSPACE);
        SendNumbers(myMenu);
        Erase();
        break;

      case 2:                                 // Receive - Numbers
        tft.setCursor(0, TOPDEADSPACE);
        SendPunctuation(myMenu);
        Erase();
        break;


      case 3:                                 // Receive - Words
        tft.setCursor(0, TOPDEADSPACE);
        srand((unsigned int)millis());  // Seed random number generator
        SendWords(myMenu);
        break;

      case 4:                                   // Receive - Numbers and letters
        tft.setCursor(0, TOPDEADSPACE);
        srand((unsigned int)millis());  // Seed random number generator
        SendLettersAndNumbers(myMenu);
        break;

      case 5:                                   // Receive - Call sign
        tft.setCursor(0, TOPDEADSPACE);
        DoCallSigns(myMenu);
        break;

      case 6:                                   // Receive - QSO
        tft.setCursor(0, TOPDEADSPACE);
        DoQSO(myMenu);
        break;

      case 8:
        break;
      // ============================== Send ====================================

      case 100:         // By two's
      case 101:         // Mix
        Listen(myMenu);
      case 102:         // CopyCat
        DoCopyCat(myMenu);
        break;
      case 103:         // Flash card
        DoFlashCard(myMenu);
        break;
        
      case 104:         // exit
        break;


      // ============================== Config ==================================
 
      case 200:                                                               // WPM Speed
        SetKeyerSpeed();
        statusFlag = WriteEEPROMValue(WORDSPERMINUTE, wordsPerMinute);
        break;

      case 201:                                                               // Encoding type
        encodingType = SetEncodingType();
        if (encodingType == FARNSWORTHENCODING) {                             // Farnsworth = 1
          MyDelay(200UL);
          targetWPM     = SetTargetWPM();                                     // The speed they want to reach
          MyDelay(200UL);
          farnsworthWPM = SetFarnsworthWPM();                                 // The speed slow enough to eventually get there
          farnsworthDelay = 1200 / farnsworthWPM;
        }
        statusFlag = WriteEEPROMValue(WORDSPERMINUTE, wordsPerMinute);
        statusFlag = WriteEEPROMValue(KOCHSPEED,      targetWPM);
        statusFlag = WriteEEPROMValue(FARSWORTHSPEED, farnsworthWPM);
        statusFlag = WriteEEPROMValue(ENCODINGACTIVE, encodingType);       
        break;

      case 202:                                                                 // Tone
        SetNewSidetone();
        statusFlag = WriteEEPROMValue(SIDETONE,       sidetoneFrequency);
        break;

      case 203:                                                                 // ditpaddle
        SetDitPaddle();
        statusFlag = WriteEEPROMValue(WHICHISDITPADDLE,       ditPaddle);
        break;

      case 204:
        SaveConfigData();
        break;
        
      default:                                                                  // Error
        break;
    }
    activeMenuLevel = activeMenuIndex = 0;
    myMenu.itemCount = ELEMENTS(menuLevel1);
    myMenu.spacing = myMenu.width / myMenu.itemCount;
    myMenu.eraseMenus(tft);
    myMenu.showMenu(tft, menuLevel1);
    selectedMenuOption = 0;
  }
}
