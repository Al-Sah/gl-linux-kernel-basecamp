#include <stdio.h>
#include <stdint-gcc.h>
#include <malloc.h>

#include "k-module/matrix_controller.h"
const char* filename = "/dev/matrix_pixels";


int main() {

    FILE *fp = fopen(filename,"wb");
    if(!fp){
        printf("Failed to open %s file", filename);
        return 1;
    }

    frame_buffer_t *frame = malloc(sizeof(frame_buffer_t));
    frame->len = 64;
    frame->pixels = malloc(sizeof(pixel_t) * frame->len);

    int colour = 0;
    for(int x = 0; x < 64; x++ ){
        frame->pixels[x].colours[RED]   = colour;
        frame->pixels[x].colours[GREEN] = colour;
        frame->pixels[x].colours[BLUE]  = colour;
        colour+=2;
    }

    fwrite(frame, sizeof(frame_buffer_t), 1 , fp);
    fclose(fp);
    return 0;
}
