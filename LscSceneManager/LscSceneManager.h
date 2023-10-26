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
#include <algorithm>
#include <deque>




struct BaseUI_element;

//The ElementTracker keeps track of all UI_element instances that are created. When an element is created, a pointer
//to the element will be stored in the ElementTracker. The tracker provides two functions one to register elments and 
//one to remove elements. The Tracker is implemented as singelton. 
struct ElementTracker{
    private:
        //private constructor to follow singelton pattern
        ElementTracker(){}
    public:
        //vecor that holds the pointer of all elements
        static std::vector<BaseUI_element*> elements;
        //singelton lazy init
        static ElementTracker& getInstance() {
            static ElementTracker instance;
        return instance;
        }
        //Adds a UI_element to the tracker
        static void registerElement(BaseUI_element* element){
            elements.push_back(element);
        }
        //removes an element form the tracker. Takes a pointer to the element to be removed as argument
        //if the element to be removed does not exist the function does nothing.
        static void removeElement(BaseUI_element* element){
            elements.erase(std::remove(elements.begin(), elements.end(), element), elements.end());
        }
};

//Represents a UI_element all objects that reder something on the tft should inherit form this class.
//there are two functions that have to be imlemented in the child class:
//  1.  clear
//          removes everyting the elment rendered on the screen, without changing the object itself
//          this is usefull to temporaraly clear the screen to for exemple render a popup
//  2.  reDraw
//          after clear has been called reDraw should be able to rerender an object based on the internal state
struct BaseUI_element{
    private:
    public:
        virtual void clear() const  = 0;
        virtual void reDraw() = 0;
        //constructor adds the element to the ElementTracker
        BaseUI_element(){
            ElementTracker::getInstance().registerElement(this);
        }
        //destructor removes the element from the ElementTracker
        ~BaseUI_element(){
            ElementTracker::getInstance().removeElement(this);
        }
};




      //---- SCENEMANAGET EXPLANATION ----
  /*
    GENERALL CONCEPT    
        The SceneManager is responsible to handle everything that has to do with the tft display. It provides a lot of UI functions
        to manipulate the sceen and a library of ui elements like TextBoxes ProgressBars and more.
        The SceneManager is implemented as singelton and has to be implemented with the init() function and then
        started with the begin() function. The init function will prepare the tft screen and set the first sceene.
        The init function also sets the BackgroudColor of the screen. Currently the user can not change the backgroud color
        after this point. The reason behind this is, that backgroud color changes are VERY expensive on the cpu and almost always
        look very bad in the end. Local areas with different color can be crated with rectangles. Another reason for a fixed background
        color is that; removing an elment form the screen means overwriting the element with the background color, which is much
        easier the implement if the background color is fixed (this could be changed at a later point if needed).
        All SceneManager functions should be thread save. The begin() function will enter an endless loop and start managing
        scenes. Therefor all code after begin() will never be executed!
    SCENES
        To make writing ui code as easy as possible, we implemented the whole ui logic in scenes. A scene is nothing but
        a void function with a while loop in the end:
            void scene(){
                //setup code here like defining ui_elements
                while(!sceneManager.switchScene()){
                    //main ui loop. Manipulate the ui_elements here. Here lives the ui_logic the user can interact with
                }
            }
        To switch to another scene sceneManager.loadScene(void (*scene)()); can be called. This tells the SceneManager that
        a scene change is pending. Practically this will make sceneManager.switchScene() return true, thus terminating the while
        loop. This terminates the execution of the current scene. As soon as the void scene functoin returns all elements definded
        in the context of the scene are destroyed thus automatically removing everything belonging to these elements form the screen
        To interact with the user there are two main ways:
        1.  Set Button handler:
                    lsc.buttons.setOnClickHandler(lsc.buttons.bt_2, switchTo2);
                this will execute the switchTo2 function when button 2 is pressed. the switchTo2 could look like this:
                    void switchTo2(){
                        sceneManager.loadScene(scene2);
                    }
                in this exemple we switch to scene2 when button 2 is pressed. This methode is most usefull when the function
                of a button is the same in the whole scene and does not change. Because then the handler can be set in the 
                setup part of the scene such that one does not not have to querry the state of the button in the ui loop.
        2.  Querry button state:
                    lsc.buttons.bt_4.hasBeenClicked()
                checks if button 4 has been clicked. The button does not have to be pressed at the exact time the funtion
                is called. As soon as the button is pressed an internal flag is set such that the function returns true 
                when it is called. By calling hasBeenClicked() the flag is reset. i.e. until the button is pressed again the
                function will return false. This means you can use hasBeenClicked() to clear previous putton presses.
                When switching to a new scene the state of all buttons is automatically cleared.
  */
    //---- END SCENEMANAGET EXPLANATION ----
