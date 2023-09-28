/*
AUTHOR:       Tobias Hofmänner
DATE:         14.07.2023
USAGE:        TODO
DESCRIPTION:  This class provides objects that represent the hardware of the LSC device. It is written in an application agnostic way, i.e. the class stops at the hardware level with abstraction.
              Simply put: All periferals of the lsc are available in a high level representation but there is no Specific logic in place that represents a specific system. And there are no functions like
               vent, or openValve.
TODO:       Optimize
RECOURCES:  TIMER: TC3-0 FOR: struct Beeper  
*/

#ifndef LSC_H
#define LSC_H

#include <Arduino.h>

#include "LscError.h"
#define ERROR_HANDLER ErrorHandler::getInstance() // macro for the ErrorHandler singleton
#define BEEPER Beeper::getInstance() // macro for the ErrorHandler singleton

//##################################################
//########## INPUT/OUTPUT TYPE DEFINITION ##########
//##################################################
//We provide a struct for each peripheral pin of the LSC which contains all necessary functions to access the pins of the D-SUB connectors.
//The assignment- and equality operator is overloaded, i.e. one can set the state of a pin simply by assigning a value with "=" 

//::::: BASE STRUCTS :::::
/* We have a lot of similar input/output options. Here we define the base Struct for:
  - Digital IN
  - Digital OUT 
  - Analog Input
  - Analog Output
  If we want to implement a specific peripheral pin, we can inherit from these Structs.
  These themplates represent the accutal arduino board. 
  All of the Structs contain a member called description this is a string which always contains a string that expains in human readable form which physical hardware the object is using.
  This is important for error handling. As en example if there is a overvoltage condition on one of the inputs the user needs to understand what he has to disconnect.
*/



//===== DIGITAL OUT =====

// Structure representing a physical arduino pin configured as output
struct DigitalOutBase { 
  private:
    uint8_t arduinoPin; // the physical Arduino Pin
  protected:
    String description; 
  public:
    //--- CONSTRUCTOR ---

    //Arduino PIN 
    DigitalOutBase(uint8_t arduinoPin):arduinoPin(arduinoPin), description("Digital Out ArduinoPin: " + String(arduinoPin)){
      pinMode(arduinoPin, OUTPUT);
    }
    //--- PUBLIC VARS ---
    bool state;
    //--- PUBLIC FUNCTIONS ---

    //Update the state of the Output
    void get() {                  
      state = digitalRead(arduinoPin);
    }
    //Set the state of the Output
    void set(bool value)  {
      digitalWrite(arduinoPin, value);
      state = value;
    }
    //--- OVERLOADS ---
    // =
    DigitalOutBase& operator=(bool value) {
      set(value);
      return *this;
    }
    // implicit bool conversion
    operator bool() const {
      return state;
    }
};
//===== DIGITAL IN =====

// Structure representing a physical arduino pin configured as digital input
struct DigitalInBase {
  private:
    uint8_t arduinoPin;
  protected:
    String description;
  public:
    //--- CONSTRUCTOR ---

    //Arduino PIN 
    DigitalInBase(uint8_t arduinoPin) : arduinoPin(arduinoPin), description("Digital In ArduinoPin: " + String(arduinoPin)) {
      pinMode(arduinoPin, INPUT);
    }
    //--- PUBLIC VARS ---
    bool state;
    //--- PUBLIC FUNCTIONS ---

    //Update the state of the Input
    bool get() {
      state = digitalRead(arduinoPin);
      return state;
    }
    void update() {
      state = digitalRead(arduinoPin);
    }
    //--- OVERLOADS ---

    // implicit bool conversion
    operator bool() {
      get();
      return state;
    }
};

//===== ANALOG OUT =====

// Structure representing a physical arduino pin configured as analog output
struct AnalogOutBase {
  private:
  protected:
     String description;
  public:
    uint8_t arduinoPin;
    //--- CONSTRUCTOR ---

