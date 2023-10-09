#include "LscOS.h"
#include "LscHardwareAbstraction.h"



namespace OS{

    bool volatile restoreContext = false;

    void init(){
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


void TC5_Handler(){
    TC_GetStatus(TC1, 2);
    

}

}
