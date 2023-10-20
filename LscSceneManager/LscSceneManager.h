/*
AUTHOR:       Tobias Hofmänner
DATE:         29.09.2023
USAGE:        TODO
DESCRIPTION:  TODO
TODO:       
RECOURCES:  TODO  
*/


#ifndef LSCSCENEMANAGER_H
#define LSCSCENEMANAGER_H

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library (CW)
#include "../Fonts/Free_Fonts.h"
#include "../Fonts/Final_Frontier_28.h"
#include "LscOS.h"
#include "vector"

struct BaseUI_element{
    private:
        
    public:
        static std::vector<BaseUI_element*> elementTracker;
        virtual void clear()=0;
        BaseUI_element(){
            elementTracker.push_back(this);
        }
};




class SceneManager{
    private:
        void (*volatile currentScene)();
        void (*volatile nextScene)();

        SceneManager(){
        }
 
    public:
        void init(void (*scene)()){
            tft.init();
            tft.setRotation(3);
            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setFreeFont(FMB12);     
            currentScene = scene;    
            nextScene = scene;
        }
        void begin(){
            while(true){
                currentScene();
                currentScene = nextScene; 
                Serial.println("in manager");
            }
        }
        static TFT_eSPI tft;
        static SceneManager& getInstance() {
            static SceneManager instance;
            return instance;
        }

        void loadScene(void (*scene)()){
            nextScene = scene;
        }

        bool noSwitch(){
            //delay(1);
            if (nextScene == currentScene){
                return true;
            }else{
                return false;
            }   
        }



        struct UI_elements{

            struct TextBox : BaseUI_element{
                private:
                    uint16_t xPos;
                    uint16_t yPos;
                    String text;
                    uint32_t backColour;
                    uint32_t fontColour;
                    const GFXfont* font;
                    
                    void clearChar(char Character, uint16_t xPosition, uint16_t yPosition){
                        tft.setFreeFont(font);
                        tft.fillRect(xPosition,yPosition - tft.fontHeight() + 1, tft.textWidth(String(Character)),tft.fontHeight(),backColour);
                    }
                    void update(String Text){  
                        tft.setFreeFont(font);
                        tft.setTextColor(fontColour, backColour);

                        uint16_t textLength = text.length();
                        uint16_t textSubstringLength = 0;
                        uint16_t TextSubstringLength = 0;
                        uint16_t TextCurrentCharWidth = 0;
                        uint16_t textCurrentCharWidth = 0;

                        for(int i = 0; i < Text.length();i++){
                            textSubstringLength = tft.textWidth(text.substring(0,i + 1));
                            TextSubstringLength = tft.textWidth(Text.substring(0,i + 1));
                            TextCurrentCharWidth = tft.textWidth(String(Text[i]));

                            if(i <= textLength){
                                if(Text[i] != text[i] || textSubstringLength != TextSubstringLength){

                                    if(TextCurrentCharWidth >= tft.textWidth(String(text[i]))){
                                        clearChar(Text[i], TextSubstringLength + xPos - TextCurrentCharWidth,yPos );
                                    }else{
                                        clearChar(text[i], TextSubstringLength + xPos - TextCurrentCharWidth,yPos );
                                    }
                                    tft.drawChar(Text[i],TextSubstringLength + xPos - TextCurrentCharWidth,yPos );
                                }
                            }else{
                                tft.drawChar(Text[i], TextSubstringLength + xPos - TextCurrentCharWidth,yPos);
                            }
                        }
                            if(tft.textWidth(text) > tft.textWidth(Text)){
                                tft.fillRect(tft.textWidth(Text) + xPos, yPos - tft.fontHeight() + 10 , tft.textWidth(text) - tft.textWidth(Text),tft.fontHeight(),backColour);
                            }
                            text = Text;
                            
                    }

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
                    uint16_t getTextLengthInPixels(){
                        tft.setFreeFont(font);
                        return tft.textWidth(text);
                    }
                    uint16_t getTextHeightInPixels(){
                        tft.setFreeFont(font);
                        return tft.fontHeight();
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
                    ~TextBox(){
                        clear();
                    }
                    void setText(String Text){
                        if(text != Text){
                            update(Text);
                        }
                        text = Text;
                    }
                    void clear() override{
                        setText("");
                    }




                    //void replaceChar(uint16_t Index)
            };  
            struct ProgressBar : BaseUI_element{
                private:
                    uint32_t xPos;
                    uint32_t yPos;
                    uint32_t height;
                    uint32_t width;
                    uint32_t foreColour;
                    uint32_t backColour;
                    long progress;
                    uint32_t progressInPixel;

                public:
                    ProgressBar(uint32_t xPos, uint32_t yPos, uint32_t height, uint32_t width, uint32_t foreColour = TFT_WHITE, uint32_t backColour = TFT_BLACK): xPos(xPos), yPos(yPos), height(height), width(width), progress(0), foreColour(foreColour), backColour(backColour){
                        tft.drawRect(xPos,yPos, width,height,foreColour);
                        progressInPixel = 0;
                    }
                    ~ProgressBar(){
                        clear();
                    }
                    long getProgress(){
                        return progress;
                    }
                    void setProgress(long Progress){
                        if(Progress >= 0 && Progress <= 100){
                            
                            if(Progress > progress){
                                uint32_t fillTo = (uint32_t)map(Progress, 0., 100., 1., (long)(width-1));
                                tft.fillRect(progressInPixel + xPos  ,yPos + 1,fillTo - progressInPixel , height - 2,foreColour);
                                progressInPixel = fillTo;
                            }else{
                                uint16_t fillTo = map(Progress, 0, 100, 1., (long)(width-1));
                                tft.fillRect(fillTo + xPos,yPos + 1 , progressInPixel - fillTo   , height - 2,backColour);
                                progressInPixel = fillTo;
                            }
                            progress = Progress;
                        }
                    }    
                    void setForeColour(uint32_t Colour){
                        foreColour = Colour;
                        tft.drawRect(xPos,yPos, width,height,foreColour);
                        long tempProgress = progress;
                        setProgress(0);
                        setProgress(tempProgress);
                    }
                    void setBackColour(uint32_t Colour){
                        backColour = Colour;
                        tft.drawRect(xPos,yPos, width,height,foreColour);
                        long tempProgress = progress;
                        setProgress(100);
                        setProgress(tempProgress);
                    }

                    void clear() override{
                        setProgress(0);
                        tft.drawRect(xPos,yPos, width,height,backColour);
                    }          
            };
            struct Chart : BaseUI_element{
                public:
                    void clear() override{

                    }
                private:

            };
        };
};


#endif