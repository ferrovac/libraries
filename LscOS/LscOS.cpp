/*  
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

#include "LscOS.h"
#include "LscPersistence.h"


namespace OS{
    bool watchdogRunning = false;
    bool bootUpFault = false;
    uint32_t watchdogStartTime = 0;
    uint32_t cycleCount=0;
    uint32_t timekeeper = 0;
    uint32_t lastOsCall = 0;    
    String version = "vX.X.X";

    void init(String Version = "vX.X.X"){
        version = Version;
        Serial.begin(115200);
        pmc_set_writeprotect(false);
        pmc_enable_periph_clk(TC5_IRQn); 
        TC_Configure(TC1, 2, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK4); 
        TC_SetRC(TC1, 2, 65620); 
        TC1->TC_CHANNEL[2].TC_IER=TC_IER_CPCS;
        TC1->TC_CHANNEL[2].TC_IDR=~TC_IER_CPCS;
        NVIC_ClearPendingIRQ(TC5_IRQn);
        NVIC_EnableIRQ(TC5_IRQn);
        NVIC_SetPriority(TC5_IRQn, 4);
        TC_Start(TC1, 2);  
        NVIC_SetPriority(SysTick_IRQn, 0); //delay, micros millis isr

        //setup power monitor: see page 273: https://ww1.microchip.com/downloads/en/devicedoc/atmel-11057-32-bit-cortex-m3-microcontroller-sam3x-sam3a_datasheet.pdf#M8.9.22773.h1heading.1.16.Supply.Controller.SUPC
        SUPC->SUPC_SMMR = (SUPC->SUPC_SMMR & ~SUPC_SMMR_SMTH_Msk) | (0xDu << 0);
        SUPC->SUPC_SMMR |= SUPC_SMMR_SMIEN;
        SUPC->SUPC_SMMR |= SUPC_SMMR_SMSMPL_CSM;
        NVIC_EnableIRQ(SUPC_IRQn);
        NVIC_SetPriority(SUPC_IRQn, 0);

        if (!SD.begin(31)){
            Serial.println("No SD Card");
        } 
        else{
            Serial.println("SD Card OK");
            auto file = SD.open("/");
            if(SD.exists("FFF")){
                Serial.println("purging SD");
                while(true){
                    auto entry = file.openNextFile();
                    if(!entry) break;
                    Serial.println("deleted: " + String(entry.name()));
                    const char* name = entry.name();
                    if(entry.isDirectory()){
                        entry.close();
                        SD.rmdir(name);
                    }else{
                        entry.close();
                        SD.remove(name);
                    }
                }
                SD.rmdir("FF");
                SD.rmdir("FFF");
            }
            
        }
  

        BasePersistent::initComplete = true;
        
        for(BasePersistent* basePersistent : *PersistentTracker::getInstance().getInstances()){
            basePersistent->init();
            
            Serial.println(String(reinterpret_cast<const char*>(basePersistent)));
            
        }

        
        for(auto &pair : ComponentTracker::getInstance().states){
            pair.second->readFromSD();
        }
        if(bootUpFault) Serial.println("Bootup Failure");
        
        if (SD.exists("F")){
            bootUpFault = true;
        }
        
    }

    bool getBootUpState(){
        return bootUpFault;
    }
    


    uint32_t getCycleCount(){
        return cycleCount;
    }

    void startWatchdog(){
        if (!watchdogRunning){
            watchdogRunning = true;
            watchdogStartTime = millis();
        }else{
            ERROR_HANDLER.throwError(0x00,"Trying to start watchdog while whatchdog is already running.", SeverityLevel::NORMAL);
        }
    }

    void stopWatchdog(){
        if (watchdogRunning){
            watchdogRunning = false;
        }else{
            ERROR_HANDLER.throwError(0x00,"Trying to stop watchdog while whatchdog is not running.", SeverityLevel::NORMAL);
        }
    }


    void checkSystemHealth(){

    }

    void cleanUp(){

    }
    uint32_t getNextOsCall_ms(){
        return 100 - (millis() - lastOsCall);
    }
    bool saveToRead(){
        if(getNextOsCall_ms() < 10){
            return false;
        }else{
            return true;
        }
    }
    
}

[[noreturn]] void HardFault_Handler() {
    digitalWrite(52, true);
    if (!SD.begin(31)){
        
    }else{
        if(!SD.exists("F")){
            SD.mkdir("F");
            NVIC_SystemReset();
        } 
        if(SD.exists("F") && !SD.exists("FF")){
            SD.mkdir("FF");
            NVIC_SystemReset();
        } 
         
        if(SD.exists("FF") && !SD.exists("FFF")){
            SD.mkdir("FFF");
            NVIC_SystemReset();
        } 
    }
 
}

void SUPC_Handler(void) {
    OS::powerFailureImminent = true;
    PersistentTracker::getInstance().powerFailureImminent = true;        
}
void TC5_Handler(){
    TC_GetStatus(TC1, 2);
    OS::cycleCount = micros() - OS::timekeeper;
    OS::timekeeper = micros();

    //Serial.println("now");
    ComponentTracker::getInstance().lastOsCall = millis();
    int start = micros();
    for(BaseComponent* comp : ComponentTracker::getInstance().getComponets()){
        comp->update();
    }
    
    if (OS::watchdogRunning){
        if(millis() - OS::watchdogStartTime > 5000){
            Serial.println("Detected a timeout, restarting the system!");
            for(int i = 0; i< 4200000;i ++){asm("NOP");}
            //NVIC_SystemReset();
            __asm volatile ("BKPT #0");
        }
    }

    
   // Serial.println(micros()-start);
   /*
    if(Serial.find('@')){
        Serial.println(OS::version);
        auto file = SD.open("/");
                        while(true){
                            auto entry = file.openNextFile();
                            if(!entry) break;
                            const char* name = entry.name();
                            if(entry.isDirectory()){
                                entry.close();
                                SD.rmdir(name);
                            }else{
                                entry.close();
                                SD.remove(name);
                            }
                        }
                        Serial.println("purged");
                        Serial.flush();
                        NVIC_SystemReset();
    }
   */

    
}
