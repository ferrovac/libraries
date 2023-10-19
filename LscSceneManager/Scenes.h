/*
AUTHOR:       Tobias Hofmänner
DATE:         29.09.2023
USAGE:        TODO
DESCRIPTION:  TODO
TODO:       
RECOURCES:  TODO  
*/


#ifndef SCENES_H
#define SCENES_H
#include <Arduino.h>
#include "LscSceneManager.h"

#define elements SceneManager::UI_elements

struct Scene{

};

namespace Scenes{

    struct Scene1 : Scene{
        public:
            static elements::TextBox tb_1;
            static elements::ProgressBar pgb;
            static void loop();
    };

}


#endif