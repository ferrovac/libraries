#include "LscOS.h"
#include "LscHardwareAbstraction.h"



namespace OS{
    bool watchdogRunning = false;
    uint32_t watchdogStartTime = 0;
   
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

    
}
void TC5_Handler(){
    TC_GetStatus(TC1, 2);

    if (OS::watchdogRunning){
        if(millis() - OS::watchdogStartTime > 2000){
            Serial.println("You Fucked up! Which is why we are restarting the whole thing!");
            for(int i = 0; i< 4200000;i ++){asm("NOP");}
            NVIC_SystemReset();
        }
    }

    
}
