#include "heltec.h"

//E1,E2: Motor Enable Pin & PWM Control
//M1,M2: Motor Direction Pin. 

//I2C sensors: L3G4200D[0x69] + ADXL345[0x53] + HMC5883L[0x1E] + BMP085[0x77]


#include <Wire.h>
#include <MPU6050_tockn.h>
#include <WiFi.h>
#include <U8x8lib.h>

//built in OLED
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

const char* ssid     = "Vodafone-F97B";
const char* password = "qeHRmEJ8atMEHQR3";
const char* host = "192.168.0.177";

String line;


MPU6050 mpu6050(Wire);

int E1 = 12;
int M1 = 14;
int E2 = 26;
int M2 = 27;

int direc1 = LOW;
int direc2 = LOW;

// 10 bits, max frequency = 78.125 KHz
//  8 bits, max frequency = 312.50 kHz
const int freq = 25000;
const int motorChannel = 0;
const int resolution = 10; //Resolution 8, 10, 12, 15


// gain value for pid controller, K0:K3 -> Kp, Ki, Kd, target angle
float K_pid[4];

int interval = 5;

float lastError = 0;
float integral = 0;

void set_pid_gain(float p, float i, float d, float a, int t)
{
   K_pid[0] = p;
   K_pid[1] = i;
   K_pid[2] = d;
   K_pid[3] = a;
   interval = t;
}

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

// This is just like "angle", but it is based solely on the
// accelerometer.
float angle_acce = 0;
float angle_adjusted = 0;

unsigned long lastCorrectionTime = 0;
unsigned long nowTime = 0;

//moter drive function
void motors_drive(int value)
{

    if(value > 0)
    {
        direc1 = HIGH;
        direc2 = HIGH;
    }
    else
    {
        direc1 = LOW;
        direc2 = LOW;
    }

    value = abs(value) + 600;

    if(value > 1023)
    {
        value = 1023;
    }
    
    digitalWrite( M1, direc1 );
    digitalWrite( M2, direc2 );
    ledcWrite(motorChannel, value);
}


// PID controller implementation
void setMotors_pid()
{
    int32_t speed = 0;
    float error = 0;
    float errorDifference = 0;
    float constrain_i = 50;
  
    if (abs(angle_adjusted) > 50)
    {  speed = 0;  }
    else
    {
        error = angle_adjusted - K_pid[3];

        integral += error;
        integral = constrain(integral, -constrain_i, constrain_i);

        errorDifference = error - lastError;
        
        // control law of the pid controller
        // K0:K3 -> Kp, Ki, Kd, target angle
        speed = K_pid[0]*error + K_pid[1]*integral + K_pid[2]*errorDifference;

        lastError = error;
    }

    speed = constrain(speed, -400, 400);

    Serial.print("angle:");
    Serial.print(angle_adjusted);
    Serial.print(", ");
    Serial.print("speed:");
    Serial.print(speed);
    Serial.print(", ");
    Serial.print("error:");
    Serial.print(error);
    Serial.print(", ");
    Serial.print("integral:");
    Serial.print(integral);
    Serial.print(", ");
    Serial.print("errorDifference:");
    Serial.print(errorDifference);
    Serial.println(", ");
    
    motors_drive(speed);
}


void setup()
{
    // set initial PID value
    //############### pid controller ###############
    // official default value (35, 5, 10, 20);
    // without lcd, Kp, Ki, Kd, sample_interval
    //set_pid_gain(35.0, 5.0, 30.0, 3, 20);
    
    //set_pid_gain(50, 0, 20, -8.5, 5);

    u8x8.begin();
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    
    Serial.begin(115200);
    Wire.begin();

    mpu6050.begin();
    mpu6050.calcGyroOffsets(true);

    // We start by connecting to a WiFi network
    Serial.print("\nConnecting to ");
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


    delay(1500);
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

    //motor setup
    pinMode(M1, OUTPUT);
    pinMode(M2, OUTPUT);
    pinMode(E1, OUTPUT);
    pinMode(E2, OUTPUT);
    
    ledcSetup(motorChannel, freq, resolution);
    ledcAttachPin(E1, motorChannel);
    ledcAttachPin(E2, motorChannel);

    
}

void loop()
{
    // Update the angle using the gyro as often as possible.
    //updateAngle();
    mpu6050.update();
    angle_adjusted = mpu6050.getAngleY();

    //Serial.println(angle_adjusted);
    //Serial.print(count1);
    //Serial.print(", ");
    //Serial.println(count2);
    
    // Every 20 ms (50 Hz), correct the angle
    nowTime = millis();
  
    if ((float)(nowTime - lastCorrectionTime) >= interval)
    {
        lastCorrectionTime = nowTime;
        setMotors_pid();
    }
    
}