class SceneManager{
    private:
        //holds a pointer to the currently running scene
        void (*volatile currentScene)();
        //holds a pointer to the next scene to be loded. If this is not the same as currentScene a scene switch will be triggered
        void (*volatile nextScene)();
        //holds the global backgroud color of the tft display. This is set in the init function and should not be changed later
        uint32_t backgroundColor;
        //private constructor for singelton pattern
        SceneManager(){}
        
 
    public:
        static void clearAllElements(){
            for(BaseUI_element* element: ElementTracker::getInstance().elements){
                element->clear();
            }
        }
        static void reDrawAllElements(){
            for(BaseUI_element* element: ElementTracker::getInstance().elements){
                element->reDraw();
            }
        }
        static int getNumberOfElements(){
            return ElementTracker::getInstance().elements.size();
        }
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
                LSC::getInstance().buttons.bt_0.hasBeenClicked();
                LSC::getInstance().buttons.bt_1.hasBeenClicked();
                LSC::getInstance().buttons.bt_2.hasBeenClicked();
                LSC::getInstance().buttons.bt_3.hasBeenClicked();
                LSC::getInstance().buttons.bt_4.hasBeenClicked();
                LSC::getInstance().buttons.bt_5.hasBeenClicked();
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

            /*
                UI ELEMENTS GERNERELL CONCEPT
                The idea is that the ui elements are created in functions in the main loop. Typically these functions 
                have a sections where all the ui elements are created followed by a loop where the ui elements are updated
                It is importante, that the element is renedred as soon as the instance is created. i.e. the initial render
                should be done in the constructor. It is also best if the whole element can be fully defined at cration (this 
                makes the mandatory reDraw function very easy to implement by simply calling the constructor).
                The idea is that the element is on the screen for as loong as the element instance exists. And that the 
                element is removed from the screan when the instance is deleted. This is nice because it means, that as soon
                as a scene function returns all variables and instances of the elemenst go out of scope and are this destructed,
                making it very intuitive and easy to organize scenes in void functions. This means you have to make sure, that 
                the constructor is porpperly defined and removes everything belonging to the element form the screen.

                UI ELEMENTS DESIGNE GUIDS
                1. All elements should inherit form BaseUI_element
                2. All elements need to have the following functions:
                    2.1 void clear() const override {}
                        This function should clear the screan of the whole whole UI element.
                        The function sould only clear the screen and not change any of the elements parameters
                        this is important because the clear function can be used to "blank" the screan after which
                        it should be possible to recreate the old state again with the reDraw() function (this is enforced
                        by the const statement, dont change this.)
                    2.1 void reDraw() override {}
                        This function sould be able to re render the whole screan according to the elements internal
                        state after the screen has been cleared.
                3.  There might be an issue if member variables are changed by an interrupt. Consider for which variables this
                    might be the case and declare them as volatile.
                
            */

            struct TextBox : BaseUI_element{
                private:
                    uint16_t xPos;
                    uint16_t yPos;
                    String text;
                    uint32_t backColour;
                    uint32_t fontColour;
                    const GFXfont* font;
                    
