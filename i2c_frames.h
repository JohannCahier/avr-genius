#ifndef _I2C_FRAMES_LINKED_LIST_H_
#define _I2C_FRAMES_LINKED_LIST_H_

typedef struct node {
    uint8_t length;
    uint8_t buffer[16];
    struct node *_next; //intrernal use ONLY, will be set to NULL when passed to user
} frame_t;


// return NULL if no frame available
// user _MUST_ free() the returned pointer when frame has been processed
frame_t *get_next_i2c_frame();


// callbacks for i2c_salve_init()
void i2c_frame_receive_byte(uint8_t data);
void i2c_frame_startcond();

#endif
