int incomingByte = 0;    // for incoming serial data

void setup() {
    Serial.begin(115200);    // opens serial port, sets data rate to 9600 bps
}

void loop() {
  // send data only when you receive data:
  if (Serial.available() > 0) {
  
    // read the incoming byte:
    incomingByte = Serial.read();
  
    // say what you got:
    Serial.print((char)incomingByte);
  }

  Serial.println("hello there");
  delay(1000);
  
}