    //Arduino PIN 
    AnalogOutBase(uint8_t arduinoPin) : arduinoPin(arduinoPin), description("Analog Out ArduinoPin: " + String(arduinoPin)) {
      pinMode(arduinoPin, OUTPUT);
    }
    //--- PUBLIC VARS ---
    double state;
    //--- PUBLIC FUNCTIONS ---

    //Set the state of the Output
    virtual void set(double value) {
      Assert::dacResolutionIs12bit(); // make sure the resolution is set to 12bit raise an error otherwise
      analogWrite(arduinoPin, (int)value);
      state = value;
    }
    //--- OVERLOADS ---
    // =
    AnalogOutBase& operator=(double value) {
      set(value);
      return *this;
    }
        // +=
    AnalogOutBase& operator+=(double other) {
      state += other;
      set(state);
      return *this;
    }
    // -=
    AnalogOutBase& operator-=(double other) {
      state -= other;
      set(state);
      return *this;
    }
    // implicit double conversion
    operator double() const {
      return state;
    }
};

//===== ANALOG IN =====

// Structure representing a physical arduino pin configured as analog input
struct AnalogInBase {
  private:

  protected:
    String description;
    double state;
  public:
    uint8_t arduinoPin;
    //--- CONSTRUCTOR ---

    //Arduino PIN 
    AnalogInBase(uint8_t arduinoPin) : arduinoPin(arduinoPin), description("Analog In ArduinoPin: " + String(arduinoPin)){
      pinMode(arduinoPin, INPUT);
    }
    //--- PUBLIC VARS ---
    
    //--- PUBLIC FUNCTIONS ---

    virtual void update(){
      Assert::adcResolutionIs12bit();
      int analogReadDAC = 0;
      analogReadDAC = analogRead(arduinoPin);
      state = (double)analogReadDAC / 4096.0 * 3.3;
    }
    //gets the pin Voltage
    virtual double get() {
      update();
      return state;  
    }
    //--- OVERLOADS ---
    // =
    // implicit double conversion
    operator double() {
      get();
      return state;
    }
};

//::::: PIN FUNCTION DEFINITIONS :::::

/* 
PowerSwitch (Leistungsschalter)
24V, 0.25A max
*/
struct PowerSwitch : DigitalOutBase{
  public:
    using DigitalOutBase::operator=;
    //--- CONSTRUCTOR ---
    PowerSwitch(uint8_t arduinoPin) : DigitalOutBase(arduinoPin){
      description = "PowerSwitch (Leistungsshalter)\n24V 0.25A max\nArduino PIN: " + String(arduinoPin);
    } 
};

/* 
OpenCollectorOutput (open Collector Ausgang)
40V, 50mA max
*/
struct OpenCollectorOutput : DigitalOutBase{
  public:
    using DigitalOutBase::operator=;
    //--- CONSTRUCTOR ---
    OpenCollectorOutput(uint8_t arduinoPin) : DigitalOutBase(arduinoPin) {
      description = "OpenCollectorOutput (open Collector Ausgang)\n40V 50A max\nArduino PIN: " + String(arduinoPin);
    }
};

/* 
MOSContact (MOS-Kontakt)
60V max, 0.3A max 0=open
*/
struct MOSContact : DigitalOutBase{
  public:
    using DigitalOutBase::operator=;
    //--- CONSTRUCTOR ---
    MOSContact(uint8_t arduinoPin) : DigitalOutBase(arduinoPin) {
      description = "MOSContact (MOS-Kontakt)\n60V max, 0.3A max 0=open\nArduino PIN: " + String(arduinoPin);
    }
};

/* 
AnalogOutIsolated (Analog Ausgang isoliert)
0V to 10V, 12bit the arduino due will deliver a range of 0.55V to 2.75V which will then be converted to the porper 10V range by the LSC hardware.
*/
struct AnalogOutIsolated : AnalogOutBase{
  public:
    //--- CONSTRUCTOR ---
    AnalogOutIsolated(uint8_t arduinoPin) : AnalogOutBase(arduinoPin) {
      description = "AnalogOutIsolated (Analog Ausgang isoliert)\n0V to 10V, 12bit the arduino due will deliver a range of 0.55V to 2.75V which will then be converted to the porper 10V range by the LSC hardware.\nArduino PIN: " + String(arduinoPin);
    }

