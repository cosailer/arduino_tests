
const byte input = 2;
unsigned long voltage = 0;
unsigned long vcc = 0;

volatile byte button_press = 0;
unsigned long button_timer_start = 0;
unsigned long button_timer_stop = 0;
unsigned long button_diff = 0;
         byte button = 0;
         byte button_flag = 0;

//read internal vcc voltage
uint16_t read_vcc()
{
    uint16_t vcc_mv;

    //reset
    ADCSRA = 0;
    ADMUX  = 0;

    //read 1.1V band gap against AVcc
    ADMUX |= (1<<REFS0)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1);
    
    _delay_ms(10);
    
    //10-bit resolution with ADC clock speed of 50 kHz to 200 kHz
    //Enable ADC, set prescaller to /64, ADC clock of 16mHz/64=125kHz
    ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);

    _delay_ms(10);

    //dummy read
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADSC)) != 0);

    _delay_ms(10);
    
    //actual read
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADSC)) != 0);
    
    //calculate AVcc in mV, 1.1*1024*1000/ADC
    //                      1.1*1024*1000 = 1126400
    //                     1.08*1024*1000 = 1105920
    vcc_mv = (uint16_t)(1126400/ADC);
     
    return vcc_mv;
}

void setup()
{
    Serial.begin(115200);
    pinMode(input, INPUT);
    attachInterrupt(digitalPinToInterrupt(input), button_low_isr, FALLING);  //LOW, CHANGE, RISING, FALLING
}

void loop()
{
    /*
    if(button == 1)
    {
        Serial.print("press start = ");
        Serial.println(button_timer_start);
        button = 0;
    }
    else if(button == 2)
    {
        Serial.print("press stop = ");
        Serial.println(button_timer_start);
        button = 0;
    }
    */

    if(button == 2)
    {
        button_diff = button_timer_stop-button_timer_start;

        if( (button_diff > 50)&&(button_diff < 1000000) )
        {
            Serial.print("pressed = ");
            Serial.println(button_diff);
            button = 0;
            button_timer_start = 0;
            button_timer_stop = 0;
            button_diff = 0;
        }
    }
}

void button_low_isr()
{
    button = 1;
    button_timer_start = millis();
    attachInterrupt(digitalPinToInterrupt(input), button_high_isr, RISING);
}

void button_high_isr()
{
    button = 2;
    button_timer_stop = millis();
    attachInterrupt(digitalPinToInterrupt(input), button_low_isr, FALLING);
}
