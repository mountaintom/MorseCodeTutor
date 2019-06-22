#ifndef BEENHERE
#include "MorseTutor.h"
#endif

/*****
  Purpose: Read the rotary encoder for encoder movement based on econder states

  Parameter list:
    void

  Return value:
    void
*****/
int8_t ReadEncoder()
{
  static int8_t enc_states[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
  static uint8_t old_AB = 0;
  /**/
  old_AB <<= 2;                   //remember previous state
  old_AB |= (digitalRead(ENCODER1PINB) << 1) | digitalRead(ENCODER1PINA);  //add current state
  return (enc_states[( old_AB & 0x0f )]);
}

/*****
  Purpose: Process encoder movement

  Parameter list:
    void

  Return value:
    int             1 on CW, -1 on CCW, 0 on no movement
*****/
int ProcessEncoder()
{
  int8_t result;
  int localDir;
  static int val = 0;

  while (val != 2) {
    result = ReadEncoder();
    if (result == 0) {
      return 0;;
    } else {
      if (result == 1)
        val++;                      // Clockwise          (CW)
      else
        val--;                      // Counter-clockwise  (CCW)
      if (val == 2 || val == -2) {  // It takes 2 pulses to make a rotation
        if (val > 0) {              // CW??
          localDir = 1;
        } else {                    // Nope, CCW
          localDir = -1;
        }
        val = 0;
        return localDir;
      }
    }
  }
}
