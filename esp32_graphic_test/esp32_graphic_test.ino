

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16


//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, OLED_SCL, OLED_SDA, /* reset=*/ OLED_RST);

U8G2_UC1611_EA_DOGXL240_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 17);	// DOGXL240 Test Board
//U8G2_UC1611_EA_DOGXL240_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);   // Due, SW SPI, DOGXL240 Test Board

//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 16, /* data=*/ 17, /* reset=*/ U8X8_PIN_NONE);   // ESP32 Thing, pure SW emulated I2C
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 16, /* data=*/ 17);   // ESP32 Thing, HW I2C with pin remapping

// End of constructor list

void setup(void)
{
  u8g2.begin();
  //u8g2.setBusClock(2000000);
  u8g2.setFont( u8g2_font_amstrad_cpc_extended_8f );
  //u8g2.setFontRefHeightExtendedText();
  //u8g2.setDrawColor(1);
  //u8g2.setFontPosTop();
  //u8g2.setFontDirection(0);

  u8g2.clearBuffer();

}

char c = 32;
int i = 8;

void loop(void)
{
    /*
    u8g2.drawStr( 0, i, "hello from esp32, 1234567890");
    u8g2.sendBuffer();
    delay(1000);
    i+=8;
    if(i>128) { i = 8; }
    */
    draw1();

}

void draw1()
{
    for(int y = 0; y < 128; y+=9)
    {
        for(int x = 0; x < 240; x+=8)
        {
            u8g2.drawStr( x, y, &c);
            u8g2.sendBuffer();
            delay(100);

            if (c == 128) { c = 32; }
            c++;
        }
      
    }
}
