/*
 * This sketch reads the IMU values and prints them to the LCD and Serial monitor. 
 * 
 */

#include <ICM_20948.h>
#include <LCD_I2C.h>

LCD_I2C lcd(0x27);


//IMU
#define WIRE_PORT Wire1
#define AD0_VAL 1
ICM_20948_I2C imu; //Create an ICM_20948_I2C object
float IMU_LeftRight = 0;
float IMU_ForwardBack = 0;
float IMU_Z = 0;

bool IMU_FlatGround = 1; //1 if flat, 0 if tilted and would deploy.
elapsedMillis updateLCD;
//Accel scale: divide by 16604.0 to normalize. These corrections are quite small and probably can be ignored.
float A_B[3]  {   79.60,  -18.56,  383.31};

float A_Ainv[3][3]
{ {  1.00847,  0.00470, -0.00428},
  {  0.00470,  1.00846, -0.00328},
  { -0.00428, -0.00328,  0.99559}
};

//Mag scale divide by 369.4 to normalize. These are significant corrections, especially the large offsets.
float M_B[3]  { -156.70,  -52.79, -141.07};

float M_Ainv[3][3]
{ {  1.12823, -0.01142,  0.00980},
  { -0.01142,  1.09539,  0.00927},
  {  0.00980,  0.00927,  1.10625}
};

// local magnetic declination in degrees
float declination = 6.28;
float p[] = {1, 0, 0};  //X marking on sensor board points toward yaw = 0
float Axyz[3], Mxyz[3]; //centered and scaled accel/mag data

float last_IMU_LR = 0;
float IMU_LR_Filtered = 0;
float last_IMU_FB = 0;
float IMU_FB_Filtered = 0;

const int numReadings = 10;
float readings[numReadings];
int readIndex = 0;              // the index of the current reading
float total = 0;                  // the running total
float average = 0;                // the average

void setup() {

  Serial.begin(115200);
  while (!Serial) {}; 

  lcd.begin(false);  // If you are using more I2C devices using the Wire library use lcd.begin(false)
                 // this stop the library(LCD_I2C) from calling Wire.begin()
  

  //IMU Initialization
  WIRE_PORT.begin();
  WIRE_PORT.setClock(400000); //for the IMU
  imu.begin(WIRE_PORT, AD0_VAL);
  if (imu.status != ICM_20948_Stat_Ok) {
    Serial.println(F("ICM_20948 not detected"));
    while (1);
  }
  lcd.backlight();

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

}

void loop() {
  //IMU - Needs some testing. 
  // Update the sensor values whenever new data is available
  if ( imu.dataReady() ) 
  {
    imu.getAGMT();
    get_scaled_IMU(Axyz, Mxyz);
    Mxyz[1] = -Mxyz[1]; //align magnetometer with accelerometer (reflect Y and Z)
    Mxyz[2] = -Mxyz[2];

      // Print the raw values
//    Serial.print(Axyz[0], 3);
//    Serial.print("\t");
//    Serial.print(Axyz[1], 3);
//    Serial.print("\t");
//    Serial.print(Axyz[2], 3);
//    Serial.println("");

    
  //#### Data averaging to smooth out spikes and noise ######
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = Mxyz[2];
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }
  // calculate the average:
  average = total / numReadings;
  
  }

    IMU_LeftRight = Mxyz[2]; 
    IMU_ForwardBack = Mxyz[0]; 

    //"filter" out the random spikes it gets
    //IMU_FB_Filtered = min(IMU_ForwardBack, last_IMU_FB);
    
    last_IMU_LR = IMU_LeftRight; //Use the actual reading not filtered. 
    IMU_LR_Filtered = average;
    //last_IMU_FB = IMU_ForwardBack;
    
    //if (value is not tilted over)
    IMU_FlatGround = 0;  //tbd on actually setting its value. 


  if(updateLCD > 10)
  {
    updateLCD = 0;
    lcd.clear();
    lcd.setCursor(0,0);// column, row.  Starting at 0,0.  
    lcd.print(String(IMU_LeftRight));
    lcd.setCursor(0,1);
    lcd.print(String(IMU_ForwardBack));
    delay(1);

    Serial.println(String(IMU_LR_Filtered) + "\t\t" + String(IMU_ForwardBack));
  }
  delay(4);
  

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
