#include "LscComponents.h"

/*
    ARDUINO IDE FIX 
    There seams to be an issue with the standard lib in arduino, leading to errors working with std:vectors
    adding the following code fixes the porblem.
*/


void waitForSaveReadWrite(){
    while(100-(millis()-ComponentTracker::getInstance().lastOsCall) < 3){
    }
}



//std::vector<BaseComponent*> ComponentTracker::components;
//std::vector<std::pair<BaseComponent*, BaseExposedState*>> ComponentTracker::states;
namespace std
{
    void __throw_bad_alloc()
    {
        Serial.println("Unable to allocate memory");
    }
    void __throw_length_error(char const *e)
    {
        Serial.print("Length Error :");
        Serial.println(e);
    }
}
