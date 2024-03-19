/*  
    Copyright (C) 2024 Ferrovac AG

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    NOTE: This specific version of the license has been chosen to ensure compatibility 
          with the SD library, which is an integral part of this application and is 
          licensed under the same version of the GNU General Public License.
*/

#include "LscSceneManager.h"

std::vector<BaseUI_element*> ElementTracker::elements;
std::vector<std::vector<BaseUI_element*>> ElementTracker::clearLayers;

volatile bool Rules::allowed = true;


uint32_t SceneManager::backGroundColor = TFT_BLACK;
uint32_t SceneManager::defaultForeGroundColor = TFT_WHITE;
volatile bool SceneManager::systemStableFor20Sec = false;
const GFXfont* SceneManager::defaultFont = FM12;
TFT_eSPI SceneManager::tft = TFT_eSPI();
