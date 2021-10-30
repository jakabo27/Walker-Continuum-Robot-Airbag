void PrintLog()
{
  Serial.println("Fall Direction:  " + String(fallDirection));
  Serial.println("Data log of that run:");

  File dataFile = SD.open("walklog.csv", FILE_WRITE);
  
  for(byte i = 0; i < logDataCols-1; i++)
  {
    Serial.print(logHeaders[i]);
    Serial.print(",");
     
    dataFile.print(logHeaders[i]); 
    dataFile.print(","); 
  }
  Serial.println(logHeaders[logDataCols-1]);
  dataFile.println(logHeaders[logDataCols-1]);

  //Data
  for (int i = 0; i < logDataRows; i++) {
    for(byte j = 0; j< logDataCols - 1; j++)
      {
        if(j >= 14)
        {
          // Save the IMU columns with 5 decimal places
          Serial.print(logData[i][j], 5);
          dataFile.print(logData[i][j], 5);
        }
        else
        {
          Serial.print(logData[i][j]);
          dataFile.print(logData[i][j]);
        }
   
        Serial.print(",");
        dataFile.print(",");
      }
      // Save the IMU columns with more 5 places
      Serial.println(logData[i][logDataCols-1], 5);
      dataFile.println(logData[i][logDataCols-1], 5);
     
      delay(2);
  }
  dataFile.close();
  Serial.println("\nLog printing done!");
}

void AllStepperMotorsOFF()
{
  digitalWrite(S1_ENABLE, 1);
  digitalWrite(S2_ENABLE, 1);
  digitalWrite(S3_ENABLE, 1);
  digitalWrite(S4_ENABLE, 1);
}

void AllStepperMotorsON()
{
  digitalWrite(S1_ENABLE, 0);
  digitalWrite(S2_ENABLE, 0);
  digitalWrite(S3_ENABLE, 0);
  digitalWrite(S4_ENABLE, 0);
}

//Convert a number of encoder counts to stepper steps.  192 encoder counts is 200 steps (1 revolution on each)
//Returns the motor steps
int stepEquivalent (int encoderValue)
{
  //Stepper to encoder conversion.  
  int stepEquiv = encoderValue * 200/EncoderCPR;
  return stepEquiv;
}

int encoderEquivalent(int stepperSteps)
{
  int encoderEquiv = stepperSteps * EncoderCPR / 200;
  return encoderEquiv;
}


//#####################   DRILL HELPER FUNCTIONS   #####################
void DrillSetSpeed(int newSpeed)
{
  newSpeed = constrain(newSpeed, 0, 99);

  //Set them both to the new speed - could do more fine tuning later.  
  pot1.set(newSpeed);
  pot2.set(newSpeed);
}

//Same thing but different function name
void SetDrillSpeed(int newSpeed)
{
  newSpeed = constrain(newSpeed, 0, 99);

  //Set them both to the new speed - could do more fine tuning later.  
  pot1.set(newSpeed);
  pot2.set(newSpeed);

  drillAwakeTimer = 0;
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
  aw.digitalWrite(DRILL_DIR_PIN, 1); 
  delay(10);
  drillCurrentDir = 1;
}

void DrillCCW()
{
    
    aw.digitalWrite(DRILL_DIR_PIN, 0); 
    delay(10);
    drillCurrentDir = 0;
}

void DrillFlipDirection()
{
  aw.digitalWrite(DRILL_DIR_PIN, !drillCurrentDir);
  drillCurrentDir = !drillCurrentDir;
}


//#####################   BRAKE HELPER FUNCTIONS   #####################

//Brake "ON" means braking.  Off = not braking.  
void BrakeOFF(byte brakeNum)
{
  if(brakeNum == 1)
    aw.digitalWrite(BRAKE1, 1);
  if(brakeNum == 2)
    aw.digitalWrite(BRAKE2, 1);
  if(brakeNum == 3)
    aw.digitalWrite(BRAKE3, 1);
  if(brakeNum == 4)
    aw.digitalWrite(BRAKE4, 1);
}

void BrakeON(byte brakeNum)
{
  if(brakeNum == 1)
    aw.digitalWrite(BRAKE1, 0);
  if(brakeNum == 2)
    aw.digitalWrite(BRAKE2, 0);
  if(brakeNum == 3)
    aw.digitalWrite(BRAKE3, 0);
  if(brakeNum == 4)
    aw.digitalWrite(BRAKE4, 0);
}

void BrakesAllON()
{
  aw.digitalWrite(BRAKE1, 0);
  aw.digitalWrite(BRAKE2, 0);
  aw.digitalWrite(BRAKE3, 0);
  aw.digitalWrite(BRAKE4, 0);
}