                    void clearChar(String Text, uint16_t index, uint16_t yPosition) const {
                        tft.setFreeFont(font);
                        tft.setTextColor(backColour);
                        if(Text.length() > index){
                            tft.drawChar(Text[index], xPos + tft.textWidth(Text.substring(0,index+1)) -tft.textWidth(String(Text[index])),yPos);
                        }
                        tft.setTextColor(fontColour);
                    }

                    void update(String Text) const {  
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
                            textSubstringLength = tft.textWidth(text.substring(0,i + 1));
                            TextSubstringLength = tft.textWidth(Text.substring(0,i + 1));
                            TextCurrentCharWidth = tft.textWidth(String(Text[i]));

                            if(i < textLength){
                                    //WE NEED TO CLEAER

                                    if(tft.textWidth(Text.substring(0,i)) == tft.textWidth(text.substring(0,alreadyCleared))){
                                        if(text[alreadyCleared]==Text[i]){
                                            alreadyCleared++;
                                            continue;
                                        }
                                    }
                                    while(true){
                                        clearChar(text,alreadyCleared,yPos);

                                        alreadyCleared += 1;

                                        if(alreadyCleared>= text.length()) break;
                                        if(tft.textWidth(Text.substring(0,i+1)) <= tft.textWidth(text.substring(0,alreadyCleared))) break;
                                    }
                                    
                                    tft.drawChar(Text[i], xPos + tft.textWidth(Text.substring(0,i+1))-tft.textWidth(String(Text[i])),yPos );
                            }else{
                                tft.drawChar(Text[i], TextSubstringLength + xPos - TextCurrentCharWidth,yPos);
                            }
                        }
                            
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
                        xPos = xPosition;
                        yPos = yPosition;
                        font = Font;
                        backColour = BackColour;
                        fontColour = FontColour;
                        tft.setFreeFont(font);
                        setText(Text);
                    }
                    ~TextBox(){
                        clear();
                    }
                    void reDraw(){
                        String temp_text = text;
                        text = "";
                        setText(temp_text);
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
                    void clear() const override{
                        update("");
                        //setText("");
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
                    CheckBox(uint16_t xPosition, uint16_t yPosition, uint16_t Size, bool Checked = false, uint32_t BackColour = TFT_BLACK, uint32_t ForeColour = TFT_WHITE): xPos(xPosition), yPos(yPosition), size(Size),checked(Checked), backColour(BackColour), foreColour(ForeColour) {
                        tft.drawRect(xPos,yPos,size,size,foreColour);
                        setChecked(Checked);
                    }
                    ~CheckBox(){
                        clear();
                    }
                    void reDraw(){
                        tft.drawRect(xPos,yPos,size,size,foreColour);
                        bool temp_checked = checked;
                        checked = false;
                        setChecked(temp_checked);
                    }

                    void clear()const override{
                        if(checked) tft.fillRect(xPos,yPos,size,size,backColour);
                        if(!checked) tft.drawRect(xPos,yPos,size,size,backColour);
                    }const
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
                    ProgressBar(uint32_t xPos, uint32_t yPos, uint32_t height, uint32_t width, long Progress = 0, uint32_t foreColour = TFT_WHITE, uint32_t backColour = TFT_BLACK): xPos(xPos), yPos(yPos), height(height), width(width), progress(0), foreColour(foreColour), backColour(backColour){
                        tft.drawRect(xPos,yPos, width,height,foreColour);
                        progressInPixel = 0;
                        setProgress(Progress);
                    }
                    ~ProgressBar(){
                        clear();
                    }
                    void reDraw(){
                        tft.drawRect(xPos,yPos, width,height,foreColour);
                        long temp_progress  = progress;
                        progress = 0;
                        progressInPixel = 0;
                        setProgress(temp_progress);
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

                    void clear() const override {
                        tft.fillRect(xPos,yPos, width,height,backColour);
                    }          
            };
            struct Chart : BaseUI_element{ //TODO: Implement
                public:
                    void clear() const override{

                    }
                private:

            };


        };
        
        bool showMessageBox(String Title, String Message, String OptionFalse="NO", String OptionTrue="YES"){
                LSC::getInstance().buttons.bt_0.active = false;
                LSC::getInstance().buttons.bt_1.active = false;
                LSC::getInstance().buttons.bt_2.active = false;
                LSC::getInstance().buttons.bt_3.active = false;
                LSC::getInstance().buttons.bt_4.active = false;
                LSC::getInstance().buttons.bt_5.active = false;
                bool returnValue;
                LSC::getInstance().buttons.bt_2.hasBeenClicked();
                LSC::getInstance().buttons.bt_5.hasBeenClicked();

                clearAllElements();
                tft.drawRect(0,0,320,200,TFT_WHITE);
                UI_elements::TextBox* title = new UI_elements::TextBox(5,20,Title);
                tft.drawLine(0,25,320,25,TFT_WHITE);
                
                //tft.drawLine(0,240,320,240,TFT_WHITE);
                UI_elements::TextBox* no = new UI_elements::TextBox(80 -tft.textWidth(OptionFalse)/2,225,OptionFalse);
                UI_elements::TextBox* yes = new UI_elements::TextBox(240-tft.textWidth(OptionTrue)/2,225,OptionTrue);
                tft.drawLine(160,200,160,240,TFT_WHITE);
                std::deque<String> linesUnderScreen;
                std::deque<String> linesOverScreen;
                std::vector<UI_elements::TextBox*> messageTextBoxCollection;
                int lastSpace = 0;
                int lastLineFeed = -1;
                int index = 0;
                int numberOfLines = 0;
                
                bool tooManyLinesForScreen = false;
                tft.setFreeFont(FM9);
                int maxLinesOnScreen = 150 / tft.fontHeight(); //there will be one more line on the screen then this number indecates...
                int pages = 0;
                int startTime = millis();
                for(char character : Message){
                    if(character == ' ') lastSpace = index;
                    if(tft.textWidth(Message.substring(lastLineFeed + 1,index )) > 280){

                        if(!tooManyLinesForScreen) messageTextBoxCollection.push_back(new UI_elements::TextBox(5,45+numberOfLines*tft.fontHeight(),Message.substring(lastLineFeed + 1,lastSpace),FM9));
                        
                        if( tooManyLinesForScreen) linesUnderScreen.push_back(Message.substring(lastLineFeed + 1,lastSpace));
                        lastLineFeed = lastSpace ;
                        numberOfLines++;
                        if(numberOfLines >= maxLinesOnScreen) tooManyLinesForScreen = true;
                    }
                    index++;
                }




                if((lastLineFeed + 1 < Message.length())){
                    if(!tooManyLinesForScreen) messageTextBoxCollection.push_back(new UI_elements::TextBox(5,45+numberOfLines*tft.fontHeight(),Message.substring(lastLineFeed + 1,Message.length()),FM9));
                    if( tooManyLinesForScreen) linesUnderScreen.push_back(Message.substring(lastLineFeed + 1,Message.length()));
                    numberOfLines++;
                }
                Serial.println( numberOfLines );
                Serial.println( maxLinesOnScreen);
                
                int addedNumberOfLinesForPadding = 0;

                    if((numberOfLines % maxLinesOnScreen) != 0){
                        for(int k = 0; k < maxLinesOnScreen-(numberOfLines % maxLinesOnScreen); k++){
                            linesUnderScreen.push_back("");
                            addedNumberOfLinesForPadding++;
                        }
                    }

                numberOfLines += addedNumberOfLinesForPadding;

                pages = numberOfLines / maxLinesOnScreen;
                                Serial.println( numberOfLines );

                Serial.println((float)numberOfLines / (float)maxLinesOnScreen);



                int page = 0;
                int tbPageCounterXpos = 150;
                UI_elements::TextBox* tb_page = new UI_elements::TextBox(304,tbPageCounterXpos,"",FM9);
                UI_elements::TextBox* tb_slash = new UI_elements::TextBox(304,tbPageCounterXpos + 15,"",FM9);
                UI_elements::TextBox* tb_pages = new UI_elements::TextBox(304,tbPageCounterXpos + 30,"",FM9);
                int yDepth = 130;
                int xDepth = 20;

                while(true){
                    if(LSC::getInstance().buttons.bt_2.hasBeenClicked()){
                        returnValue = false;
                        break;
                    }
                    if(LSC::getInstance().buttons.bt_5.hasBeenClicked()){
                        returnValue = true;
                        break;
                    }
                    if(!tooManyLinesForScreen) continue;


                    tft.drawLine(319,0,319,yDepth,TFT_BLACK);
                    tft.drawLine(320-xDepth,0,320,0,TFT_BLACK);
                    tft.drawLine(320-xDepth,25,320,25,TFT_BLACK);
                    tft.drawLine(320-xDepth,0,320-xDepth,200,TFT_WHITE);
                    tft.drawLine(320-xDepth,yDepth,320,yDepth,TFT_WHITE);
                    tb_page->setText(String(page+1));
                    tb_slash->setText("/");
                    tb_pages->setText(String(pages));

                    if(LSC::getInstance().buttons.bt_3.hasBeenClicked() && page > 0){
                         tb_page->setText(String(page+1));                       
                        for(int32_t i = messageTextBoxCollection.size() -1; i >=0 ; i--){
                            linesUnderScreen.push_front(messageTextBoxCollection[i]->getText());
                        }
                        for(int32_t i = messageTextBoxCollection.size() -1; i >=0 ; i--){
                            messageTextBoxCollection[i]->setText(linesOverScreen.back());
                            linesOverScreen.pop_back();
                        }

                        page--;                       

                    }
                    if(LSC::getInstance().buttons.bt_4.hasBeenClicked() && page < pages-1){
                        int linesToPrint = linesUnderScreen.size();
                        tb_page->setText(String(page+1));
                        for(int32_t i = 0; i < messageTextBoxCollection.size(); i++){
                            linesOverScreen.push_back(messageTextBoxCollection[i]->getText());
                            if(i > linesToPrint) continue;
                            messageTextBoxCollection[i]->setText(linesUnderScreen.front());
                            linesUnderScreen.pop_front();
                        }
                        page++;

                        
                    }
                    
                }
                
                delete(title);
                delete(yes);
                delete(no);
                delete(tb_page);
                delete(tb_slash);
                delete(tb_pages);
                for(UI_elements::TextBox* tbs : messageTextBoxCollection){
                    delete(tbs);
                }
                tft.drawLine(320-xDepth,0,320-xDepth,200,TFT_BLACK);
                tft.drawLine(320-xDepth,yDepth,320,yDepth,TFT_BLACK);

                tft.drawRect(0,0,320,200,TFT_BLACK);
                tft.drawLine(0,25,320,25,TFT_BLACK);
                tft.drawLine(0,240,320,240,TFT_BLACK);
                tft.drawLine(160,200,160,240,TFT_BLACK);
                reDrawAllElements();
                LSC::getInstance().buttons.bt_0.active = true;
                LSC::getInstance().buttons.bt_1.active = true;
                LSC::getInstance().buttons.bt_2.active = true;
                LSC::getInstance().buttons.bt_3.active = true;
                LSC::getInstance().buttons.bt_4.active = true;
                LSC::getInstance().buttons.bt_5.active = true;

                LSC::getInstance().buttons.bt_0.hasBeenClicked();
                LSC::getInstance().buttons.bt_1.hasBeenClicked();
                LSC::getInstance().buttons.bt_2.hasBeenClicked();
                LSC::getInstance().buttons.bt_3.hasBeenClicked();
                LSC::getInstance().buttons.bt_4.hasBeenClicked();
                LSC::getInstance().buttons.bt_5.hasBeenClicked();
                return returnValue;
            }

};


#endif