#include <Wire.h>
#include <Adafruit_PCD8544.h>
#include <Adafruit_INA219.h>

Adafruit_PCD8544 display = Adafruit_PCD8544(19, 18, 15, 14, 13);

Adafruit_INA219 ina219;

unsigned long last_power = 0;
unsigned long last_sd = 0;

uint16_t interval_power = 100;
uint16_t interval_sd = 1000;

unsigned long time_current = 0;

float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float energy = 0;


void setup()
{
    display.begin();
    display.setContrast(55);
    display.clearDisplay();

    display.setTextColor(BLACK);
    display.setTextSize(1);
    display.setCursor(0, 0);

    display.println("flag 1");
    
    //switch on relay
    Serial.begin(115200);
    
    Wire.begin();
    ina219.begin();
}

void loop()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Unknow error at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(500);           // wait 5 seconds for next scan
}
