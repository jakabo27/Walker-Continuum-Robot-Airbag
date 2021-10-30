void CheckInputs()
{
	// Dual Switch A (Middle Switch)
  if(!digitalRead(DUAL_A_2_PIN)) 
    dualA_State = 1;
  else if (!digitalRead(DUAL_A_1_PIN))
    dualA_State = 2;
  else
    dualA_State = 0;

  // Dual Switch B (Bottom Switch)
  // Wired so either position (1 or 2) is ON, middle is OFF
  if(!digitalRead(DUAL_B_12_PIN))
    dualB_State = 1;
  else
    dualB_State = 0;

  // Dual Switch C (Top Switch)
  // Wired so either position (1 or 2) is ON, middle is OFF
  if(!digitalRead(DUAL_C_12_PIN))
    dualC_State = 1;
  else
    dualC_State = 0;

  // ArmSwitch
  armSw_State = !digitalRead(ARMSW_PIN);
  if(armSw_State == 0)
    alreadyDeployed = 0; //Only fully reset after armSw goes back to 0. 

  // Power Switch
  powSw_State = !digitalRead(POWSW_PIN);

  // Deploy button
  deployButton.update();
  deployButton_State = deployButton.isPressed();

  // Rotory Knob Button
  knobButton.update();
  knobButton_State = knobButton.isPressed();

  //IMU 
  if ( imu.dataReady() ) 
  {
    imu.getAGMT();
    get_scaled_IMU(Axyz, Mxyz); // In HelperFunctions.ino
    Mxyz[1] = -Mxyz[1]; // Align magnetometer with accelerometer 
    Mxyz[2] = -Mxyz[2]; //  (reflect Y and Z)

    //Save the raw non-averaged data
    IMU_LR_RAW = Mxyz[2];
    IMU_FB_RAW = Mxyz[0];
    
    //Filter the IMU data with a rolling average 
    // Subtract the last reading
    IMU_LR_total = IMU_LR_total - IMU_LR_readings[readIndex]; 
    IMU_FB_total = IMU_FB_total - IMU_FB_readings[readIndex];
    
    // Read from the sensor
    IMU_LR_readings[readIndex] = Mxyz[2];                     
    IMU_FB_readings[readIndex] = Mxyz[0];
     
    // Add the reading to the total
    IMU_LR_total = IMU_LR_total + IMU_LR_readings[readIndex]; 
    IMU_FB_total = IMU_FB_total + IMU_FB_readings[readIndex];
    
    // Advance to the next position in the array
    readIndex = readIndex + 1;
    // Calculate the average  
    if (readIndex >= numReadings) { readIndex = 0; }
    IMU_LR_average = IMU_LR_total / numReadings;              
    IMU_FB_average = IMU_FB_total / numReadings;

    // Update global variables
    IMU_LeftRight = IMU_LR_average; 
    IMU_ForwardBack = IMU_FB_average; 
  }
}

//Figure out which way the walker is falling (or not)
int determineFallDirection()
{
  fallDirection = STABLE;

  if(IMU_LeftRight < (IMU_Stable_LR - FallRightDelta))
  {
    fallDirection = FALL_RIGHT;
  }
  else if(IMU_LeftRight > (IMU_Stable_LR + FallLeftDelta))
  {
    fallDirection = FALL_LEFT;
  }
  else if(IMU_ForwardBack < (IMU_Stable_FB - FallForwardDelta))
  {
    fallDirection = FALL_FORWARD;
  } 
}

void DetermineState()
{
  // Update global variables
	CheckInputs();
  determineFallDirection();

  // Motor tuning functions
	if (dualA_State == 1)
		state = TUNE_ONE_STEPPER;
	else if (dualA_State == 2)
		state = TUNE_ALL_STEPPERS;
	else if (dualB_State != 0)
		state = TUNE_DRILL_MOTOR;

  // Require resetting the ARM switch upon boot
  // (alreadyDeployed is set to 1 in the setup file)
  else if (alreadyDeployed == 1 && armSw_State == 1)
		state = DEPLOYED_WAIT_FOR_RESET; 
	else if (armSw_State == 1 &&
		dualA_State == 0 &&
		dualB_State == 0 &&
		alreadyDeployed == 0 
    )
		state = ARMED_AND_READY; //Listening to IMU sensor for falling. 

  else if (armSw_State == 0 && dualA_State == 0 && dualB_State == 0)
    state = STANDBY;// Maybe?
	else
		state = UNDEFINED;

  //Keep track of if it's the first time in the Arm state or not
  // Other functions use the "newStateFirstTime" variable. 
  if(state != ARMED_AND_READY)
    armedReadyFirstTime = 1;

  if(state != oldState)
  {
    Serial.println("New State:  " + String(state));
    newStateFirstTime = 1;
  }
    
  oldState = state;
}

void UndefinedState()
{
  //turn off motors
  AllStepperMotorsOFF();
  DrillSwitchOFF(); 

  //Make lights all different colors
  setPixel(0, BLUE);
  setPixel(1, RED);
  setPixel(2, GREEN);
  setPixel(3, WHITE);
  leds.show();
//Update LCD 
  if(LCDUpdate > LCDRefreshRate)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("UNDEFINED");
    LCDUpdate = 0; //Reset Timer
  }
}

void Standby()
{
  //Disable motors
  AllStepperMotorsOFF();

  //Drill not spinning
  DrillSwitchOFF(); 

  if(LCDUpdate > LCDRefreshRate)
  {
    lcd.clear();
    
    digitalWrite(DEPLOY_LED, LOW); //Lights off
    lcd.setCursor(0,4);
    lcd.print("STANDBY");
    lcd.noBacklight();
    setAll(BLACK);
    leds.show();
  }

  //Write the entire log to the serial monitor if the knob button is pressed
  if (knobButton_State == 1 && logCounter > 5) //If we had a run already.  
  {
    PrintLog();
    
    //Wait until button is not pressed
    while (knobButton_State == 1)
    {
      knobButton.update();
      knobButton_State = knobButton.isPressed();
    }
  }
  
  
  
}
