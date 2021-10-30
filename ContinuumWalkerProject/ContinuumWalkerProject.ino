/*
	Name:       WalkersWalkerV3.ino
	Created:	8/7/2021 12:01:20 AM
	Author:     Jacob Thompson
  
	Library Links:  
	WS2812Serial:  https://github.com/PaulStoffregen/WS2812Serial/blob/master/examples/BasicTest_RGBW/BasicTest_RGBW.ino
	Bounce2:  https://github.com/thomasfredericks/Bounce2  See the Button example at the bottom
  DigiPotX9Cxxx (embedded within this code for use with the GPIO expander) - https://electropeak.com/learn/interfacing-x9c103s-10k-digital-potentiometer-module-with-arduino/
  ICM_20948:  https://github.com/sparkfun/SparkFun_ICM-20948_ArduinoLibrary
  
 
*/

#include <Encoder.h>           // Encoders
#include <AccelStepper.h>      // Stepper Motor Drivers
#include <ICM_20948.h>         // IMU (Intertial Measurement Unit)
#include <WS2812Serial.h>      // LED Strip
#include <Bounce2.h>           // Debouncing buttons and switches
#include <LCD_I2C.h>           // LCD Display
#include <Adafruit_AW9523.h>   // GPIO Expander
#include <SD.h>                // SD card
#include <SPI.h>               // SD card

// SD card
const int chipSelect = BUILTIN_SDCARD;
bool SDWorking = false;


//IMU
#define WIRE_PORT Wire1 //Wire1 for IMU and LCD
#define AD0_VAL 1
ICM_20948_I2C imu; //Create an ICM_20948_I2C object
float IMU_LeftRight = 0; //Values to keep track of the current value
float IMU_ForwardBack = 0;
float IMU_Stable_LR = 0; //Used for saving the "Stable" value when you Arm the device.
float IMU_Stable_FB = 0;
float FallLeftDelta= 0.08; //How much change in that direction to count as falling. 
float FallRightDelta = 0.11;
float FallForwardDelta = 0.10;

//More IMU variables
float declination = 6.28;
float p[] = {1, 0, 0};  //X marking on sensor board points toward yaw = 0
float Axyz[3], Mxyz[3]; //centered and scaled accel/mag data
float A_B[3]  {   79.60,  -18.56,  383.31};
float A_Ainv[3][3]
{ {  1.00847,  0.00470, -0.00428},
  {  0.00470,  1.00846, -0.00328},
  { -0.00428, -0.00328,  0.99559}
};
float M_B[3]  { -156.70,  -52.79, -141.07};
float M_Ainv[3][3]
{ {  1.12823, -0.01142,  0.00980},
  { -0.01142,  1.09539,  0.00927},
  {  0.00980,  0.00927,  1.10625}
};
//IMU Filtering/smoothing.
const int numReadings = 10;         // Number of measurements to average
int readIndex = 0;
float IMU_LR_readings[numReadings];
float IMU_FB_readings[numReadings];
float IMU_LR_total = 0;
float IMU_FB_total = 0;
float IMU_LR_average = 0;
float IMU_FB_average = 0;
float IMU_LR_RAW = 0;
float IMU_FB_RAW = 0;

//LCD 
//NOTE - I went into the LCD library files and changed all "Wire" to "Wire1"
LCD_I2C lcd(0x27); 
elapsedMillis LCDUpdate;
int LCDRefreshRate = 250; //ms between updating LCD 

// GPIO Expander
Adafruit_AW9523 aw;

// Stepper Encoders
// Positive direction = spooling out (arm pulling), negative is winding in.  
//Positive = clockise if looking at the back of the motor like as a drill. 
#define KNOB_ENCODER_A 4 
#define KNOB_ENCODER_B 5
#define ROTARY_KNOB_BUTTON 32
#define S1_ENCODER_A 7
#define S1_ENCODER_B 6
#define S2_ENCODER_A 9
#define S2_ENCODER_B 8
#define S3_ENCODER_A 11
#define S3_ENCODER_B 10
#define S4_ENCODER_A 25
#define S4_ENCODER_B 24
Encoder encoderS1(S1_ENCODER_A, S1_ENCODER_B);
Encoder encoderS2(S2_ENCODER_A, S2_ENCODER_B);
Encoder encoderS3(S3_ENCODER_A, S3_ENCODER_B);
Encoder encoderS4(S4_ENCODER_A, S4_ENCODER_B);
int EncoderCPR = 192;	//Count Per Revolution (CPR) for the encoder.  They're all set to 48 PPR = 192 CPR. 
int StepsPerRevolution = 200; //In case we change and use microstepping.

