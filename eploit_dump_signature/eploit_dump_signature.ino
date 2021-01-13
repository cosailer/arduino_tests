
#include <Arduino.h>
#include <U8x8lib.h>

#include <avr/boot.h>
#define SIGRD 5

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

U8X8_ST7565_NHD_C12832_4W_HW_SPI u8x8(/* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);

//backlight variables
char lcd_buffer[16]; //char buffer for one row
const byte back_led_pin = PD6;
      byte back_led = 0;

void setup(void)
{
    //backlight
    pinMode(back_led_pin, OUTPUT);
    digitalWrite(back_led_pin, back_led); 
    
    u8x8.begin();
    u8x8.setFlipMode(1);
    u8x8.setPowerSave(0);
    u8x8.setInverseFont(1);

    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.refreshDisplay();
    u8x8.setCursor(0, 0);

    dump_signature();
}

void dump_signature()
{
    byte line_total[32];
  
    for(uint8_t i = 0; i < 32; i++)
    {
        line_total[i] = boot_signature_byte_get(i);
 
        if( i == 8) { u8x8.setCursor(0, 1); }
        if( i == 16) { u8x8.setCursor(0, 2); }
        if( i == 24) { u8x8.setCursor(0, 3); }

        sprintf(lcd_buffer, "%02X", line_total[i]);
        u8x8.print(lcd_buffer);
    }
}

void loop(void)
{
    delay(1000);
}