void BrakesAllOFF()
{
  aw.digitalWrite(BRAKE1, 1);
  aw.digitalWrite(BRAKE2, 1);
  aw.digitalWrite(BRAKE3, 1);
  aw.digitalWrite(BRAKE4, 1);
}


//################# LED Helper Functions #####################
void showStrip() {
   leds.show();
}
void setPixel(int Pixel, int color) {
   leds.setPixel(Pixel, color);
   //leds.show();
}
void setAll(int color) {
  for (int i=0; i < leds.numPixels(); i++) {
    leds.setPixel(i, color);
  }
    //leds.show();
}

//#####################   IMU Helper functions   #####################

// Does the stuff with global variables for the IMU things.  Used in Deploy to get IMU data
//      without fully checking all inputs. 
void refreshIMUData()
{
  imu.getAGMT();
    get_scaled_IMU(Axyz, Mxyz);
    Mxyz[1] = -Mxyz[1]; //align magnetometer with accelerometer (reflect Y and Z)
    Mxyz[2] = -Mxyz[2];

    //Save the raw non-averaged data
    IMU_LR_RAW = Mxyz[2];
    IMU_FB_RAW = Mxyz[0];

    //Filter the IMU data with a rolling average 
    IMU_LR_total = IMU_LR_total - IMU_LR_readings[readIndex]; // subtract the last reading
    IMU_FB_total = IMU_FB_total - IMU_FB_readings[readIndex];
    IMU_LR_readings[readIndex] = Mxyz[2];                     // read from the sensor
    IMU_FB_readings[readIndex] = Mxyz[0]; 
    IMU_LR_total = IMU_LR_total + IMU_LR_readings[readIndex]; // add the reading to the total
    IMU_FB_total = IMU_FB_total + IMU_FB_readings[readIndex];
    readIndex = readIndex + 1;  // advance to the next position in the array
    if (readIndex >= numReadings) { readIndex = 0; }
    IMU_LR_average = IMU_LR_total / numReadings;              // Calculate the average
    IMU_FB_average = IMU_FB_total / numReadings;

    IMU_LeftRight = IMU_LR_average; 
    IMU_ForwardBack = IMU_FB_average; 
}

void get_scaled_IMU(float Axyz[3], float Mxyz[3]) {
  byte i;
  float temp[3];
  Axyz[0] = imu.agmt.acc.axes.x;
  Axyz[1] = imu.agmt.acc.axes.y;
  Axyz[2] = imu.agmt.acc.axes.z;
  Mxyz[0] = imu.agmt.mag.axes.x;
  Mxyz[1] = imu.agmt.mag.axes.y;
  Mxyz[2] = imu.agmt.mag.axes.z;
  //apply offsets (bias) and scale factors from Magneto
  for (i = 0; i < 3; i++) temp[i] = (Axyz[i] - A_B[i]);
  Axyz[0] = A_Ainv[0][0] * temp[0] + A_Ainv[0][1] * temp[1] + A_Ainv[0][2] * temp[2];
  Axyz[1] = A_Ainv[1][0] * temp[0] + A_Ainv[1][1] * temp[1] + A_Ainv[1][2] * temp[2];
  Axyz[2] = A_Ainv[2][0] * temp[0] + A_Ainv[2][1] * temp[1] + A_Ainv[2][2] * temp[2];
  vector_normalize(Axyz);

  //apply offsets (bias) and scale factors from Magneto
  for (int i = 0; i < 3; i++) temp[i] = (Mxyz[i] - M_B[i]);
  Mxyz[0] = M_Ainv[0][0] * temp[0] + M_Ainv[0][1] * temp[1] + M_Ainv[0][2] * temp[2];
  Mxyz[1] = M_Ainv[1][0] * temp[0] + M_Ainv[1][1] * temp[1] + M_Ainv[1][2] * temp[2];
  Mxyz[2] = M_Ainv[2][0] * temp[0] + M_Ainv[2][1] * temp[1] + M_Ainv[2][2] * temp[2];
  vector_normalize(Mxyz);
}

// basic vector operations
void vector_cross(float a[3], float b[3], float out[3])
{
  out[0] = a[1] * b[2] - a[2] * b[1];
  out[1] = a[2] * b[0] - a[0] * b[2];
  out[2] = a[0] * b[1] - a[1] * b[0];
}

float vector_dot(float a[3], float b[3])
{
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void vector_normalize(float a[3])
{
  float mag = sqrt(vector_dot(a, a));
  a[0] /= mag;
  a[1] /= mag;
  a[2] /= mag;
}
