#include "heltec.h"

//E1,E2: Motor Enable Pin & PWM Control
//M1,M2: Motor Direction Pin. 

//I2C sensors: L3G4200D[0x69] + ADXL345[0x53] + HMC5883L[0x1E] + BMP085[0x77]


#include <Wire.h>
#include <L3G.h>
#include <ADXL345.h>
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

//encoder pin: 13, 25
int C1 = 13;
int C2 = 25;

// 10 bits, max frequency = 78.125 KHz
//  8 bits, max frequency = 312.50 kHz
const int freq = 25000;
const int motorChannel = 0;
const int resolution = 10; //Resolution 8, 10, 12, 15


// gain value for pid controller, K0:K3 -> Kp, Ki, Kd, target angle
float K_pid[4];
int interval = 10;

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

// gain value for lqr controller, K0:K3 -> phi, d_phi, theta, d_theta
float K_lqr[4];

void set_lqr_gain(float k1, float k2, float k3, float k4, int t)
{
   K_lqr[0] = k1;
   K_lqr[1] = k2;
   K_lqr[2] = k3;
   K_lqr[3] = k4;
   interval = t;
}

float angle_offset = 0;

L3G gyroscope;

// This is the average reading obtained from the gyro's Y axis
// during calibration.
float gyroOffsetY;

// This variable holds our estimation of the robot's angle based
// on the gyro and the accelerometer.  A value of 0 means the
// robot is perfectly vertical.  A value of -90 means that the
// robot is horizontal and the battery holder is facing down.  A
// value of 90 means that the robot is horizontal and the battery
// holder is facing up.
float angle_gyro = 0;


ADXL345 accelerometer;
int x,y,z;

// This is just like "angle", but it is based solely on the
// accelerometer.
float angle_acce = 0;
float angle_adjusted = 0;

unsigned long lastCorrectionTime = 0;
unsigned long nowTime = 0;

int count1, count2;

void IRAM_ATTR encoder1_isr()
{
//    int direc = digitalRead(M1);

    if( direc1 == HIGH )
    {
        count1++;
    }
    else
    {
        count1--;
    }
}

void IRAM_ATTR encoder2_isr()
{
//    int direc = digitalRead(M2);

    if( direc2 == HIGH )
    {
        count2++;
    }
    else
    {
        count2--;
    }
}

// Reads the gyro and uses it to update the angle estimation.
void updateAngle()
{
    // Figure out how much time has passed since the last update.
    static uint16_t lastUpdate = 0;
    uint16_t m = micros();
    uint16_t dt = m - lastUpdate;
    lastUpdate = m;

    gyroscope.read();

    // Calculate how much the angle has changed, in degrees, and
    // add it to our estimation of the current angle.  The gyro's
    // sensitivity is 0.07 dps per digit.
    angle_gyro += ((float)gyroscope.g.y - gyroOffsetY) * 70 * dt / 1000000000;

    // read angle from accelerometer
    accelerometer.readXYZ(&x, &y, &z);
    
    //float pitch = (atan2(x,sqrt(y*y+z*z)) * 180.0) / PI;
    float roll = (atan2(y,(sqrt(x*x+z*z))) * 180.0) / PI;  //roll = 12, stable balance target
    angle_acce = roll - angle_offset;
//}

// Reads the accelerometer and uses it to adjust the angle
// estimation.
//void correctAngleAccel()
//{
  //compass.read();

    // Calculate the angle according to the accelerometer.
    //aAngle = -atan2(compass.a.z, -compass.a.x) * 180 / M_PI;

/*
    double xyz[3];
    double ax,ay,az;
    accelerometer.getAcceleration(xyz);
    ax = xyz[0];
    ay = xyz[1];
    az = xyz[2];
  
    // Calculate the magnitude of the measured acceleration vector,
    // in units of g.
    float mag = sqrt(ax*ax+ay*ay+az*az);

    //Serial.println(mag);

    // Calculate how much weight we should give to the
    // accelerometer reading.  When the magnitude is not close to
    // 1 g, we trust it less because it is being influenced by
    // non-gravity accelerations, so we give it a lower weight.
    float weight = 1 - 5 * abs(1 - mag);
    weight = constrain(weight, 0, 1);
    weight /= 10;

    // Adjust the angle estimation.  The higher the weight, the
    // more the angle gets adjusted.
    angle_adjusted = weight * angle_acce + (1 - weight) * angle_gyro;
    */

    angle_adjusted = angle_acce;
}

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

    value = abs(value) + 550;

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
    int32_t speed;
  
    if (abs(angle_adjusted) > 40)
    {  speed = 0;  }
    else
    {
        static float lastError = 0;
        static float integral = 0;

        float error = angle_adjusted - K_pid[3];

        integral += error;
        integral = constrain(integral, -40, 40);

        float errorDifference = error - lastError;
    
        // control law of the pid controller
        // K0:K3 -> Kp, Ki, Kd, target angle
        speed = error*K_pid[0] + integral*K_pid[1] + errorDifference*K_pid[2];

        lastError = error;
    }

    //Serial.println(speed);

    speed = constrain(speed, -400, 400);
    motors_drive(speed);
}

