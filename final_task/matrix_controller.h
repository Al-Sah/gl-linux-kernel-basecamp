//
// Created by al_sah on 08.10.21.
//

#ifndef MATRIX_CONTROLLER_H
#define MATRIX_CONTROLLER_H

typedef struct uint24_t uint24_t;
typedef struct pixel_t pixel_t;

struct uint24_t {
    unsigned int value : 24;
};

struct pixel_t {
    union {
        uint24_t grb;
        uint8_t  colours[3];
    };
};
struct pixel_t *pixels_buffer = NULL;


enum colours{
    GREEN = 0,
    RED = 1,
    BLUE = 2,
    NONE = 3,
};


#endif //MATRIX_CONTROLLER_H