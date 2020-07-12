#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#if F_CPU != 16000000
#warning This code assumes a 16MHz frequency !!
#endif

static volatile uint32_t _millis;

void timer_init() {
//     DDRD |= (1<<2);
//     PORTD |= (1<<2);
    TCCR0A = 0x02; // CTC, no pin driven
    TCCR0B = 0x03; // prescaler 1/64 @ 16 MHz
    TCNT0 = 0x00;  // init counter
    OCR0A = 249;   // 250*64=16000 (1ms)
    OCR0B = 0;
    TIMSK0 = 0x02; // enable TIMER2_COMPA ISR
}


ISR(TIMER0_COMPA_vect) {
//     PORTD &= ~(1<<2);
    _millis++;
//     PORTD |= (1<<2);
}

uint32_t timer_get_ms() {
    uint32_t cache;
    cli();
    cache = _millis;
    sei();
    return cache;
}
