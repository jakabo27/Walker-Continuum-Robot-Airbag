/* MotorTune functions Control the motors for short movements.
 * Used for spinning the drill using the control panel knob, or 
 * resetting stepper motor positions between tests.  
*/

//Change something on the drill every 8 seconds to keep it awake
void KeepDrillAwake()
{
  DrillSwitchON(); //Will do the initial waking up if nothing else
  SetDrillSpeed(4);
  delay(40);
  SetDrillSpeed(0);
  delay(50);

  //Flip direction
  if(!armSw_State)
  {
    DrillFlipDirection();
  }
  
}

void TuneDrillMotor()
{
  //First time
  if (newStateFirstTime)
  {
    setAll(BLACK);
    leds.setPixel(0, ORANGE);  leds.setPixel(3, ORANGE);
    leds.setPixel(1, RED);  leds.setPixel(2, RED);
    leds.show();
    Serial.println("Control Drill");

    //LCD
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TUNE DRILL");
    lcd.setCursor(2, 1);
    lcd.print("Clockwise");

    DrillClockwise();

    //Encoder
    shaftEncoder.readAndReset();

    newStateFirstTime = false;
  }

  if(drillAwakeTimer > 8000)
  {
    KeepDrillAwake();
    drillAwakeTimer = 0;
  }


  knobPosition = KnobEncoder.readAndReset();

  if (knobPosition != knobOldPosition)
  {
    //Update LCD
    if (LCDUpdate > LCDRefreshRate)
    {
      lcd.clear();
      lcd.noCursor();
      lcd.setCursor(0, 0);
      lcd.print("TUNE DRILL");
      lcd.setCursor(0, 1);
      if (knobPosition > knobOldPosition) lcd.print("Clockwise");
      else lcd.print("CCW");
      lcd.setCursor(11, 1);
      lcd.print(shaftEncoder.read());
      //Serial.println("ShaftEncoder:  " + String(shaftEncoder.read()));
      LCDUpdate = 0; //Reset Timer
    }

    //Serial.println("KnobPosition=" + String(knobPosition) + "   OldPos:" + String(knobOldPosition));
    //Knob turned clockwise(?) need to test.
    if (knobPosition > knobOldPosition)
    { DrillClockwise();
      Serial.println("Go Clockwise");
    }
    else
    { DrillCCW();
      Serial.println("Go CCW");
    }

    
    DrillSwitchON(); 
    unsigned long thisStartms = millis();
    DrillSetSpeed(drillTuneSpeed);
    unsigned long thisRunTime = millis() - thisStartms;
    drillAwakeTimer = 0;

    
    //Delay so the total delay is as set.
    delay(drillTuneTime);
    

    //Turn off (stop spinning)
    DrillSetSpeed(0);
    DrillSwitchOFF(); //Undo the switch
    
    Serial.println("Runtime:  " + String(thisRunTime));
    lcd.setCursor(11, 1);
    lcd.print(shaftEncoder.read());
    Serial.println("ShaftEncoder:  " + String(shaftEncoder.read()));
    delay(100); //Give the relays time to latch

    knobOldPosition = 0;
    //KnobEncoder.readAndReset();
  }

}

