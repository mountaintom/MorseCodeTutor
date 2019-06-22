#ifndef BEENHERE
#include "MorseTutor.h"
#endif


/*****
  Purpose: This is the constructor for the menuing system.

  Parameter list:
    int displayWidth     the max width for this menu
    int menuLevels       what it the depth of the menus
    unit16 foreColor        display menu item in this fore color...
    unit16 backColor        ...using this background color
    unit16 activeFore       Highlight active menu choice with this fore color...
    unit16 activeBack       ...and this back color
    int font             using this font size

  Return value:
    void
*****/
Menuing::Menuing(int displayWidth, int displayHeight, int foreColor, int backColor, int activeFore, int activeBack, int font)
{
  width                 = displayWidth;
  height                = displayHeight;
  foregroundColor       = foreColor;        // Unselected
  backgroundColor       = backColor;
  selectForegroundColor = activeFore;       // Selected
  selectBackgroundColor = activeBack;
  fontSize              = font;
}


/*****
  Purpose: To show a menu option

  Paramter list:
    const char *whichMenu[]     // Array of pointers to the menu option
    int len;                    // The number of menus

  Return value:
    void
*****/
void Menuing::showMenu(Adafruit_ILI9341 myDisplay, char *whichMenu[])
{
  int i;

  myDisplay.setTextColor(foregroundColor, backgroundColor);
  activeRow = 0;
  for (i = 0; i < itemCount; i++) {
    myDisplay.setCursor(i * spacing, activeRow);
    myDisplay.print(whichMenu[i]);
  }
  activeColumn = activeMenuIndex * spacing;

  myDisplay.setCursor(activeColumn, activeRow);
  myDisplay.setTextColor(selectForegroundColor, selectBackgroundColor);
  myDisplay.print(whichMenu[activeMenuIndex]);
}




/*****
  Purpose: To erase all sub menus while leaving the top-most menu unchanged

  Parameter list:

  Return value:
    void
*****/
void Menuing::eraseMenus(Adafruit_ILI9341 myDisplay)
{
  myDisplay.fillRect(0, TOPDEADSPACE, DISPLAYWIDTH, DISPLAYHEIGHT, BLACK);   // Erase old speed line
}

/*****
  Purpose: To erase all sub menus while leaving the top-most menu unchanged

  Parameter list:

  Return value:
    void
*****/
void Menuing::eraseDisplay(Adafruit_ILI9341 myDisplay)
{
  myDisplay.fillRect(0, 0, DISPLAYWIDTH, DISPLAYHEIGHT, BLACK);   // Erase old speed line
}

/*****
  Purpose: This presents a new submenu, displays it, and waits for a choice

  Parameter list:
    int activeMenuLevel     the current menu level
    char *menu[]            the menu choices
    int orientation         horizontal or dropdown

  Return value:
    int                     the choice selected
*****/
void Menuing::deselectMenuItem(Adafruit_ILI9341 myDisplay, char *menu[])
{
  int col, row;
  col = activeMenuIndex * spacing;

  if (activeMenuLevel == 0) {
    row = 0;                                  // Start below top menu and work down
    activeRow = row;
    activeColumn = col;                                // Where is top-most column?
  } else {
    col = spacing * activeMenuIndex;
    row = ROWSPACING * activeMenuIndex;
  }
  myDisplay.setTextColor(foregroundColor, backgroundColor);
  myDisplay.setCursor(col, row);                // Put on old choice...
  myDisplay.print(menu[activeMenuIndex]);       // ...and de-select it.
}

/*****
  Purpose: This moves to the next menu item

  Parameter list:
    char *menu[]         pointer to the current menu list
    int encoder          direction of rotation

  Return value:
    int                 currently active index
*****/
int Menuing::updateMenu(Adafruit_ILI9341 myDisplay, char *menu[], int encoder)
{
  int col, row;

  if (encoder == 1)             // Based on encoder movement...
    activeMenuIndex++;          // ...advance to next item, or...
  else
    activeMenuIndex--;          // ...backup to previous item

  if (activeMenuIndex > itemCount - 1) // Advanced too far, set to first item
    activeMenuIndex = 0;
  if (activeMenuIndex < 0)                                    // Backed up too far, set to end item
    activeMenuIndex = itemCount - 1;

  col = activeMenuIndex * spacing;
  activeColumn = col;
  if (activeMenuLevel == 0) {
    row = ROWSPACING * activeMenuLevel;           // Upper most menu always goes on first line
  } else {
    row = TOPDEADSPACE + ROWSPACING * activeMenuIndex;
  }
  activeRow = row;

  myDisplay.setTextColor(selectForegroundColor, selectBackgroundColor);
  myDisplay.setCursor(col, row);
  myDisplay.print(menu[activeMenuIndex]);
  return activeMenuIndex;
}

