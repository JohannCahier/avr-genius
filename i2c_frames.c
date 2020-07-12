#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "i2c_frames.h"


static frame_t *_frames = NULL; // first frame in the list
static frame_t *_last_frame = NULL; // last valid frame
static frame_t *_rx_cache = NULL; // "work in progress" frame

frame_t *get_next_i2c_frame() {
    cli(); // avoid race condition with TWI ISR
    // TODO : mask only TWIE ??
    PORTD &= ~(1<<PD2);
    frame_t *frame = _frames;
    if (_frames) {
        _frames = _frames->_next;
        if (!_frames) _last_frame = NULL;
        frame->_next = NULL; // remove internal ref
    }
    PORTD |= (1<<PD2);
    sei(); // reenable ISR
    return frame;
}



// ?OTE : both function below are callbacks for TWI ISR,
// => cli()/sei() is managed by the interrupt controller.
void i2c_frame_receive_byte(uint8_t data) {
    if (_rx_cache) {
        _rx_cache->buffer[_rx_cache->length++] = data;
    } else {
        // should never happend
        printf("rx: no cache\n");
        while(1) {PORTD ^= 1<<PD4;}
    }
}


// TODO : enqueue frame on STOP cond.
// WARNING Stop callback *MUST* be as short as possible (maybe only set a flag)
//         otherwise we may miss start copndition / lose frames

void i2c_frame_startcond() {
    PORTD &= ~(1<<PD4);
    PORTD &= ~(1<<PD5);
    if (_rx_cache) {
        if (_last_frame) {
            _last_frame->_next = _rx_cache;
        } else {
            // no last frame => no frame in the list.
            _frames = _rx_cache;
        }
    }
    _last_frame = _rx_cache;
    _rx_cache = malloc(sizeof(frame_t));
    if (!_rx_cache) {
        printf("can't malloc() rx_cahe !\n");
        while(1) {PORTD ^= 1<<PD4;}
    }
    _rx_cache->length = 0;
    _rx_cache->_next = NULL;
    PORTD |= (1<<PD4);
    PORTD |= (1<<PD5);
}
