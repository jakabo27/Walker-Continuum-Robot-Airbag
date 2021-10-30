void CheckInputs()
{
  //Dual Switch A
  if(!digitalRead(DUAL_A_2_PIN)) //Yes I know these are swapped it's fine. 
    dualA_State = 1;
  else if (!digitalRead(DUAL_A_1_PIN))
    dualA_State = 2;
  else
    dualA_State = 0;

  //Dual Switch B
  if(!digitalRead(DUAL_B_12_PIN))
    dualB_State = 1;
  else
    dualB_State = 0;

  //Dual Switch C
  if(!digitalRead(DUAL_C_12_PIN))
    dualC_State = 1;
  else
    dualC_State = 0;

  //ArmSwitch
  armSw_State = !digitalRead(ARMSW_PIN);
  if(armSw_State == 0)
    alreadyDeployed = 0; //only fully reset after armSw goes back to 0. 

  //Power Switch
  powSw_State = !digitalRead(POWSW_PIN);

  //Deploy button
  deployButton.update();
  deployButton_State = deployButton.isPressed();

  //Rotory Knob Button
  knobButton.update();
  knobButton_State = knobButton.isPressed();
  
}


void DetermineState()
{
  CheckInputs();
  //determineFallDirection();

  if (dualA_State == 1)
    state = TUNE_ONE_STEPPER;
  else if (dualA_State == 2)
    state = TUNE_ALL_STEPPERS;
  else if (dualB_State != 0)
    state = TUNE_DRILL_MOTOR;
  
  else if (alreadyDeployed == 1 && armSw_State == 1)
    state = DEPLOYED_WAIT_FOR_RESET; //Turn off all motors.
  else if (armSw_State == 1 &&
    dualA_State == 0 &&
    dualB_State == 0 &&
    alreadyDeployed == 0 //TODO make sure this is determined often enough for this...
    )
    state = ARMED_AND_READY; //Listening to IMU sensor for falling. 

  else if (armSw_State == 0 && dualA_State == 0 && dualB_State == 0)
    state = STANDBY;// Maybe?
  else
    state = UNDEFINED;

  if(state != oldState)
  {
    Serial.println("New State:  " + String(state));
    newStateFirstTime = 1;
  }
    
  oldState = state;
}