void TuneAllSteppers()
{
  
  //First time
  if (newStateFirstTime)
  {
    //Turn on all LED lights to show displaying all of them
    setAll(BLUE);
    leds.show();
    Serial.println("Control ALL Stepper Motors");

    //LCD
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("TUNE ALL");
    lcd.setCursor(3, 1);
    lcd.print("STEPPERS");

    //Reset stepper motor positions to 0
    stepper1.setCurrentPosition(0); stepper1.moveTo(0);
    stepper2.setCurrentPosition(0); stepper2.moveTo(0);
    stepper3.setCurrentPosition(0); stepper3.moveTo(0);
    stepper4.setCurrentPosition(0); stepper4.moveTo(0);

    //Read all their encoders and reset to zero for simplicity
    encoderS1.readAndReset();
    encoderS2.readAndReset();
    encoderS3.readAndReset();
    encoderS4.readAndReset();

    //Turn off all stepper motor brakes
    BrakesAllOFF();

    //Enable all the steppers if the Arm switch is on 
    if(armSw_State == 1)
    {
      AllStepperMotorsON();
    }

    newStateFirstTime = false;
  }


  //Set parameters to slower settings for knob control
  stepper1.setMaxSpeed(knobMaxSpeed); stepper1.setAcceleration(knobAcceleration);
  stepper2.setMaxSpeed(knobMaxSpeed); stepper2.setAcceleration(knobAcceleration);
  stepper3.setMaxSpeed(knobMaxSpeed); stepper3.setAcceleration(knobAcceleration);
  stepper4.setMaxSpeed(knobMaxSpeed); stepper4.setAcceleration(knobAcceleration);



  knobPosition = KnobEncoder.read();

  if (knobPosition != knobOldPosition)
  { Serial.println("Change All Steppers");
    stepper1.moveTo(stepper1.currentPosition() - knobStepAmount * (knobPosition - knobOldPosition));
    stepper2.moveTo(stepper2.currentPosition() - knobStepAmount * (knobPosition - knobOldPosition));
    stepper3.moveTo(stepper3.currentPosition() - knobStepAmount * (knobPosition - knobOldPosition));
    stepper4.moveTo(stepper4.currentPosition() - knobStepAmount * (knobPosition - knobOldPosition));
 
    knobOldPosition = 0;
    KnobEncoder.readAndReset();
  }
  //only enable the steppers when moving them. //HIGH = free spinning
  //Keep them in place if the ARM switch is on.  
  if (stepper1.distanceToGo() == 0) {if(armSw_State == 0) digitalWrite(S1_ENABLE, 1);}  else digitalWrite(S1_ENABLE, 0);
  if (stepper2.distanceToGo() == 0) {if(armSw_State == 0) digitalWrite(S2_ENABLE, 1);}  else digitalWrite(S2_ENABLE, 0);
  if (stepper3.distanceToGo() == 0) {if(armSw_State == 0) digitalWrite(S3_ENABLE, 1);}  else digitalWrite(S3_ENABLE, 0);
  if (stepper4.distanceToGo() == 0) {if(armSw_State == 0) digitalWrite(S4_ENABLE, 1);}  else digitalWrite(S4_ENABLE, 0);


  //Run them to their new locations
  stepper1.run();
  stepper2.run();
  stepper3.run();
  stepper4.run();

  //Print encoder values
  if(LCDUpdate > LCDRefreshRate)
  { 
    Serial.println("E1: "  + String(encoderS1.read()) + 
                  " \tE2: " + String(encoderS2.read()) + 
                  " \tE3: "   + String(encoderS3.read()) + 
                  " \tE4: " + String(encoderS4.read()) + 
                  " \tShaft: "+ String(shaftEncoder.read()));

    LCDUpdate = 0;
  }
}