    // Forward declaration this makes sure that we use the operator in the scope of the new Struct. without this we woud call the old set() function.
    using AnalogOutBase::operator=;
    using AnalogOutBase::operator+=;
    using AnalogOutBase::operator-=; 
    //Sets the analog output in Volt 
    void set(double value) override{
      if((value >= 0.) && (value <= 10.) ){
        int analogWriteDAC = 0;
        analogWriteDAC = (uint)(value / 10.0 * 4095.0);
        analogWrite(arduinoPin, analogWriteDAC);
        state = value;
      } else{
        ERROR_HANDLER.throwError(0x0, "The DAC value on Arduino pin:" + String(arduinoPin) + " has to be between 0V and 10V but is: " + String(value) + "V. Nothing will be written to the Output!", SeverityLevel::NORMAL);
      }

    }
};

/* 
AnalogIn (Analog Eingang)
0V to 10V, 12bit, Voltage divider 10KOhm
*/
struct AnalogIn : AnalogInBase{
  public:
    //--- CONSTRUCTOR ---
    AnalogIn(uint8_t arduinoPin) : AnalogInBase(arduinoPin) {
      description = "AnalogIn (Analog Eingang)\n0V to 10V, 12bit, Voltage divider 10KOhm \nArduino PIN: " + String(arduinoPin);
    }

    // Forward declaration this makes sure that we use the implicit conversion double() in the scope of the new Struct.
    using AnalogInBase::operator double; 
    void update() override {
      int analogReadADC = 0;
      analogReadADC = analogRead(arduinoPin);
      state = (double)analogReadADC / 4096.0 * 10;
    }
    //gets the pin Voltage
    double get() override {
      //TODO: assert analogReadResolution == 12
      update();
      return state;
    }
};

/* 
AnalogInPt100 (Temperaturmessung)
PT100, 0.2V to 3.3V, 12bit
*/
struct AnalogInPt100 : AnalogInBase{
  public:
    //--- CONSTRUCTOR ---
    AnalogInPt100(uint8_t arduinoPin) : AnalogInBase(arduinoPin) {
      description = "AnalogInPt100 (Temperaturmessung)\nPT100, 0.2V to 3.3V, 12bit\nArduino PIN: " + String(arduinoPin);
    }

    // Forward declaration this makes sure that we use the implicit conversion double() in the scope of the new Struct.
    using AnalogInBase::operator double; 

    void update() override{
      Assert::dacResolutionIs12bit();
      int analogReadADC = 0;
      analogReadADC = analogRead(arduinoPin);
      state = (double)analogReadADC / 4096.0 * 3.1 + 0.2;
    }
    //gets the pin Voltage
    double get() override {
      update();
      return state;
    }
};

/* 
AnalogInGauge (Pfeifer Messzelle)
2.2V to 3.3V, Spannungsteiler Rin=12kOhm, 12bit
*/
struct AnalogInGauge : AnalogInBase{
  public:
    //--- CONSTRUCTOR ---
    AnalogInGauge(uint8_t arduinoPin) : AnalogInBase(arduinoPin) {
      description = "AnalogInGauge (Pfeifer Messzelle)\n2.2V to 3.3V, Spannungsteiler Rin=12kOhm, 12bit\nArduino PIN: " + String(arduinoPin);
    }

    // Forward declaration this makes sure that we use the implicit conversion double() in the scope of the new Struct.
    using AnalogInBase::operator double; 
    void update() override{
      Assert::dacResolutionIs12bit();
      int analogReadADC = 0;
      analogReadADC = analogRead(arduinoPin);
      state = (double)analogReadADC / 4096.0 * 6.3 + 2.2;
    }
    //gets the pin Voltage
    double get() override {
      update();
      return state;
    }
};

