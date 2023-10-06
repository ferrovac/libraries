#include "LscOS.h"
#include "LscHardwareAbstraction.h"

void osInit(){

 // Beeper::getInstance().beep(10);

  pmc_set_writeprotect(false);
  pmc_enable_periph_clk(TC5_IRQn); // Set interrupt handler to TCLK5
  TC_Configure(TC1, 2, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK4); // Set external clock input
  TC_SetRC(TC1, 2, 65620); // Every 100th clock will be invoked interrupt 
  TC1->TC_CHANNEL[2].TC_IER=TC_IER_CPCS;
  TC1->TC_CHANNEL[2].TC_IDR=~TC_IER_CPCS;
  NVIC_ClearPendingIRQ(TC5_IRQn);
  NVIC_EnableIRQ(TC5_IRQn);
  NVIC_SetPriority(TC5_IRQn, 4);
  TC_Start(TC1, 2);  // Start the counter on DAC1 pin
        


}

void TC5_Handler(){
    TC_GetStatus(TC1,2);
    /*
    if(LSC::getInstance().powerSwitch_0.getState() == true){
        LSC::getInstance().powerSwitch_0.setState(true);
    }else{
        LSC::getInstance().powerSwitch_0.setState(false);
    }
    */
    //Beeper::getInstance().beep(1);
   LSC::getInstance().println("llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll");
}