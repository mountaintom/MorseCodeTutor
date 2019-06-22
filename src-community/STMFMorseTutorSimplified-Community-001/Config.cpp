#ifndef BEENHERE
#include "MorseTutor.h"
#endif


/*****
  Purpose: Sets default values for the system

  Parameter list:
    void

  Return value:
    void
*****/
void SetDefaults()
{
  if (wordsPerMinute == 0) {                  // In case it's the first time here
    wordsPerMinute = 15;
  }
  SetDitLength(wordsPerMinute);
  sidetoneFrequency = FREQUENCY;              // Def 700Hz
  encodingType      = FARNSWORTHENCODING;
  targetWPM         = GOALSPEED;              // Def 35WPM
  farnsworthDelay = 1200 / FARNSWORTHSTART;   // Def 5WPM
  displayState      = 1;
}

/*****
  Purpose: To change the code speed of the keyer

  Parameter list:
    void

  Return value:
    void
*****/
void SetKeyerSpeed()
{
  char temp[4];
  int8_t result;
  int i;
  static int val = 0;

  if (wordsPerMinute == 0) {
    wordsPerMinute = 15;
  }

  myMenu.eraseMenus(tft);
  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(70, 72);
  tft.print("Current WPM");

  tft.setTextColor(WHITE, BLACK);
  tft.setCursor(120, 110);
  tft.print(wordsPerMinute);

  while (digitalRead(ENCODERSWITCH1) != LOW) {
    result = ReadEncoder();                                   //Hacky stuff again...
    if (result == 0) {
      continue;
    } else {
      if (result == 1)
        val++;                      // Clockwise          (CW)
      else
        val--;                      // Counter-clockwise  (CCW)

      if (val == 2 || val == -2) {  // It takes 2 pulses to make a rotation
        if (val > 0) {              // CW??
          //         if (result == 1) {
          dir = 1;
        } else {                    // Nope, CCW
          dir = -1;
        }
        wordsPerMinute  += dir;                        // Works because encorderDirection can be +/-
        SetDitLength(wordsPerMinute);

        Serial.print("wpm = ");
        Serial.println(wordsPerMinute);

        if (wordsPerMinute > WPMMAX) {                              // Check max wpm
          wordsPerMinute = WPMMAX;
        }
        if (wordsPerMinute < WPMMIN) {                              // Check min wpm
          wordsPerMinute = WPMMIN;
        }
        tft.setCursor(120, 110);
        tft.println(wordsPerMinute);                                // Show wpm

        dir = 0;
        val = 0;
      }
    }
  }
  i = EEPROM.update((uint16) WORDSPERMINUTE, wordsPerMinute);
  if (i != EEPROM_SAME_VALUE || i != FLASH_COMPLETE) {              // Something went wrong...
    Serial.print("EEPROM wpm write sucks, Status = ");
    Serial.println(i);
  }
  SetDitLength(wordsPerMinute);                                     // Activate new speed
  tutorMode == SENDSTATE;
  SendCode((char) 0b11010);       // 'C'
  SendCode((char) 0b11101);       // 'Q'
  tutorMode == RECEIVESTATE;
}

/*****
  Purpose: To change the sidetone

  Parameter list:
    void

  Return value:
    void
*****/

void SetNewSidetone()
{
  char temp[4];
  int8_t result;
  int frequency = FREQUENCY;
  static int val = 0;

  myMenu.eraseMenus(tft);
  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(60, 100);
  tft.print("Sidetone Frequency");

  tft.setTextColor(WHITE, BLACK);
  tft.setCursor(120, 130);
  tft.print(frequency);

  tone(SPEAKERPIN, frequency); // Send 1KHz sound signal...

  while (digitalRead(ENCODERSWITCH1) != LOW) {
    result = ReadEncoder();
    if (result == 0) {
      continue;
    } else {
      if (result == 1)
        val++;                      // Clockwise          (CW)
      else
        val--;                      // Counter-clockwise  (CCW)

      if (val == 2 || val == -2) {  // It takes 2 pulses to make a rotation
        if (val > 0) {              // CW??
          //         if (result == 1) {
          dir = 1;
        } else {                    // Nope, CCW
          dir = -1;
        }
        frequency += dir * FREQUENCYINCREMENT;      // Bump the frequncy
        if (frequency > MAXFREQ) {                  // Check max frequncy
          frequency = MAXFREQ;
        }
        if (frequency < MINFREQ) {                  // Check min frequncy
          frequency = MINFREQ;
        }

        tone(SPEAKERPIN, frequency);                // Send 1KHz sound signal...
        Serial.print("frequency = ");
        Serial.println(frequency);

        //    tft.setTextColor(WHITE, BLACK);
        // itoa(wordsPerMinute, temp, DEC);
        tft.setCursor(120, 130);
        tft.println(frequency);                      // Show frequncy
        dir = 0;
        val = 0;
      }
    }
  }
  sidetoneFrequency = frequency;
  noTone(SPEAKERPIN);                                // Shut up!
}

