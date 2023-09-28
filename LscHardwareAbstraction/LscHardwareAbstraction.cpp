// LSC.cpp
#include <vector>
#include "LscHardwareAbstraction.h"
#define BEEPER Beeper::getInstance() // macro for the ErrorHandler singleton


void (*Buttons::bt_0_external_callback)() = nullptr;
void (*Buttons::bt_1_external_callback)() = nullptr;
void (*Buttons::bt_2_external_callback)() = nullptr;
void (*Buttons::bt_3_external_callback)() = nullptr;
void (*Buttons::bt_4_external_callback)() = nullptr;
void (*Buttons::bt_5_external_callback)() = nullptr;

//Handles TC3 interupts. See Beeper for detailed explanation
void TC3_Handler() {
   // Clear the interrupt flag
  TC_GetStatus(TC1, 0);
    if(BEEPER.beepFor >= 1){ //checks if we still have to beep
      BEEPER.beepFor -= 1; // decrease beeping time
    }else{ //if we are done beeping
      digitalWrite(52, false); //stop beeping
      TC_Stop(TC1, 0); //stop the timer. (we start it again when Beeper.beep() is called.)
        BEEPER.isRunning = false;
    } 
}



//EOF