void TuneOneStepper()
{

  //First time
  if (newStateFirstTime)
  {
    setAll(BLACK);
    leds.setPixel(knobSingleControlNum - 1, BLUE);
    leds.show();
    Serial.println("Control motor " + String(knobSingleControlNum));

    //LCD
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TUNE ONE STEPPER");
    lcd.setCursor(7, 1);
    lcd.print(knobSingleControlNum);

    //Brakes off
    BrakesAllOFF();

    //Reset stepper motor positions to 0
    stepper1.setCurrentPosition(0); stepper1.moveTo(0);  encoderS1.readAndReset();
    stepper2.setCurrentPosition(0); stepper2.moveTo(0);  encoderS2.readAndReset();
    stepper3.setCurrentPosition(0); stepper3.moveTo(0);  encoderS3.readAndReset();
    stepper4.setCurrentPosition(0); stepper4.moveTo(0);  encoderS4.readAndReset();

      //Set speeds to slower settings for knob control
    //Serial.println("Set knob max speeds");
    stepper1.setMaxSpeed(knobMaxSpeed); stepper1.setAcceleration(knobAcceleration);
    stepper2.setMaxSpeed(knobMaxSpeed); stepper2.setAcceleration(knobAcceleration);
    stepper3.setMaxSpeed(knobMaxSpeed); stepper3.setAcceleration(knobAcceleration);
    stepper4.setMaxSpeed(knobMaxSpeed); stepper4.setAcceleration(knobAcceleration);

    newStateFirstTime = false;
  }

  //Figure out if the button was pressed to increment which to do
  if (knobButton_State == 1)
  {
    //Increment which motor we're controlling
    knobSingleControlNum++;
    if (knobSingleControlNum > 4)
      knobSingleControlNum = 1;

    //Change LED lights
    setAll(BLACK);
    leds.setPixel(knobSingleControlNum - 1, BLUE);
    leds.show();
    Serial.println("Control motor " + String(knobSingleControlNum));

    //LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TUNE ONE STEPPER");
    lcd.setCursor(7, 1);
    lcd.print(knobSingleControlNum);

    //Wait until button is not pressed
    while (knobButton_State == 1)
    {
      knobButton.update();
      knobButton_State = knobButton.isPressed();
    }
  }

  knobPosition = KnobEncoder.read();
  int amountToMove = knobStepAmount;
  if(armSw_State == 1)
  { //Move less if the arm switch is active. 
    amountToMove = knobArmedStepAmount;
  }

  if (knobPosition != knobOldPosition)
  {
    //Serial.println("Move To:  " + String(stepper1.currentPosition() - knobStepAmount * (knobPosition - knobOldPosition)));
    if		(knobSingleControlNum == 1) {
      
      stepper1.moveTo(stepper1.currentPosition() - amountToMove * (knobPosition - knobOldPosition));
    }
    else if (knobSingleControlNum == 2) {
      stepper2.moveTo(stepper2.currentPosition() - amountToMove * (knobPosition - knobOldPosition));
    }
    else if (knobSingleControlNum == 3) {
      stepper3.moveTo(stepper3.currentPosition() - amountToMove * (knobPosition - knobOldPosition));
    }
    else if (knobSingleControlNum == 4) {
      stepper4.moveTo(stepper4.currentPosition() - amountToMove * (knobPosition - knobOldPosition));
    }

    knobOldPosition = 0;
    KnobEncoder.readAndReset();
  }


  //only enable the steppers when moving them. //1 == free spinning
  if (stepper1.distanceToGo() == 0) {if(armSw_State == 0) digitalWrite(S1_ENABLE, 1);}  else digitalWrite(S1_ENABLE, 0);
  if (stepper2.distanceToGo() == 0) {if(armSw_State == 0) digitalWrite(S2_ENABLE, 1);}  else digitalWrite(S2_ENABLE, 0);
  if (stepper3.distanceToGo() == 0) {if(armSw_State == 0) digitalWrite(S3_ENABLE, 1);}  else digitalWrite(S3_ENABLE, 0);
  if (stepper4.distanceToGo() == 0) {if(armSw_State == 0) digitalWrite(S4_ENABLE, 1);}  else digitalWrite(S4_ENABLE, 0);

  //Run them to their new locations
  stepper1.run();
  stepper2.run();
  stepper3.run();
  stepper4.run();

  //Print encoder values
  if(LCDUpdate > LCDRefreshRate)
  { 
    Serial.println("E1: "  + String(encoderS1.read()) + 
                  " \tE2: " + String(encoderS2.read()) + 
                  " \tE3: "   + String(encoderS3.read()) + 
                  " \tE4: " + String(encoderS4.read()) + 
                  " \tShaft: "+ String(shaftEncoder.read()));

    LCDUpdate = 0;
  }
}
