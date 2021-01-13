

int drive_min = 620;
int E1 = 12;
int M1 = 14;
int E2 = 26;
int M2 = 27;

volatile int direc1 = 0;
volatile int direc2 = 0;

//encoder pin: 13, 25
int C1 = 13;
int C2 = 25;

int count1, count2;

void IRAM_ATTR encoder1_isr()
{
//    int direc = digitalRead(M1);

    if( direc1 == 1 )
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

    if( direc2 == 1 )
    {
        count2++;
    }
    else
    {
        count2--;
    }
}


const int freq = 25000;
const int motorChannel = 0;
const int resolution = 10; //Resolution 8, 10, 12, 15

//moter drive function
//moter drive function
void motors_drive(int value)
{
    if(value > 0)
    {
        direc1 = 1;
        direc2 = 1;
    }
    else
    {
        direc1 = 0;
        direc2 = 0;
    }

    value = abs(value) + drive_min;

    if(value > 1023)
    {
        value = 1023;
    }
    
    digitalWrite( M1, direc1 );
    digitalWrite( M2, direc2 );
    ledcWrite(motorChannel, value);
}

void setup()
{
    Serial.begin(115200);
    
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
    /*
    for(int i = 600; i < 800; i+=10)
    {
        Serial.println(i);
        motors_drive(i);
        delay(2000);
    }
    */
    Serial.print(direc1);
    Serial.print(", ");
    Serial.print(count1);
    Serial.print("     ");
    Serial.print(direc2);
    Serial.print(", ");
    Serial.println(count2);

    motors_drive(100);
    delay(500);

    Serial.print(direc1);
    Serial.print(", ");
    Serial.print(count1);
    Serial.print("     ");
    Serial.print(direc2);
    Serial.print(", ");
    Serial.println(count2);

    motors_drive(-100);
    delay(500);
    

    
}
