#ifndef BEENHERE
#include "MorseTutor.h"
#endif 

/*
  Display wiring:
    Display       STM32f103
   SDO/MISO         A6
      LED           Vcc (3.3V)
      SCK           A5
   SDI/MOSI         A7
    DC/RS           A0
    RESET           Vcc via a 10K resistor
      CS            A1
     GND            GND
     Vcc            3.3V
*/

// text size 3 gives 17 letters per line
// text size 2       26

/*****
   This erases the LCD screen

   Parameter List:
      void

   Return value:
      void

   CAUTION: Assumes a spaces[] array is defined
 *****/
void Erase()
{
  tft.setCursor(0, 0);
  tft.print(spaces);
  tft.setCursor(0, 1);
  tft.print(spaces);
}

/*****
   This routine displays a menu item from the current menu selection

   Parameter List:
      void

   Return value:
      void

   CAUTION: This function assume activeMenuIndex and menuState are set
            prior to the call.
 *****/
void ShowCurrentSelection(char *m[], int which)
{
  Erase();
  tft.setCursor(0, 0);
  tft.print(m[which]);
}


/*****
  Purpose: write the display state last used to EEPROM

  Parameter list:
    void

  Retrun value:
    void
*****/
void WriteDisplayState()
{
  EEPROM.write(DISPLAYOFFSET, displayState);    // Display state one byte higher than WPM
}

/*****
  Purpose: read the display state last used from EEPROM

  Parameter list:
    void

  Retrun value:
    void
*****/
void ReadDisplayState()
{
  displayState = EEPROM.read(DISPLAYOFFSET);
}

/*****
  Purpose: Turn off display for display of info being sent in Morse. Menus still show.

  Parameter list:
    void

  Return value:
    void
*****/
void SetDisplayState()
{
  int switchState = HIGH;

  Erase();
  tft.setCursor(0, 0);
  tft.print("Display: ");
  if (displayState == ON)
    tft.print("ON");
  else
    tft.print("OFF");

  while (true) {
    switchState = digitalRead(ENCODERSWITCH1);
    MyDelay(ROTARYDELAY);
    if (switchState == LOW) {
      WriteDisplayState();                  // Update EEPROM
      return;
    }

    if (rotationDirection != 0) {           // Encoder rotated??
      MyDelay(ROTARYDELAY);
      displayState = !displayState;         // Toggle state
      tft.setCursor(9, 0);
      if (displayState == ON)
        tft.print("ON ");
      else
        tft.print("OFF");
      rotationDirection = 0;
    }
  }
}


/*****
  Purpose: To display a string on the LCD

  Parameter list:
    void

  Return value:
    void
*****/
void ShowString(char s[])
{
  int i;

  for (i = 0; s[i]; i++) {
    ShowChar(s[i]);
    Send(s[i]);
    LetterSpace();
  }
}


/*****
  Purpose: To construct a string for a Sweepstakes contest. Example:
  
        NU1AW would respond to W1AW’s call by sending: W1AW 123 B NU1AW 71 CT, means:
        QSO number 123, B for Single Op High Power, NU1AW, first licensed in 1971, and in the Connecticut section

        The code sends BK at the end of the exchange.

  Parameter list:
    void

  Return value:
    void
*****/

void ShowSweepStakes()
{
  int myRand;
  char temp[6];
  char fakeCall[10];

  myRand = (int) random(0, 1000);         // Generate QSO number
  itoa(myRand, temp, 10);
  strcpy(QSO, temp);
  strcat(QSO, " ");
  myRand = (int) random(0, 6);            // Class
  temp[0] = ssClasses[myRand];
  temp[1] = ' ';
  temp[2] = '\0';
  strcat(QSO, temp);
  GenerateDomesticCall(fakeCall);
  strcat(QSO, fakeCall);
  strcat(QSO, " ");
  myRand = (int) random(1930, 2017);      // Generate year licensed
  if (myRand < 2000)
    myRand %= 1900;      // 1900
  else
    myRand %= 2000;      // 2000
  itoa(myRand, temp, 10); 
  strcat(QSO, temp);
  strcat(QSO, " ");
  strcat(QSO, sectionList[myRand]);
  strcat(QSO, " BK");
  ShowString(QSO);                        // Display exchange
}
