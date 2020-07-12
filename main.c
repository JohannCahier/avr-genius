// Wire Slave Receiver
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Receives data as an I2C/TWI slave device
// Refer to the "Wire Master Writer" example for use with this

// Created 29 March 2006

// This example code is in the public domain.

// #define F_CPU 16000000UL


#include <util/delay.h>
#include "I2CSlave.h"
#include "avr_uart.h"
#include "i2c_frames.h"
#include <stdlib.h>

// some util function in other c files, with no header
void timer_init();
uint32_t timer_get_ms();
unsigned int adc_read(uint8_t);
// ---------------


// TODO return code on error !!

void config_sensor(uint8_t from){
    if(i2c_slave_MT_init(0x08)) goto end;
    if(i2c_slave_MT_data(from)) goto end;
    if (i2c_slave_MT_data(0x0B)) goto end;
    if (i2c_slave_MT_data(0x00)) goto end;
    if (i2c_slave_MT_data(0x02)) goto end;
    if (i2c_slave_MT_data(0x04)) goto end;
    if (i2c_slave_MT_data(0x05)) goto end;
    if (i2c_slave_MT_data(0x06)) goto end;
    if (i2c_slave_MT_data(0x07)) goto end;
    if (i2c_slave_MT_data(0x08)) goto end;
    if (i2c_slave_MT_data(0x11^from)) goto end;
end:
    i2c_slave_listen_again();
}


void heartbeat(uint8_t from, uint8_t val) {
    if(i2c_slave_MT_init(0x08)) goto end;
    if(i2c_slave_MT_data(from)) goto end;
    if(i2c_slave_MT_data(0x07)) goto end;
    if(i2c_slave_MT_data(0x06)) goto end;
    if(i2c_slave_MT_data(0x00)) goto end;
    if(i2c_slave_MT_data(val)) goto end;
    // chksum
    if(i2c_slave_MT_data(0x11^val^from)) goto end;
end:
    i2c_slave_listen_again();
}

void send_intensity(uint8_t from, uint16_t intensity) {
    uint8_t int_low = intensity & 0xFF;
    uint8_t int_hi = (intensity>>8);
    uint8_t chksum = 0x1d ^ int_low ^ int_hi ^from;
    if(i2c_slave_MT_init(0x08)) goto end;
    if(i2c_slave_MT_data(from)) goto end;
    if(i2c_slave_MT_data(0x08)) goto end;
    if(i2c_slave_MT_data(0x05)) goto end;
    if(i2c_slave_MT_data(0x00)) goto end;
    if(i2c_slave_MT_data(int_hi)) goto end;
    if(i2c_slave_MT_data(int_low)) goto end;
    if(i2c_slave_MT_data(chksum)) goto end;
end:
    i2c_slave_listen_again();
}


void send_config_reply() {
    if(i2c_slave_MT_init(0x20)) goto end;
    if(i2c_slave_MT_data(0x08)) goto end;
    if (i2c_slave_MT_data(0x06)) goto end;
    if (i2c_slave_MT_data(0x00)) goto end;
    if (i2c_slave_MT_data(0x20)) goto end;
end:
    i2c_slave_listen_again();
}


// TODO return code on error !!

void send_button_down(uint8_t from) {
    if(i2c_slave_MT_init(0x08)) goto end;
    if(i2c_slave_MT_data(from)) goto end;
    if(i2c_slave_MT_data(0x08)) goto end;
    if(i2c_slave_MT_data(0x07)) goto end;
    if(i2c_slave_MT_data(0x00)) goto end;
    if(i2c_slave_MT_data(0x01)) goto end;
    if(i2c_slave_MT_data(0x00)) goto end;
    if(i2c_slave_MT_data(0x1e^from)) goto end;
end:
    i2c_slave_listen_again();
}

// TODO return code on error !!

void send_button_up(uint8_t from, uint8_t delay) {
    if(i2c_slave_MT_init(0x08)) goto end;
    if(i2c_slave_MT_data(from)) goto end;
    if(i2c_slave_MT_data(0x08)) goto end;
    if(i2c_slave_MT_data(0x07)) goto end;
    if(i2c_slave_MT_data(0x00)) goto end;
    if(i2c_slave_MT_data(0x00)) goto end;
    if(i2c_slave_MT_data(delay)) goto end;
    if(i2c_slave_MT_data(0x1f^from^delay)) goto end;
end:
    i2c_slave_listen_again();
}

// void receiveEvent(int howMany);
#ifdef IS_LAMP
#warning I am the LAMP
#define SLAVE_ADDRESS 0x08
#else
#warning I am the GENIUS
#define SLAVE_ADDRESS 0x20
#endif
#define HEARTBEAT_PERIOD 500 //0.5s
#define VALUE_PERIOD 1300 // 1.3s
#define CONFIG_PERIOD 10000 // 10s
#define LED_HALF_PERIOD 1500
#define DEBOUNCE_PERIOD 50 // ms


#define PERIOD 1000
#define INTERFRAME_DELAY 200

/*extern uint64_t mymillis();
extern void init_mymillis();*/

void timer_init();
uint32_t timer_get_ms();

