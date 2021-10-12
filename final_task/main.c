#include <stdio.h>
#include <stdint-gcc.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "k-module/matrix_control.h"
const char* filename = "/dev/matrix_pixels";

#define Y 0x21DBE3
#define B 0x005000
#define R 0x005000

const pixel_t sprite[] = {
        {0}, {0}, {0}, {Y}, {Y}, {0}, {0}, {0},
        {0}, {0}, {Y}, {Y}, {Y}, {Y}, {0}, {0},
        {0}, {Y}, {Y}, {Y}, {Y}, {Y}, {Y}, {0},
        {0}, {Y}, {B}, {Y}, {Y}, {B}, {Y}, {0},
        {Y}, {Y}, {Y}, {Y}, {Y}, {Y}, {Y}, {Y},
        {0}, {Y}, {R}, {Y}, {Y}, {R}, {Y}, {0},
        {0}, {Y}, {Y}, {R}, {R}, {Y}, {Y}, {0},
        {0}, {0}, {Y}, {Y}, {Y}, {Y}, {0}, {0},
        {0}, {0}, {0}, {Y}, {Y}, {0}, {0}, {0},
};

void test_wight_gradient(FILE *fp, frame_buffer_t *frame){
    int colour = 0;
    for(int x = 0; x < 64; x++ ){
        frame->pixels[x].colours[RED]   = colour;
        frame->pixels[x].colours[GREEN] = colour;
        frame->pixels[x].colours[BLUE]  = colour;
        colour+=2;
    }
    fp = fopen(filename,"wb");
    fwrite(frame, sizeof(frame_buffer_t), 1 , fp);
    fclose(fp);
    sleep(1);
}

void test_random_values(FILE *fp, frame_buffer_t *frame){
    int times = 0;
    while (times < 120){
        for(int x = 0; x < 64; x++ ){
            frame->pixels[x].colours[RED]   = rand() % 255;
            frame->pixels[x].colours[GREEN] = rand() % 255;
            frame->pixels[x].colours[BLUE]  = rand() % 255;
        }
        times++;
        fp = fopen(filename,"wb");
        fwrite(frame, sizeof(frame_buffer_t), 1 , fp);
        fclose(fp);
        usleep(1000000/5); // 5 frames per second
    }
}

void clear_frame(frame_buffer_t *frame){
    for(int x = 0; x < 64; x++ ){
        frame->pixels[x].colours[RED]   = 0;
        frame->pixels[x].colours[GREEN] = 0;
        frame->pixels[x].colours[BLUE]  = 0;
    }
}

void flash(FILE *fp, frame_buffer_t *frame, int colour){
    clear_frame(frame);
    double brightness = 1;

    while (1){
        for(int x = 0; x < frame->len; x++ ){
            frame->pixels[x].colours[colour] = (int)brightness;
        }
        brightness *= 1.02;
        if(brightness > 255){
            for(int x = 0; x < frame->len; x++ ){
                frame->pixels[x].colours[colour] = (int)brightness;
            }
            fp = fopen(filename,"wb");
            fwrite(frame, sizeof(frame_buffer_t), 1 , fp);
            fclose(fp);
            break;
        }
        fp = fopen(filename,"wb");
        fwrite(frame, sizeof(frame_buffer_t), 1 , fp);
        fclose(fp);
        usleep(500);
    }
    while (1){
        for(int x = 0; x < frame->len; x++ ){
            frame->pixels[x].colours[colour] = (int)brightness;
        }
        brightness *= 0.98;
        if(brightness < 5){
            clear_frame(frame);
            fp = fopen(filename,"wb");
            fwrite(frame, sizeof(frame_buffer_t), 1 , fp);
            fclose(fp);
            break;
        }
        fp = fopen(filename,"wb");
        fwrite(frame, sizeof(frame_buffer_t), 1 , fp);
        fclose(fp);
        usleep(500);
    }
    sleep(1);
}

void test_random_values_part(FILE *fp, frame_buffer_t *frame){
    int times = 0;
    while (times < 120){
        clear_frame(frame);
        for(int x = 0; x < rand() % 10 + 10; x++ ){
            int ind = rand() % (frame->len-1);
            frame->pixels[ind].colours[RED]   = rand() % 255;
            frame->pixels[ind].colours[GREEN] = rand() % 255;
            frame->pixels[ind].colours[BLUE]  = rand() % 255;
        }
        times++;
        fp = fopen(filename,"wb");
        fwrite(frame, sizeof(frame_buffer_t), 1 , fp);
        fclose(fp);
        usleep(1000000/5); // 5 frames per second
    }
}



int main() {
    srand(time(NULL));

    FILE *fp = NULL;
    if( access( filename, F_OK ) == 0 ) {
        fp = fopen(filename,"wb");
    }
    if(!fp){
        printf("Failed to open %s file", filename);
        return 1;
    }

    frame_buffer_t frame;// = malloc(sizeof(frame_buffer_t));
    frame.len = 64;

    frame.pixels = sprite;
    fp = fopen(filename,"wb");
    fwrite(&frame, sizeof(frame_buffer_t), 1 , fp);
    fclose(fp);
    sleep(2);

    frame.pixels = malloc(sizeof(pixel_t) * frame.len);

    flash(fp, &frame, RED);
    test_random_values_part(fp, &frame);
    test_wight_gradient(fp, &frame);
    test_random_values(fp, &frame);

    free(frame.pixels);
    return 0;
}