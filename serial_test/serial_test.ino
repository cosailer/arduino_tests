
unsigned long count = 0;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

}

void loop()
{
  // put your main code here, to run repeatedly:
  Serial.print("count = ");
  Serial.println(count);
  delay(100);
  count++;

}
