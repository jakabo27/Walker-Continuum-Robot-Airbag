/*
  This sketch prints the values of the control panel inputs
  
*/


#include <Bounce2.h>

//Rotary Knob Button
#define ROTARY_KNOB_BUTTON 32
bool knobButton_State = 0; 
Bounce2::Button knobButton = Bounce2::Button();

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
byte dualB_State = 0; //0 to 1
byte dualC_State = 0; //0 to 1

//Arm switch
#define ARMSW_PIN 31
bool armSw_State = 0; 

//Power switch AKA Magnet brake switch
#define POWSW_PIN 34
bool powSw_State = 0;
                      
//Falling directions and thresholds
byte fallDirection = 0;
enum fallDirectionEnum{
  STABLE,
  FALL_LEFT, 
  FALL_RIGHT,
  FALL_FORWARD
};
String fallDirNames[] = {"Stable", "Left","Right","Forward"};

enum robotState { 
  STANDBY, //Keep the drill awake, display the IMU data on LCD
  TUNE_ALL_STEPPERS, //Using rotary knob to turn all the stepper motors at once
  TUNE_ONE_STEPPER,  //Using rotary knob to move one stepper at a time
  TUNE_DRILL_MOTOR, //Slowly turning the drill motor
  ARMED_AND_READY,  // ready to deploy via button or tilting. All steppers at 0. 
  DEPLOYING, 
  DEPLOYED_WAIT_FOR_RESET,
  RESETTING,
  UNDEFINED
};


//Misc
int state = 0;    //The current global robot state
int oldState = 0; //Previous robot state.  
bool newStateFirstTime = 0;
bool alreadyDeployed = 1;
elapsedMillis printTimer;



void setup()
{

  Serial.begin(115200); delay(100);
  //while (!Serial) {delay(1);}; //Doesn't start program until serial monitor open. Bad for battery powered 
  Serial.println("Starting!");

  //Buttons and switches
  pinMode(ARMSW_PIN,    INPUT_PULLUP); //TBD on if we need debouncing here, probably not. 
  pinMode(POWSW_PIN, INPUT_PULLUP);
  pinMode(DUAL_A_1_PIN,   INPUT_PULLUP);
  pinMode(DUAL_A_2_PIN,   INPUT_PULLUP);
  pinMode(DUAL_B_12_PIN,   INPUT_PULLUP);
  pinMode(DUAL_C_12_PIN,   INPUT_PULLUP);
  deployButton.attach(DEPLOY_BUTTON_PIN, INPUT_PULLUP);
  deployButton.interval(25); // Use a debounce interval of 25 milliseconds
  deployButton.setPressedState(LOW); // INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON
  knobButton.attach(ROTARY_KNOB_BUTTON, INPUT_PULLUP);
  knobButton.interval(15); // Use a debounce interval of 25 milliseconds
  knobButton.setPressedState(LOW); // INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON


  CheckInputs();


}

void loop()
{
	DetermineState(); //Read inputs and global variables and determine the robot state

  if(printTimer > 100)
  {
  Serial.print("state:" + String(state));
  Serial.print("\tAState:"+String(dualA_State));
  Serial.print("\tBState:"+ String(dualB_State));
  Serial.print("\tCState:"+ String(dualC_State));
  Serial.print("\tArmSW:"+String(armSw_State));
  Serial.print("\tPowSW:"+String(powSw_State));
  Serial.print("\tDeploy:"+String(deployButton_State));
  Serial.println("\tKnob:"+ String(knobButton_State));\
  printTimer = 0;
  }
      
}
