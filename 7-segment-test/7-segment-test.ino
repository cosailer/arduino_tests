//---------------------------------------//
// ----------===[ KEBUN IT]===-----------//
// --=[ Contact : kebunit@gmail.com ]=-- //
//---------------------------------------//

// for Cathode Seven Segment
#define ON LOW
#define OFF HIGH

// definisi port yang di pakai
#define data 26
#define clock 27
#define D1 28
#define D2 29
#define D3 30
#define D4 31

void setup() {
 pinMode(data,OUTPUT);
 pinMode(clock,OUTPUT);
 pinMode(D1,OUTPUT);
 pinMode(D2,OUTPUT);
 pinMode(D3,OUTPUT);
 pinMode(D4,OUTPUT);
 Serial.begin(9600);
}

// Angka 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 pada Shift Register
byte segment[]=
{
     B10000100,  // = 0
     B10111110,  // = 1
     B11001000,  // = 2
     B10011000,  // = 3
     B10110010,  // = 4
     B10010001,  // = 5
     B10000001,  // = 6
     B10111100,  // = 7
     B10000000,  // = 8
     B10010000   // = 9
};

int j = 0;

void loop()
{
  for (int i = 0; i < 100000; i++){
    for (int j = 0; j <=9;j++)
    {
    shiftOut(data, clock, MSBFIRST, segment[j]); // menampilkan angka 0 sampai 9
    digitalWrite(D1, ON);
    digitalWrite(D2, ON);
    digitalWrite(D3, ON);
    digitalWrite(D4, ON);
        delay(1000);
    }
  }
}