/* 
DigitalInIsolated (Dig. IN isoliert)
3V to 24V, Rin=, Inverted with PullUp
*/
struct DigitalInIsolated : DigitalInBase{
  public:
    using DigitalInBase::operator bool;
    //--- CONSTRUCTOR ---
    DigitalInIsolated(uint8_t arduinoPin) : DigitalInBase(arduinoPin) {
      description = "DigitalInIsolated (Dig. IN isoliert)\n3V to 24V, Rin=, Inverted with PullUp\nArduino PIN: " + String(arduinoPin);
    }
};

/* 
Beeper (Piepser) Singelton
*/
struct Beeper {
    //---- BEEPER EXPLANATION ----
  /*
    The reason the beeper struct looks so complicated is because, we want to implement the beeper in a non blocking way.
    i.e. when Beeper.Beep(10) is called we don't want the program to wait until the beep has completed.
    We can achive this by using a timer:
    Timer TC3-0 is setup to trigger an ISR TC3_Handler every 100ms.
    We use a variable beepFor to keep track, how long the beeper has to beep. This works togeter with the timer as follows:
      The funtion beep(unsigned long duration) adds the duration to beepFor, sets the aruino beeper output to high (start beeping) and starts the timer
      The timer ISR TC3_Handler decreases the value of beepFor by 1 every time is is called. When beepFor is 0 beeping is stoped and the timer disabled.
    This implementation is non blocking and allows for proper handling of calls to beep while the device is already beeping.
    We use the singelton pattern to make the Beeper available globally.
  */
    //---- END PEEPER EXPLANATION ----

    friend void TC3_Handler(); //grant access to private members to TC3_Handler
    
  private:
    volatile unsigned long beepFor; //reflects how long the timer has to beep for in real time (beepFor=1 -> will beep for 0.1s). The variable can be changed by ISRs which is why it has to be volatile
    volatile bool isRunning; // keeps track if the beeper is already beeping.
    String description;

    //private constructor, following the singelton pattern
    Beeper() {
      pinMode(52, OUTPUT); //the beeper is connected to pin 52 of the arduino -> set it as output
      description = "Beeper (Piepser)\nArduino PIN: 52";
      beepFor = 0;
      isRunning = false;
        // ---- SETTUNG UP TIMER TC3-0 START ----
        // Set Timer 3 to trigger an interrupt every 100m  
        // Enable Timer 3 peripheral clock
        pmc_set_writeprotect(false);
        pmc_enable_periph_clk((uint32_t)TC3_IRQn);
        // Set up Timer 3 channel 0 (TC1) for 10Hz frequency
        TC_Configure(TC1, 0, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK4);
        TC_SetRC(TC1, 0, 21000); // (84MHz / 4) / 62500 = 100ms
        // Enable Timer 3 channel 0 interrupt
        TC1->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
        TC1->TC_CHANNEL[0].TC_IDR = ~TC_IER_CPCS;
        NVIC_SetPriority(TC3_IRQn, 0); // Set priority for Timer 3 interrupt
        NVIC_EnableIRQ(TC3_IRQn); // Enable Timer 3 interrupt
        // ---- SETTUNG UP TIMER TC3-0 END ----
     }

  public:
    // Function that returns the instance of Beeper (if the instance does not exist yet, it is created)
    static Beeper& getInstance() { 
      static Beeper instance;     // Public member holding the single instance of Beeper with lazy initialization of the singleton
      return instance;
    }

    //Beeps for the specified amount of time
    //duration: the beep duration in 0.1s (ex: Beep(10) would beep for 1s)
    void beep(unsigned long duration) {
      digitalWrite(52, true); //set the pin that is connected to the beeper to high
      beepFor += duration;  //the beeper will beep whenever beepFor > 0. Whenever beepFor is > 0 it will be decreased by 1 every 100ms until it hits 0, which is when the beeper turns off (see TC3_Handler)
      if( isRunning == false){
        TC_Start(TC1, 0); //start the timer i.e. TC3_Handler will be called every 100ms
        isRunning = true;
      }

    }  
};


