//https://github.com/arduino/ArduinoCore-samd/blob/1.6.16/variants/mkrzero/variant.cpp
//mkrzero led 32
//sparkfun-samd21-pro-breakout led 9, rx 17, tx 30
//aspduno led 20, 21


#define LED_B 13
#define LED_1 9
//#define LED_2 21
//#define LED_3 PC7
//#define LED_4 PE6

void setup()
{
  pinMode(LED_B, OUTPUT);
  pinMode(LED_1, OUTPUT);
  //pinMode(LED_2, OUTPUT);
  //pinMode(LED_3, OUTPUT);
  //pinMode(LED_4, OUTPUT);
}

void loop()
{
  //digitalWrite(LED_B, HIGH);
  digitalWrite(LED_1, HIGH);
  //digitalWrite(LED_2, HIGH);
  //digitalWrite(LED_3, HIGH);
  //digitalWrite(LED_4, HIGH);
  delay(1000);
  //digitalWrite(LED_B, LOW);
  digitalWrite(LED_1, LOW);
  //digitalWrite(LED_2, LOW);
  //digitalWrite(LED_3, LOW);
  //digitalWrite(LED_4, LOW);
  delay(100);
}
