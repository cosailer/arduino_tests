// Potentiometer is connected to GPIO 34 (Analog ADC1_CH6) 
const int potPin = 0;

// variable for storing the potentiometer value
int potValue = 0;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("testing");
}

void loop()
{
  // Reading potentiometer value
  potValue = analogRead(potPin);
  Serial.println(potValue);
  delay(100);
}