/*
Button (Bedienungstasten FrontPlatte)
Structure that represents the physical buttons on the fort panel of the LSC
*/
struct Button{
  private:
    uint8_t arduinoPin;

  public:
    //--- CONSTRUCTOR ---
    Button(uint8_t arduinoPin, void (*callback)()) : arduinoPin(arduinoPin){
    attachInterrupt(digitalPinToInterrupt(arduinoPin), callback , CHANGE);
    }
    bool isPressed(){
      return digitalRead(arduinoPin);
    }
    uint8_t getPin(){
      return arduinoPin;
    }
};

/*
Singelton class that represents all six physical buttons of the LSC:
  -bt1
  -bt2
  -bt3
  -bt4
  -bt5
  -bt6
The class provides a function to attach handlers to button press events:
  setOnClickHandler(Button& button, void (*handler)())
*/
class Buttons{
      //---- BUTTONS EXPLANATION ----
  /*
  The class is implemented as singelton to make sure we only ever have one instance of the Buttons class (which makes sense because the buttons are tied to specific arduino pins).
  Every button has 2 callback funcions which are called, when a button press is registerd (trough interrupts defined in the button struct):
    bt_internal_callback():
      This function is executed when a button press is registered. This allows us to performe some checks or performe some acctions, before handing over to the:
    bt_external_callback():
      This function is provided by the main programm. And can be changed dynamically.
  */
    //---- END BUTTON EXPLANATION ----
  private:
    // --- button 1 Callback ---
    //internal callback gives the optin to execute some local code before calling the external callback
    static void bt1_internal_callback(){
      if(Buttons::getInstance().bt1.isPressed() == true){ //we only want to execute if the button is pressed, not if it is realeaded
        BEEPER.beep(1);
        if(bt1_external_callback != nullptr){ //make sure the external function is set propperly
           bt1_external_callback();
        }
      }
    }
    // --- button 2 Callback ---
    //internal callback gives the optin to execute some local code before calling the external callback
    static void bt2_internal_callback(){
      if(Buttons::getInstance().bt2.isPressed() == true){ //we only want to execute if the button is pressed, not if it is realeaded
        BEEPER.beep(1);
        if(bt2_external_callback != nullptr){ //make sure the external function is set propperly
          bt2_external_callback();
        } 
      }
    }
    // --- button 3 Callback ---
    //internal callback gives the optin to execute some local code before calling the external callback
    static void bt3_internal_callback(){
      if(Buttons::getInstance().bt3.isPressed() == true){ //we only want to execute if the button is pressed, not if it is realeaded
        BEEPER.beep(1);
        if(bt3_external_callback != nullptr){ //make sure the external function is set propperly
          bt3_external_callback();
        } 
      }
    }    
    // --- button 4 Callback ---
    //internal callback gives the optin to execute some local code before calling the external callback
    static void bt4_internal_callback(){
      if(Buttons::getInstance().bt4.isPressed() == true){ //we only want to execute if the button is pressed, not if it is realeaded
        BEEPER.beep(1);
        if(bt4_external_callback != nullptr){ //make sure the external function is set propperly
          bt4_external_callback();
        } 
      }
    }    
    // --- button 5 Callback ---
    //internal callback gives the optin to execute some local code before calling the external callback
    static void bt5_internal_callback(){
      if(Buttons::getInstance().bt5.isPressed() == true){ //we only want to execute if the button is pressed, not if it is realeaded
        BEEPER.beep(1);
        if(bt5_external_callback != nullptr){ //make sure the external function is set propperly
          bt5_external_callback();
        } 
      }
    }    
    // --- button 6 Callback ---
    //internal callback gives the optin to execute some local code before calling the external callback
    static void bt6_internal_callback(){
      if(Buttons::getInstance().bt6.isPressed() == true){ //we only want to execute if the button is pressed, not if it is realeaded
        BEEPER.beep(1);
        if(bt6_external_callback != nullptr){ //make sure the external function is set propperly
          bt6_external_callback();
        } 
      }
    }    