/*****
  Purpose: This displays a new submenu

  Parameter list:
    int activeMenuLevel     the current menu level
    char *menu[]            the menu choices
    int itemCount           the number of choices in this menu
    int orientation         horizontal or dropdown
    int defaultIndex        which menu choice is the default?

  Return value:
    int                     the default menu selection
*****/
int Menuing::showMenuLevel(Adafruit_ILI9341 myDisplay, char menu[], int itemCount, int orientation,  int defaultIndex)
{
  int col, i, row, spacing;
  int encoderTurn;

  spacing = myMenu. width / ELEMENTS(menu);
  row = activeMenuLevel + ROWSPACING;
  col = activeMenuIndex * spacing;
  for (i = 0; i < itemCount; i++) {
    if (i == defaultIndex) {
      myDisplay.setTextColor(selectForegroundColor);
      activeMenuIndex = i;                       // This is the currently-active menu selection
    } else {
      myDisplay.setTextColor(foregroundColor);         // All others are unselected
    }
    if (orientation == DROPDOWN) {
      row = TOPDEADSPACE + ROWSPACING * i;       // Start below top menu and work down
      col = activeMenuIndex * spacing;           // Start at main menu column
    } else {
      col = i * spacing;
    }
    myDisplay.setCursor(col, row);
    myDisplay.print(menu[i]);
  }
  row = TOPDEADSPACE + ROWSPACING * activeMenuIndex;    // This is the default, which is highlighted now
  return activeMenuIndex;
}


/*****
  Purpose: This presents a new menu depending upon the encoder direction
  Parameter list:
    char *menu           pointer to the current menu list
    int whichLevel       the setup is for this menu
    int encoder          direction of rotation
    Menuing mine         the menu being used
    int defaults[]       the array that tells which element of the menu is the default

  Return value:
    int                  the default option being presented
*****/
void Menuing::showNewMenu(Adafruit_ILI9341 myDisplay, char *menu[], int encoder, int defaults[])
{
  int col, i, row;

  activeColumn = col = spacing * activeMenuIndex;         // Start at main menu column

  for (i = 0; i < itemCount; i++) {
    row = TOPDEADSPACE + ROWSPACING * i;              // Start below top menu and work down
    if (i == defaults[activeMenuIndex]) {
      myDisplay.setTextColor(selectForegroundColor, selectBackgroundColor);          // All others are unselected
      defaultOption = defaults[activeMenuIndex];            // This is the currently-active menu selection
      activeRow       = row;
      activeColumn    = col;
    } else {
      myDisplay.setTextColor(foregroundColor, backgroundColor);          // All others are unselected
    }
    myDisplay.setCursor(col, row);
    myDisplay.print(menu[i]);
  }
  MyDelay(500UL);
  activeMenuIndex = defaultOption;
}

/*****
  Purpose: This scrolls the top-most menu horizontally

  Parameter list:
    Adafruit_ILI9341 myDisplay    display object
    char *menu[]                  pointer to menu options
    int encoder                   direction of rotation
    int defaults[]       the array that tells which element of the menu is the default

  Return value:
    void
*****/
void Menuing::scrollTopMenu(Adafruit_ILI9341 myDisplay, char *menu[], int encoder, int defaults[])
{
  if (encoder == 1)             // Based on encoder movement...
    activeMenuIndex++;          // ...advance to next item, or...
  else
    activeMenuIndex--;          // ...backup to previous item

  if (activeMenuIndex > itemCount - 1) // Advanced too far, set to first item
    activeMenuIndex = 0;
  if (activeMenuIndex < 0)                                    // Backed up too far, set to end item
    activeMenuIndex = itemCount - 1;

  activeColumn = col = activeMenuIndex * spacing;
  activeRow    = row = 0;           // Upper most menu always goes on first line

  myDisplay.setTextColor(selectForegroundColor, selectBackgroundColor);
  myDisplay.setCursor(col, row);
  myDisplay.print(menu[activeMenuIndex]);
}

/*****
  Purpose: This scrolls the top-most menu horizontally

  Parameter list:
    Adafruit_ILI9341 myDisplay    the tft display
    char *menu                    pointer to the current menu list
    Menuing mine                  the menu being used

  Return value:
    int                   the submenu choice made
*****/
int Menuing::selectFromMenu(Adafruit_ILI9341 myDisplay, char *menu[])
{
  static int val = 0;

  activeMenuIndex = defaultOption;
  while (digitalRead(ENCODERSWITCH1) != LOW) {
    dir = ReadEncoder();

    if (dir) {
      if (dir == 1)
        val++;                      // Clockwise          (CW)
      else
        val--;                      // Counter-clockwise  (CCW)
      if (val == 2 || val == -2) {  // It takes 2 pulses to make a rotation
        
        myDisplay.setTextColor(foregroundColor, backgroundColor);    // Turn off current option
        myDisplay.setCursor(activeColumn, activeRow);
        myDisplay.print(menu[activeMenuIndex]);      
                                                  // Find next option...
        if (val > 0) {                            // CW??
          activeMenuIndex++;
        } else {                                  // Nope, CCW
          activeMenuIndex--;
        }
        if (activeMenuIndex > itemCount - 1) // Advanced too far, set to first item
          activeMenuIndex = 0;
        if (activeMenuIndex < 0)                                  // Backed up too far, set to end item
          activeMenuIndex = itemCount - 1;

        activeRow = TOPDEADSPACE + ROWSPACING * activeMenuIndex;  // Determine active row

        myDisplay.setTextColor(selectForegroundColor, selectBackgroundColor);
        myDisplay.setCursor(activeColumn, activeRow);
        myDisplay.print(menu[activeMenuIndex]);

        val = 0;
      }
    }
  }
  return activeMenuIndex;
}
