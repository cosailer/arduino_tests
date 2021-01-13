// preliminary arduino code for Epilot E4 powerbank
// version 1.0 by CoSailer
//
// display info: https://os.mbed.com/components/128x32-LCD/

// device calibration data
//                                    E4      S4
//                          offset  m168pa  m168pa
// Device signature byte 1  0x0000   0x1E    0x1E
// Device signature byte 2  0x0002   0x94    0x94
// Device signature byte 3  0x0004   0x0B    0x0B
// RC calibration byte      0x0001   0xA9    0x92

#include <Arduino.h>
#include <avr/sleep.h>
#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

#define TEMP_OFFSET   301   //temperature sensor offset value

#define BACKLIT_VALUE 0   //display brightness (0-255, inverted), 200
#define LCD_UPDATE_T  500   //display update interval, 500ms
#define BACKLIT_T     10000 //display back light on time, 10s
#define USB_TIMEOUT   5000  //disable usb output 5s after enabled
#define SYS_TIMEOUT   20000 //put system to sleep after 20s, unless cells are charging or usb output nonzero
#define MIN_DISCHARGE 2850  //minimum discharge voltage, 2850mV

U8X8_ST7565_NHD_C12832_4W_HW_SPI u8x8(/* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);

//display variables
      char lcd_buffer[16]; //char buffer for one row
const byte back_led_pin = PD6;
volatile byte back_led = 0;

//battery variables
unsigned long vcc = 0;

unsigned long v_b1 = 0;
unsigned long v_b2 = 0;
unsigned long v_b3 = 0;
unsigned long v_b4 = 0;

   const byte v_pin_b1 = PC2;
   const byte v_pin_b2 = PC3;
   const byte v_pin_b3 = PC4;
   const byte v_pin_b4 = PC5;

   const byte charge_det_pin = 12; //PB4;
volatile byte charge_det = 0;
volatile byte charge_det_last = 0;

   const byte charge_pin_b12 = 20; //PB6
   const byte charge_pin_b34 = 21; //PB7

//usb port variables
unsigned long v_usb = 0;

   const byte v_pin_usb = A6;
   const byte mt5033_en_pin = PD4;
volatile byte mt5033_en = 0;
   const byte usb_det_pin = 2; //PD2
volatile byte usb_det = 1;

   const byte usb_1_en_pin = PD0;
   const byte usb_2_en_pin = PD1;

//front led light and button variables
   const byte led_pin = PD5;
volatile byte led = 0;
   const byte button_pin = PD3;
volatile byte button = 0;

//usb port current measurement variables
unsigned long c_usb_1 = 0;
unsigned long c_usb_2 = 0;
   const byte c_usb_1_pin = A7;
   const byte c_usb_2_pin = PC1;

//temperature sensors variables
const byte temp_0_pin = PC0;
   int16_t temp_0 = 1;
   int16_t temp_1 = 1;

//timer variables
unsigned long time_current = 0;
unsigned long backlit_timer_start = 0;
unsigned long backlit_timer_current = 0;

unsigned long i_timer_start = 0;
unsigned long i_timer_current = 0;
unsigned long i_timer_diff = 0;

unsigned long measure_timer_start = 0;
unsigned long measure_timer_current = 0;

unsigned long button_timer_start = 0;
unsigned long button_timer_stop = 0;
unsigned long button_duration = 0;
volatile byte button_state = 0;   // 0:not pressed,  1:pressed,  2:released

//sleep variables
volatile byte sleep_state = 0;   // 0:not sleep,  1:sleeping

//read internal vcc voltage
uint16_t read_vcc()
{
    uint16_t vcc_mv;

    //reset
    ADCSRA = 0;
    ADMUX  = 0;

    //read 1.1V band gap against AVcc
    ADMUX |= (1<<REFS0)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1);
    
    //10-bit resolution with ADC clock speed of 50 kHz to 200 kHz
    //Enable ADC, set prescaller to /64, ADC clock of 8mHz/64=125kHz
    ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);

    //dummy read
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADSC)) != 0);

    _delay_ms(10);
    
    //actual read
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADSC)) != 0);
    
    //calculate AVcc in mV, 1.1*1024*1000/ADC
    //                      1.1*1024*1000 = 1126400
    vcc_mv = (uint16_t)(1126400/ADC);
     
    return vcc_mv;
}

//read internal temperature sensor
int16_t read_inter_temp()
{
    int16_t temp_c;

    //reset
    ADCSRA = 0;
    ADMUX  = 0;
    
    //setup ADC to use internal 1.1V reference
    ADMUX |= (1<<REFS1)|(1<<REFS0)|(1<<MUX3);
    
    //10-bit resolution with ADC clock speed of 50 kHz to 200 kHz
    //Enable ADC, set prescaller to /64, ADC clock of 8mHz/64=125kHz
    ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);
    
    //dummy read
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADSC)) != 0);

    _delay_ms(10);
    
    // start a new conversion
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADSC)) != 0);

    // Calculate the temperature in C,
    temp_c = (ADC-TEMP_OFFSET)*5/6;
    //temp_c = ADC;
    
    return temp_c;
}

void display_init()
{
    u8x8.setPowerSave(0);
    u8x8.setFlipMode(1);
    u8x8.setInverseFont(1);

    u8x8.setFont(u8x8_font_chroma48medium8_r);
}

void setup(void)
{
    //button
    pinMode(button_pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(button_pin), button_low_isr, FALLING);  //LOW, CHANGE, RISING, FALLING
    //attachInterrupt(digitalPinToInterrupt(usb_det_pin), usb_load_isr, FALLING);
  
    //backlight
    back_led = BACKLIT_VALUE;
    pinMode(back_led_pin, OUTPUT);
    analogWrite(back_led_pin, BACKLIT_VALUE);

    u8x8.begin();
    display_init();

    pinMode(charge_det_pin, INPUT);
    pinMode(usb_det_pin, INPUT);
    
    pinMode(charge_pin_b12, OUTPUT);
    pinMode(charge_pin_b34, OUTPUT);
    pinMode(mt5033_en_pin, OUTPUT);
    pinMode(usb_1_en_pin, OUTPUT);
    pinMode(usb_2_en_pin, OUTPUT);
    pinMode(led_pin, OUTPUT);

    disable_usb_output();

    u8x8.drawString(0, 0, "  Project 4E03  ");
    u8x8.drawString(0, 1, "                ");
    u8x8.drawString(0, 2, "  by CoSailer   ");
    u8x8.drawString(0, 3, "                ");
    u8x8.refreshDisplay();
    _delay_ms(2000);
}

void read_info()
{
    vcc = read_vcc();
    temp_1 = read_inter_temp();
    
    v_b1 = analogRead(v_pin_b1);
    v_b2 = analogRead(v_pin_b2);
    v_b3 = analogRead(v_pin_b3);
    v_b4 = analogRead(v_pin_b4);
    
    v_usb = analogRead(v_pin_usb);
    //temp_0 = analogRead(temp_0_pin);

    charge_det = digitalRead(charge_det_pin);
    usb_det = digitalRead(usb_det_pin);

    c_usb_1 = analogRead(c_usb_1_pin);
    c_usb_2 = analogRead(c_usb_2_pin);

         //v_b1*vcc*2/1024
    v_b1 = v_b1*vcc/512;
    v_b2 = v_b2*vcc/512;
    v_b3 = v_b3*vcc/512;
    v_b4 = v_b4*vcc/512;
    
          //v_usb*vcc*2.24/1024
    v_usb = v_usb*vcc*7/3200;

            //c_usb_1*vcc*10/1024;
    c_usb_1 = c_usb_1*vcc*5/512;
    c_usb_2 = c_usb_2*vcc*5/512;

    //reset timer for system timeout
    if( charge_det != charge_det_last)
    {
        i_timer_start = millis();
        backlit_timer_start = millis();
        charge_det_last = charge_det;
    }
}

void display_info()
{
    //u8x8.clearDisplay();
    
    sprintf(lcd_buffer, "%4ld|U%4ld|CHG:%1d", v_b1, v_usb, charge_det);
    u8x8.drawString(0, 0, lcd_buffer);

    sprintf(lcd_buffer, "%4ld|R%4ld|USB:%1d", v_b2, vcc, usb_det);
    u8x8.drawString(0, 1, lcd_buffer);

    sprintf(lcd_buffer, "%4ld|T%4ld|I%4ld", v_b3, millis()/1000, c_usb_1);
    u8x8.drawString(0, 2, lcd_buffer);

    sprintf(lcd_buffer, "%4ld|t%4d|I%4ld", v_b4, temp_1, c_usb_2);
    u8x8.drawString(0, 3, lcd_buffer);

    u8x8.refreshDisplay();
}

void button_low_isr()
{
    i_timer_start = millis();
    backlit_timer_start = millis();
    
    //turn on the backlight
    if(back_led == 255)
    {
        back_led = BACKLIT_VALUE;
        analogWrite(back_led_pin, back_led);
    }
    else //toggle the led front light
    {
        led = !led;
        digitalWrite(led_pin, led);
    }
    
    if(button==0)
    {
        button = 1;
        button_timer_start = millis();
        attachInterrupt(digitalPinToInterrupt(button_pin), button_high_isr, RISING);
    }
}

void button_high_isr()
{
    if(button==1)
    {
        button = 2;
        button_timer_stop = millis();
        attachInterrupt(digitalPinToInterrupt(button_pin), button_low_isr, FALLING);
    }
}

void usb_load_isr()
{
    i_timer_start = millis();
    enable_usb_output();
}

void enable_cell_charging()
{
    digitalWrite(charge_pin_b12, LOW);     //pull high to disable charger
    digitalWrite(charge_pin_b34, LOW);     //pull low to enable charger
}

void disable_cell_charging()
{
    digitalWrite(charge_pin_b12, HIGH);     //pull high to disable charger
    digitalWrite(charge_pin_b34, HIGH);     //pull low to enable charger
}

void enable_usb_output()
{
    digitalWrite(mt5033_en_pin, HIGH);    //enable boost converter
    digitalWrite(usb_1_en_pin, HIGH);     //enable usb port 1
    digitalWrite(usb_2_en_pin, HIGH);     //enable usb port 2
}

void disable_usb_output()
{
    digitalWrite(mt5033_en_pin, LOW);    //disable boost converter
    digitalWrite(usb_1_en_pin, LOW);     //disable usb port 1
    digitalWrite(usb_2_en_pin, LOW);     //disable usb port 2
}

void sleep_shutdown()
{
    sleep_enable();
    attachInterrupt(digitalPinToInterrupt(button_pin), wake_up, LOW);     //setup wakeup call on button low
    //attachInterrupt(digitalPinToInterrupt(usb_det_pin), wake_up, LOW);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    
    //turn off everything here
    analogWrite(back_led_pin, 255); //turn off display backlight
    digitalWrite(led_pin, 0);       //turn off front led
    u8x8.setPowerSave(1);           //turn off display
    disable_cell_charging();
    disable_usb_output();
    
    _delay_ms(500);  //wait a while
    sleep_cpu();      //activating sleep mode
    
    //resume everything here
    analogWrite(back_led_pin, BACKLIT_VALUE); //turn on display backlight
    display_init();               //turn on display
    //enable_cell_charging();
    //enable_usb_output();
}

void wake_up()
{
    sleep_disable();
    
    i_timer_start = millis();
    backlit_timer_start = millis();

    //turn on the backlight
    back_led = BACKLIT_VALUE;
    analogWrite(back_led_pin, back_led);

    //set normal button interrupt
    attachInterrupt(digitalPinToInterrupt(button_pin), button_low_isr, FALLING);
    //attachInterrupt(digitalPinToInterrupt(usb_det_pin), usb_load_isr, FALLING);
}

//simple battery check to make sure the voltage of all cells
//are not below 2850mV
void battery_check()
{
    if( (v_b1<MIN_DISCHARGE)&&(v_b2<MIN_DISCHARGE)&&(v_b3<MIN_DISCHARGE)&&(v_b4<MIN_DISCHARGE) )
    {
        u8x8.drawString(0, 0, "Warning: Battery");
        u8x8.drawString(0, 1, "Over Discharge !");
        u8x8.drawString(0, 2, "or not present  ");
        u8x8.drawString(0, 3, "System Sleeping.");
        u8x8.refreshDisplay();
        _delay_ms(4000);
        sleep_shutdown();
    }
}

void loop(void)
{
    //////////////////////////////////////////////////////////////
    //check button press
    //if button pressed for more than 3 seconds, shutdown and sleep
    if(button == 2)
    {
        button_duration = button_timer_stop - button_timer_start;

        //short press
        if( (button_duration > 50)&&(button_duration < 3000) )
        {
            //disable_usb_output();
        }

        //long press to sleep
        else if(button_duration < 10000000)
        {
            sleep_shutdown();
        }

        button = 0;
        button_timer_start = 0;
        button_timer_stop = 0;
        button_duration = 0;
    }
    else
    {
        //////////////////////////////////////////////////////////////
        //update info on display every 500ms
        measure_timer_current = millis();
    
        if( (measure_timer_current-measure_timer_start) > LCD_UPDATE_T )
        {
            measure_timer_start = measure_timer_current;
            read_info();
            display_info();
            battery_check();
        }

        //////////////////////////////////////////////////////////////
        //if backlit is on, turn it off after 10 seconds
        if(back_led < 255)
        {
            backlit_timer_current = millis();
        
            if( (backlit_timer_current-backlit_timer_start) > BACKLIT_T )
            {
                back_led = 255;
                analogWrite(back_led_pin, back_led);
            }
        }

        //////////////////////////////////////////////////////////////
        //enable battery charging if usb input detected
        if(charge_det == 1)
        {
            enable_cell_charging();
        }
        else
        {
            disable_cell_charging();
        }
        
        //////////////////////////////////////////////////////////////
        //if usb load detected, enable usb output
        // or use usb_det_isr()
        if(usb_det == 0)
        {
            i_timer_start = millis();
            enable_usb_output();
        }
    
        //if no current draw at both usb outputs
        //disable usb output 5 seconds after enabled
        if( (c_usb_1==0)&&(c_usb_2==0) )
        {
            i_timer_current = millis();
            i_timer_diff = i_timer_current - i_timer_start;
        
            if( i_timer_diff > USB_TIMEOUT )
            {
                disable_usb_output();
            }

            if( (i_timer_diff > SYS_TIMEOUT)&&(charge_det == 0) )
            {
                //sleep_shutdown();
            }
        }
        
    }
}