//Shaft encoder
#define SHAFT_ENCODER_A 3
#define SHAFT_ENCODER_B 2 //Clockwise from top perspective (like of drill) = positive counting. 
Encoder shaftEncoder(SHAFT_ENCODER_A, SHAFT_ENCODER_B);
//Same CPR as the other encoders. 

//Rotary Knob Encoder and Button
Encoder KnobEncoder(KNOB_ENCODER_A, KNOB_ENCODER_B);
bool knobButton_State = 0; 
Bounce2::Button knobButton = Bounce2::Button();
long knobPosition = 0;
long knobOldPosition = 0;
int knobStepAmount = 70; //Steps to turn motors when controlled by the encoder knob
int knobAcceleration = 3000; //Motor acceleration while tuning
int knobMaxSpeed = 200;      //Motor  speed while tuning
int knobSingleControlNum = 1; //1 - 4 which motor to move in single tune mode
int knobArmedStepAmount = 45;


// Motor Setup
AccelStepper stepper1(AccelStepper::DRIVER, 23, 21); // (Driver==1, Step, Dir)
AccelStepper stepper2(AccelStepper::DRIVER, 22, 20);
AccelStepper stepper3(AccelStepper::DRIVER, 15, 41);
AccelStepper stepper4(AccelStepper::DRIVER, 14, 40);
#define S1_ENABLE 39 //Driver enable pins for each stepper
#define S2_ENABLE 38
#define S3_ENABLE 37
#define S4_ENABLE 36
int globalMinPulseWidth = 2; //uS.  Library recommends 20, driver supports down to 1.9 
int deployAcceleration = 25000; //Fastest we can go is 25000 ish
int deployMaxSpeed = 5000;
int deployWhenEncoderMoves = 2; //Counts for the encoder to be pulled by cable before deploying the stepper. 

//Magnet brakes - on GPIO Expander
#define BRAKE1 8
#define BRAKE2 9
#define BRAKE3 10
#define BRAKE4 11

//Drill Control
#define DIGIPOT_UP   HIGH
#define DIGIPOT_DOWN LOW
#define DIGIPOT_MAX_AMOUNT 99
#define DIGIPOT_UNKNOWN 255
#define DRILL_DIR_PIN 15 // Clockwise == 1, CCW = 0
#define DRILL_SW_PIN 14 //1 = not pulled, 0 = pulled

//Digital potentiometer library modified for GPIO Expander is included in these files
class DigiPot
{
 public:
  DigiPot(uint8_t incPin, uint8_t udPin, uint8_t csPin);
  void increase(uint8_t amount);
  void decrease(uint8_t amount);
  void change(uint8_t direction, uint8_t amount);
  void set(uint8_t value);
  uint8_t get();
  void reset();
 private:
  uint8_t _incPin;
  uint8_t _udPin;
  uint8_t _csPin;
  uint8_t _currentValue;
};
#define POT1_INC 35 //Increment pins are NOT on GPIO Expander for delay issue.  
#define POT1_UPDOWN 3 //UPDOWN and CS pins are on GPIO expander.  See DigiPot.ino
#define POT1_CS 2
#define POT2_INC 33
#define POT2_UPDOWN 6
#define POT2_CS 5

DigiPot pot1(POT1_INC, POT1_UPDOWN, POT1_CS); // Digital Potentiometer 1 (lower) on GPIO Expander
DigiPot pot2(POT2_INC, POT2_UPDOWN, POT2_CS); // Digital Potentiometer 2 (upper) on GPIO Expander

