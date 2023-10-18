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
                    uint32_t backColour;
                    uint32_t fontColour;
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
                    TextBox(uint16_t xPosition, uint16_t yPosition, const GFXfont* Font=FMB12, uint32_t BackColour = TFT_BLACK, uint32_t FontColour = TFT_WHITE){
                        setX(xPosition);
                        setY(yPosition);
                        font = Font;
                        backColour = BackColour;
                        fontColour = FontColour;
                        tft.setFreeFont(font);
                        tft.drawString(text,xPos,yPos);
                        
                    }
                    void setText(String Text){
                        if(text != Text){
                            update(Text);
                        }
                        text = Text;
                    }

                    void update(String Text){  
                        tft.setFreeFont(font);
                        tft.setTextColor(fontColour, backColour);
                        int startTime = millis();            
                        Serial.println(millis()-startTime);
                        uint16_t oldTextLength = text.length();
                        uint16_t textSubstringLength = 0;

                        for(int i = 0; i < Text.length();i++){
                            textSubstringLength = tft.textWidth(text.substring(0,i+1));
                            
                            if(i <= oldTextLength){
                                if(Text[i] != text[i] || tft.textWidth(text.substring(0,i+1)) != tft.textWidth(Text.substring(0,i + 1))){

                                    startTime = micros();
                                    if(tft.textWidth(String(Text[i])) >= tft.textWidth(String(text[i]))){
                                        clearChar(Text[i], tft.textWidth(Text.substring(0,i+1)) + xPos - tft.textWidth(String(Text[i])),yPos );
                                    }else{
                                        clearChar(text[i], tft.textWidth(Text.substring(0,i+1)) + xPos - tft.textWidth(String(Text[i])),yPos );
                                    }
                                    tft.drawChar(Text[i],tft.textWidth(Text.substring(0,i+1)) + xPos - tft.textWidth(String(Text[i])),yPos );
                                    
                                    
                                }
                            }else{
                                tft.drawChar(Text[i],tft.textWidth(Text.substring(0,i+1)) + xPos - tft.textWidth(String(Text[i])),yPos);
                            }
                        }
                            if(tft.textWidth(text) > tft.textWidth(Text)){
                                tft.fillRect(tft.textWidth(Text) + xPos, yPos - tft.fontHeight() + 1 , tft.textWidth(text) - tft.textWidth(Text),tft.fontHeight(),backColour);
                            }
                            text = Text;
                            Serial.println(micros()-startTime);

                    }

                    void clearChar(char Character, uint16_t xPosition, uint16_t yPosition){
                        tft.setFreeFont(font);
                        tft.fillRect(xPosition,yPosition - tft.fontHeight() + 1, tft.textWidth(String(Character)),tft.fontHeight(),backColour);
                    }
                    //void replaceChar(uint16_t Index)
            };  
        };
};


#endif