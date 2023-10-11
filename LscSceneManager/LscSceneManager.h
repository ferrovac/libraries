/*
AUTHOR:       Tobias Hofmänner
DATE:         29.09.2023
USAGE:        TODO
DESCRIPTION:  TODO
TODO:       
RECOURCES:  TODO  
*/
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library (CW)
#include "../Fonts/Free_Fonts.h"
#include "../Fonts/Final_Frontier_28.h"

#ifndef LSCSCENEMANAGER_H
#define LSCSCENEMANAGER_H

class SceneManager{
    private:
        SceneManager() {

        }
    public:
        void init(){
            tft.init();
            tft.setRotation(3);
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            //tft.setTextDatum(MC_DATUM);
            tft.setFreeFont(FMB12);                 // Select the font
        }
        static TFT_eSPI tft;
        static SceneManager& getInstance() {
            static SceneManager instance;
            return instance;
        }

        struct UI_elements{
            struct TextBox{
                private:
                    uint16_t xPos;
                    uint16_t yPos;
                    String text;
                    const GFXfont* font;

                public:
                    void setX(uint16_t pos){
                        xPos = pos;
                    }
                    void setY(uint16_t pos){
                        yPos = pos;
                    }
                    uint16_t getX(){
                        return xPos;
                    }
                    uint16_t getY(){
                        return yPos;
                    }
                    TextBox(uint16_t xPosition, uint16_t yPosition, String Text ="", const GFXfont* Font=FMB12){
                        setX(xPosition);
                        setY(yPosition);
                        text = Text;
                        font = Font;
                        tft.drawString(text,xPos,yPos);
                        
                    }
                    void setText(String Text){
                        if(text != Text){
                            update(Text);
                        }
                        text = Text;
                    }

                    void update(String Text){          
                        uint16_t xOffSet = xPos;              
                        for(int i = 0; i < Text.length();i++){
                            if(Text[i] != text[i] || tft.textWidth(text.substring(0,i)) != tft.textWidth(Text.substring(0,i))){
                                tft.setTextPadding(tft.textWidth(String(Text[i])));
                                tft.drawChar(Text[i], xOffSet  ,yPos);
                            }
                            xOffSet += tft.textWidth(String(Text[i]));
                        }
                    }
            };  
        };

};




#endif