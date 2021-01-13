/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <WiFi.h>
#include <U8x8lib.h>

//built in OLED
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

const char* ssid     = "Vodafone-F97B";
const char* password = "qeHRmEJ8atMEHQR3";
const char* host = "192.168.0.177";

String line;

float K_pid[4];

// split the input string into different K values with comma
void split_pid_k(String input)
{
  int index = 1;
  int j = 0;
  
  for (unsigned int i = 1; i < input.length(); i++)
  {
    if (input.substring(i, i+1) == ",")
    {
      K_pid[j] = input.substring(index, i).toFloat();
      index = i+1;
      j++;
      
      if(j == 4) { break; }
    }
  }
}

void display_pid_k()
{
  Serial.print(F("   "));
  Serial.print(F("Kp = "));
  Serial.print(K_pid[0]);
  Serial.print(F("  Ki = "));
  Serial.print(K_pid[1]);
  Serial.print(F("  Kd = "));
  Serial.print(K_pid[2]);
  Serial.print(F("  Ta = "));
  Serial.println(K_pid[3]);
}

void setup()
{
    u8x8.begin();
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    
    Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());


    delay(3000);
    Serial.print("connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    
    if (!client.connect(host, httpPort))
    {
        Serial.println("connection failed");
        return;
    }

    // We now create a URI for the request
    String url = "/pid.conf";

    Serial.print("Requesting URL: ");
    Serial.println(url);

    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0)
    {
        if (millis() - timeout > 5000)
        {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }
    
    // Read all the lines of the reply from server and print them to Serial
    while(client.available())
    {
        line = client.readStringUntil('\r');
    }
    
    Serial.println("closing connection");
    
    WiFi.mode(WIFI_OFF);
    btStop();

    split_pid_k(line);

    Serial.print(line);
    display_pid_k();

    char temp[15];

    u8x8.drawString(0, 0, "PID setting");
    
    sprintf(temp, "P = %3.1f", K_pid[0] );
    u8x8.drawString(0, 2, temp );

    sprintf(temp, "I = %3.1f", K_pid[1] );
    u8x8.drawString(0, 3, temp );

    sprintf(temp, "D = %3.1f", K_pid[2] );
    u8x8.drawString(0, 4, temp );

    sprintf(temp, "Ta = %3.1f", K_pid[3] );
    u8x8.drawString(0, 5, temp );
}


void loop()
{
    Serial.print(line);
    display_pid_k();
    delay(1000);
}
