// Basic demo for accelerometer readings from Adafruit LIS3DH
// https://www.instructables.com/id/Stabilize-Sensor-Readings-With-Kalman-Filter/
// https://www.st.com/content/ccc/resource/technical/document/application_note/d2/d6/22/4f/b9/8f/45/59/DM00119046.pdf/files/DM00119046.pdf/jcr:content/translations/en.DM00119046.pdf
// https://wiki.dfrobot.com/How_to_Use_a_Three-Axis_Accelerometer_for_Tilt_Sensing
// https://www.youtube.com/watch?v=biY7F-tLwE8


#include <Wire.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

float roll = 0.0;
float pitch = 0.00; 

float in = 0.0;

float x = 0.0;
float x_p = 0.0;

float P = 0.0;
float P_p = 1.0;

float var_r = 0.11;//;10.135;
float var_p = 1.0e-4;
float K = 0.0;


void setup(void)
{
  Serial.begin(115200);
  //Serial.println("LIS3DH test!");
  
  if (! lis.begin(0x19))
  {
    Serial.println("Couldnt start");
    while (1);
  }
  //Serial.println("LIS3DH found!");
  
  lis.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!
  
  //Serial.print("Range = "); Serial.print(2 << lis.getRange());  
  //Serial.println("G");
}

void loop()
{
  lis.read();      // get X Y and Z data at once

  pitch = atan2( lis.x_g, sqrt(lis.y_g * lis.y_g + lis.z_g * lis.z_g)) * RAD_TO_DEG;
  roll  = atan2( lis.y_g, sqrt(lis.x_g * lis.x_g + lis.z_g * lis.z_g)) * RAD_TO_DEG;

  x_p = x;
  P = P_p + var_p;
  K = P/(P+var_r);
  x = x_p + K*( pitch-x_p );
  P_p = ( 1-K )*P;
  
  // Then print out the raw data
  //Serial.print("X:  ");     Serial.print(lis.x); 
  //Serial.print("    Y:  "); Serial.print(lis.y); 
  //Serial.print("    Z:  "); Serial.print(lis.z); 
  Serial.print(roll);
  Serial.print(" ");
  Serial.print(pitch); 
  Serial.print(" ");
  Serial.println(x); 
 
  delay(20);
}
