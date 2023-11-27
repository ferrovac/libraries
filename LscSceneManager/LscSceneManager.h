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
#include <unordered_set>

namespace LinAlg{
    struct Matrix_2x2{
        double mat[2][2]; //[row][element in row]
        Matrix_2x2(double A11,double A12,double A21,double A22){
            mat[0][0] = A11;
            mat[0][1] = A12;
            mat[1][0] = A21;
            mat[1][1] = A22;
        }
        static Matrix_2x2 getRotMat(double Angle){
            return Matrix_2x2(cos(Angle), -1*sin(Angle), sin(Angle), cos(Angle));
        }
    };

    struct Vector_2D{
        int vec[2];
        Vector_2D(int X, int Y){
            vec[0] = X;
            vec[1] = Y;
        }
        Vector_2D operator* (const Matrix_2x2& other) const{
            return Vector_2D((int) round(((double)vec[0] * other.mat[0][0] + (double)vec[1] * other.mat[1][0])), (int)round((double)vec[0] * other.mat[0][1] + (double)vec[1] * other.mat[1][1]));
        }
        Vector_2D operator* (double& other) const{
            return Vector_2D((int)round((double)vec[0] *other), (int)round((double)vec[1]*other) );
        }
        Vector_2D operator+ (Vector_2D& other) const{
            return Vector_2D(vec[0] + other.vec[0], vec[1] + other.vec[1]);
        }
        Vector_2D operator- (Vector_2D& other) const{
            return Vector_2D(vec[0] - other.vec[0], vec[1] - other.vec[1]);
        }
    };
    constexpr double pi = 3.1415926535897932384626433832795028841971693993751058209749445923078164;

}

