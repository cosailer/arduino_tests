#include <avr/power.h>

volatile byte i;
volatile byte data[12];
volatile byte addr[8];
volatile float temp_max = 0;


volatile long count = 0;
volatile int32_t temp_inter = 0;
volatile uint16_t vcc = 0;
volatile int flag = 0;

//read internal vcc voltage
uint16_t read_vcc()
{
    uint16_t vcc_mv;

    //reset
    ADCSRA = 0;
    ADCSRB = 0;
    ADMUX  = 0;

    //read 1.1V band gap against AVcc
    ADMUX |= (1<<REFS0)|(1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1);
    
    _delay_ms(2);
    
    //10-bit resolution with ADC clock speed of 50 kHz to 200 kHz
    //Enable ADC, set prescaller to /64, ADC clock of 16mHz/64=125kHz
    ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);

    _delay_ms(125);

    //dummy read
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADSC)) != 0);

    _delay_ms(20);
    
    //actual read
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADSC)) != 0);
    
    //calculate AVcc in mV, 1.1*1024*1000/ADC
    vcc_mv = (uint16_t)(1126400/ADC); 
    return vcc_mv;
}

//read internal temperature sensor
int32_t read_inter_temperature()
{
    int32_t temp_c;

    //reset
    ADCSRA = 0;
    ADCSRB = 0;
    ADMUX  = 0;
    
    //setup ADC to use internal 2.56V reference
    //ADC right adjust result
    ADMUX |= (1<<REFS1)|(1<<REFS0)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0);
    ADCSRB |= (1<<MUX5);

    _delay_ms(2);
    
    //10-bit resolution with ADC clock speed of 50 kHz to 200 kHz
    //Enable ADC, set prescaller to /64, ADC clock of 16mHz/64=125kHz
    ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);

    _delay_ms(125);
    
    //dummy read
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADSC)) != 0);

    _delay_ms(20);
    
    // start a new conversion
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADSC)) != 0);
    
    // Calculate the temperature in C, offset = 275
    temp_c = ADC - 275;
    
    return temp_c;
}



void setup(void)
{

    Serial.begin(115200);
    //power_adc_enable();

}

void loop(void)
{
    temp_inter = read_inter_temperature();
    vcc = read_vcc();
    
    Serial.print("COUNT = ");
    Serial.print(count);
    Serial.print(", VCC = ");
    Serial.print(vcc);
    Serial.print("mv, TEMP = ");
    Serial.print(temp_inter);
    Serial.println(" degree");
    
    count++;

    delay(1000);
}