// LQR controller implementation
void setMotors_lqr()
{
    int32_t speed;
  
    if (abs(angle_adjusted) > 40)
    {  speed = 0;  }
    else
    {
        // last phi, theta, and time stamp
        static float phi_last = 0;
        static float theta_last = 0;
        static uint32_t time_stamp_last = 0;
    
        // get phi and theta
        float phi = (count1 + count2)*0.09375; //360/48(motor gear)/40(encoder)/2
        float theta = angle_adjusted;
    
        // get dt, micros() is better than millis()
        uint32_t time_stamp = micros();
        float d_t = (time_stamp - time_stamp_last)/1000000.0;
        time_stamp_last = time_stamp;
    
        // get dx and dtheta
        float d_phi = (phi-phi_last)/d_t;
        float d_theta = (theta-theta_last)/d_t;
    
        // use control law (u = -Kx) to get the output speed
        // K0:K3 -> phi, d_phi, theta, d_theta
        speed = -(phi*K_lqr[0] + d_phi*K_lqr[1] + theta*K_lqr[2] + d_theta*K_lqr[3]);
        speed = constrain(speed, -400, 400);
    
        // update last variable
        phi_last = phi;
        theta_last = theta;
    }
    
    motors_drive(speed);
}


void setup()
{
    // set initial PID value
    //############### pid controller ###############
    // official default value (35, 5, 10, 20);
    // without lcd, Kp, Ki, Kd, sample_interval
    //set_pid_gain(35.0, 5.0, 30.0, 3, 20);
    
    //set_pid_gain(28, 0.1, 3, -1, 20);

    //############### lqr controller ###############
    // with lcd, K1, K2, K3, K4 (x,  d_x, theta, d_theta)
    //K =  -1.0954   -0.5666  -25.3475   -1.2627
    set_lqr_gain(-1.0954, -0.5666, -25.3475, -1.2627, 10);
    
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
    

    //motor setup
    pinMode(M1, OUTPUT);
    pinMode(M2, OUTPUT);
    pinMode(E1, OUTPUT);
    pinMode(E2, OUTPUT);
    
    ledcSetup(motorChannel, freq, resolution);
    ledcAttachPin(E1, motorChannel);
    ledcAttachPin(E2, motorChannel);

    //encoder setup
    pinMode(C1, INPUT_PULLUP);
    pinMode(C2, INPUT_PULLUP);
    attachInterrupt(C1, encoder1_isr, CHANGE);
    attachInterrupt(C2, encoder2_isr, CHANGE);
}

void loop()
{
    // Update the angle using the gyro as often as possible.
    //updateAngle();
    mpu6050.update();
    angle_adjusted = mpu6050.getAngleY() + K_pid[3];

    Serial.println(angle_adjusted);
    //Serial.print(count1);
    //Serial.print(", ");
    //Serial.println(count2);
    
    // Every 20 ms (50 Hz), correct the angle
    nowTime = millis();
  
    if ((float)(nowTime - lastCorrectionTime) >= interval)
    {
        lastCorrectionTime = nowTime;
    
        setMotors_pid();
        //setMotors_lqr();
    }
    
}
