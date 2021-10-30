/*
	This sketch reads the values of 5 AMT102-V (or AMT103) encoders
      and prints the value to the serial monitor. 
*/

#include <Encoder.h>

// Timer
elapsedMillis updateTimer;
int refreshRate = 250; //ms between updating values on Serial monitor

// Stepper Encoders
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

// Shaft encoder
#define SHAFT_ENCODER_A 3
#define SHAFT_ENCODER_B 2 //Clockwise from top perspective (like of drill) = positive counting. 
Encoder shaftEncoder(SHAFT_ENCODER_A, SHAFT_ENCODER_B);

// Rotary Knob Encoder
#define KNOB_ENCODER_A 4
#define KNOB_ENCODER_B 5
Encoder KnobEncoder(KNOB_ENCODER_A, KNOB_ENCODER_B);


void setup()
{
	Serial.begin(115200); delay(100);
  Serial.println("Starting encoder tester!");
}


void loop()
{
	if(updateTimer > refreshRate)
  {
    // Print all the encoder values
    Serial.print("Shaft: " + String(shaftEncoder.read()));
    Serial.print("\tKnob: " + String(KnobEncoder.read()));
    Serial.print("  \tS1: " + String(encoderS1.read()));
    Serial.print(" \tS2: " + String(encoderS2.read()));
    Serial.print(" \tS3: " + String(encoderS3.read()));
    Serial.print(" \tS4: " + String(encoderS4.read()));
    Serial.println("");

    updateTimer = 0;
  }    
}
