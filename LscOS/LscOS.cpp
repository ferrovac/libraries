#include "LscOS.h"
#include "LscPersistence.h"

namespace OS{
    bool watchdogRunning = false;
    bool bootUpFault = false;
    uint32_t watchdogStartTime = 0;
    uint32_t cycleCount=0;
    uint32_t timekeeper = 0;
    uint32_t lastOsCall = 0;
    File myFile;
    
    struct versionTracker{
        static char tracker[21];
    };
    char versionTracker::tracker[] = __DATE__ " " __TIME__;

    Persistent<versionTracker> compileTime("xxx");

    void init(){
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
        NVIC_SetPriority(SysTick_IRQn, 0);

        if (!SD.begin(31)){
            Serial.println("No SD Card");
        } 
        else{
            Serial.println("SD Card OK");
            if (SD.exists("F")){
                bootUpFault = true;
                SD.rmdir("F");
            }
        }
        
        compileTime.setMinIntervall(0);
        BasePersistent::initComplete = true;
        
        for(BasePersistent* basePersistent : *PersistentTracker::getInstance().getInstances()){
            basePersistent->init() ;
        }

        Serial.println("Current Compile Time: " + String(__DATE__ " " __TIME__));
        for(size_t i = 0; i < compileTime.getNumbersOfEntries(); i++){
            Serial.print("Previsous version: ");
            for(size_t k = 0; k<21; k++){
                Serial.print(String(compileTime[i].tracker[k]));
            }
            Serial.println();
            
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

void HardFault_Handler() {
    digitalWrite(52, true);
    if (!SD.begin(31)){
        
    }else{
        SD.mkdir("F");
    }

  for(int i = 0; i < 1000000; i++){
    __asm("NOP");
  }
  NVIC_SystemReset(); // Soft reset the system
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
        if(millis() - OS::watchdogStartTime > 2000){
            Serial.println("You Fucked up! Which is why we are restarting the whole thing!");
            for(int i = 0; i< 4200000;i ++){asm("NOP");}
            NVIC_SystemReset();
        }
    }

    
   // Serial.println(micros()-start);


    
}