int drillTuneSpeed = 7; //Between 0 and 127, 0 is slowest.  Speed to turn drill in tuning mode
int drillTuneTime = 150; //ms to turn the drill in tune mode
int shaftStepsBeforeStopping = -600; //encoder counts to turn the drill before turning it off
int drillMaxDeployTime = 700; //ms to spin the drill during deployment max (typical ~700)
int deployDrillSpeed = 95; //0 to 99, 99 is fastest. 90 maybe needed for left/right with ring
elapsedMillis drillAwakeTimer; //used for keeping the drill from turning off in certain functions
bool drillCurrentDir = 0;

// DEPLOY button
#define DEPLOY_BUTTON_PIN 30
#define DEPLOY_LED 0
bool deployButton_State = 0;
Bounce2::Button deployButton = Bounce2::Button();

//Dual mode switches
#define DUAL_A_1_PIN 26
#define DUAL_A_2_PIN 27
#define DUAL_B_12_PIN 28
#define DUAL_C_12_PIN 29
byte dualA_State = 0; //0 to 2
byte dualB_State = 0; //0 to 1 (wired as a nomal switch)
byte dualC_State = 0; //0 to 1

//Arm switch
#define ARMSW_PIN 31
bool armSw_State = 0; 

//Power switch AKA Magnet brake switch
#define POWSW_PIN 34
bool powSw_State = 0;

//WS2811 strip
const int NUM_LEDS = 4;
const int DATA_PIN = 1;
byte drawingMemory[NUM_LEDS*3];         //  3 bytes per LED
DMAMEM byte displayMemory[NUM_LEDS*12]; // 12 bytes per LED
WS2812Serial leds(NUM_LEDS, displayMemory, drawingMemory, DATA_PIN, WS2812_GRB); //Non-blocking library
#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define YELLOW 0xFFFF00
#define ORANGE 0xE05800
#define WHITE  0xFFFFFF
#define BLACK  0x000000

//Data logging
int logPeriod = 5000; //micro seconds 
String logHeaders[] = {"timeSinceDeploy","E1","E2","E3","E4","Shaft",
                        "DrillSwitch",
                        "S1Enable","S2Enable","S3Enable","S4Enable",
                        "S1Driving","S2Driving","S3Driving","S4Driving",
                        "IMU_LR","IMU_FB", "IMU_LR_Accel", "IMU_FB_Accel",
                        "IMU_Other_Accel"};
const int logDataRows = 401; //= MaxDeployTime / logPeriod. And one extra row because I print a 0 row.  
const byte logDataCols = 20;
float logData[logDataRows][logDataCols]; 
int logCounter = 0;
                        

//Falling directions and thresholds
//Direction is from the users perspective.
byte fallDirection = 0;  // Used in deploy to determine how far encoder spots are.
const int deployButtonNum = 1; // Which direction it does when the deploy button is pressed
enum fallDirectionEnum{
  STABLE,
  FALL_LEFT, 
  FALL_RIGHT,
  FALL_FORWARD
};
String fallDirNames[] = {"Stable", "Left","Right","Forward"};

//Stepper motor lengths to spool out for LEFT, RIGHT, FORWARD
int goalEncoderSpots [3][4]
{
  {50, 1500, 500, 50},   //FALL LEFT around hip 
  {1300, 50, 5,  900},   //FALL RIGHT- on ground (not hip)
//  {50, 10, 900, 900}    //FALL FORWARD - Downward (1 and 2 are bottom motors)
  {1200, 1200, 50, 50}    //FALL FORWARD - upward (3 and 4 are the top motors)
};



enum robotState { 
	STANDBY,  
	TUNE_ALL_STEPPERS, //Using rotary knob to turn all the stepper motors at once
	TUNE_ONE_STEPPER,  //Using rotary knob to move one stepper at a time
	TUNE_DRILL_MOTOR,  //Using rotary knob to slowly turn the drill motor
	ARMED_AND_READY,	 // ready to deploy via button or tilting. All steppers at 0. 
	DEPLOYING, 
	DEPLOYED_WAIT_FOR_RESET,
	UNDEFINED
};

//Misc
int state = 0;    //The current global robot state
int oldState = 0; //Previous robot state.  
bool armedReadyFirstTime = 1; 
bool newStateFirstTime = 0;
bool alreadyDeployed = 1;


