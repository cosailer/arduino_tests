
char* buffer;

void setup()
{
    buffer = (char*) ps_malloc(3.5*1024*1024);

    Serial.begin(115200);
}

void loop()
{
    if (buffer)
    {
       Serial.println(F("PSRAM! Allocate 2MB - Success"));
    }
    else
    {
      Serial.println(F("PSRAM! Allocate 2MB - Failed"));
    }

    delay(1000);
}
