  /*  
   *   
   *   This sketch tests the wiring of two X9C103 digital potentiometers.
   *   
   *   Values entered in the Serial monitor (0-100 or 1000-1100) are sent 
   *   to the digipot (0-100, if the value is >=1000 it is decreased by 
   *   1000 and written to the second digipot)
   *   
   *   The digital potentiometers should be wired in series, and the resulting 
   *   voltage is read by analog input pin 0
   *   
    Modified by Mehran Maleki from Arduino Examples
    https://electropeak.com/learn/

    Download the library from here:  https://electropeak.com/learn/interfacing-x9c103s-10k-digital-potentiometer-module-with-arduino/
*/

/*                                                                                                                                     
 * For this example, connect your first X9C1043:
 * 1 - INC - Arduino pin 2
 * 2 - U/D - Arduino pin 3
 * 3 - RH  - NC (not connected)
 * 5 - RW  - Output: to digipot2 RL
 * 6 - RL  - GND
 * 7 - CS  - Arduino pin 4
 * 8 - VCC - 5V
 * 
 * * For this example, connect your second X9C1043:
 * 1 - INC - Arduino pin 5
 * 2 - U/D - Arduino pin 6
 * 3 - RH  - 5V
 * 4 - VSS - GND
 * 5 - RW  - Output: Arduino pin A0 for analogRead
 * 6 - RL  - from digipot1 RW
 * 7 - CS  - Arduino pin 7
 * 8 - VCC - 5V
 * 
 */

#include <DigiPotX9Cxxx.h>

DigiPot pot1(2,3,4);
DigiPot pot2(5,6,7);

unsigned long printTimer = millis();
float volt1; 
int newSpot = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {

  if (Serial.available() > 0) {
    newSpot = Serial.parseInt();
    if(newSpot < 1000)
    {
      pot1.set(newSpot);
    }
    else
    {
      pot2.set(newSpot-1000);
    }
    
  }
  // Print out the voltage read from the digipot circuit to see if it's working
  if(millis() - printTimer > 20)
  {
    printTimer = millis();
    volt1 = 5.0 * analogRead(A0)  / 1024;
    Serial.println(volt1);
  }
  

//######## Ramp the digipot up and down ############
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
 
