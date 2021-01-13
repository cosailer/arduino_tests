#include <Wire.h>
#include <SPI.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Adafruit_INA219.h>
//#include "SdFat.h"
//SdFat SD;
//File file_log;

// Software SPI (slower updates, more flexible pin options):
// pin C3 - 19 - Serial clock out (SCLK)
// pin C2 - 18 - Serial data out (DIN)
// pin D7 - 15 - Data/Command select (D/C)
// pin D6 - 14 - LCD chip select (CS)
// pin D5 - 13 - LCD reset (RST)
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

//#define PWR_PIN 11

void setup()
{
    //switch on relay
    //pinMode(PWR_PIN, OUTPUT);
    //digitalWrite(PWR_PIN, HIGH);

    display.begin();
    display.setContrast(55);
    display.clearDisplay();

    display.setTextColor(BLACK);
    display.setTextSize(1);
    display.setCursor(0, 0);

    //display.println("flag 1");
    //display.display();

    delay(1000);

    Serial.begin(115200);
    Wire.begin();
    ina219.begin();
    //SD.begin(SS);
}

void loop()
{
    time_current = millis();
    
    if (time_current - last_power >= interval_power)
    {
        last_power = time_current;
      
        ina219_get_values();

        display_data();

        Serial.print('[');
        Serial.print(time_current);
        Serial.print(", ");
        Serial.print(loadvoltage);
        Serial.print(", ");
        Serial.print(current_mA);
        Serial.print(", ");
        Serial.print(energy);
        Serial.println(']');
        /*
        if( busvoltage < 2850 )
        {
            
            digitalWrite(PWR_PIN, LOW);

            display.clearDisplay();
            display.setTextColor(BLACK);
            display.setTextSize(1);
            display.setCursor(0, 0);

            display.println("discharge");
            display.println("complete");

            display.print(" E=");
            display.print(energy);
            display.println("mWh");
           
            display.display();

            //stuck here
            while(1) { delay(1000); }
            
        }
        else
        {
            display_data();
        }
        */
        
    }

    /*
    if (time_current - last_sd >= interval_sd)
    {
        last_sd = time_current;



        
        file_log = SD.open("power.txt", FILE_WRITE);
        if (file_log)
        {
            file_log.print(time_current);
            file_log.print(", ");
            file_log.print(loadvoltage);
            file_log.print(", ");
            file_log.print(current_mA);
            file_log.print(", ");
            file_log.println(energy);
            file_log.close();
        }
        
    }
    */
}

void display_data()
{
    display.clearDisplay();
    display.setTextColor(BLACK);
    display.setTextSize(1);
    display.setCursor(0, 0);

    display.print("Ul=");
    display.print(loadvoltage);
    display.println("mV");

    display.print("Us=");
    display.print(shuntvoltage);
    display.println("mV");

    display.print("Ub=");
    display.print(busvoltage);
    display.println("mV");

    display.print(" I=");
    display.print(current_mA);
    display.println("mA");

    //display.print(" P=");
    //display.print(loadvoltage * current_mA);
    //display.println("mW");

    display.print(" E=");
    display.print(energy);
    display.println("mWh");
    
    display.display();
}

void ina219_get_values()
{
    shuntvoltage = ina219.getShuntVoltage_mV();  //in mV
    busvoltage = ina219.getBusVoltage_raw();     //in mV
    current_mA = ina219.getCurrent_mA();         //in mA
    loadvoltage = busvoltage + shuntvoltage;     //in mV
    
    // mWh = (mv)*(ma)*(0.1s)/1000 = mv*ma*1/36000/1000
    energy = energy + loadvoltage * current_mA / 36000000;
}