template <ExposedStateType StateType, typename T>
T* getStateFromDerived(BaseExposedState* basePtr) {
    // Use dynamic_cast to check the actual type of the object
    if (auto derivedPtr = dynamic_cast<ExposedState<StateType, T>*>(basePtr)) {
        // Access the T* state member of the derived class
        return derivedPtr->state;
    } else {
        // Handle the case where the dynamic_cast fails (object is not of the expected type)
        return nullptr;
    }
}

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
        static std::vector<std::vector<BaseUI_element*>> clearLayers;
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
        virtual ~BaseUI_element(){
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
        void (*volatile currentScene)();    //holds a pointer to the currently running scene
        void (*volatile nextScene)();   //holds a pointer to the next scene to be loded. If this is not the same as currentScene a scene switch will be triggered
        static uint32_t backGroundColor;   //holds the global backgroud color of the tft display. This is set in the init function and should not be changed later
        static uint32_t defaultForeGroundColor; //holds the default fore ground color
        static const GFXfont* defaultFont; //holds the defualt font
        static volatile bool systemStableFor20Sec;  
        
        class UI_Options : BaseComponent {
            public:
                uint32_t bColor;
                uint32_t fColor;
                Selection<uint32_t> colorSelection;
                ExposedState<ExposedStateType::ReadWriteSelection, uint32_t> bgc;
                ExposedState<ExposedStateType::ReadWriteSelection, uint32_t> fgc;
                bool debugMode;
                Selection<bool> debugModeSelection;
                ExposedState<ExposedStateType::ReadWriteSelection, bool> debugModePtr; 
            
                UI_Options(uint32_t bcolor, uint32_t fcolor) 
                    :   BaseComponent("System"),
                        bColor(bcolor),
                        fColor(fcolor),
                        colorSelection({{TFT_BLACK,"Black"},{TFT_WHITE, "White"},{TFT_BLUE, "Blue"},{TFT_BROWN, "Brown"}, {TFT_CYAN, "Cyan"},{TFT_DARKCYAN, "Dark Cyan"}}),
                        bgc("Background Color", &bColor,colorSelection),
                        fgc("Foreground Color",&fColor, colorSelection),
                        debugMode(false),
                        debugModeSelection({{false, "User Mode"}, {true, "Debug Mode"}}),
                        debugModePtr("Access Mode", &debugMode, debugModeSelection)
                    {
                }
                void update()  override {
                }
                //Retuns the component type
                String const getComponentType()   {
                    return "Display";
                }
                //Returns the component Name
                const char* const getComponentName() {
                    return "Display";
                }
        };

        UI_Options options;

        //private constructor for singelton pattern
        SceneManager(): options(backGroundColor,defaultForeGroundColor){}
        
        std::vector<String> getComponentListAsString(){
            std::vector<String> retVec;
            waitForSaveReadWrite();
            for(BaseComponent* component : ComponentTracker::getInstance().components){
                retVec.push_back(String(component->componentName));
            }
            return retVec;
        }
        std::vector<BaseExposedState*> getComponentStateListByIndex(uint16_t index){
            std::vector<BaseExposedState*> retVec;
            waitForSaveReadWrite();
            for(std::pair<BaseComponent*,BaseExposedState*> pair : ComponentTracker::getInstance().states){
                if(pair.first == ComponentTracker::getInstance().components[index]){
                retVec.push_back(pair.second);
                }
            }
            return retVec;
        }

        std::vector<String> componentStateListToString(std::vector<BaseExposedState*> list){
            std::vector<String> retVec;
            waitForSaveReadWrite();
            for(BaseExposedState* state: list){
                retVec.push_back(state->stateName);
            }
            return retVec;
        }
        
    public:
        //clears all elements form the screen
        static void clearAllElements(){
            //The ElementTracker holds pointers to all elements we can simply iterate through all elements and clear them
            //This works because all elements inherit form BaseUI_element making clear a mandatory function
            for(BaseUI_element* element: ElementTracker::getInstance().elements){
                element->clear(); //dereference the pointer and clear the element
            }
        }
        static void clearAllElementsLayer(){
            std::vector<BaseUI_element*> newLayer;
            for(BaseUI_element* element: ElementTracker::getInstance().elements){
                bool alreadyInLayer = false;
                for(std::vector<BaseUI_element*> &layers : ElementTracker::getInstance().clearLayers){
                    for(BaseUI_element* &layerElement : layers){
                        if(layerElement == element){
                            alreadyInLayer = true;
                            break;
                        }
                    }
                    if(alreadyInLayer) break; 
                }
                if(!alreadyInLayer){
                    newLayer.push_back(element);
                    element->clear();
                }   
            }
            ElementTracker::getInstance().clearLayers.push_back(newLayer);
        }
        static void reDrawLastLayer(){
            if(ElementTracker::getInstance().clearLayers.empty()) return;
            for(BaseUI_element* element : ElementTracker::getInstance().clearLayers.back()){
                element->reDraw();
            }
            ElementTracker::getInstance().clearLayers.pop_back();
        }


        //re draws all the defined elements on the screen
        static void reDrawAllElements(){
            //See clearAllElements() for implentation
            for(BaseUI_element* element: ElementTracker::getInstance().elements){
                element->reDraw();
            }
        }
        //returns the number of all currently defined elements
        static int getNumberOfElements(){
            waitForSaveReadWrite();
            return ElementTracker::getInstance().elements.size();
        }
        //inizialises the SceneManager setting the first scene and background color
        void init(void (*scene)(), uint32_t BackGroundColor=TFT_BLACK, uint32_t ForeGroundColor=TFT_WHITE, const GFXfont* DefaultFont=FF12){
            backGroundColor = BackGroundColor;
            defaultForeGroundColor = ForeGroundColor;
            defaultFont = DefaultFont;
            tft.init();
            tft.setRotation(3);
            tft.fillScreen(backGroundColor);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setFreeFont(FMB12);     
            currentScene = scene;    
            nextScene = scene;            
        }
        //starts the sceneManager. This will enter an endless loop 
        [[noreturn]] void begin(){
            while(true){
                currentScene(); //execute the scene function
                //We want to clear the state of all buttons when switching scenes
                LSC::getInstance().buttons.bt_0.hasBeenClicked();
                LSC::getInstance().buttons.bt_1.hasBeenClicked();
                LSC::getInstance().buttons.bt_2.hasBeenClicked();
                LSC::getInstance().buttons.bt_3.hasBeenClicked();
                LSC::getInstance().buttons.bt_4.hasBeenClicked();
                LSC::getInstance().buttons.bt_5.hasBeenClicked();
                //load next scene
                currentScene = nextScene; 
            }
        }
        //defines the tft display
        static TFT_eSPI tft;
        //singelton lazy init
        static SceneManager& getInstance() {
            static SceneManager instance;
            return instance;
        }
        //sets the next scene to be loaded if scene is different than the current scene a scene switch will be triggered
        void loadScene(void (*scene)()){
            nextScene = scene;
        }
        //will return false for as long as now new scene has to be loaded. Use this in a while loop in the every scene:
        //while(!sceneManager.switchScene()) 
        bool colorSwitch = false;
        bool switchScene(){
            if(!systemStableFor20Sec && millis() > 20000){
                if (SD.exists("F")){
                    SD.rmdir("F");
                }
                if (SD.exists("FF")){
                    SD.rmdir("FF");
                }
                systemStableFor20Sec = true;
            }
            if(colorSwitch){
                tft.fillScreen(backGroundColor);
                reDrawAllElements();
                colorSwitch = false;
            }
            if(options.bColor != backGroundColor){
                backGroundColor = options.bColor;
                tft.fillScreen(backGroundColor);
                reDrawAllElements();
                colorSwitch = true;
                return true;
            }
            if(options.fColor != defaultForeGroundColor){
                clearAllElements();
                defaultForeGroundColor = options.fColor;
                reDrawAllElements();
                colorSwitch = true;
                return true;
            }

            if (nextScene == currentScene){
                return false;
            }else{
                return true;
            }   
        }
        
        struct ConstructionLine{
            LinAlg::Vector_2D* start;
            LinAlg::Vector_2D* end;

            ConstructionLine(LinAlg::Vector_2D* Start, LinAlg::Vector_2D* End) : start(Start), end(End){}
            void draw(LinAlg::Vector_2D offset, uint32_t Color) const {
                tft.drawLine(start->vec[0] + offset.vec[0] ,start->vec[1] + offset.vec[1] ,end->vec[0]+ offset.vec[0], end->vec[1]+ offset.vec[1],Color);
            }
        };
        struct ConstructionPointCollection{
            std::vector<LinAlg::Vector_2D*> collection;
            void addPoint(LinAlg::Vector_2D* Point){
                collection.push_back(Point);
            }
            void rotate(double Angle){
                LinAlg::Matrix_2x2 rotMat =  LinAlg::Matrix_2x2::getRotMat(Angle);
                for(LinAlg::Vector_2D* point : collection){
                    *point = *point * rotMat;
                }
            }
            void scale(double factor){
                for(LinAlg::Vector_2D* point : collection){
                    *point = *point * factor;
                }
            }
        };
        struct ConstructionLineCollection{
            std::vector<ConstructionLine> collection;
            LinAlg::Vector_2D*  offset;
            ConstructionLineCollection(LinAlg::Vector_2D* Offset): offset(Offset){}

            void addLine(ConstructionLine Line){
                collection.push_back(Line);
            }
            
            void draw(uint32_t Color) const {
                for(ConstructionLine line : collection){
                    line.draw(*offset, Color);
                }
            }
        };


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
            //A TextBox can be used to display text. 
            struct StatusIndicator : BaseUI_element{
                LinAlg::Vector_2D* offset;
                LinAlg::Vector_2D* position;
                bool active;
 
                StatusIndicator(LinAlg::Vector_2D* Position, LinAlg::Vector_2D* Offset = nullptr , bool Active = false): position(Position), offset(Offset), active(Active){
                    reDraw();
                }
                ~StatusIndicator(){
                    clear();
                }
                void setStatus(bool Active){
                    if(Active == active) return;
                    active = Active;
                    reDraw();
                }
                void reDraw() override{
                    if(offset != nullptr){
                        if(active){
                            tft.fillCircle((*offset + *position).vec[0],(*offset + *position).vec[1],5,TFT_GREEN);
                        }else{
                            tft.fillCircle((*offset + *position).vec[0],(*offset + *position).vec[1],5,TFT_RED);
                            tft.fillCircle((*offset + *position).vec[0],(*offset + *position).vec[1],2,backGroundColor);
                            
                        }
                    }else{
                        if(active){
                            tft.fillCircle(position->vec[0],position->vec[1],5,TFT_GREEN);
                        }else{
                            tft.fillCircle(position->vec[0],position->vec[1],5,TFT_RED);
                            tft.fillCircle((*offset + *position).vec[0],(*offset + *position).vec[1],2,backGroundColor);
                        }
                    }
                }
                void clear() const override{
                    tft.fillCircle((*offset + *position).vec[0],(*offset + *position).vec[1],5,backGroundColor);
                }
            };
            

            struct TextBox : BaseUI_element{
                private:
                    uint16_t xPos;  //holds the x position of the element on the tft
                    uint16_t yPos;  //holds the y position of the element on the tft
                    String text;    //holds the text 
                    uint32_t backColour;
                    uint32_t fontColour;
                    const GFXfont* font;
                    
                    //removes a single character from the tft
                    void clearChar(String Text, uint16_t index) const {
                        tft.setFreeFont(font);
                        tft.setTextColor(backColour);
                        //we basically overrite the character on the screen with the exact same char in the background color
                        //by doing this we achive pixel level difference update which is as efficient as we can get
                        //there are a lot of highlevel function calls to string functions, but compared to the time if takes to 
                        //write to the tft its neglectable
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
                                clearChar(text,cleared);
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
                                        clearChar(text,alreadyCleared);

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
                    

                    TextBox(uint16_t xPosition, uint16_t yPosition, String Text="" , const GFXfont* Font=defaultFont , uint32_t FontColour=defaultForeGroundColor , uint32_t BackColour=backGroundColor){
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
                    void setColor(uint32_t Color){
                        if(Color == fontColour) return;
                        uint32_t oldColor = backColour;
                        backColour = Color;
                        update("");
                        backColour = oldColor;
                        fontColour = Color;
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
                    CheckBox(uint16_t xPosition, uint16_t yPosition, uint16_t Size, bool Checked = false,  uint32_t ForeColour = defaultForeGroundColor, uint32_t BackColour = backGroundColor): xPos(xPosition), yPos(yPosition), size(Size),checked(Checked), foreColour(ForeColour), backColour(BackColour) {
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
                    uint16_t xPosition;
                    uint16_t yPosition;
                    uint16_t height;
                    uint16_t width;
                    uint32_t foreColour;
                    uint32_t backColour;
                    long progress;
                    uint16_t progressInPixel;

                public:
                    ProgressBar(uint16_t xPos, uint16_t yPos, uint16_t Height, uint16_t Width, long Progress = 0, uint32_t ForeColor = defaultForeGroundColor, uint32_t BackColor = backGroundColor)
                                :xPosition(xPos), 
                                yPosition(yPos), 
                                height(Height), 
                                width(Width), 
                                progress(0), 
                                foreColour(ForeColor), 
                                backColour(BackColor),
                                progressInPixel(0)
                                {
                        
                        tft.drawRect(xPosition,yPosition, width,height,foreColour);
                        setProgress(Progress);
                    }
                    ~ProgressBar(){
                        clear();
                    }
                    void reDraw(){
                        tft.drawRect(xPosition,yPosition, width,height,foreColour);
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
                                uint16_t fillTo = (uint32_t)map(Progress, 0., 100., 1., (long)(width-1));
                                tft.fillRect(progressInPixel + xPosition  ,yPosition + 1,fillTo - progressInPixel , height - 2,foreColour);
                                progressInPixel = fillTo;
                            }else{
                                uint16_t fillTo = map(Progress, 0, 100, 1., (long)(width-1));
                                tft.fillRect(fillTo + xPosition,yPosition + 1 , progressInPixel - fillTo   , height - 2,backColour);
                                progressInPixel = fillTo;
                            }
                            progress = Progress;
                        }
                    }    
                    void setForeColour(uint32_t Colour){
                        foreColour = Colour;
                        tft.drawRect(xPosition,yPosition, width,height,foreColour);
                        long tempProgress = progress;
                        setProgress(0);
                        setProgress(tempProgress);
                    }
                    void setBackColour(uint32_t Colour){
                        backColour = Colour;
                        tft.drawRect(yPosition,yPosition, width,height,foreColour);
                        long tempProgress = progress;
                        setProgress(100);
                        setProgress(tempProgress);
                    }

                    void clear() const override {
                        tft.fillRect(xPosition,yPosition, width,height,backColour);
                    }          
            };
            struct Line : BaseUI_element{ //TODO: implement pixel lvl update (not sure if it is worth it tho)
                public:
                    Line(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint32_t ForeColour = defaultForeGroundColor, uint32_t BackColour = backGroundColor ): xPosStart(xStart), yPosStart(yStart), xPosEnd(xEnd), yPosEnd(yEnd), foreColor(ForeColour), backColor(BackColour) {
                        tft.drawLine(xPosStart,yPosStart,xPosEnd,yPosEnd,foreColor);
                    }
                    Line(LinAlg::Vector_2D Start, LinAlg::Vector_2D End, uint32_t ForeColour = defaultForeGroundColor, uint32_t BackColour = backGroundColor ): xPosStart(Start.vec[0]), yPosStart(Start.vec[1]), xPosEnd(End.vec[0]), yPosEnd(End.vec[1]), foreColor(ForeColour), backColor(BackColour) {
                        tft.drawLine(xPosStart,yPosStart,xPosEnd,yPosEnd,foreColor);
                    }
                    ~Line(){
                        clear();
                    }
                    void clear() const{
                        tft.drawLine(xPosStart,yPosStart,xPosEnd,yPosEnd,backColor);
                    }
                    void reDraw(){
                        tft.drawLine(xPosStart,yPosStart,xPosEnd,yPosEnd,foreColor);
                    }

                private:
                    uint16_t xPosStart;
                    uint16_t xPosEnd;
                    uint16_t yPosStart;
                    uint16_t yPosEnd;
                    uint32_t foreColor;
                    uint32_t backColor;
            };

            struct Rectangle : BaseUI_element{
                public:
                    Rectangle(uint16_t xPosition, uint16_t yPosition, uint16_t xWidth, uint16_t yWidth, bool Filled = false, uint32_t ForeColour = defaultForeGroundColor, uint32_t BackColour = backGroundColor) : xPos(xPosition),yPos(yPosition), x_width(xWidth), y_width(yWidth), filled(Filled), foreColor(ForeColour), backColor(BackColour) {
                        reDraw();
                    } 
                    ~Rectangle() override {
                        clear();
                    }
                    void clear() const{
                        if(filled){
                            tft.fillRect(xPos,yPos,x_width,y_width,backColor);
                        }else{
                            tft.drawRect(xPos,yPos,x_width,y_width,backColor);
                        }
                    }
                    void reDraw(){
                        if(filled){
                            tft.fillRect(xPos,yPos,x_width,y_width,foreColor);
                        }else{
                            tft.drawRect(xPos,yPos,x_width,y_width,foreColor);
                        }
                    }

                private:
                    uint16_t xPos;
                    uint16_t x_width;
                    uint16_t yPos;
                    uint16_t y_width;
                    uint32_t foreColor;
                    uint32_t backColor;
                    bool filled;

            };


            struct Chart : BaseUI_element{ //TODO: Implement
                public:
                    void clear() const override{

                    }
                private:

            };


            struct Valve : BaseUI_element{
                private:
                    bool open;
                    ConstructionPointCollection pointCollection;
                    ConstructionLineCollection lineCollection;
                    uint32_t lineColor;
                    LinAlg::Vector_2D zeroPoint;
                    LinAlg::Vector_2D upperLeft;
                    LinAlg::Vector_2D upperRight;
                    LinAlg::Vector_2D lowerLeft;
                    LinAlg::Vector_2D lowerRight;
                    LinAlg::Vector_2D leftConnection;
                    LinAlg::Vector_2D leftMidPoint;
                    LinAlg::Vector_2D rightConnection;
                    LinAlg::Vector_2D rightMidPoint;
                    LinAlg::Vector_2D valveActuator;
                    LinAlg::Vector_2D center;
                    StatusIndicator indicator;
                public:
                    Valve(uint16_t xPos, uint16_t yPos, bool Open = false ,double Rotation = 0,double Scale = 1.3, uint32_t LineColor = defaultForeGroundColor) 
                            :zeroPoint(xPos,yPos),
                            open(Open),
                            lineCollection(&zeroPoint),
                            upperLeft(-10,10),
                            upperRight(10,10),
                            lowerLeft(-10,-10),
                            lowerRight(10,-10),
                            leftConnection(-15,0),
                            leftMidPoint(-10,0),
                            rightConnection(15,0),
                            rightMidPoint(10,0),
                            valveActuator(0,10),
                            center(0,0),
                            lineColor(LineColor),
                            indicator(&valveActuator,&zeroPoint)
                            {

                        pointCollection.addPoint(&upperLeft);
                        pointCollection.addPoint(&upperRight);
                        pointCollection.addPoint(&lowerLeft);
                        pointCollection.addPoint(&lowerRight);
                        pointCollection.addPoint(&leftConnection);
                        pointCollection.addPoint(&leftMidPoint);
                        pointCollection.addPoint(&rightConnection);
                        pointCollection.addPoint(&rightMidPoint);
                        pointCollection.addPoint(&valveActuator);
                        pointCollection.addPoint(&center);

                        lineCollection.addLine(ConstructionLine(&upperLeft,&lowerRight));
                        lineCollection.addLine(ConstructionLine(&lowerLeft,&upperRight));
                        lineCollection.addLine(ConstructionLine(&upperLeft,&lowerLeft));
                        lineCollection.addLine(ConstructionLine(&upperRight,&lowerRight));
                        lineCollection.addLine(ConstructionLine(&leftConnection,&leftMidPoint));
                        lineCollection.addLine(ConstructionLine(&rightConnection,&rightMidPoint));
                        lineCollection.addLine(ConstructionLine(&center,&valveActuator));
                        if(Rotation !=0 ) rotate(Rotation);
                        if(Scale !=1 ) scale(Scale);
                        reDraw();
                    }
                    ~Valve(){
                        clear();
                    }
                    void rotate(double Angle){
                        clear();
                        pointCollection.rotate(Angle);
                        reDraw();
                    }
                    void scale(double factor){
                        clear();
                        pointCollection.scale(factor);
                        reDraw();
                    }
                    void setState(bool state){
                        indicator.setStatus(state);
                        open = state;
                    }
                    LinAlg::Vector_2D getLeftConnectionPoint(){
                        return LinAlg::Vector_2D((leftConnection + zeroPoint).vec[0],(leftConnection + zeroPoint).vec[1]);
                    }
                    LinAlg::Vector_2D getRightConnectionPoint(){
                        return LinAlg::Vector_2D((rightConnection + zeroPoint).vec[0],(rightConnection + zeroPoint).vec[1]);;
                    }
                    void reDraw() override{
                        lineCollection.draw(lineColor);
                        indicator.setStatus(open);
                        indicator.reDraw();
                    }
                    void clear() const override{
                        lineCollection.draw(backGroundColor);
                        indicator.clear();
                    }
            };
            struct GateValve : BaseUI_element{
                private:
                    bool open;
                    ConstructionPointCollection pointCollection;
                    ConstructionLineCollection lineCollection;
                    uint32_t lineColor;
                    LinAlg::Vector_2D zeroPoint;
                    LinAlg::Vector_2D upperLeft;
                    LinAlg::Vector_2D upperRight;
                    LinAlg::Vector_2D lowerLeft;
                    LinAlg::Vector_2D lowerRight;
                    LinAlg::Vector_2D leftConnection;
                    LinAlg::Vector_2D leftMidPoint;
                    LinAlg::Vector_2D rightConnection;
                    LinAlg::Vector_2D rightMidPoint;
                    LinAlg::Vector_2D valveActuator;
                    LinAlg::Vector_2D center;
                    LinAlg::Vector_2D gateUL;
                    LinAlg::Vector_2D gateUR;
                    LinAlg::Vector_2D gateLL;
                    LinAlg::Vector_2D gateLR;
                    StatusIndicator indicator;
                public:
                    GateValve(uint16_t xPos, uint16_t yPos, bool Open = false ,double Rotation = 0,double Scale = 1.3, uint32_t LineColor = defaultForeGroundColor) 
                            :zeroPoint(xPos,yPos),
                            open(Open),
                            lineCollection(&zeroPoint),
                            upperLeft(-10,10),
                            upperRight(10,10),
                            lowerLeft(-10,-10),
                            lowerRight(10,-10),
                            leftConnection(-15,0),
                            leftMidPoint(-10,0),
                            rightConnection(15,0),
                            rightMidPoint(10,0),
                            valveActuator(0,10),
                            center(0,0),
                            lineColor(LineColor),
                            gateUL(-2,10),
                            gateUR(2,10),
                            gateLL(-2,-10),
                            gateLR(2,-10),
                            indicator(&valveActuator,&zeroPoint)
                            {

                        pointCollection.addPoint(&upperLeft);
                        pointCollection.addPoint(&upperRight);
                        pointCollection.addPoint(&lowerLeft);
                        pointCollection.addPoint(&lowerRight);
                        pointCollection.addPoint(&leftConnection);
                        pointCollection.addPoint(&leftMidPoint);
                        pointCollection.addPoint(&rightConnection);
                        pointCollection.addPoint(&rightMidPoint);
                        pointCollection.addPoint(&valveActuator);
                        pointCollection.addPoint(&center);
                        pointCollection.addPoint(&gateUL);
                        pointCollection.addPoint(&gateUR);
                        pointCollection.addPoint(&gateLL);
                        pointCollection.addPoint(&gateLR);

                        lineCollection.addLine(ConstructionLine(&upperLeft,&lowerRight));
                        lineCollection.addLine(ConstructionLine(&lowerLeft,&upperRight));
                        lineCollection.addLine(ConstructionLine(&upperLeft,&lowerLeft));
                        lineCollection.addLine(ConstructionLine(&upperRight,&lowerRight));
                        lineCollection.addLine(ConstructionLine(&leftConnection,&leftMidPoint));
                        lineCollection.addLine(ConstructionLine(&rightConnection,&rightMidPoint));
                        lineCollection.addLine(ConstructionLine(&center,&valveActuator));
                        if(Rotation !=0 ) rotate(Rotation);
                        if(Scale !=1 ) scale(Scale);
                        reDraw();
                    }
                    ~GateValve(){
                        clear();
                    }
                    void rotate(double Angle){
                        clear();
                        pointCollection.rotate(Angle);
                        reDraw();
                    }
                    void scale(double factor){
                        clear();
                        pointCollection.scale(factor);
                        reDraw();
                    }
                    void setState(bool state){
                        indicator.setStatus(state);
                        open = state;
                    }
                    LinAlg::Vector_2D getLeftConnectionPoint(){
                        return LinAlg::Vector_2D((leftConnection + zeroPoint).vec[0],(leftConnection + zeroPoint).vec[1]);
                    }
                    LinAlg::Vector_2D getRightConnectionPoint(){
                        return LinAlg::Vector_2D((rightConnection + zeroPoint).vec[0],(rightConnection + zeroPoint).vec[1]);
                    }
                    void reDraw() override{
                        lineCollection.draw(lineColor);
                        tft.fillTriangle(gateLL.vec[0] + zeroPoint.vec[0],gateLL.vec[1] + zeroPoint.vec[1],gateLR.vec[0] + zeroPoint.vec[0],gateLR.vec[1] + zeroPoint.vec[1],gateUL.vec[0] + zeroPoint.vec[0],gateUL.vec[1] + zeroPoint.vec[1],lineColor);
                        tft.fillTriangle(gateUL.vec[0] + zeroPoint.vec[0],gateUL.vec[1] + zeroPoint.vec[1],gateUR.vec[0] + zeroPoint.vec[0],gateUR.vec[1] + zeroPoint.vec[1],gateLR.vec[0] + zeroPoint.vec[0],gateLR.vec[1] + zeroPoint.vec[1],lineColor);
                        indicator.setStatus(open);
                        indicator.reDraw();
                    }
                    void clear() const override{
                        lineCollection.draw(backGroundColor);
                        indicator.clear();
                        tft.fillTriangle(gateLL.vec[0] + zeroPoint.vec[0],gateLL.vec[1] + zeroPoint.vec[1],gateLR.vec[0] + zeroPoint.vec[0],gateLR.vec[1] + zeroPoint.vec[1],gateUL.vec[0] + zeroPoint.vec[0],gateUL.vec[1] + zeroPoint.vec[1],backGroundColor);
                        tft.fillTriangle(gateUL.vec[0] + zeroPoint.vec[0],gateUL.vec[1] + zeroPoint.vec[1],gateUR.vec[0] + zeroPoint.vec[0],gateUR.vec[1] + zeroPoint.vec[1],gateLR.vec[0] + zeroPoint.vec[0],gateLR.vec[1] + zeroPoint.vec[1],backGroundColor);
                    }
            };

            struct TurboMolecularPump : BaseUI_element{
                private:
                    bool state;
                    uint16_t radius;
                    ConstructionPointCollection pointCollection;
                    ConstructionLineCollection lineCollection;
                    uint32_t lineColor;
                    LinAlg::Vector_2D zeroPoint;
                    LinAlg::Vector_2D center;
                    LinAlg::Vector_2D indicatorPos;
                    LinAlg::Vector_2D funnelUL;
                    LinAlg::Vector_2D funnelLL;
                    LinAlg::Vector_2D funnelUR;
                    LinAlg::Vector_2D funnelLR;
                    LinAlg::Vector_2D leftConnection;
                    LinAlg::Vector_2D leftConnectionOnCircle;
                    LinAlg::Vector_2D rightConnection;
                    LinAlg::Vector_2D rightConnectionOnCircle;
                    StatusIndicator indicator;
                public:
                    TurboMolecularPump(uint16_t xPos, uint16_t yPos, bool State = false ,double Rotation = 0,double Scale = 1, uint32_t LineColor = defaultForeGroundColor) 
                            :zeroPoint(xPos,yPos),
                            state(State),
                            radius(25),
                            lineCollection(&zeroPoint),
                            center(0,0),
                            indicatorPos(0,-1*radius/3*2),
                            lineColor(LineColor),
                            funnelUL(0,radius),
                            funnelLL(0,radius),
                            funnelUR(0,radius),
                            funnelLR(0,radius),
                            leftConnection(0,-radius -5),
                            leftConnectionOnCircle(0,-radius),
                            rightConnection(0,radius + 5),
                            rightConnectionOnCircle(0,radius),
                            indicator(&indicatorPos,&zeroPoint)
                            {

                        pointCollection.addPoint(&funnelUL);
                        pointCollection.addPoint(&funnelLL);
                        pointCollection.addPoint(&funnelUR);
                        pointCollection.addPoint(&funnelLR);
                        pointCollection.addPoint(&indicatorPos);
                        pointCollection.addPoint(&leftConnection);
                        pointCollection.addPoint(&leftConnectionOnCircle);
                        pointCollection.addPoint(&rightConnection);
                        pointCollection.addPoint(&rightConnectionOnCircle);

                        funnelLL = funnelLL * LinAlg::Matrix_2x2::getRotMat(3.14159/8);
                        funnelUL = funnelUL * LinAlg::Matrix_2x2::getRotMat(3.14159/1.5);
                        funnelLR = funnelLR * LinAlg::Matrix_2x2::getRotMat(-3.14159/8);
                        funnelUR = funnelUR * LinAlg::Matrix_2x2::getRotMat(-3.14159/1.5);

                       lineCollection.addLine(ConstructionLine(&funnelUL,&funnelLL));
                       lineCollection.addLine(ConstructionLine(&funnelUR,&funnelLR));
                       lineCollection.addLine(ConstructionLine(&leftConnection,&leftConnectionOnCircle));
                       lineCollection.addLine(ConstructionLine(&rightConnection,&rightConnectionOnCircle));
                        
                        if(Rotation !=0 ) rotate(Rotation);
                        if(Scale !=1 ) scale(Scale);
                        rotate(-3.141596/2);
                    }

                    ~TurboMolecularPump(){
                        clear();
                    }
                    LinAlg::Vector_2D getLeftConnectionPoint(){
                        return LinAlg::Vector_2D((leftConnection + zeroPoint).vec[0],(leftConnection + zeroPoint).vec[1]);
                    }
                    LinAlg::Vector_2D getRightConnectionPoint(){
                        return LinAlg::Vector_2D((rightConnection + zeroPoint).vec[0],(rightConnection + zeroPoint).vec[1]);
                    }
                    void rotate(double Angle){
                        clear();
                        pointCollection.rotate(Angle);
                        reDraw();
                    }
                    void scale(double factor){
                        clear();
                        pointCollection.scale(factor);
                        radius = radius * factor;
                        reDraw();
                    }
                    void setState(bool State){
                        indicator.setStatus(State);
                        state = State;
                    }

                    void reDraw() override{
                        tft.drawCircle(center.vec[0] + zeroPoint.vec[0], center.vec[1] + zeroPoint.vec[1], radius, lineColor);
                        tft.drawCircle(center.vec[0] + zeroPoint.vec[0], center.vec[1] + zeroPoint.vec[1], radius/3, lineColor);
                        tft.drawCircle(center.vec[0] + zeroPoint.vec[0], center.vec[1] + zeroPoint.vec[1], radius/3 + 2, lineColor);
                        lineCollection.draw(lineColor);
                        indicator.setStatus(state);
                        indicator.reDraw();
                    }
                    void clear() const override{
                        tft.drawCircle(center.vec[0] + zeroPoint.vec[0], center.vec[1] + zeroPoint.vec[1], radius, backGroundColor);
                        tft.drawCircle(center.vec[0] + zeroPoint.vec[0], center.vec[1] + zeroPoint.vec[1], radius/3, backGroundColor);
                        tft.drawCircle(center.vec[0] + zeroPoint.vec[0], center.vec[1] + zeroPoint.vec[1], radius/3 + 2, backGroundColor);
                        lineCollection.draw(backGroundColor);
                        indicator.clear();
                    }
            };
            struct Pump : BaseUI_element{
                private:
                    bool state;
                    uint16_t radius;
                    ConstructionPointCollection pointCollection;
                    ConstructionLineCollection lineCollection;
                    uint32_t lineColor;
                    LinAlg::Vector_2D zeroPoint;
                    LinAlg::Vector_2D center;
                    LinAlg::Vector_2D indicatorPos;
                    LinAlg::Vector_2D funnelUL;
                    LinAlg::Vector_2D funnelLL;
                    LinAlg::Vector_2D funnelUR;
                    LinAlg::Vector_2D funnelLR;
                    LinAlg::Vector_2D leftConnection;
                    LinAlg::Vector_2D leftConnectionOnCircle;
                    LinAlg::Vector_2D rightConnection;
                    LinAlg::Vector_2D rightConnectionOnCircle;
                    StatusIndicator indicator;
                public:
                    Pump(uint16_t xPos, uint16_t yPos, bool State = false ,double Rotation = 0,double Scale = 1, uint32_t LineColor = defaultForeGroundColor) 
                            :zeroPoint(xPos,yPos),
                            state(State),
                            radius(25),
                            lineCollection(&zeroPoint),
                            center(0,0),
                            indicatorPos(0,-1*radius/3*2),
                            lineColor(LineColor),
                            funnelUL(0,radius),
                            funnelLL(0,radius),
                            funnelUR(0,radius),
                            funnelLR(0,radius),
                            leftConnection(0,-radius -5),
                            leftConnectionOnCircle(0,-radius),
                            rightConnection(0,radius + 5),
                            rightConnectionOnCircle(0,radius),
                            indicator(&indicatorPos,&zeroPoint)
                            {

                        pointCollection.addPoint(&funnelUL);
                        pointCollection.addPoint(&funnelLL);
                        pointCollection.addPoint(&funnelUR);
                        pointCollection.addPoint(&funnelLR);
                        pointCollection.addPoint(&indicatorPos);
                        pointCollection.addPoint(&leftConnection);
                        pointCollection.addPoint(&leftConnectionOnCircle);
                        pointCollection.addPoint(&rightConnection);
                        pointCollection.addPoint(&rightConnectionOnCircle);

                        funnelLL = funnelLL * LinAlg::Matrix_2x2::getRotMat(3.14159/8);
                        funnelUL = funnelUL * LinAlg::Matrix_2x2::getRotMat(3.14159/1.5);
                        funnelLR = funnelLR * LinAlg::Matrix_2x2::getRotMat(-3.14159/8);
                        funnelUR = funnelUR * LinAlg::Matrix_2x2::getRotMat(-3.14159/1.5);

                       lineCollection.addLine(ConstructionLine(&funnelUL,&funnelLL));
                       lineCollection.addLine(ConstructionLine(&funnelUR,&funnelLR));
                       lineCollection.addLine(ConstructionLine(&leftConnection,&leftConnectionOnCircle));
                       lineCollection.addLine(ConstructionLine(&rightConnection,&rightConnectionOnCircle));
                        
                        if(Rotation !=0 ) rotate(Rotation);
                        if(Scale !=1 ) scale(Scale);
                        rotate(-1.* LinAlg::pi/2.);
                    }

                    ~Pump(){
                        clear();
                    }
                    LinAlg::Vector_2D getLeftConnectionPoint(){
                        return LinAlg::Vector_2D((leftConnection + zeroPoint).vec[0],(leftConnection + zeroPoint).vec[1]);
                    }
                    LinAlg::Vector_2D getRightConnectionPoint(){
                        return LinAlg::Vector_2D((rightConnection + zeroPoint).vec[0],(rightConnection + zeroPoint).vec[1]);
                    }
                    void rotate(double Angle){
                        clear();
                        pointCollection.rotate(Angle);
                        reDraw();
                    }
                    void scale(double factor){
                        clear();
                        pointCollection.scale(factor);
                        radius = radius * factor;
                        reDraw();
                    }
                    void setState(bool State){
                        indicator.setStatus(State);
                        state = State;
                    }

                    void reDraw() override{
                        tft.drawCircle(center.vec[0] + zeroPoint.vec[0], center.vec[1] + zeroPoint.vec[1], radius, lineColor);
                        lineCollection.draw(lineColor);
                        indicator.setStatus(state);
                        indicator.reDraw();
                    }
                    void clear() const override{
                        tft.drawCircle(center.vec[0] + zeroPoint.vec[0], center.vec[1] + zeroPoint.vec[1], radius, backGroundColor);
                        lineCollection.draw(backGroundColor);
                        indicator.clear();
                    }
            };
            struct VacuumChamber : BaseUI_element{
                private:
                    ConstructionPointCollection pointCollection;
                    ConstructionLineCollection lineCollection;
                    uint32_t lineColor;
                    LinAlg::Vector_2D zeroPoint;
                    double rotation;
                    double _scale;
                    LinAlg::Vector_2D LeftUpper;
                    LinAlg::Vector_2D LeftLower;
                    LinAlg::Vector_2D RightUpper;
                    LinAlg::Vector_2D RightLower;
                    LinAlg::Vector_2D UpperLeft;
                    LinAlg::Vector_2D UpperRight;
                    LinAlg::Vector_2D LowerLeft;
                    LinAlg::Vector_2D LowerRight;
                    LinAlg::Vector_2D leftMidPoint;
                    LinAlg::Vector_2D leftConnection;
                    LinAlg::Vector_2D rightConnection;
                    LinAlg::Vector_2D rightMidPoint;
                    LinAlg::Vector_2D UpperLeftCorner;
                    LinAlg::Vector_2D UpperRightCorner;
                    LinAlg::Vector_2D LowerLeftCorner;
                    LinAlg::Vector_2D LowerRightCorner;
                    static constexpr int cornerR = 10;

                    double normalizeAngle(double Angle) const{
                        double ret = Angle;
                        int watchdog = 0;
                        while(ret >= LinAlg::pi *2.){
                            ret -= LinAlg::pi*2.;
                            if(watchdog>=10000) {
                                ret = 0;
                                break;
                            }
                        }
                        if(ret<0) ret = 0;
                        watchdog = 0;
                        while(ret < 0){
                            ret += LinAlg::pi*2.;
                            if(watchdog>=10000) {
                                ret = 0;
                                break;
                            }
                        }
                        return ret;
                    }
                    void drawCircleSegment(uint16_t x , uint16_t y, uint16_t r, double startAngle, double endAngle, uint32_t color) const {
                        
                        startAngle = normalizeAngle(startAngle) ;
                        endAngle = normalizeAngle (endAngle);

                        if(endAngle > startAngle){
                            tft.drawArc(x,y,r+1,r, startAngle/LinAlg::pi/2.*360., endAngle/LinAlg::pi/2.*360.,color,backGroundColor,false );
                        }else{
                            tft.drawArc(x,y,r+1,r, startAngle/LinAlg::pi/2.*360., 360.,color,backGroundColor,false );
                            tft.drawArc(x,y,r+1,r, 0., endAngle/LinAlg::pi/2.*360.,color,backGroundColor,false );
                        }           
                    }

                public:
                    VacuumChamber(uint16_t xPos, uint16_t yPos, uint16_t Width, uint16_t Height ,double Rotation = 0,double Scale = 1, uint32_t LineColor = defaultForeGroundColor) 
                            :zeroPoint(xPos,yPos),
                            rotation(0.),
                            _scale(1),
                            lineCollection(&zeroPoint),
                            LeftUpper(-1*Width/2 , Height/2 - cornerR),
                            LeftLower(-1*Width/2 , -1 * Height/2 + cornerR),
                            RightUpper(Width/2 , Height/2 - cornerR),
                            RightLower(Width/2 , -1 * Height/2 + cornerR),
                            LowerLeft(-1*Width/2 + cornerR,-1*Height/2),
                            LowerRight(Width/2 - cornerR,-1*Height/2),
                            UpperLeft(-1 * Width/2 +cornerR,Height/2),
                            UpperRight(Width/2 - cornerR,Height/2),
                            leftMidPoint(-1*Width/2,0),
                            leftConnection(-1 * Width/2 -5,0),
                            rightConnection(Width/2+5,0),
                            rightMidPoint(Width/2,0),
                            UpperLeftCorner(-1*Width/2 +cornerR , Height/2 - cornerR),
                            UpperRightCorner(Width/2 - cornerR, Height/2 - cornerR),
                            LowerLeftCorner(-1*Width/2 + cornerR,-1*Height/2 + cornerR),
                            LowerRightCorner(Width/2 - cornerR,-1*Height/2 + cornerR),
                            lineColor(LineColor)
                            {

                        pointCollection.addPoint(&LeftUpper);
                        pointCollection.addPoint(&LeftLower);
                        pointCollection.addPoint(&RightUpper);
                        pointCollection.addPoint(&RightLower);
                        pointCollection.addPoint(&UpperLeft);
                        pointCollection.addPoint(&UpperRight);
                        pointCollection.addPoint(&LowerLeft);
                        pointCollection.addPoint(&LowerRight);
                        pointCollection.addPoint(&UpperLeftCorner);
                        pointCollection.addPoint(&UpperRightCorner);
                        pointCollection.addPoint(&LowerLeftCorner);
                        pointCollection.addPoint(&LowerRightCorner);

                        pointCollection.addPoint(&leftConnection);
                        pointCollection.addPoint(&leftMidPoint);
                        pointCollection.addPoint(&rightConnection);
                        pointCollection.addPoint(&rightMidPoint);


                        lineCollection.addLine(ConstructionLine(&UpperLeft,&UpperRight));
                        lineCollection.addLine(ConstructionLine(&LowerLeft,&LowerRight));
                        lineCollection.addLine(ConstructionLine(&LeftLower,&LeftUpper));
                        lineCollection.addLine(ConstructionLine(&RightLower,&RightUpper));

                        //lineCollection.addLine(ConstructionLine(&UpperLeftCorner,&LowerRightCorner));
                        //lineCollection.addLine(ConstructionLine(&UpperRightCorner,&LowerLeftCorner));


                        lineCollection.addLine(ConstructionLine(&leftConnection,&leftMidPoint));
                        lineCollection.addLine(ConstructionLine(&rightConnection,&rightMidPoint));
                        if(Rotation !=0 ) rotate(Rotation);
                        if(Scale !=1 ) scale(Scale);
                        reDraw();
                    }
                    ~VacuumChamber(){
                        clear();
                    }
                    void rotate(double Angle){
                        clear();
                        pointCollection.rotate(Angle);
                        rotation -= Angle;
                        while(rotation<0){
                            rotation += LinAlg::pi*2.;
                        }
                        while(rotation>=LinAlg::pi*2.){
                            rotation -= LinAlg::pi*2.;
                        }
                        reDraw();
                    }
                    void scale(double factor){
                        clear();
                        _scale *= factor;
                        pointCollection.scale(factor);
                        reDraw();
                    }

                    LinAlg::Vector_2D getLeftConnectionPoint(){
                        return LinAlg::Vector_2D((leftConnection + zeroPoint).vec[0],(leftConnection + zeroPoint).vec[1]);
                    }
                    LinAlg::Vector_2D getRightConnectionPoint(){
                        return LinAlg::Vector_2D((rightConnection + zeroPoint).vec[0],(rightConnection + zeroPoint).vec[1]);
                    }
                    void reDraw() override{
                        lineCollection.draw(lineColor);
                        drawCircleSegment(zeroPoint.vec[0] + UpperLeftCorner.vec[0],zeroPoint.vec[1] +UpperLeftCorner.vec[1],cornerR * _scale,rotation,rotation+ LinAlg::pi/2.,lineColor);
                        drawCircleSegment(zeroPoint.vec[0] + UpperRightCorner.vec[0],zeroPoint.vec[1] +UpperRightCorner.vec[1],cornerR * _scale,rotation + LinAlg::pi*3./2.,rotation,lineColor);
                        drawCircleSegment(zeroPoint.vec[0] + LowerLeftCorner.vec[0],zeroPoint.vec[1] +LowerLeftCorner.vec[1],cornerR * _scale,rotation + LinAlg::pi/2.,rotation + LinAlg::pi,lineColor);
                        drawCircleSegment(zeroPoint.vec[0] + LowerRightCorner.vec[0],zeroPoint.vec[1] +LowerRightCorner.vec[1],cornerR * _scale,rotation + LinAlg::pi,rotation+ LinAlg::pi*3./2.,lineColor);

                   }
                    void clear() const override{
                        lineCollection.draw(backGroundColor);
                        drawCircleSegment(zeroPoint.vec[0] + UpperLeftCorner.vec[0],zeroPoint.vec[1] +UpperLeftCorner.vec[1],cornerR * _scale,rotation,rotation+ LinAlg::pi/2.,backGroundColor);
                        drawCircleSegment(zeroPoint.vec[0] + UpperRightCorner.vec[0],zeroPoint.vec[1] +UpperRightCorner.vec[1],cornerR * _scale,rotation + LinAlg::pi*3./2.,rotation,backGroundColor);
                        drawCircleSegment(zeroPoint.vec[0] + LowerLeftCorner.vec[0],zeroPoint.vec[1] +LowerLeftCorner.vec[1],cornerR * _scale,rotation + LinAlg::pi/2.,rotation + LinAlg::pi,backGroundColor);
                        drawCircleSegment(zeroPoint.vec[0] + LowerRightCorner.vec[0],zeroPoint.vec[1] +LowerRightCorner.vec[1],cornerR * _scale,rotation + LinAlg::pi,rotation+ LinAlg::pi*3./2.,backGroundColor);
                    }
            };


        };
        
        struct StandardMenu : BaseUI_element{
            public:
                StandardMenu(String Title, String LeftOption, String RightOption, uint32_t TitleColor = defaultForeGroundColor, uint32_t LeftOptionColor = defaultForeGroundColor, uint32_t RightOptionColor = defaultForeGroundColor, uint32_t LineColor = defaultForeGroundColor, const GFXfont* Font = FMB12): titleColor(TitleColor), leftOptionColor(LeftOptionColor), rightOptionColor(RightOptionColor),lineColor(LineColor),font(Font){
                    rightControllDrawn = false;
                    tft.drawRect(0, 0, 320 - 19, 200, LineColor); //Big Box
                    tft.drawRect(1, 1, 320 - 21, 24, LineColor);  //Title Box
                    tft.drawLine(1, 25,320 - 20, 25, LineColor);   //Title Box under Line fat
                    tft.drawLine(160,200,160,240,LineColor);    //yes / no separator
                    int yDepth = 140;
                    int xDepth = 19;
                    tft.drawLine(320 - xDepth, yDepth, 320, yDepth, LineColor);
                    tft.drawLine(320 - xDepth, 199, 320, 199, LineColor);
                    tft.drawLine(319 , yDepth, 319, 199, LineColor);



                    tft.setFreeFont(font);
                    title = new UI_elements::TextBox((301 - tft.textWidth(Title)) / 2, 20 - (25 - tft.fontHeight()) / 2, Title, font, titleColor);
                    
                    leftOption = new UI_elements::TextBox(80 - tft.textWidth(LeftOption) / 2, 234 - (40 - tft.fontHeight()) / 2, LeftOption, font, leftOptionColor);
                    rightOption = new UI_elements::TextBox(240 - tft.textWidth(RightOption) / 2, 234 - (40 - tft.fontHeight()) / 2, RightOption, font, rightOptionColor);
                    int tbPageCounterYpos = 159;
                    page = new UI_elements::TextBox(305,tbPageCounterYpos,"1", FM9,lineColor);
                    slash = new UI_elements::TextBox(305,tbPageCounterYpos + 15,"/", FM9,lineColor);
                    pages = new UI_elements::TextBox(305,tbPageCounterYpos + 30,"1", FM9,lineColor);
                }
                void reDraw(){
                    title->reDraw();
                    leftOption->reDraw();
                    rightOption->reDraw();
                    pages->reDraw();
                    slash->reDraw();
                    tft.drawRect(0, 0, 320 - 19, 200, lineColor); //Big Box
                    tft.drawRect(1, 1, 320 - 21, 24, lineColor);  //Title Box
                    tft.drawLine(1, 25,320 - 20, 25, lineColor);   //Title Box under Line fat
                    tft.drawLine(160,200,160,240,lineColor);    //yes / no separator
                    int yDepth = 140;
                    int xDepth = 19;
                    tft.drawLine(320 - xDepth, yDepth, 320, yDepth, lineColor);
                    tft.drawLine(320 - xDepth, 199, 320, 199, lineColor);
                    tft.drawLine(319 , yDepth, 319, 199, lineColor);
                    if(rightControllDrawn){
                        // Lower triabgle
                        tft.drawLine(320 - xDepth + 1, yDepth - 25, 320 - (xDepth / 2), yDepth, lineColor);
                        tft.drawLine(320 - 1, yDepth - 25, 320 - (xDepth / 2), yDepth, lineColor);
                        // upe driangle
                        tft.drawLine(320 - xDepth + 1, 25, 320 - (xDepth / 2), 0, lineColor);
                        tft.drawLine(320 - 1, 25, 320 - (xDepth / 2), 0, lineColor);
                    }
                }
                void clear() const {
                    title->clear();
                    leftOption->clear();
                    rightOption->clear();
                    pages->clear();
                    slash->clear();
                    tft.drawRect(0, 0, 320 - 19, 200, backGroundColor); //Big Box
                    tft.drawRect(1, 1, 320 - 21, 24, backGroundColor);  //Title Box
                    tft.drawLine(1, 25,320 - 20, 25, backGroundColor);   //Title Box under Line fat
                    tft.drawLine(160,200,160,240,backGroundColor);    //yes / no separator
                    int yDepth = 140;
                    int xDepth = 19;
                    tft.drawLine(320 - xDepth, yDepth, 320, yDepth, backGroundColor);
                    tft.drawLine(320 - xDepth, 199, 320, 199, backGroundColor);
                    tft.drawLine(319 , yDepth, 319, 199, backGroundColor);
                    if(rightControllDrawn){
                        // Lower triabgle
                        tft.drawLine(320 - xDepth + 1, yDepth - 25, 320 - (xDepth / 2), yDepth, backGroundColor);
                        tft.drawLine(320 - 1, yDepth - 25, 320 - (xDepth / 2), yDepth, backGroundColor);
                        // upe driangle
                        tft.drawLine(320 - xDepth + 1, 25, 320 - (xDepth / 2), 0, backGroundColor);
                        tft.drawLine(320 - 1, 25, 320 - (xDepth / 2), 0, backGroundColor);
                    }
                }

                ~StandardMenu(){
                    clear();
                    delete(title);
                    delete(leftOption);
                    delete(rightOption);
                    delete(pages);
                    delete(page);
                    delete(slash);

                }

                void drawRightControll(){
                    if(!rightControllDrawn){
                        int yDepth = 140;
                        int xDepth = 19;
                        // Lower triabgle
                        tft.drawLine(320 - xDepth + 1, yDepth - 25, 320 - (xDepth / 2), yDepth, lineColor);
                        tft.drawLine(320 - 1, yDepth - 25, 320 - (xDepth / 2), yDepth, lineColor);
                        // upe driangle
                        tft.drawLine(320 - xDepth + 1, 25, 320 - (xDepth / 2), 0, lineColor);
                        tft.drawLine(320 - 1, 25, 320 - (xDepth / 2), 0, lineColor);
                        rightControllDrawn = true;
                    }
                }
                void setCurrentPageNumber(uint16_t number){
                    page->setText(String(number));
                }
                void setOfPagesNumber(uint16_t number){
                    pages->setText(String(number));
                }
                void setTitle(String Title){
                    delete(title);
                    tft.setFreeFont(font);
                    title = new UI_elements::TextBox((301 - tft.textWidth(Title)) / 2, 20 - (25 - tft.fontHeight()) / 2, Title, font, titleColor);
                    tft.drawLine(1, 25,320 - 20, 25, lineColor);
                    tft.drawLine(1, 24,320 - 20, 24, lineColor);
                }

            private:
                UI_elements::TextBox* title;
                UI_elements::TextBox* leftOption;
                UI_elements::TextBox* rightOption;
                UI_elements::TextBox* page;
                UI_elements::TextBox* pages;
                UI_elements::TextBox* slash;
                uint32_t titleColor;
                uint32_t leftOptionColor;
                uint32_t rightOptionColor;
                uint32_t lineColor;
                const GFXfont* font;
                bool rightControllDrawn;


        };
        struct SelectionBox{
            private:
                StandardMenu *menuFramePtr;
                std::deque<String> linesUnderScreen;
                std::deque<String> linesOverScreen;
                std::vector<UI_elements::TextBox *> messageTextBoxCollection;
                std::vector<UI_elements::TextBox *> arrowCollection;
                int maxLinesOnScreen;
                int pages;
                bool tooManyLinesForScreen;
                int numberOfLines;
                String title;
                std::vector<String> options;
                uint32_t titleColor;
                uint32_t textColor;
                uint32_t optionFalseColor;
                uint32_t optionTrueColor;
                uint32_t lineColor;
                const GFXfont* titleFont;
                const GFXfont* textFont;
                int selectedItem;
                int page;

                void createTextBoxList(){
                    for(int i = 0; i < maxLinesOnScreen; i++){
                        messageTextBoxCollection.push_back(new UI_elements::TextBox(5+tft.fontHeight(),45+i*tft.fontHeight(), "",textFont,textColor));
                        arrowCollection.push_back(new UI_elements::TextBox(5,45+i*tft.fontHeight(), "",textFont,textColor));
                    }
                }
                void destroyTextBoxList(){
                    for(UI_elements::TextBox* tbs : arrowCollection){
                        delete(tbs);
                    }
                    for(UI_elements::TextBox* tbs : messageTextBoxCollection){
                        delete(tbs);
                    }
                }



            public:
                SelectionBox(String Title, std::vector<String> Options ,int SelectedItem = 0, uint32_t TitleColor = defaultForeGroundColor, uint32_t TextColor = defaultForeGroundColor, uint32_t OptionFalseColor = defaultForeGroundColor, uint32_t OptionTrueColor = defaultForeGroundColor, uint32_t LineColor = defaultForeGroundColor, const GFXfont* TitleFont = FMB12, const GFXfont* TextFont = FM9)
                :   title(Title),
                    selectedItem(SelectedItem),
                    titleColor(TitleColor), 
                    textColor(TextColor), 
                    optionFalseColor(OptionFalseColor), 
                    optionTrueColor(OptionTrueColor), 
                    lineColor(LineColor),
                    titleFont(TitleFont),
                    textFont(TextFont)
                {
                    // first we disable all butttons we dont want buttion handlers to be executed while the textbox is shown
                    LSC::getInstance().buttons.bt_0.active = false;
                    LSC::getInstance().buttons.bt_1.active = false;
                    LSC::getInstance().buttons.bt_2.active = false;
                    LSC::getInstance().buttons.bt_3.active = false;
                    LSC::getInstance().buttons.bt_4.active = false;
                    LSC::getInstance().buttons.bt_5.active = false;

                    // reset button 2 and 5 (yes / no button)
                    LSC::getInstance().buttons.bt_2.hasBeenClicked();
                    LSC::getInstance().buttons.bt_5.hasBeenClicked();
                    //clearAllElements(); // clear everything on the screen
                    menuFramePtr = new StandardMenu(Title, "Back", "Select", TitleColor, OptionFalseColor, OptionTrueColor, LineColor, TitleFont);
                    maxLinesOnScreen = 174 / tft.fontHeight(); //there will be one more line on the screen then this number indecates... because of reasons
                    pages = Options.size()/maxLinesOnScreen;
                    tooManyLinesForScreen = false;
                    numberOfLines = 0;
                    menuFramePtr->drawRightControll();
                    createTextBoxList();
                    loadList(Options);
                    arrowCollection[selectedItem]->setText(">");
                    menuFramePtr->setOfPagesNumber(pages);
                }
                void loadList(std::vector<String> list){
                    if(options != list){
                        linesUnderScreen.clear();
                        linesOverScreen.clear();

                        tooManyLinesForScreen = false;
                        numberOfLines = 0;
                        for(String selectionLine : list){
                            if(!tooManyLinesForScreen) messageTextBoxCollection[numberOfLines]->setText(selectionLine);// .push_back(new UI_elements::TextBox(5+tft.fontHeight(),45+numberOfLines*tft.fontHeight(), selectionLine,textFont,textColor));    
                            if( tooManyLinesForScreen) linesUnderScreen.push_back(selectionLine);
                            numberOfLines++;
                            if(numberOfLines >= maxLinesOnScreen) tooManyLinesForScreen = true;   
                        }
                        for(int i = list.size(); i < maxLinesOnScreen; i++){
                            messageTextBoxCollection[i]->setText("");
                        }
                        tft.setFreeFont(textFont);
                        int selectedItem = 0;
                        
                    }
                }
                void setTitle(String Title){
                    menuFramePtr->setTitle(Title);
                }
                void setSelectedIndex(int index){
                    if(index < maxLinesOnScreen && index >= 0){
                        selectedItem = index;
                        update();
                    }
                }
                void setColorOfItemByIndex(int index, uint32_t Color){
                    if(index > maxLinesOnScreen || index < 0) return;
                    messageTextBoxCollection[index]->setColor(Color);
                }
                void setColorOfAllItems(uint32_t Color){
                    for(UI_elements::TextBox* tb : messageTextBoxCollection){
                        tb->setColor(Color);
                    }
                }
                int getSelectedIndex(){
                    return selectedItem;
                }
                bool selectHasBeenClicked(){
                    return LSC::getInstance().buttons.bt_5.hasBeenClicked();
                }
                bool backHasBeenClicked(){
                    return LSC::getInstance().buttons.bt_2.hasBeenClicked();
                }
                void update(){
                    if(LSC::getInstance().buttons.bt_3.hasBeenClicked() && selectedItem > 0){
                        selectedItem--;

                    }
                    if(LSC::getInstance().buttons.bt_4.hasBeenClicked() && selectedItem < numberOfLines -1){
                        selectedItem++;
                        /*
                        if(selectedItem > (page+1) * maxLinesOnScreen){
                        int linesToPrint = linesUnderScreen.size();
                        menuFramePtr->setCurrentPageNumber(page+1);
                        for(int32_t i = 0; i < messageTextBoxCollection.size(); i++){
                            linesOverScreen.push_back(messageTextBoxCollection[i]->getText());
                            if(i > linesToPrint) continue;
                            messageTextBoxCollection[i]->setText(linesUnderScreen.front());
                            linesUnderScreen.pop_front();
                        }
                        page++;
                        }
                        */
                    }
                    int counter = 0;
                        for(UI_elements::TextBox* tb : arrowCollection){
                            if(counter == selectedItem){
                                tb->setText(">");
                            }else{
                                tb->setText("");
                            }
                            counter++;
                        }
                }
    

            ~SelectionBox(){
                delete(menuFramePtr);
                destroyTextBoxList();
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
            }

        };

        
        void showConfigMenu(){
            clearAllElementsLayer();
            std::vector<String> componentListString = getComponentListAsString();
            std::vector<BaseExposedState*> exposedStateList;
            SelectionBox* selectionBox = new SelectionBox("Components",componentListString);
            int menuLevel = 0;
            int selectionOnMenuLevel_0 = 0;
            int selectionOnMenuLevel_1 = 0;
            int selectionOnMenuLevel_2 = 0;
            
            
            while(true){
                waitForSaveReadWrite();
                if(menuLevel == 0){
                    selectionBox->update();
                    if(selectionBox->backHasBeenClicked()) break;
                    if(selectionBox->selectHasBeenClicked()){
                        selectionOnMenuLevel_0 = selectionBox->getSelectedIndex();
                        selectionBox->setTitle(componentListString[selectionOnMenuLevel_0]);
                        exposedStateList = getComponentStateListByIndex(selectionOnMenuLevel_0);
                        selectionBox->loadList(componentStateListToString(exposedStateList));
                        selectionBox->setSelectedIndex(0);
                        menuLevel++;
                    }
                }
                if(menuLevel == 1){
                    selectionBox->update();
                    if(selectionBox->backHasBeenClicked()){
                        waitForSaveReadWrite();
                        selectionBox->setTitle("Components");
                        selectionBox->loadList(componentListString);
                        selectionBox->setSelectedIndex(selectionOnMenuLevel_0);
                        menuLevel--;
                        LSC::getInstance().buttons.bt_5.hasBeenClicked();
                    } 
                    if(selectionBox->selectHasBeenClicked()){
                        waitForSaveReadWrite();
                        selectionOnMenuLevel_1 = selectionBox->getSelectedIndex();
                        selectionBox->setTitle(componentStateListToString(exposedStateList)[selectionOnMenuLevel_1]);
                        // --- ReadWriteSelection ---
                        if(exposedStateList[selectionOnMenuLevel_1]->stateType  == ExposedStateType::ReadWriteSelection){
                            auto myPtr = static_cast<ExposedState<ExposedStateType::ReadWriteSelection, void*>*>(exposedStateList[selectionOnMenuLevel_1]);
                            ExposedStateInterface stateInterface(exposedStateList[selectionOnMenuLevel_1]);
                            std::vector<String> tempBuf;
                            for(const char* item : stateInterface.getOptions()){
                                tempBuf.push_back(String(item));
                            }
                            
                            selectionBox->loadList(tempBuf);
                            int indexOfCurrentSetting = stateInterface.getStateValue<int>();
                           
                            selectionBox->setSelectedIndex(indexOfCurrentSetting);
                            selectionBox->setColorOfItemByIndex(indexOfCurrentSetting,TFT_GREEN);
                            
                            while(true){
                                waitForSaveReadWrite();
                                selectionBox->update();
                                if(selectionBox->selectHasBeenClicked()){ //One onf the selection options has been chosen
                                    waitForSaveReadWrite();
        
                                    stateInterface.setStateValue(selectionBox->getSelectedIndex());
                                    selectionBox->setColorOfAllItems(defaultForeGroundColor);
                                    selectionBox->setColorOfItemByIndex(selectionBox->getSelectedIndex(), TFT_GREEN);
                                    stateInterface.saveState();
                                }
                                if(selectionBox->backHasBeenClicked()){ // Go back to menu level 1
                                    selectionBox->setTitle(componentListString[selectionOnMenuLevel_0]);
                                    selectionBox->loadList(componentStateListToString(exposedStateList));
                                    selectionBox->setSelectedIndex(selectionOnMenuLevel_1);
                                    selectionBox->setColorOfAllItems(defaultForeGroundColor);
                                    LSC::getInstance().buttons.bt_5.hasBeenClicked();
                                    break;
                                }

                            }
                            
                        }
                        ExposedStateInterface stateInterface(exposedStateList[selectionOnMenuLevel_1]);
                        if(exposedStateList[selectionOnMenuLevel_1]->stateType  == ExposedStateType::ReadOnly){                            
                            while(true){
                                waitForSaveReadWrite();
                                selectionBox->loadList({"ReadOnly State:",stateInterface.getStateValueAsString()});
                                selectionBox->setColorOfItemByIndex(1,TFT_GREEN);
                                selectionBox->update();
                                if(selectionBox->backHasBeenClicked()){ // Go back to menu level 1
                                    selectionBox->setTitle(componentListString[selectionOnMenuLevel_0]);
                                    selectionBox->loadList(componentStateListToString(exposedStateList));
                                    selectionBox->setSelectedIndex(selectionOnMenuLevel_1);
                                    selectionBox->setColorOfAllItems(defaultForeGroundColor);
                                    LSC::getInstance().buttons.bt_5.hasBeenClicked();
                                    break;
                                }

                            }
                        }
                        if(stateInterface.getStateType() == ExposedStateType::Action){
                            waitForSaveReadWrite();
                            if(!options.debugMode){
                                showMessageBox("Not In Debug Mode", "Switch to debug mode to change the state of the system!","","ok");
                            }else{
                                stateInterface.executeAction();
                            }
                            
                        }

                    }
                }
                if(menuLevel == 2){
                    waitForSaveReadWrite();
                    selectionBox->update();
                    if(selectionBox->backHasBeenClicked()){
                        selectionBox->setTitle(componentListString[selectionOnMenuLevel_0]);
                        selectionBox->loadList(componentStateListToString(exposedStateList));
                        selectionBox->setSelectedIndex(selectionOnMenuLevel_1);
                        LSC::getInstance().buttons.bt_5.hasBeenClicked();
                    menuLevel--;
                    }


                }

            }
            
            delete(selectionBox);
            reDrawLastLayer();
        }

        
        static bool showMessageBox(String Title, String Message, String OptionFalse="NO", String OptionTrue="YES", uint32_t TitleColor = defaultForeGroundColor, uint32_t TextColor = defaultForeGroundColor, uint32_t OptionFalseColor = defaultForeGroundColor, uint32_t OptionTrueColor = defaultForeGroundColor, uint32_t LineColor = defaultForeGroundColor, const GFXfont* TitleFont = FMB12, const GFXfont* TextFont = FM9){
               //first we disable all butttons we dont want buttion handlers to be executed while the textbox is shown
                LSC::getInstance().buttons.bt_0.active = false;
                LSC::getInstance().buttons.bt_1.active = false;
                LSC::getInstance().buttons.bt_2.active = false;
                LSC::getInstance().buttons.bt_3.active = false;
                LSC::getInstance().buttons.bt_4.active = false;
                LSC::getInstance().buttons.bt_5.active = false;


                bool returnValue; //the value the textbox will return in the end depends on user choice
                //reset button 2 and 5 (yes / no button)
                LSC::getInstance().buttons.bt_2.hasBeenClicked();
                LSC::getInstance().buttons.bt_5.hasBeenClicked();

                clearAllElementsLayer(); //clear everything on the screen
                StandardMenu* menuFramePtr = new StandardMenu(Title,OptionFalse,OptionTrue,TitleColor,OptionFalseColor,OptionTrueColor,LineColor,TitleFont);
                
                std::deque<String> linesUnderScreen;
                std::deque<String> linesOverScreen;

                std::vector<UI_elements::TextBox*> messageTextBoxCollection;
                int lastSpace = 0;
                int lastLineFeed = -1;
                int index = 0;
                int numberOfLines = 0;
                
                bool tooManyLinesForScreen = false;

                tft.setFreeFont(TextFont);
                int maxLinesOnScreen = 174 / tft.fontHeight(); //there will be one more line on the screen then this number indecates... because of reasons
                int pages = 0;
                int startTime = millis();

                for(char character : Message){
                    if(character == ' ') lastSpace = index;
                    if(tft.textWidth(Message.substring(lastLineFeed + 1,index )) > 280){

                        if(!tooManyLinesForScreen) messageTextBoxCollection.push_back(new UI_elements::TextBox(5,45+numberOfLines*tft.fontHeight(),Message.substring(lastLineFeed + 1,lastSpace),TextFont,TextColor));
                        
                        if( tooManyLinesForScreen) linesUnderScreen.push_back(Message.substring(lastLineFeed + 1,lastSpace));
                        lastLineFeed = lastSpace ;
                        numberOfLines++;
                        if(numberOfLines >= maxLinesOnScreen) tooManyLinesForScreen = true;
                    }
                    index++;
                }

                if((lastLineFeed + 1 < Message.length())){
                    tft.setFreeFont(TextFont);
                    if(!tooManyLinesForScreen) messageTextBoxCollection.push_back(new UI_elements::TextBox(5,45+numberOfLines*tft.fontHeight(),Message.substring(lastLineFeed + 1,Message.length()),TextFont,TextColor));
                    if( tooManyLinesForScreen) linesUnderScreen.push_back(Message.substring(lastLineFeed + 1,Message.length()));
                    numberOfLines++;
                }
                int addedNumberOfLinesForPadding = 0;

                    if((numberOfLines % maxLinesOnScreen) != 0){
                        for(int k = 0; k < maxLinesOnScreen-(numberOfLines % maxLinesOnScreen); k++){
                            linesUnderScreen.push_back("");
                            addedNumberOfLinesForPadding++;
                        }
                    }

                numberOfLines += addedNumberOfLinesForPadding;
                pages = numberOfLines / maxLinesOnScreen;

                int page = 0;
                int yDepth = 140;
                int xDepth = 19;
                bool trianglesAlreadyDrawn = false;

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
                    int triangleSize = 25;
                    if(!trianglesAlreadyDrawn){
                        menuFramePtr->drawRightControll();
                    }

                    menuFramePtr->setCurrentPageNumber(page+1);
                    menuFramePtr->setOfPagesNumber(pages);

                    if(LSC::getInstance().buttons.bt_3.hasBeenClicked() && page > 0){
                        menuFramePtr->setCurrentPageNumber(page+1);                       
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
                        menuFramePtr->setCurrentPageNumber(page+1);
                        for(int32_t i = 0; i < messageTextBoxCollection.size(); i++){
                            linesOverScreen.push_back(messageTextBoxCollection[i]->getText());
                            if(i > linesToPrint) continue;
                            messageTextBoxCollection[i]->setText(linesUnderScreen.front());
                            linesUnderScreen.pop_front();
                        }
                        page++;

                        
                    }
                    
                }
                
                delete(menuFramePtr);

                for(UI_elements::TextBox* tbs : messageTextBoxCollection){
                    delete(tbs);
                }
                
                reDrawLastLayer();
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


enum struct RulePolicy{
  Deny,
  DenyInform,
  Ask,
  AllowInform
};


extern void ruleList();

class Rules{
    private:
        static volatile bool allowed;    

    public:
        Rules(){}
        static void check(bool eval, RulePolicy policy){
            switch(policy){
                case RulePolicy::Deny:
                    allowed = false;
                    break;
                case RulePolicy::DenyInform:
                    SceneManager::showMessageBox("asdf","asf");

            }
        }
        static void checkRuleList(){

        }

};

#endif