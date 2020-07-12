#include <stdint.h>
#include <avr/io.h>

unsigned int adc_read(uint8_t analogChannel)
{
    if (analogChannel > 5) {
        return 0xFFFF;
    }
    unsigned int adc_value; // Variable to hold ADC result


    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS0);
    // ADEN: Set to turn on ADC , by default it is turned off
    //ADPS2: ADPS2 and ADPS0 set to make division factor 32
    ADMUX=analogChannel | (1<<REFS0); // ADC input channel set to PC5
    ADCSRA |= (1<<ADSC); // Start conversion
        while (ADCSRA & (1<<ADSC)); // wait for conversion to complete
    adc_value = ADCW; //Store ADC value

    return adc_value;
}

