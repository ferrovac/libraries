/*  Main arduino programm that controlls the Ferrovac GloveBox 
    Copyright (C) 2024 Ferrovac AG

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    NOTE: This specific version of the license has been chosen to ensure compatibility 
          with the SD library, which is an integral part of this application and is 
          licensed under the same version of the GNU General Public License.
*/

#include "LscHardwareAbstraction.h"
#define BEEPER Beeper::getInstance() // macro for the ErrorHandler singleton
#define min(a,b) ((a)<(b)?(a):(b))

void (*volatile Buttons::bt_0_external_callback)() = nullptr;
void (*volatile Buttons::bt_1_external_callback)() = nullptr;
void (*volatile Buttons::bt_2_external_callback)() = nullptr;
void (*volatile Buttons::bt_3_external_callback)() = nullptr;
void (*volatile Buttons::bt_4_external_callback)() = nullptr;
void (*volatile Buttons::bt_5_external_callback)() = nullptr;
//defining the static uart buffer see sync UART explanation
RingBuf<char, 3450> LSC::uartBuffer;

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

//Handles TC2 interupts. This timer is responsible for sending the data from the uartBuffer to the uart see sync UART explanation
//TODO: Optimize, there might be a way of writing directly form the ring buffer to the uart buffer without the need for the outBuffer
                  //but im not sure what overhead Seria.write is connected with... To be tested...
void TC2_Handler(){
    TC_GetStatus(TC0,2);
    //LSC::getInstance().powerSwitch_0.setState(true);
    char charBuf = ' ';
    uint16_t dataLen = 0;
    //we never want so send more then 40B of data at a time
    dataLen = min(LSC::getInstance().uartBuffer.size(),40);
    //we ned to arrange the data in the ring buffer in a way we can send it
    char outBuffer[3451];;
    //pop the data from the ringbuffer and place it in the send buffer 
    for(int i = 0; i< dataLen; i++){
      LSC::getInstance().uartBuffer.pop(charBuf);
      outBuffer[i] = charBuf;
    } 
    //zero terminate the outBuffer
    outBuffer[dataLen] = '\0';
    //send the data to the uart buffer
    if(dataLen >0) Serial.print(outBuffer);
    //  LSC::getInstance().powerSwitch_0.setState(false);
    
}
//EOF