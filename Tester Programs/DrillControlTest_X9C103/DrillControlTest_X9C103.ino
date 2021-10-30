/*  
    This sketch tests two X9C103 digital potentiometers, using a 
        GPIO expander for the UP/DOWN and ChipSelect pins.

    It uses commands input from the Serial monitor to set the values of the digipots and relays.  

    The X9C103 library has been integrated to this sketch so that the pins could be modified to use
    the GPIO expander pins instead of normal I/O pins. 

    See the thesis Electronics section for the circuit diagram.  

    Modified from ElectroPeak example:  
      https://electropeak.com/learn/interfacing-x9c103s-10k-digital-potentiometer-module-with-arduino/
*/

/*                                                                                                                                     
 * For this example, connect your X9C104 (or the like) as follows:
 * 1 - INC - Arduino pin 2
 * 2 - U/D - Arduino pin 3
 * 3 - RH  - 5V
 * 4 - VSS - GND
 * 5 - RW  - Output: Arduino pin A0 for analogRead
 * 6 - RL  - GND
 * 7 - CS  - Arduino pin 4
 * 8 - VCC - 5V
 */

#include <Adafruit_AW9523.h> //GPIO Expander
//#include <Adafruit_DS3502.h> // I2C Digital Potentiometer


// GPIO Expander
Adafruit_AW9523 aw;

//Drill
int DRILL_DIR_PIN = 15;
int DRILL_SW_PIN = 14;
bool currentDirection = 0;

int drillTuneMaxSpeed = 15; //Between 0 and 127, 0 is slowest.  Speed to turn drill in tuning mode
int drillTuneTime = 190; //ms to turn the drill in tune mode
int shaftStepsBeforeStopping = -600; //encoder counts to turn the drill before turning it off
int drillMaxDeployTime = 800; //ms to spin the dril during deployment max
int deployDrillSpeed = 127; //Normally 127 (full speed), set slower for initial testing. 


#define DIGIPOT_UP   HIGH
#define DIGIPOT_DOWN LOW
#define DIGIPOT_MAX_AMOUNT 99
#define DIGIPOT_UNKNOWN 255

unsigned long printTimer = millis();
float volt1; 
int newSpot = 0;

// X9C DigiPot class setup
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

DigiPot pot(35,3,2);
DigiPot pot2(7,6,5);

void setup() {
  Serial.begin(115200);

  while (! aw.begin(0x58)) {
    Serial.println("AW9523 not found? Check wiring!");
    delay(100);  
  }
  Serial.println("Adafruit AW9523 GPIO Expander found!");
  aw.pinMode(DRILL_DIR_PIN, OUTPUT);
  aw.pinMode(DRILL_SW_PIN, OUTPUT);
  aw.digitalWrite(DRILL_DIR_PIN, 0);
  DrillSwitchOFF();

  aw.pinMode(4, OUTPUT);
  aw.pinMode(3, OUTPUT);
  aw.pinMode(2, OUTPUT);
  aw.digitalWrite(2, HIGH);
  aw.pinMode(7, OUTPUT);
  aw.pinMode(6, OUTPUT);
  aw.pinMode(5, OUTPUT);
  aw.digitalWrite(5, HIGH);
  
}

void loop() {

  if (Serial.available() > 0) {
    newSpot = Serial.parseInt();
    if(newSpot == 200)
    {
      Serial.println("Turned ON Drill Switch");
      DrillSwitchON();
    }
    else if(newSpot == 400)
    {
      Serial.println("Turned OFF Drill Switch");
      DrillSwitchOFF();
    }

    else if(newSpot == 700)
    {
      Serial.println("Set Direction to CCW");
      DrillCCW();
    }
    else if(newSpot == 800)
    {
      Serial.println("Set Direction to Clockwise");
      DrillClockwise();
    }
    else if(newSpot >= 1000 && newSpot <1100)
    {
      pot2.set(newSpot - 1000);
      Serial.println("Set pot 2 to " + String(newSpot - 1000));
    }
    else if(newSpot >= 1100 && newSpot <=1200)
    {//because I keep typing 1120 etc
      pot2.set(newSpot - 1100);
      Serial.println("Set pot 2 to " + String(newSpot - 1100));
    }
    else
    {
      pot.set(newSpot);
      Serial.println("Set pot 1 to " + String(newSpot));
    }
  }
  delay(1);

//  if(millis() - printTimer > 20)
//  {
//    printTimer = millis();
//    volt1 = 5.0 * analogRead(A0)  / 1024;
//    Serial.println(volt1);
//  }
  
//##### Increase it from minimum to maximum ########
//  for (int i=0; i<100; i++) {
//    pot.increase(1);
//    volt1 = 5.0 * analogRead(A0)  / 1024;
//    Serial.println(volt1);
//    delay(20);
//  }
//  
//  
//  for (int i=0; i<100; i++) {
//    pot.decrease(1);
//    volt1 = 5.0 * analogRead(A0) / 1024;
//    Serial.println(volt1);
//    delay(20);
//  }

}





 
