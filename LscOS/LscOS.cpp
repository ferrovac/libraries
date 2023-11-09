#include "LscOS.h"

namespace OS{
    bool watchdogRunning = false;
    uint32_t watchdogStartTime = 0;
    uint32_t cycleCount=0;
    uint32_t timekeeper = 0;
    uint32_t lastOsCall = 0;

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

    void updateComponents(){

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
  for(int i = 0; i < 10000000; i++){
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