    Buttons():bt1(24,bt1_internal_callback), bt2(23,bt2_internal_callback), bt3(22,bt3_internal_callback), bt4(25,bt4_internal_callback), bt5(26,bt5_internal_callback), bt6(27,bt6_internal_callback) {
    }
    
  public:
    //these pointers stor the addreses of the external callbacks
    static void (*bt1_external_callback)();
    static void (*bt2_external_callback)();
    static void (*bt3_external_callback)();
    static void (*bt4_external_callback)();
    static void (*bt5_external_callback)();
    static void (*bt6_external_callback)();
    Button bt1;
    Button bt2;
    Button bt3;
    Button bt4;
    Button bt5;
    Button bt6;
    //singelton lazy init
    static Buttons& getInstance(){
      static Buttons instance;
      return instance;
    }
    // sets the external_callback function for the corresponding buttons
    // button: the button you want to set the handler for
    // handler: a function pointer to the handler function
    static void setOnClickHandler(Button& button, void (*handler)()){ //TODO: ERROR HANDLING we need to make sure that void (*handler)() is a callable funciton. I'm not sure if this is even possible...
      if(&button == &getInstance().bt1){
          bt1_external_callback = handler;
      } else if (&button == &getInstance().bt2){
          bt2_external_callback = handler;        
      } else if (&button == &getInstance().bt3){
          bt3_external_callback = handler;        
      } else if (&button == &getInstance().bt4){
          bt4_external_callback = handler;        
      } else if (&button == &getInstance().bt5){
          bt5_external_callback = handler;        
      } else if (&button == &getInstance().bt6){
          bt6_external_callback = handler;        
      } else{
        ERROR_HANDLER.throwError(0, "You are trying to attach a ButtonOnClick handler to a button that does not exist!", SeverityLevel::NORMAL); //throw an exception if an invalid button has been passed as argument.
      }
    }
};

class Dsub37{
  /*
    TESTED: All connectin have been tested 25.07.23 HOTO
    TODO: On the particualr unit used for testing pin13 seems to be broken
  */
  private:
      Dsub37() : pin5(37), pin6(38),pin7(39), pin8(40), pin9(41), pin28(42), pin10(43), pin29(44), pin11(45), pin30(46), pin13(53), pin32(51), pin14(50), pin33(49), pin15(48), pin34(47), pin17(34), pin18(35), pin19(36){
      }

  public:
    PowerSwitch pin5;
    PowerSwitch pin6;
    PowerSwitch pin7;
    PowerSwitch pin8;
    OpenCollectorOutput pin9;
    OpenCollectorOutput pin28;
    OpenCollectorOutput pin10;
    OpenCollectorOutput pin29;
    OpenCollectorOutput pin11;
    OpenCollectorOutput pin30;
    DigitalInIsolated pin13;
    DigitalInIsolated pin32;
    DigitalInIsolated pin14;
    DigitalInIsolated pin33;
    DigitalInIsolated pin15;
    DigitalInIsolated pin34;
    MOSContact pin17;
    MOSContact pin18;
    MOSContact pin19;

    static Dsub37& getInstance(){
      static Dsub37 instance;
      return instance;
    }
};

class Dsub25{
  /*
    TESTED: 
    TODO: 
  */
  private:
      Dsub25() : pin15(1), pin12(DAC0), pin13(DAC1){
      }

  public:
    AnalogInPt100 pin15;
    AnalogOutIsolated pin12;
    AnalogOutIsolated pin13;

    static Dsub25& getInstance(){
      static Dsub25 instance;
      return instance;
    }
};

class LSC { //TODO: This is just a dummy
  private:
    int myPrivateVariable;

  public:
    Dsub37& dsub37;
    Dsub25& dsub25;
    LSC(): dsub37(Dsub37::getInstance()), dsub25(Dsub25::getInstance()){

    } // Constructor declaration
    void myPublicMethod(); // Method de 

};

#endif