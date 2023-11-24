#include "LscSceneManager.h"

std::vector<BaseUI_element*> ElementTracker::elements;
std::vector<std::vector<BaseUI_element*>> ElementTracker::clearLayers;

uint32_t SceneManager::backGroundColor = TFT_BLACK;
uint32_t SceneManager::defaultForeGroundColor = TFT_WHITE;
volatile bool SceneManager::systemStableFor20Sec = false;
const GFXfont* SceneManager::defaultFont = FM12;
TFT_eSPI SceneManager::tft = TFT_eSPI();
