#include "LscSceneManager.h"

std::vector<BaseUI_element*> ElementTracker::elements;
uint32_t SceneManager::backGroundColor = TFT_BLACK;
uint32_t SceneManager::defaultForeGroundColor = TFT_WHITE;
const GFXfont* SceneManager::defaultFont = FM12;
TFT_eSPI SceneManager::tft = TFT_eSPI();