void setup()
{

	Serial.begin(115200); delay(100);
	//while (!Serial) {delay(1);}; //Doesn't start program until serial monitor open. Good for PC tethered testing. 
  Serial.println("Starting!");

	//Buttons and switches
  pinMode(ARMSW_PIN,      INPUT_PULLUP); 
  pinMode(POWSW_PIN,      INPUT_PULLUP);
  pinMode(DUAL_A_1_PIN,   INPUT_PULLUP);
  pinMode(DUAL_A_2_PIN,   INPUT_PULLUP);
  pinMode(DUAL_B_12_PIN,  INPUT_PULLUP);
  pinMode(DUAL_C_12_PIN,  INPUT_PULLUP);
  deployButton.attach(DEPLOY_BUTTON_PIN, INPUT_PULLUP);
  deployButton.interval(50); // Use a debounce interval of 25 milliseconds
  deployButton.setPressedState(LOW); // INDICATE THAT THE LOW STATE == button physically pressed
  knobButton.attach(ROTARY_KNOB_BUTTON, INPUT_PULLUP);
  knobButton.interval(15); // Use a debounce interval of 25 milliseconds
  knobButton.setPressedState(LOW); 


	//LEDs
	pinMode(DEPLOY_LED, 	OUTPUT); //LED ring around the Deploy button
  digitalWrite(DEPLOY_LED, LOW);
	leds.begin(); //addressable WS2812 led strip on control panel
 	leds.setBrightness(150);
	setAll(BLACK);
	leds.show();


  //GPIO Expander
  while (! aw.begin(0x58)) {
    Serial.println("AW9523 not found? Check wiring!");
    delay(100);  
  }
  Serial.println("Adafruit AW9523 GPIO Expander found!");
  aw.pinMode(BRAKE1, OUTPUT);
  aw.pinMode(BRAKE2, OUTPUT);
  aw.pinMode(BRAKE3, OUTPUT);
  aw.pinMode(BRAKE4, OUTPUT);
  aw.pinMode(POT1_INC,    OUTPUT);
  aw.pinMode(POT1_UPDOWN, OUTPUT);
  aw.pinMode(POT1_CS,     OUTPUT);
  aw.pinMode(POT2_INC,    OUTPUT);
  aw.pinMode(POT2_UPDOWN, OUTPUT);
  aw.pinMode(POT2_CS,     OUTPUT);
  aw.digitalWrite(POT1_CS, HIGH);   // Enable the digital pots
  aw.digitalWrite(POT2_CS, HIGH);
  aw.pinMode(DRILL_DIR_PIN, OUTPUT); // Direction and Switch for drill trigger
  aw.pinMode(DRILL_SW_PIN,  OUTPUT);
  aw.digitalWrite(DRILL_DIR_PIN, LOW);
  DrillSwitchOFF();
  BrakesAllOFF(); // Turn all brakes off so we can spin steppers freely. Uses a lot of power.  
  DrillSetSpeed(0);
  
  //LCD
  lcd.begin(false);
  lcd.backlight();

  //More setup functions
	MotorSetup();
	IMUSetup();
  KeepDrillAwake(); //Wake up the drill 
  
	CheckInputs();

  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    for (int i = 0; i<5; i++)
      {
        // No SD card, so don't do anything more
        Serial.println("No card found... trying " + String(i) + " more times.");
        delay(100);
      }
  }
  else{
  Serial.println("  card initialized.");
  SDWorking = true;
  }
}


void loop()
{
	DetermineState(); 

	switch (state) {
    case STANDBY:
      Standby(); //keep alive function basically
      break;
    case TUNE_ALL_STEPPERS:
      TuneAllSteppers();
      break;
    case TUNE_ONE_STEPPER:
      TuneOneStepper();
      break;
    case TUNE_DRILL_MOTOR:
      TuneDrillMotor();
      break;
    case ARMED_AND_READY:
      ArmedAndReady();
      break;
    case DEPLOYING:
      DeployingNOW(); //blocking function
      break;
    case DEPLOYED_WAIT_FOR_RESET:
      DeployedWaitForReset();
      break;
    case UNDEFINED:
      //continue to check the switches until a valid state exists
      break;
    default:
      //same as UNDEFINED. 
      break;
	}  
}
