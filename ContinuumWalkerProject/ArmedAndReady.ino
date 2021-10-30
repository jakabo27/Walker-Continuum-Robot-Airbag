void ArmedAndReady()
{
  //Use stepper motors to hold arm in place
  AllStepperMotorsON();
  
  CheckInputs();

  //Keep the drill awake
  if(drillAwakeTimer > 8000)
  {
    KeepDrillAwake();
    drillAwakeTimer = 0;
  }
  
  //The first time running Armed and Ready, save the IMU values and do relative to that. 
  if(armedReadyFirstTime == 1)
  {
    Serial.println("Armed and ready first time running!");
    IMU_Stable_LR = IMU_LeftRight;
    IMU_Stable_FB = IMU_ForwardBack;

    //Manually set fall direction to stable for this first one
    fallDirection = STABLE;
    digitalWrite(DEPLOY_LED, 1);

    //Turn on the LCD backlight. Do NOT do lcd.begin() because it breaks stepper motor movement for some reason.
    lcd.backlight();

    armedReadyFirstTime = 0;

    stepper1.setMaxSpeed(deployMaxSpeed); stepper1.setAcceleration(deployAcceleration); stepper1.setMinPulseWidth(globalMinPulseWidth);
    stepper2.setMaxSpeed(deployMaxSpeed); stepper2.setAcceleration(deployAcceleration); stepper2.setMinPulseWidth(globalMinPulseWidth);
    stepper3.setMaxSpeed(deployMaxSpeed); stepper3.setAcceleration(deployAcceleration); stepper3.setMinPulseWidth(globalMinPulseWidth);
    stepper4.setMaxSpeed(deployMaxSpeed); stepper4.setAcceleration(deployAcceleration); stepper4.setMinPulseWidth(globalMinPulseWidth);
    encoderS1.readAndReset();   encoderS2.readAndReset();
    encoderS3.readAndReset();   encoderS4.readAndReset();
    

    //Get drill ready for CCW rotation
    DrillSetSpeed(0); //Set speed to 0 so the new drill doesn't start spinning. 
    DrillCCW();

    //Wake up the drill
    DrillSwitchON();
    delay(100);
    DrillSwitchOFF();

    //Motor brakes off
    BrakesAllOFF();   
  }

  //Update LCD 
  if(LCDUpdate > LCDRefreshRate)
  {
    LCDUpdate = 0; //Reset Timer
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("LR:"+String(IMU_Stable_LR - IMU_LeftRight));
    lcd.setCursor(11,0);
    lcd.print("ARMED");
    lcd.setCursor(0,1);
    lcd.print("FB:"+String(IMU_Stable_FB - IMU_ForwardBack));
    //Calculate the one closest to deploying and print that percent to the LCD
    float rightPercent = 0;
    float leftPercent = 0;
    float forwardPercent = 0;
    float highestPercent = 0;
    if(IMU_Stable_LR - IMU_LeftRight > 0)
      rightPercent = (IMU_Stable_LR - IMU_LeftRight) / FallRightDelta;
    else
      leftPercent = (IMU_Stable_LR - IMU_LeftRight) / FallLeftDelta * -1;
    forwardPercent = (IMU_Stable_FB - IMU_ForwardBack) / FallForwardDelta;
    highestPercent = max(rightPercent, leftPercent);
    highestPercent = max(highestPercent, forwardPercent);
    lcd.setCursor(9,1);
    lcd.print(String(int(highestPercent * 100)) + "%");
    
    Serial.print("RP:" + String(rightPercent*100) + "%\tLP:" + String(leftPercent*100) + "%\tFP:" + String(forwardPercent*100) + "%");
    
    //Serial.println("LR: " + String(IMU_Stable_LR - IMU_LeftRight) +
    //               "   FB: " + String(IMU_Stable_FB - IMU_ForwardBack));

    //Print the encoder values 
     Serial.print("\t\tE1: "  + String(encoderS1.read()) + 
                  " \tE2: " + String(encoderS2.read()) + 
                  " \tE3: "   + String(encoderS3.read()) + 
                  " \tE4: " + String(encoderS4.read()));

     Serial.println("\tIMU_LR=" + String(IMU_LeftRight) + "    IMU_FB=" + String(IMU_ForwardBack));
  } //Update LCD end

  //Set the "home" position of the steppers if the knob button pressed. 
  if (knobButton_State == 1)
  {
    Serial.println("SET HOME POSITION OF THE STEPPERS!!");
    Serial.println("\tPlease manually move steppers to wherever you want");
    encoderS1.readAndReset();
    encoderS2.readAndReset();
    encoderS3.readAndReset();
    encoderS4.readAndReset();
    
    
    //Wait until button is not pressed
    while (knobButton_State == 1)
    {
      knobButton.update();
      knobButton_State = knobButton.isPressed();
    }
  }

  

  //Deploy based on IMU OR based on the deploy button for testing. 
  if(deployButton_State == 1)
  {
    state = DEPLOYING;
    fallDirection = deployButtonNum; //Default to the fall directio defined at the end of setup
  }
  else if(fallDirection != STABLE) 
    state = DEPLOYING; 
  
  if(state == DEPLOYING)
  {
    DeployingNOW(); //force it to jump there without anything else. 
  }
  
}
