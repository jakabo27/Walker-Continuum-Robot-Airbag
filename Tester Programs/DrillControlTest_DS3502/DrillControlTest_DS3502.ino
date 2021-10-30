/*
 * This sketch tests the DS3502 Digital Potentiometer by spinning the drill
 *    from slow to fast and back to slow.  
 * The sketch uses an AW9523 GPIO expander for some of the drill relay pins. 
 * 
 * Originally, we used a DS3502 digital potentiometer from Adafruit
 * Product link:  https://www.adafruit.com/product/4286
 * Arduino tutorial:  https://learn.adafruit.com/ds3502-i2c-potentiometer/arduino
 * Library:  https://github.com/adafruit/Adafruit_DS3502
 * 
*/


#include <Adafruit_DS3502.h>
#include <Adafruit_AW9523.h>

// DigiPot
Adafruit_DS3502 ds3502 = Adafruit_DS3502();
//0x28 ///< DS3502 default I2C address
//By default the library uses Wire - https://github.com/adafruit/Adafruit_DS3502/blob/master/Adafruit_DS3502.h
//To specify, do this in setup:
//      ds3502.begin(0x28, Wire); //and change Wire to Wire1 or Wire2

// GPIO Expander
Adafruit_AW9523 aw;


int DRILL_DIR_PIN = 15;
int DRILL_SW_PIN = 14;
bool currentDirection = 0;

int drillTuneMaxSpeed = 15; //Between 0 and 127, 0 is slowest.  Speed to turn drill in tuning mode
int drillTuneTime = 190; //ms to turn the drill in tune mode
int shaftStepsBeforeStopping = -600; //encoder counts to turn the drill before turning it off
int drillMaxDeployTime = 800; //ms to spin the dril during deployment max
int deployDrillSpeed = 127; //Normally 127 (full speed), set slower for initial testing. 



void setup() {
  Serial.begin(115200);

  Serial.println("Drill Control Test");
  while (!ds3502.begin()) {
    Serial.println("Couldn't find DS3502 chip");
    delay(200);
  }
  Serial.println("Found DS3502 chip");

  ds3502.setWiper(0);
  delay(100);

  //GPIO Expander
  while (! aw.begin(0x58)) {
    Serial.println("AW9523 not found? Check wiring!");
    delay(100);  
  }
  Serial.println("Adafruit AW9523 GPIO Expander found!");
  aw.pinMode(DRILL_DIR_PIN, OUTPUT);
  aw.pinMode(DRILL_SW_PIN, OUTPUT);
  aw.digitalWrite(DRILL_DIR_PIN, 0);
  DrillSwitchOFF();
}


void loop() {
  // put your main code here, to run repeatedly:
  ds3502.setWiper(0);

  //pull trigger in as far as pot goes
  //DrillSwitchON();
  for (int i = 0; i<128; i+=1)
  {
    ds3502.setWiper(i);
    if(i%5 ==0)
      Serial.println(i);
    delay(10);
  }
  //Serial.println("Wiper value:  " + String(ds3502.getWiper()));
  
  delay(500);

  for (int i = 127; i>=0; i-=1)
  {
    ds3502.setWiper(i);
    if(i%5 ==0)
      Serial.println(i);
    delay(10); //slow down a little faster than speeding up just for fun. 
  }
  
  DrillSwitchOFF();
  //Serial.println("Wiper value:  " + String(ds3502.getWiper()));
  delay(800);

  //change direction
  if(currentDirection)
    DrillClockwise();
  else DrillCCW();

  currentDirection = !currentDirection;
  

}

void DrillSwitchOFF()
{
  aw.digitalWrite(DRILL_SW_PIN, 1);
}

void DrillSwitchON()
{
  aw.digitalWrite(DRILL_SW_PIN, 0);
}

void DrillClockwise()
{

  if(aw.digitalRead(DRILL_DIR_PIN) != 1)
  {
    //Need to really tell the drill we want to change directions
    aw.digitalWrite(DRILL_DIR_PIN, 1); 
    delay(50);
    DrillSwitchON();
    aw.digitalWrite(DRILL_DIR_PIN, 1); 
    delay(30);
    DrillSwitchOFF();
    aw.digitalWrite(DRILL_DIR_PIN, 1); 
  }
  else
  {
    aw.digitalWrite(DRILL_DIR_PIN, 1); 
  }

  //aw.digitalWrite(DRILL_DIR_PIN, 1); 
  
}

void DrillCCW()
{

  bool needSwitchFlutter = false;
  if(aw.digitalRead(DRILL_DIR_PIN) != 0)
  {
    //Need to really tell the drill we want to change directions
    aw.digitalWrite(DRILL_DIR_PIN, 0); 
    delay(70);
    DrillSwitchON();
    aw.digitalWrite(DRILL_DIR_PIN, 0); 
    delay(30);
    DrillSwitchOFF();
    aw.digitalWrite(DRILL_DIR_PIN, 0); 
    //delay(100);
  }
  else
  {
    aw.digitalWrite(DRILL_DIR_PIN, 0); 
  }
  
  //aw.digitalWrite(DRILL_DIR_PIN, 0); 
}
