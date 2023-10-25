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

#ifndef LscHardwareAbstraction_H
#define LscHardwareAbstraction_H

#include <Arduino.h>
#include "LscError.h"
#include "RingBuf.h"
#include "vector"
#include "math.h"
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

    //Returns the state of the output
    bool getState() {                  
      state = digitalRead(arduinoPin);
      return state;
    }
    //Set the state of the Output
    void setState(bool value)  {
      digitalWrite(arduinoPin, value);
      state = value;
    }
    //--- OVERLOADS ---
    // =
    DigitalOutBase& operator=(bool value) {
      setState(value);
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

    //Returns the state of the input
    bool getState() {
      state = digitalRead(arduinoPin);
      return state;
    }
    void update() {
      state = digitalRead(arduinoPin);
    }
    //--- OVERLOADS ---

    // implicit bool conversion
    operator bool() {
      getState();
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
    virtual void setVoltage(double value) {
      Assert::dacResolutionIs12bit(); // make sure the resolution is set to 12bit raise an error otherwise
      analogWrite(arduinoPin, (int)value);
      state = value;
    }
    //--- OVERLOADS ---
    // =
    AnalogOutBase& operator=(double value) {
      setVoltage(value);
      return *this;
    }
        // +=
    AnalogOutBase& operator+=(double other) {
      state += other;
      setVoltage(state);
      return *this;
    }
    // -=
    AnalogOutBase& operator-=(double other) {
      state -= other;
      setVoltage(state);
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
    //Returns the voltage of the input
    virtual double getVoltage() {
      update();
      return state;  
    }
    //--- OVERLOADS ---
    // =
    // implicit double conversion
    operator double() {
      getVoltage();
      return state;
    }
};

//::::: PIN FUNCTION DEFINITIONS :::::

/*
  If these names are changed, pls update the names in the LSC pinout document as well 
*/

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
    void setVoltage(double value) override{
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
    AnalogIn(uint8_t arduinoPin) : AnalogInBase(arduinoPin), 
      calibrationCurve( {{10, 0. },{18, 0.0012},{40, 0.1116},{65, 0.1995},{104, 0.3001},{146, 0.4001},{187, 0.5004},{228, 0.6009},{270, 0.701},{311, 0.8013},{352, 0.9014},{393, 1.0015},{805, 2.0},{1219, 3.0004},{1631, 4.006},{2043, 5.006},{2456, 6.007},{2866, 7.006},{3279, 8.007},{3689, 9.004},{4060, 9.905},{4088, 9.993}}){
      description = "AnalogIn (Analog Eingang)\n0V to 10V, 12bit, Voltage divider 10KOhm \nArduino PIN: " + String(arduinoPin);
    }

    // Forward declaration this makes sure that we use the implicit conversion double() in the scope of the new Struct.
    using AnalogInBase::operator double; 
    void update() override {
      int analogReadADC = 0;
      for(int i = 0; i< 1000;i++){
        analogReadADC += analogRead(arduinoPin);  
      }
      analogReadADC = analogReadADC / 1000.;
      state = analogReadADC;
      /*
      //analogReadADC = analogRead(arduinoPin);
      state = adcToVoltage(analogReadADC);
      double x = (double)analogReadADC;
      //state = pow(x, 10) * -1.9769377902306896e-33+pow(x, 9) * 4.45070327076997e-29+pow(x, 8) * -4.280606353961094e-25+pow(x, 7) * 2.297439128678856e-21+pow(x, 6) * -7.540210511571309e-18+pow(x, 5) * 1.5592054480380003e-14+pow(x, 4) * -2.0179194102821266e-11+pow(x, 3) * 1.5696790564130045e-08+pow(x, 2) * -6.731910720530184e-06+pow(x, 1) * 0.0037705130175919436 - 0.041751441760416154;
      if(state<0) state=0;
      */
    }
    //Returns the voltage of the input
    double getVoltage() override {
      //TODO: assert analogReadResolution == 12
      update();
      return state;
    }
  private:
    struct CalibrationPairs{
      int adcValue;
      double voltage;
    };
    double interpolate(double x, double x0, double x1, double y0, double y1) {
      return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
    }
    double adcToVoltage(int adcValue) {
      // Find the two nearest data points
      CalibrationPairs *lower = nullptr;
      CalibrationPairs *upper = nullptr;

      for (size_t i = 0; i < sizeof(calibrationCurve) / sizeof(calibrationCurve[0]); i++) {
          if (adcValue >= calibrationCurve[i].adcValue) {
              lower = &calibrationCurve[i];
          } else {
              upper = &calibrationCurve[i];
              break;
          }
      }

      // If no upper data point is found, return the voltage of the highest data point
      if (!upper) {
          return calibrationCurve[sizeof(calibrationCurve) / sizeof(calibrationCurve[0]) - 1].voltage;
      }

      // Perform linear interpolation
      return interpolate(adcValue, lower->adcValue, upper->adcValue, lower->voltage, upper->voltage);
    }
    CalibrationPairs calibrationCurve[22];


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
      /* 
        Temperature Calculation:
        Gain= 1+(49.2/2.7)
        R_v = 3300 Ohm
        R_pt = resistance of Pt100 (Temperature Dependent)
        R_tot = R_v + R_pt
        ADC12bit Value = 1.241*Gain*R_v*R_pt/R_tot
        -> This formula can be rewritten to R_pt = ADC12bit * 3300/(78720.8 -ADC12bit)
    
        ADC12bit to R_pt: R_pt = ADC12bit * 3300/(78720.8-ADC12bit)
        Once R_pt is calculated, one can calculate the Temperature by using Binomial Calculation: 
   
        T [in degrees Celsius] = a + b*R_pt + c*R_pt^2 + d*R_pt^3
        where a=-2.429E+02, b=2.279E+00, c=1.674E-03, d=-1.815E-06;
      */
      Assert::dacResolutionIs12bit();
      uint16_t analogReadADC = 0;
      analogReadADC = analogRead(arduinoPin);
      double rPT = (double)analogReadADC * 3300. / (78720.8 - (double)analogReadADC);
      rPT -= 0.5;
      const double a=-2.429E+02, b=2.279E+00, c=1.674E-03, d=-1.815E-06;
      double temp = (d * pow(rPT, 3) + c * rPT * rPT + b * rPT + a);
      state = 273.15 + temp;
    }
    //Returns the temperature of the input in K
    double getTemperature() {
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
    //Returns the voltage of the input
    double getVoltage() override {
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
        // ---- SETTUNG UP TIMER TC1-0 -> TC3_Handler START ----
        // Set Timer 1 to trigger an interrupt every 100m  
        // Enable Timer 1 peripheral clock
        pmc_set_writeprotect(false);
        pmc_enable_periph_clk((uint32_t)TC3_IRQn);
        // Set up Timer 1 channel 0 (TC1) for 10Hz frequency
        TC_Configure(TC1, 0, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK4);
        TC_SetRC(TC1, 0, 21000); // (84MHz / 4) / 62500 = 100ms
        // Enable Timer 1 channel 0 interrupt
        TC1->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
        TC1->TC_CHANNEL[0].TC_IDR = ~TC_IER_CPCS;
        NVIC_SetPriority(TC3_IRQn, 0); // Set priority for IRQn3 interrupt
        NVIC_EnableIRQ(TC3_IRQn); // Enable Timer IRQn3 interrupt
        // ---- SETTUNG UP TIMER TC1-0 END ----
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
    volatile bool clicked;
    //--- CONSTRUCTOR ---
    Button(uint8_t arduinoPin, void (*callback)()) : arduinoPin(arduinoPin), clicked(false){
      attachInterrupt(digitalPinToInterrupt(arduinoPin), callback , CHANGE);   
    }
    bool isPressed(){
      return digitalRead(arduinoPin);
    }
    uint8_t getPin(){
      return arduinoPin;
    }
    //When the button has been clicked this function will retrun true, by calling the functon the hasBeenClicked() function 
    //will be reset to false. This function provides a way to work with buttons without having to attach an interrupt handler
    //for mondane tasks. 
    bool hasBeenClicked(){
      if(clicked == true){
        clicked = false;
        return true;
      }else{
        return false;
      }
    }
};

/*
Singelton class that represents all six physical buttons of the LSC:
  -bt_0
  -bt_1
  -bt_2
  -bt_3
  -bt_4
  -bt_5
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
    // --- button 0 Callback ---
    //internal callback gives the optin to execute some local code before calling the external callback
    static void bt_0_internal_callback(){
      if(Buttons::getInstance().bt_0.isPressed() == true){ //we only want to execute if the button is pressed, not if it is realeaded
        BEEPER.beep(1);
        Buttons::getInstance().bt_0.clicked = true; //indicate that the button has been pressed for the bt.hasBeenClicked function
        if(bt_0_external_callback != nullptr){ //make sure the external function is set propperly
          bt_0_external_callback();
        } 
      }
    }    
    // --- button 1 Callback ---
    //internal callback gives the optin to execute some local code before calling the external callback
    static void bt_1_internal_callback(){
      if(Buttons::getInstance().bt_1.isPressed() == true){ //we only want to execute if the button is pressed, not if it is realeaded
        BEEPER.beep(1);
        Buttons::getInstance().bt_1.clicked = true; //indicate that the button has been pressed for the bt.hasBeenClicked function
        if(bt_1_external_callback != nullptr){ //make sure the external function is set propperly
           bt_1_external_callback();
        }
      }
    }
    // --- button 2 Callback ---
    //internal callback gives the optin to execute some local code before calling the external callback
    static void bt_2_internal_callback(){
      if(Buttons::getInstance().bt_2.isPressed() == true){ //we only want to execute if the button is pressed, not if it is realeaded
        BEEPER.beep(1);
        Buttons::getInstance().bt_2.clicked = true; //indicate that the button has been pressed for the bt.hasBeenClicked function
        if(bt_2_external_callback != nullptr){ //make sure the external function is set propperly
          bt_2_external_callback();
        } 
      }
    }
    // --- button 3 Callback ---
    //internal callback gives the optin to execute some local code before calling the external callback
    static void bt_3_internal_callback(){
      if(Buttons::getInstance().bt_3.isPressed() == true){ //we only want to execute if the button is pressed, not if it is realeaded
        BEEPER.beep(1);
        Buttons::getInstance().bt_3.clicked = true; //indicate that the button has been pressed for the bt.hasBeenClicked function
        if(bt_3_external_callback != nullptr){ //make sure the external function is set propperly
          bt_3_external_callback();
        } 
      }
    }    
    // --- button 4 Callback ---
    //internal callback gives the optin to execute some local code before calling the external callback
    static void bt_4_internal_callback(){
      if(Buttons::getInstance().bt_4.isPressed() == true){ //we only want to execute if the button is pressed, not if it is realeaded
        BEEPER.beep(1);
        Buttons::getInstance().bt_4.clicked = true; //indicate that the button has been pressed for the bt.hasBeenClicked function
        if(bt_4_external_callback != nullptr){ //make sure the external function is set propperly
          bt_4_external_callback();
        } 
      }
    }    
    // --- button 5 Callback ---
    //internal callback gives the optin to execute some local code before calling the external callback
    static void bt_5_internal_callback(){
      if(Buttons::getInstance().bt_5.isPressed() == true){ //we only want to execute if the button is pressed, not if it is realeaded
        BEEPER.beep(1);
        Buttons::getInstance().bt_5.clicked = true; //indicate that the button has been pressed for the bt.hasBeenClicked function
        if(bt_5_external_callback != nullptr){ //make sure the external function is set propperly
          bt_5_external_callback();
        } 
      }
    }    

    Buttons():bt_0(24,bt_0_internal_callback), bt_1(23,bt_1_internal_callback), bt_2(22,bt_2_internal_callback), bt_3(25,bt_3_internal_callback), bt_4(26,bt_4_internal_callback), bt_5(27,bt_5_internal_callback) {
    }
    
  public:
    //these pointers store the addreses of the external callbacks
    static void (*bt_0_external_callback)();
    static void (*bt_1_external_callback)();
    static void (*bt_2_external_callback)();
    static void (*bt_3_external_callback)();
    static void (*bt_4_external_callback)();
    static void (*bt_5_external_callback)();
    //Top Left
    Button bt_0;
    //Middle Left
    Button bt_1;
    //Bottom Left
    Button bt_2;
    //Top Right
    Button bt_3;
    //Middle Right
    Button bt_4;
    //Bottom Right
    Button bt_5;
    
    //singelton lazy init
    static Buttons& getInstance(){
      static Buttons instance;
      return instance;
    }
    // sets the external_callback function for the corresponding buttons
    // button: the button you want to set the handler for
    // handler: a function pointer to the handler function
    static void setOnClickHandler(Button& button, void (*handler)()){ //TODO: ERROR HANDLING we need to make sure that void (*handler)() is a callable funciton. I'm not sure if this is even possible...
      if(&button == &getInstance().bt_0){
          bt_0_external_callback = handler;
      } else if (&button == &getInstance().bt_1){
          bt_1_external_callback = handler;        
      } else if (&button == &getInstance().bt_2){
          bt_2_external_callback = handler;        
      } else if (&button == &getInstance().bt_3){
          bt_3_external_callback = handler;        
      } else if (&button == &getInstance().bt_4){
          bt_4_external_callback = handler;        
      } else if (&button == &getInstance().bt_5){
          bt_5_external_callback = handler;        
      } else{
        ERROR_HANDLER.throwError(0, "You are trying to attach a ButtonOnClick handler to a button that does not exist!", SeverityLevel::NORMAL); //throw an exception if an invalid button has been passed as argument.
      }
    }
};

class LSC{
  /*
    TESTED: All connectin have been tested 25.07.23 HOTO
    TODO: On the particualr unit used for testing pin13 seems to be broken
  */
  

  private:
      LSC() : buttons(Buttons::getInstance()), powerSwitch_0(37), powerSwitch_1(38),powerSwitch_2(39), powerSwitch_3(40), openCollectorOutput_0(41),
              openCollectorOutput_1(42), openCollectorOutput_2(43), openCollectorOutput_3(44), openCollectorOutput_4(45), openCollectorOutput_5(46),
              digitalInIsolated_0(53), digitalInIsolated_1(51), digitalInIsolated_2(50), digitalInIsolated_3(49), digitalInIsolated_4(48), 
              digitalInIsolated_5(47), mosContact_0(34), mosContact_1(35), mosContact_2(36),analogInPt100_0(54),analogInPt100_1(55),
              analogInPt100_2(56),analogInGauge_0(57), analogIn_0(58),analogIn_1(59),analogIn_2(60),analogIn_3(61),
              analogOutIsolated_0(66), analogOutIsolated_1(67) {
              //setting interrup priorities
              NVIC_SetPriority(PIOA_IRQn, 5); //set external gpio priority (it has to be higher than Beeper timer)
              NVIC_SetPriority(PIOB_IRQn, 5); //set external gpio priority (it has to be higher than Beeper timer)
              NVIC_SetPriority(PIOC_IRQn, 5); //set external gpio priority (it has to be higher than Beeper timer)
              NVIC_SetPriority(PIOD_IRQn, 5); //set external gpio priority (it has to be higher than Beeper timer)
              analogWriteResolution(12);
              analogReadResolution(12);

              //Setting up the uart send timer see Async UART for explenation
              pmc_set_writeprotect(false);
              pmc_enable_periph_clk(TC2_IRQn); 
              TC_Configure(TC0, 2, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK4); 
              TC_SetRC(TC0, 2, 2281); // Timer will trigger every 3.5ms 
              TC0->TC_CHANNEL[2].TC_IER=TC_IER_CPCS;
              TC0->TC_CHANNEL[2].TC_IDR=~TC_IER_CPCS;
              NVIC_ClearPendingIRQ(TC2_IRQn);
              NVIC_EnableIRQ(TC2_IRQn);
              NVIC_SetPriority(TC2_IRQn, 5);
              TC_Start(TC0, 2);  
              uartBuffer.clear();
        }
    //this is the handler function for the timer responsible for writing data to the uart see Async UART for explenation
    friend void TC2_Handler();
    //this is a ringbuffer holding data to be written to the uart by the handler above see Async UART for explenation
    static RingBuf<char, 3450> uartBuffer;

  public:
    // bt_0 -> Top Left | bt_1 -> Middle Left | bt_2 -> Bottom Left | bt_3 -> Top Right | bt_4 -> Middle Right | bt_5 -> Bottom Right
    Buttons& buttons;
    //Connector: D-SUB 37 | Connector Pins: 5,24 | Arduino Pin: 37| Ratings: 24V,0.25A | Comment: set(true)-> Outputs High | GND Pins: D-SUB-37-Pins:1,16,20
    PowerSwitch powerSwitch_0;
    //Connector: D-SUB 37 | Connector Pins: 6,25 | Arduino Pin: 38| Ratings: 24V,0.25A | Comment: set(true)-> Outputs High | GND Pins: D-SUB-37-Pins:1,16,20
    PowerSwitch powerSwitch_1;
    //Connector: D-SUB 37 | Connector Pins: 7,26 | Arduino Pin: 29| Ratings: 24V,0.25A | Comment: set(true)-> Outputs High | GND Pins: D-SUB-37-Pins:1,16,20
    PowerSwitch powerSwitch_2;
    //Connector: D-SUB 37 | Connector Pins: 8,27 | Arduino Pin: 40| Ratings: 24V,0.25A | Comment: set(true)-> Outputs High | GND Pins: D-SUB-37-Pins:1,16,20
    PowerSwitch powerSwitch_3;
    //Connector: D-SUB 37 | Connector Pins: 9 | Arduino Pin: 41| Ratings: 40V,100mA | Usable Power Pins: 3.3V(Arduino)@D-SUB-37-Pin:3 ; 5V(Arduino))@D-SUB-37-Pin:22 ; 12V,2A@D-SUB-37-Pin:2,21 ; 24V,1.2A@D-SUB-37-Pin:4,23  
    OpenCollectorOutput openCollectorOutput_0;
    //Connector: D-SUB 37 | Connector Pins: 28 | Arduino Pin: 42| Ratings: 40V,100mA | Usable Power Pins: 3.3V(Arduino)@D-SUB-37-Pin:3 ; 5V(Arduino))@D-SUB-37-Pin:22 ; 12V,2A@D-SUB-37-Pin:2,21 ; 24V,1.2A@D-SUB-37-Pin:4,23  
    OpenCollectorOutput openCollectorOutput_1;
    //Connector: D-SUB 37 | Connector Pins: 10 | Arduino Pin: 43| Ratings: 40V,100mA | Usable Power Pins: 3.3V(Arduino)@D-SUB-37-Pin:3 ; 5V(Arduino))@D-SUB-37-Pin:22 ; 12V,2A@D-SUB-37-Pin:2,21 ; 24V,1.2A@D-SUB-37-Pin:4,23
    OpenCollectorOutput openCollectorOutput_2;
    //Connector: D-SUB 37 | Connector Pins: 29 | Arduino Pin: 44| Ratings: 40V,100mA | Usable Power Pins: 3.3V(Arduino)@D-SUB-37-Pin:3 ; 5V(Arduino))@D-SUB-37-Pin:22 ; 12V,2A@D-SUB-37-Pin:2,21 ; 24V,1.2A@D-SUB-37-Pin:4,23
    OpenCollectorOutput openCollectorOutput_3;
    //Connector: D-SUB 37 | Connector Pins: 11 | Arduino Pin: 45| Ratings: 40V,100mA | Usable Power Pins: 3.3V(Arduino)@D-SUB-37-Pin:3 ; 5V(Arduino))@D-SUB-37-Pin:22 ; 12V,2A@D-SUB-37-Pin:2,21 ; 24V,1.2A@D-SUB-37-Pin:4,23
    OpenCollectorOutput openCollectorOutput_4;
    //Connector: D-SUB 37 | Connector Pins: 30 | Arduino Pin: 46| Ratings: 40V,100mA | Usable Power Pins: 3.3V(Arduino)@D-SUB-37-Pin:3 ; 5V(Arduino))@D-SUB-37-Pin:22 ; 12V,2A@D-SUB-37-Pin:2,21 ; 24V,1.2A@D-SUB-37-Pin:4,23
    OpenCollectorOutput openCollectorOutput_5;
    //Connector: D-SUB 37 | Connector Pins: 13 | Arduino Pin: 53| Ratings: 3V-24V | The input is isolated with GND@D-SUB-37-Pin:31
    DigitalInIsolated digitalInIsolated_0;
    //Connector: D-SUB 37 | Connector Pins: 32 | Arduino Pin: 51| Ratings: 3V-24V | The input is isolated with GND@D-SUB-37-Pin:31
    DigitalInIsolated digitalInIsolated_1;
    //Connector: D-SUB 37 | Connector Pins: 14 | Arduino Pin: 50| Ratings: 3V-24V | The input is isolated with GND@D-SUB-37-Pin:31
    DigitalInIsolated digitalInIsolated_2;
    //Connector: D-SUB 37 | Connector Pins: 33 | Arduino Pin: 49| Ratings: 3V-24V | The input is isolated with GND@D-SUB-37-Pin:12
    DigitalInIsolated digitalInIsolated_3;
    //Connector: D-SUB 37 | Connector Pins: 15 | Arduino Pin: 53| Ratings: 3V-24V | The input is isolated with GND@D-SUB-37-Pin:12
    DigitalInIsolated digitalInIsolated_4;
    //Connector: D-SUB 37 | Connector Pins: 34 | Arduino Pin: 53| Ratings: 3V-24V | The input is isolated with GND@D-SUB-37-Pin:12
    DigitalInIsolated digitalInIsolated_5;
    //Connector: D-SUB 37 | Connector Pins: from: 17 to: 35 | Arduino Pin: 34| Ratings: 60V,0.3A
    MOSContact mosContact_0;
    //Connector: D-SUB 37 | Connector Pins: from: 18 to: 36 | Arduino Pin: 35| Ratings: 60V,0.3A
    MOSContact mosContact_1;
    //Connector: D-SUB 37 | Connector Pins: from: 19 to: 37 | Arduino Pin: 36| Ratings: 60V,0.3A
    MOSContact mosContact_2;
    
    //Connector: 4-Pole-Binder | Connector Pins: 1=GND, 2=negativeIn, 3=positiveIn, 4=I-Ref | Arduino Pin: ADC0(54)| Ratings: 0.2V-3.3V, 12bit | It is designed for 4-point measurement connect current wire from I-Ref to GND, and measure voltage from positiveIn to negativeIn
    AnalogInPt100 analogInPt100_0;
    
    //Connector: D-SUB-25 | Connector Pins: 1=GND, 2=negativeIn, 15=positiveIn, 14=I-Ref | Arduino Pin: ADC1(55)| Ratings: 0.2V-3.3V, 12bit | It is designed for 4-point measurement connect current wire from I-Ref to GND, and measure voltage from positiveIn to negativeIn
    AnalogInPt100 analogInPt100_1;
    //Connector: D-SUB-25 | Connector Pins: 3=GND, 4=negativeIn, 17=positiveIn, 16=I-Ref | Arduino Pin: ADC2(56)| Ratings: 0.2V-3.3V, 12bit | It is designed for 4-point measurement connect current wire from I-Ref to GND, and measure voltage from positiveIn to negativeIn
    AnalogInPt100 analogInPt100_2;
    //Connector: D-SUB-25 | Connector Pins: 5=GND, 18=24V, 6=Signal | Arduino Pin: ADC3(57) | Ratings: 2.2V-8.5V, 12bit | Input resistance: 12kOhm
    AnalogInGauge analogInGauge_0;
    //Connector: D-SUB-25 | Connector Pins: Signal=19, GND=7 | Arduino Pin: ADC4(58) | Ratings: 0V-10V, 12bit
    AnalogIn analogIn_0;
    //Connector: D-SUB-25 | Connector Pins: Signal=20, GND=8 | Arduino Pin: ADC5(59) | Ratings: 0V-10V, 12bit
    AnalogIn analogIn_1;
    //Connector: D-SUB-25 | Connector Pins: Signal=21, GND=9 | Arduino Pin: ADC6(60) | Ratings: 0V-10V, 12bit
    AnalogIn analogIn_2;
    //Connector: D-SUB-25 | Connector Pins: Signal=22, GND=10 | Arduino Pin: ADC7(61) | Ratings: 0V-10V, 12bit
    AnalogIn analogIn_3;
    //Connector: D-SUB-25 | Connector Pins: Signal=12, AGND=24 | Arduino Pin: DAC0(66) | Ratings: 0V-10V, 12bit
    AnalogOutIsolated analogOutIsolated_0;
    //Connector: D-SUB-25 | Connector Pins: Signal=13, AGND=25 | Arduino Pin: DAC1(67) | Ratings: 0V-10V, 12bit
    AnalogOutIsolated analogOutIsolated_1;

    //singelton lazy init
    static LSC& getInstance(){
      static LSC instance;
      return instance;
    }
    //##################################################
    //############## SERIAL COMUNICATION ###############
    //##################################################

        //---- Start Async UART explanation----
    /*
      The maximum Baud rate of the arduino due is 115200 giving us a theoretical maximal data transfer rate of 14.4 kB/s
      The UART works asynchronously. I.e. if Serial.print() is called the data is written into a buffer and then transmitted
      in the background. The time it takes to write data into the buffer is linear with data size. It takes about 163us to write 
      125B of data. The problem is the size of the buffer. The buffer has a size of 128B once the buffer is full the function wait
      for more space to become available before continuing. I.e sending more then 128B at a time will block the program for 
      a significant amount of time (sending 1026B blocks the program for 80ms).
      We solve this problem by defining a big ring buffer "uartBuffer". When we want to send data over the uart we dont write 
      the data to the uart buffer directly but first in the uartBuffer. 
      To write the data form the uartBuffer to the acctual uart we setup a timer with the handler TC2_Handler, that runs every 
      3.5ms and writes a junk of data smaller then the uart buffer to the uart. With this approach we can ensure, that the
      uart buffer will never be full. With this implementation we can send 1.1Kb of data every 100ms, or 11kB/s.
      Be carefull when filling the buffer from another timer. If the buffer overflows bad things happen. It is the responebility
      of the sending timer to make sure that the buffer can't overflow!!!
      Using the LSC::getInstance().println() function to send 1100B takes about 1.3ms. 
      TLDR: Sending 1.1KB per OS tick (100ms) takes about 5.3 ms in cpu time.
      TLDR: As of now you are responsible to handle the buffer! there is no error handling for multithreaded applicatoins!!! 
    */
      //---- End Async UART explanation----

      //TODO: we need better error handling. there should be a watchdog and better hadling of buffer full conditions.

    //Sends a Sting using the uart. This function is asynchronous and non blocking. Data is being sent in the background. DONT OVERFLOW THE BUFFER!
    void print(String data){ 
      TC_Stop(TC0, 2);    //Stopping the uart send timer, to avoid reace conditions
      bool bufferIsFull = false; //Temporary variable indicating wether the ring buffer is full or not

      for(char character : data){ //we push char by char
      //for(size_t i = 0; i<data.length();i++){ <= This is acctually slower by about 700us idk why XD
        if (uartBuffer.isFull()){ //if the buffer is full we need to restart the urat send timer or the buffer would never get smaller
          TC_Start(TC0, 2);
          bufferIsFull = true;
        } 
        while (bufferIsFull){ // we wait for the uart timer to process the data.
          if (!uartBuffer.isFull()){
            bufferIsFull = false;
            TC_Stop(TC0, 2); // once some data has been processed and there is room in the buffer we need to stopp the timer again.
          } 
        }
        uartBuffer.push(character); //Pushing one char into the ring buffer
      }
      TC_Start(TC0, 2); //Once the write operation has concluded we can restart the send timer
    }

    //Sends a Sting using the uart. This function is asynchronous and non blocking. Data is being sent in the background
    void println(String data){ 
        print(data + '\n');
    }

};

#endif