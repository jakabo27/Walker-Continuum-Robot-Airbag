///Blocking function that once it starts is all that runs until its done. 
void DeployingNOW()
{
  // Trigger drill relays to start the drill accelerating
  DrillSetSpeed(deployDrillSpeed);
  DrillSwitchON();
  DrillCCW();
  
  // Read shaft encoder and reset it to 0.  
  //   (Other encoders set in ArmedAndReady.ino with homing functionality)
  shaftEncoder.readAndReset();
  
  // Release all stepper motors to spin freely
  AllStepperMotorsOFF();
  BrakesAllOFF();

  // LED strips to red
  setAll(RED);  leds.show(); digitalWrite(DEPLOY_LED, LOW);

  // Update LCD 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Deploying ");
  lcd.setCursor(0,1);
  lcd.print(fallDirNames[fallDirection]);
  LCDUpdate = 0; //Reset Timer

  // Timers intialize
  elapsedMillis sinceDeployStart;
  elapsedMillis printTimer; //For printing to serial monitor
  elapsedMicros logTimer; //For data logging
  elapsedMillis IMUTimer;

  // Zero the data logging array
  memset(logData, 0, sizeof(logData)); 
  logCounter = 0;
  
  // Determine the ENCODER end value for each stepper motor.  
  int E1endValue = goalEncoderSpots[fallDirection-1][0]; 
  int E2endValue = goalEncoderSpots[fallDirection-1][1];
  int E3endValue = goalEncoderSpots[fallDirection-1][2];
  int E4endValue = goalEncoderSpots[fallDirection-1][3];

  // Stepper to encoder conversion for driving motors to their spot
  //         Account for movement after "home" spot
  int S1endValue = (E1endValue - encoderS1.read()) * 200/EncoderCPR; 
  int S2endValue = (E2endValue - encoderS2.read()) * 200/EncoderCPR;
  int S3endValue = (E3endValue - encoderS3.read()) * 200/EncoderCPR;
  int S4endValue = (E4endValue - encoderS4.read()) * 200/EncoderCPR;

  // Spot to drive each motor after
  int E1driveValue = encoderS1.read() + deployWhenEncoderMoves;
  int E2driveValue = encoderS2.read() + deployWhenEncoderMoves;
  int E3driveValue = encoderS3.read() + deployWhenEncoderMoves;
  int E4driveValue = encoderS4.read() + deployWhenEncoderMoves;
  
  //Variables to keep track of if a stepper is driving or not
  bool S1driving = 0; bool S2driving = 0; 
  bool S3driving = 0; bool S4driving = 0;
  bool S1Braked = 0; bool S2Braked = 0;
  bool S3Braked = 0; bool S4Braked = 0;

  // Deploy logic
  bool stillDeploying = 1; 
  bool drillTriggerReleased = 0;
  bool drillUnwinding = 0;
  bool drillCompletelyDone = 0;
  
  //Variables to keep the encoder values in
  int E1val = 0;  int E2val = 0;
  int E3val = 0;  int E4val = 0;
  int EShaftval = 0;

  // Reset deploy timer to zero
  sinceDeployStart = 0;

  while(stillDeploying)
  {
    
    //Read encoders
    E1val = encoderS1.read(); E2val = encoderS2.read();
    E3val = encoderS3.read(); E4val = encoderS4.read();
    EShaftval = shaftEncoder.read();

    //Turn on and lock each stepper as it spools out to the goal location
    if(E1val >= E1endValue) {digitalWrite(S1_ENABLE, 0);  S1driving = 0;  if(!S1Braked){BrakeON(1); S1Braked = 1;}} else {};
    if(E2val >= E2endValue) {digitalWrite(S2_ENABLE, 0);  S2driving = 0;  if(!S2Braked){BrakeON(2); S2Braked = 1;}} else {};
    if(E3val >= E3endValue) {digitalWrite(S3_ENABLE, 0);  S3driving = 0;  if(!S3Braked){BrakeON(3); S3Braked = 1;}} else {};
    if(E4val >= E4endValue) {digitalWrite(S4_ENABLE, 0);  S4driving = 0;  if(!S4Braked){BrakeON(4); S4Braked = 1;}} else {};
    
    //Stop spinning the drill if it's spun X rotations 
    if(EShaftval <= shaftStepsBeforeStopping && !drillTriggerReleased) 
    {
      DrillSetSpeed(0);
      DrillSwitchOFF();
      drillTriggerReleased = 1; // Only run this block once. 
    }

    //Stop the drill after 500ms 
    if(sinceDeployStart > drillMaxDeployTime && !drillTriggerReleased)
    {
      DrillSetSpeed(0);
      DrillSwitchOFF();
      aw.digitalWrite(DRILL_DIR_PIN, 1);
      drillCurrentDir = 1;
      drillTriggerReleased = 1; // Only run this block once. 
     }

    //Log data
    if(logTimer >= logPeriod)
    {
      logTimer = 0; //Reset timer

      logData[logCounter][0] = sinceDeployStart;

      // Motor Encoders
      logData[logCounter][1] = E1val;   logData[logCounter][2] = E2val;
      logData[logCounter][3] = E3val;   logData[logCounter][4] = E4val;

      // Drill
      logData[logCounter][5] = EShaftval;   
      logData[logCounter][6] = drillTriggerReleased;

      // Cable Stopped
      logData[logCounter][7] = S1Braked;      
      logData[logCounter][8] = S2Braked;
      logData[logCounter][9] = S3Braked;      
      logData[logCounter][10] = S4Braked;
      logData[logCounter][11] = S1driving;                    
      logData[logCounter][12] = S2driving;
      logData[logCounter][13] = S3driving;                    
      logData[logCounter][14] = S4driving;

      // IMU data
      logData[logCounter][15] = IMU_LR_RAW; // Tilt L/R
      logData[logCounter][16] = IMU_FB_RAW; // Tilt F/B
      logData[logCounter][17] = Axyz[2]; // Acceleration L/R                  
      logData[logCounter][18] = Axyz[0]; // Acceleration F/B
      logData[logCounter][19] = Axyz[1]; // Acceleration Z(?)

      logCounter++;
    }

    //Update IMU data every few ms
    if(IMUTimer >= 2 )
    {
      if ( imu.dataReady() ) 
      { 
        refreshIMUData(); 
        IMUTimer = 0;
      }
    }

    //Stop deploying after X seconds
    if (sinceDeployStart > 1400)
      stillDeploying = false;
  }

  Serial.println("\n###### DONE DEPLOYING ######\n");

  
  //LEDs to green
  setAll(GREEN);
  leds.show();

  //Release all the cable spools
  AllStepperMotorsOFF();
  BrakesAllOFF();

  //Unwind the drill a bit 
  if(!drillTriggerReleased) 
  { // Can't change direction while trigger pulled in
    DrillSetSpeed(0);
    DrillSwitchOFF(); 
    delay(30);
  }
  DrillClockwise();
  DrillSwitchON();
  DrillSetSpeed(20);
  delay(500);
  DrillSwitchOFF();
  DrillSetSpeed(0);
  
  
  //Lastly, say we already deployed and need to be reset 
  alreadyDeployed = 1;
  state = DEPLOYED_WAIT_FOR_RESET;
  newStateFirstTime = 1;  oldState = state; 
  
  PrintLog();

  
  DeployedWaitForReset(); 
  
}


void DeployedWaitForReset()
{
  // The state == DEPLOYED_WAIT_FOR_RESET until the ARM switch is set to 0 

  // Turn off all motor drivers
  AllStepperMotorsOFF();

  // Turn off the drill
  DrillSwitchOFF();
  DrillSetSpeed(0);

  //Update LCD 
  if(LCDUpdate > LCDRefreshRate*4)
  {
    lcd.clear();          lcd.backlight();
    lcd.setCursor(0,0);   lcd.print("Please reset");
    lcd.setCursor(0,1);   lcd.print("ARM Switch");
    LCDUpdate = 0; //Reset Timer
  }

  //Write the log to the serial monitor if the knob button is pressed
  if (knobButton_State == 1 && logCounter > 5)
  {
    PrintLog();
    
    //Wait until button is not pressed
    while (knobButton_State == 1)
    {
      knobButton.update();
      knobButton_State = knobButton.isPressed();
    }
  }

  if(newStateFirstTime)
  {
    // Turn off LEDs
    setAll(BLACK); leds.show();
    newStateFirstTime = 0;
    digitalWrite(DEPLOY_LED, 0); // LED ring around the deploy button
  }
  delay(1);
}
