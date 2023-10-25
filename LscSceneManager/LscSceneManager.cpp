#include "LscSceneManager.h"

volatile bool SceneManager::messageBoxToShowPresent = false;
std::vector<BaseUI_element*> ElementTracker::elements;
TFT_eSPI SceneManager::tft = TFT_eSPI();
