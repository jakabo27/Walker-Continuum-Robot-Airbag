void MotorSetup()
{
	//Motors and encoders
	stepper1.setEnablePin(S1_ENABLE); digitalWrite(S1_ENABLE, HIGH); //HIGH = Free Spinning
	stepper2.setEnablePin(S2_ENABLE); digitalWrite(S2_ENABLE, HIGH);
	stepper3.setEnablePin(S3_ENABLE); digitalWrite(S3_ENABLE, HIGH);
	stepper4.setEnablePin(S4_ENABLE); digitalWrite(S4_ENABLE, HIGH);
  
  stepper1.enableOutputs();
  stepper2.enableOutputs();
  stepper3.enableOutputs();
  stepper4.enableOutputs();

	stepper1.setMaxSpeed(deployMaxSpeed); stepper1.setAcceleration(deployAcceleration); stepper1.setMinPulseWidth(globalMinPulseWidth);
	stepper2.setMaxSpeed(deployMaxSpeed); stepper2.setAcceleration(deployAcceleration); stepper2.setMinPulseWidth(globalMinPulseWidth);
	stepper3.setMaxSpeed(deployMaxSpeed); stepper3.setAcceleration(deployAcceleration); stepper3.setMinPulseWidth(globalMinPulseWidth);
	stepper4.setMaxSpeed(deployMaxSpeed); stepper4.setAcceleration(deployAcceleration); stepper4.setMinPulseWidth(globalMinPulseWidth);

   //Invert direction of them so they match the encoders
   stepper1.setPinsInverted(true, false, false); //directionInvert,  stepInvert, enableInput
   stepper2.setPinsInverted(true, false, false);
   stepper3.setPinsInverted(true, false, false);
   stepper4.setPinsInverted(true, false, false);
}

void IMUSetup()
{
	//IMU Initialization
  WIRE_PORT.begin();
  WIRE_PORT.setClock(400000); //for the IMU
  imu.begin(WIRE_PORT, AD0_VAL);
  while(imu.status != ICM_20948_Stat_Ok) {
    imu.begin(WIRE_PORT, AD0_VAL);
    Serial.println(F("IMU (ICM_20948) not detected"));
    delay(100);
  }
  Serial.println("Found IMU!!");
}