int main()
{
    // led pin
    DDRB |= 1<<PB5;
    PORTB |= 1<<PB5;

    // button pin input
    DDRB &= ~(1<<PB4);
    PORTB |= 1<<PB4;

    // debug pins
    DDRD|= 1<<PD2;
    PORTD |= 1<<PD2;
    DDRD |= (1<<PD5);
    PORTD |= (1<<PD5);

    timer_init();
    i2c_slave_init(SLAVE_ADDRESS);
    PORTD &= ~(1<<PD2);
    _delay_ms(100);
    PORTD |= 1<<PD2;
    i2c_slave_setCallbacks(
        i2c_frame_startcond,
        i2c_frame_receive_byte,
        NULL
    );
    _delay_ms(100);
    PORTD &= ~(1<<PD2);
    avr_uart_init();
    stdout = &avr_uart_output;
    stdin  = &avr_uart_input_echo;
    _delay_ms(100);
    PORTD |= 1<<PD2;
#ifndef IS_LAMP
    printf("I AM THE GENIUS, address: 0x%02x\n", SLAVE_ADDRESS);
#else
    printf("I AM THE LAMP, address: 0x%02x\n", SLAVE_ADDRESS);
#endif
//    init_sensor();

    uint32_t timestamp = timer_get_ms();
    uint32_t led_ts = timestamp;
    #ifndef IS_LAMP
    uint32_t heartbeat_ts = timestamp;
    uint32_t config_ts = timestamp;
    uint32_t value_ts = timestamp;
    uint16_t light_value = 0x200;

    uint32_t button_down_ts = 0xFFFF;
    uint32_t button_up_ts = 0xFFFF;

    char data;
    char data_buf[5];
    uint8_t data_buf_len = 0;
    #endif

    #if 0
    //ndef IS_LAMP
    config_sensor(SLAVE_ADDRESS);
    _delay_ms(200);
    heartbeat(SLAVE_ADDRESS, 0);
    _delay_ms(200);
    heartbeat(SLAVE_ADDRESS, 1);
    _delay_ms(200);
    send_intensity(SLAVE_ADDRESS, 0xDEAD);
    #endif

    while(1){
        timestamp = timer_get_ms();
        if (timestamp - led_ts >= LED_HALF_PERIOD) {
            PORTB ^= 1<<PB5;
            led_ts = timestamp;
        }

    #ifndef IS_LAMP
        if (avr_uart_data_avaiable(&data)) {
            printf("[%02x] ", data);
            if (data_buf_len == 4 && !(data == 0x0d)) {
                printf("Input to big please restart\n");
                data_buf_len = 0; data_buf[0] = 0;
            }
            else if (   ((data >= 'a') && (data <= 'f'))
                     || ((data >= 'A') && (data <= 'F'))
                     || ((data >= '0') && (data <= '9'))) {
                            data_buf[data_buf_len++] = data;
                            data_buf[data_buf_len] = 0;
                            printf("Input buffer [%s] (%d)\n", data_buf, data_buf_len);
            }
            else if (data == 'i' || data == 'I') {
                printf("Click!\n");
                send_button_down(0x20);
                _delay_ms(50);
                send_button_up(0x20, 1);
            }
            else if (data_buf_len>0 && (data ==  0x0d)) {
                sscanf(data_buf, "%x", &light_value);
                data_buf_len = 0; data_buf[0] = 0;
            }
        }

        if (timestamp - heartbeat_ts >= HEARTBEAT_PERIOD) {
            heartbeat(SLAVE_ADDRESS, 0);
            _delay_ms(200);
            heartbeat(SLAVE_ADDRESS, 1);
            _delay_ms(200);
            heartbeat_ts = timestamp;
            printf(".");
        }

        if (timestamp - value_ts >= VALUE_PERIOD) {
            /*uint16_t val = adc_read(0) << 4;
            send_intensity(SLAVE_ADDRESS, val);
            value_ts = timestamp;
            printf("%4x\n", val);*/
            send_intensity(SLAVE_ADDRESS, light_value);
            _delay_ms(20);
            value_ts = timestamp;
            printf("%4x\n", light_value);
        }

        if (timestamp - config_ts >= CONFIG_PERIOD) {
            config_sensor(SLAVE_ADDRESS);
            _delay_ms(20);
            config_ts = timestamp;
            printf("X");
        }

        if (   !(PINB & (1<<PB4))
            && (button_down_ts == 0xFFFF)
            && (button_up_ts == 0xFFFF)) {
                button_down_ts = timestamp;
                send_button_down(SLAVE_ADDRESS);
                _delay_ms(20);
        }
        if (   (button_down_ts != 0xFFFF)
            && (timestamp - button_down_ts >= DEBOUNCE_PERIOD)
            && (PINB & (1<<PB4))) {
                button_down_ts = 0xFFFF;
                button_up_ts = timestamp;
                send_button_up(SLAVE_ADDRESS, (button_up_ts - button_down_ts)/100);
                _delay_ms(20);
        }
        if (    (button_up_ts != 0xFFFF)
            &&  (timestamp - button_up_ts >= DEBOUNCE_PERIOD)) {
                button_up_ts = 0xFFFF;
        }

    #endif
        frame_t *frame = get_next_i2c_frame();
        if (frame) {
            if (  frame->length == 0x0a
               && frame->buffer[0] == 0x20
               && frame->buffer[2] == 0x00)
                    send_config_reply();

            printf("> ");
            for(int i=0; i<frame->length; i++) {printf("0x%02x ", frame->buffer[i]);}
            printf("\n");
            free(frame);
        }
    }

    return 0;
}

