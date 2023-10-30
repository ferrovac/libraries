#include "LscComponents.h"

/*
    ARDUINO IDE FIX 
    There seams to be an issue with the standard lib in arduino, leading to errors working with std:vectors
    adding the following code fixes the porblem.
*/

std::vector<std::pair<BaseComponent*, BaseExposedState*>> ComponentTracker::states;
std::vector<BaseComponent*> ComponentTracker::components;
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
