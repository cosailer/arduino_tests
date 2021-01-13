
//---------------------------------------//
// ----------===[ KEBUN IT]===-----------//
// --=[ Contact : kebunit@gmail.com ]=-- //
//---------------------------------------//

// for Anode Seven Segment
#define ON LOW
#define OFF HIGH

// for Cathode Seven Segment
//#define ON HIGH
//#define OFF LOW

#define data 2
#define clock 3
#define D1 4
#define D2 5
#define D3 6
#define D4 7

void setup() {
 pinMode(data,OUTPUT);
 pinMode(clock,OUTPUT);
 pinMode(D1,OUTPUT);
 pinMode(D2,OUTPUT);
 pinMode(D3,OUTPUT);
 pinMode(D4,OUTPUT);
 Serial.begin(9600);
}

// digit 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
byte segment[]=
     {
         B11000000,
         B01111001,
         B0100100,
         B0110000,
         B0011001,
         B0010010,
         B0000010,
         B01111000,
         B0000000,
         B0010000
      };

// Set angka yang anda inginkan berdasarkan digit
// contoh Countdown dimulai dari 1000
int angka1 = 0; // angka satuan
int angka2 = 0; // angka puluhan
int angka3 = 0; // angka ratusan
int angka4 = 1; // angka ribuan

int digit1 = 0, a = angka1; 
int digit2 = 0, b = angka2; 
int digit3 = 0, c = angka3; 
int digit4 = 0, d = angka4; 
int pembatas = 0;
bool berhenti = true;


void loop()
{
    digit1--;
    digit2--;
    digit3--;
    digit4--;
    if ((a == angka1)&& (b == angka2 ) && (c == angka3) && (d == angka4)){ 
      for (int x =0 ; x < 200;x++){
        Number(1,angka1);
        clearLED();
        Number(2,angka2);
        clearLED();
        Number(3,angka3);
        clearLED();
        Number(4,angka4);
        clearLED();
      }
      c = angka3-1;
      berhenti = false;
      Serial.println("Tampilan pertama timer dalam detik");
    }
    // delay angka satuan
    if (digit1 % 10 == 0) {
      a--;
      Serial.println("==>> digit1");
      clearLED();
    }
    if (digit1 < 0){
        digit1 = 100;
        a = 9;
        Serial.println("=========>> digit2");
    }
    
    // delay angka puluhan
    if (digit2 % 100 == 0) {
      b--;
      clearLED();
    }
    if (digit2 < 0){
        digit2 = 1000;
        b = 9;
        Serial.println("===================>> digit3 ");
    }
    
    // delay angka ratusan
    if (digit3 % 1000 == 0) {
      c--;
      clearLED();
    }
    if ((digit3 < 0)&&(c < 0)){
        pembatas++;
        digit3 = 1000;
        c = 9;
        Serial.println("==========================================>> digit4");
        d--;  // untuk angka ribuan (digit ke-4)
        if (d < 0) {
          d = 0;
        }
    }   
    
    Number(1,a);
    clearLED();
    Number(2,b);
    clearLED();
    Number(3,c);
    clearLED();
    Number(4,d);
    clearLED();
    
    // delay ketika |0|0|0|0|
    if ((c == 0)&&(d == 0) &&(b == 0 ) && (a == 0)){
      Serial.println("Delay untuk angka |0|0|0|0| ");
      for (int y = 0; y < 500; y++){
        Number(1,a);
        clearLED();
        Number(2,b);
        clearLED();
        Number(3,c);
        clearLED();
        Number(4,d);
        clearLED();
      }
      c = angka3;
      d = angka4;
      b = angka2;
      a = angka1;
      digit1 = 0;
      digit2 = 0;
      digit3 = 0;
      digit4 = 0;
      berhenti = true;
    }
}


void clearLED(){
  delay(3);
  digitalWrite(D1,OFF);
  digitalWrite(D2,OFF);
  digitalWrite(D3,OFF);
  digitalWrite(D4,OFF);
}

void Number(int digit,int angka){
  switch (digit) {
    case 1:
      digitalWrite(D1,ON);
      break;
    case 2:
      digitalWrite(D2,ON);
      break;
    case 3:
      digitalWrite(D3,ON);
      break;
    case 4:
      digitalWrite(D4,ON);
      break;
  }
  switch (angka){ 
    case 0:
        shiftOut(data, clock, MSBFIRST, segment[0]);
        break;
     case 1:
        shiftOut(data, clock, MSBFIRST, segment[1]);
        break;
    case 2:
        shiftOut(data, clock, MSBFIRST, segment[2]);
        break;
    case 3:
        shiftOut(data, clock, MSBFIRST, segment[3]);
        break;
    case 4:
        shiftOut(data, clock, MSBFIRST, segment[4]);
        break;
    case 5:
        shiftOut(data, clock, MSBFIRST, segment[5]);
        break;
    case 6:
        shiftOut(data, clock, MSBFIRST, segment[6]);
        break;
    case 7:
        shiftOut(data, clock, MSBFIRST, segment[7]);
        break;
    case 8:
        shiftOut(data, clock, MSBFIRST, segment[8]);
        break;
    case 9:
        shiftOut(data, clock, MSBFIRST, segment[9]);
        break;
  }
}
