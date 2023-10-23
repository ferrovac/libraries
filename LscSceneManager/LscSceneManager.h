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
#include <TFT_eSPI.h> 
#include "../Fonts/Free_Fonts.h"
#include "../Fonts/Final_Frontier_28.h"
#include "LscOS.h"
#include "vector"
#include "math.h"

struct BaseUI_element{
    private:
        
    public:
        virtual void clear()=0;

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

        bool switchScene(){
            if (nextScene == currentScene){
                return false;
            }else{
                return true;
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
                    
                    void clearChar(String Text, uint16_t index, uint16_t yPosition){
                        tft.setFreeFont(font);
                        tft.setTextColor(backColour);
                        if(Text.length() > index){
                            tft.drawChar(Text[index], xPos + tft.textWidth(Text.substring(0,index+1)) -tft.textWidth(String(Text[index])),yPos);
                        }
                        tft.setTextColor(fontColour);
                    }

                    void update(String Text){  
                        tft.setFreeFont(font);
                        tft.setTextColor(fontColour, backColour);

                        uint16_t textLength = text.length();
                        uint16_t textSubstringLength = 0;
                        uint16_t TextSubstringLength = 0;
                        uint16_t TextCurrentCharWidth = 0;
                        uint16_t textCurrentCharWidth = 0;

                        if(tft.textWidth(text) > tft.textWidth(Text) && tft.textWidth(text) != 0){
                            for(int cleared = text.length(); tft.textWidth(text.substring(0,cleared)) >= tft.textWidth(Text);cleared--){
                                clearChar(text,cleared,yPos);
                                if (cleared <= 0) break;
                            }
                        }

                        uint32_t alreadyCleared = 0;


                        for(uint16_t i = 0; i < Text.length();i++){
                            Serial.println("");
                            textSubstringLength = tft.textWidth(text.substring(0,i + 1));
                            TextSubstringLength = tft.textWidth(Text.substring(0,i + 1));
                            TextCurrentCharWidth = tft.textWidth(String(Text[i]));

                            if(i < textLength){
                                    //WE NEED TO CLEAER
                                    Serial.println("Text: " + Text.substring(0,i) + " : " + tft.textWidth(Text.substring(0,i)));
                                    Serial.println("text: " + text.substring(0,alreadyCleared) + " : " + tft.textWidth(text.substring(0,alreadyCleared)));

                                
                                    if(tft.textWidth(Text.substring(0,i)) == tft.textWidth(text.substring(0,alreadyCleared))){
                                        if(text[alreadyCleared]==Text[i]){
                                            alreadyCleared++;
                                            continue;
                                        }
                                    }
                                    while(true){
                                        clearChar(text,alreadyCleared,yPos);
                                        Serial.println("Cleared: " + String(text[alreadyCleared]) + " at pos: " + String(alreadyCleared));
                                        alreadyCleared += 1;

                                        if(alreadyCleared>= text.length()) break;
                                        if(tft.textWidth(Text.substring(0,i+1)) <= tft.textWidth(text.substring(0,alreadyCleared))) break;
                                    }
                                    
                                    tft.drawChar(Text[i], xPos + tft.textWidth(Text.substring(0,i+1))-tft.textWidth(String(Text[i])),yPos );
                            }else{
                                tft.drawChar(Text[i], TextSubstringLength + xPos - TextCurrentCharWidth,yPos);
                            }
                        }
                            text = Text;
                    }

                public:
                    void setX(uint16_t pos){
                        if(xPos == pos) return;
                        String tempText = text;
                        clear();
                        xPos = pos;
                        setText(tempText);

                    }
                    void setY(uint16_t pos){
                        if(yPos == pos) return;
                        String tempText = text;
                        clear();                        
                        yPos = pos;
                        setText(tempText);
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

                    TextBox(uint16_t xPosition, uint16_t yPosition, String Text = "", const GFXfont* Font=FMB12, uint32_t BackColour = TFT_BLACK, uint32_t FontColour = TFT_WHITE){
                        text = "";
                        setX(xPosition);
                        setY(yPosition);
                        font = Font;
                        backColour = BackColour;
                        fontColour = FontColour;
                        tft.setFreeFont(font);
                        setText(Text);
                        //tft.drawString(text,xPos,yPos);
                    }
                    ~TextBox(){
                        clear();
                    }

                    String getText(){
                        return text;
                    }
                    explicit operator String(){
                        return getText();
                    }
                    
                    TextBox& operator=(String Text){
                        setText(Text);
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

            };  

            struct CheckBox : BaseUI_element{
                private:
                    uint16_t xPos;
                    uint16_t yPos;
                    uint16_t size;
                    uint32_t foreColour;
                    uint32_t backColour;
                    volatile bool checked;
                public:
                    CheckBox(uint16_t xPosition, uint16_t yPosition, uint16_t Size, bool Checked = false, uint32_t BackColour = TFT_BLACK, uint32_t ForeColour = TFT_WHITE): xPos(xPosition), yPos(yPosition), size(Size),checked(false), backColour(BackColour), foreColour(ForeColour) {
                        tft.drawRect(xPos,yPos,size,size,foreColour);
                        setChecked(Checked);
                    }
                    ~CheckBox(){
                        clear();
                    }

                    void clear(){
                        setChecked(false);
                        tft.drawRect(xPos,yPos,size,size,backColour);
                    }
                    void setChecked(bool state){
                        if(state == checked) return;
                        if(state){
                            tft.fillRect(xPos + 2,yPos + 2,size-4,size-4,foreColour);
                        }else{
                            tft.fillRect(xPos + 2,yPos + 2,size-4,size-4,backColour);
                        }
                        checked = state;
                    }

                    CheckBox& operator= (bool state){
                        setChecked(state);
                    }

                    explicit operator bool() const{
                        return checked;
                    }

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
                    long getProgress() const {
                        return progress;
                    }
                    operator long() const {
                        return getProgress();
                    }
                    ProgressBar& operator= (long Progress){
                        setProgress(Progress);
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
            struct Chart : BaseUI_element{ //TODO: Implement
                public:
                    void clear() override{

                    }
                private:

            };
        };
};


#endif