/*****
  Purpose: To set whether using standard or Farnsworth encoding
  Parameter list:
    void

  Return value:
    void
*****/
int SetEncodingType()
{
  int firstRow = TOPDEADSPACE + 20;
  int direction = 0;
  static int passCalls = 0;
  static int flag = 1;

  myMenu.eraseMenus(tft);
  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(60, firstRow);
  tft.print("Encoding Type");

  tft.setTextColor(BLACK, WHITE);
  tft.setCursor(10, firstRow + 30);
  tft.print(" Farnsworth");
  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(150, firstRow + 30);
  tft.print(" Standard");

  while (digitalRead(ENCODERSWITCH1) != LOW) {
    direction = ProcessEncoder();
    if (direction == 0) {
      continue;
    } else {
      flag += direction;
      if (flag % 2 == 1) {
        tft.setTextColor(BLACK, WHITE);
        tft.setCursor(10, firstRow + 30);
        tft.print(" Farnsworth");
        tft.setTextColor(GREEN, BLACK);
        tft.setCursor(150, firstRow + 30);
        tft.print(" Standard");
      } else {
        tft.setTextColor(GREEN, BLACK);
        tft.setCursor(10, firstRow + 30);
        tft.print(" Farnsworth");
        tft.setTextColor(BLACK, WHITE);
        tft.setCursor(150, firstRow + 30);
        tft.print(" Standard");
      }
    }
  }
  return (flag % 2);
}

/*****
  Purpose: This sets the ultimate WPM speed the user hopes to reach

  Parameter list:
    void

  Return value:
    int       the desired speed
*****/
int SetTargetWPM()
{
  int firstRow = TOPDEADSPACE + 85;
  int wpm;
  int direction = 0;
  static int passCalls = 0;
  static int flag = 1;
  int8_t result;
  int localDir;
  static int val = 0;

  if (wordsPerMinute == 0) {
    wpm = 25;
  } else {
    wpm = wordsPerMinute;
  }

  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(90, firstRow);
  tft.print("Target WPM");

  tft.setTextColor(WHITE, BLACK);
  tft.setCursor(150, firstRow + 30);
  tft.print(wpm);

  while (digitalRead(ENCODERSWITCH1) != LOW) {
    result = ProcessEncoder();

    if (result) {
      if (result == 1)
        val++;                      // Clockwise          (CW)
      else
        val--;                      // Counter-clockwise  (CCW)
      wpm += val;
      if (wpm > WPMMAX) {           // Check max wpm
        wpm = WPMMAX;
      }
      if (wpm < WPMMIN) {           // Check min wpm
        wpm = WPMMIN;
      }
      tft.setCursor(150, firstRow + 30);
      tft.println(wpm);             // Show target speed
      MyDelay(200UL);
      val = 0;
    }
  }
  return wpm;     // New target speed
}

/*****
  Purpose: The Farnsworth method for learning sends letters at the speed you hope to ultimately reach (e.g., targetWPM),
          but instead of a letter space at the target speed, the Farnsworth method has the character spacing at a much
          lower speed. For example, you might hope to get to 35wpm, but set the Farnsworth (spacing) speed at 5wpm. The
          advantage is that you listen for rhythms rather than dits and dahs.

  Parameter list:
    void

  Return value:
    int       the desired speed
*****/
int SetFarnsworthWPM()
{
  int firstRow = TOPDEADSPACE + 155;
  int wpm;
  int8_t result;
  static int val = 0;

  wpm = WPMMIN;                     // Start really slow
  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(68, firstRow);
  tft.print("Farnsworth WPM");

  tft.setTextColor(WHITE, BLACK);
  tft.setCursor(150, firstRow + 30);
  tft.print(wpm);
  while (digitalRead(ENCODERSWITCH1) != LOW) {
    result = ProcessEncoder();

    if (result) {
      if (result == 1)
        val++;                        // Clockwise          (CW)
      else
        val--;                        // Counter-clockwise  (CCW)
      wpm += val;
      if (wpm > targetWPM) {          // Check max wpm
        wpm = targetWPM;
      }
      if (wpm < WPMMIN) {             // Check min wpm
        wpm = WPMMIN;
      }
      tft.setCursor(150, firstRow + 30);
      tft.println(wpm);               // Show target speed
      MyDelay(200UL);
      val = 0;
    }
  }
  return wpm;                         // New farnsworth speed
}


/*****
  Purpose: This function lets the user change the paddle that sends a string of dits

  Parameter list:
    void

  Return value:
    int       the desired choice
*****/
int SetDitPaddle()
{
  int firstRow = TOPDEADSPACE + 20;
  int direction = 0;
  static int passCalls = 0;
  static int flag = 1;

  myMenu.eraseMenus(tft);
  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(60, firstRow);
  tft.print("Dit Paddle");

  tft.setTextColor(BLACK, WHITE);
  tft.setCursor(10, firstRow + 30);
  tft.print(" Left");
  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(150, firstRow + 30);
  tft.print(" Right");

  while (digitalRead(ENCODERSWITCH1) != LOW) {
    direction = ProcessEncoder();
    if (direction == 0) {
      continue;
    } else {
      flag += direction;
      if (flag % 2 == 1) {
        tft.setTextColor(BLACK, WHITE);
        tft.setCursor(10, firstRow + 30);
        tft.print(" Left");
        tft.setTextColor(GREEN, BLACK);
        tft.setCursor(150, firstRow + 30);
        tft.print(" Right");
      } else {
        tft.setTextColor(GREEN, BLACK);
        tft.setCursor(10, firstRow + 30);
        tft.print(" Left");
        tft.setTextColor(BLACK, WHITE);
        tft.setCursor(150, firstRow + 30);
        tft.print(" Right");
      }
    }
  }
  ditPaddle = flag % 2;

  return (flag % 2);
}
