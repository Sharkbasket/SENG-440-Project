/* 
 * University of Victoria
 * Faculty of Engineering
 * SENG 440 Embedded Systems 
 *
 * Optimization of YCC-to-RGB colour space conversion
 *
 * Authors: Shaun McGuigan and Andrew Friesen
 *
 * Last modified: 2020/08/07
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define INPUT_FILE "1024x768-Jerry-TestPattern.png"
#define OUTPUT_FILE "test_output.png"
#define WIDTH 1024
#define HEIGHT 768
#define N_PIXELS (WIDTH * HEIGHT)
#define N_CHANNELS 4 // Number of channels in the input file

int main() {
    // Array index/loop counters
    uint32_t i = 0;     // Range: 0 to (N_PIXELS * 3)
    uint16_t row = 0;   // Range: 0 to HEIGHT
    uint16_t col = 0;   // Range: 0 to WIDTH
    uint16_t c_row = 0; // Range: 0 to (HEIGHT / 2)
    uint16_t c_col = 0; // Range: 0 to (WIDTH / 2)
    
    // Dynamically allocate arrays for holding pixel data
    float* r[HEIGHT];
    float* g[HEIGHT];
    float* b[HEIGHT];
    float* y[HEIGHT];
    float* cb[HEIGHT / 2];
    float* cr[HEIGHT / 2];
    for (row = 0; row < HEIGHT; row++) {
        r[row] = (float*)malloc(WIDTH * sizeof(float));
        g[row] = (float*)malloc(WIDTH * sizeof(float));
        b[row] = (float*)malloc(WIDTH * sizeof(float));
        y[row] = (float*)malloc(WIDTH * sizeof(float));
        if (row < (HEIGHT / 2)) {
            cb[row] = (float*)malloc(WIDTH / 2 * sizeof(float));
            cr[row] = (float*)malloc(WIDTH / 2 * sizeof(float));
        }
    }
    
    // Extract pixel data from input file
    int X = WIDTH;
    int Y = HEIGHT;
    int N = N_CHANNELS;
    uint8_t* px_data = stbi_load(INPUT_FILE, &X, &Y, &N, STBI_rgb);
    
    printf("Pixel data extracted\n");
    
    // Parse the extracted data into separate R, G, and B arrays
    i = 0;
    for (row = 0; row < HEIGHT; row++) {
        for (col = 0; col < WIDTH; col++) {
            r[row][col] = px_data[i++];
            g[row][col] = px_data[i++];
            b[row][col] = px_data[i++];
        }
    }
    
    printf("RGB arrays populated\n");
    
    // Calculate YCC (4:2:0 chroma subsampling)
    for (row = 0; row < HEIGHT; row++) {
        c_row = row / 2;
        for (col = 0; col < WIDTH; col++) {
            // Calculate luma with full resolution (no downsampling)
            y[row][col] = (16 + (0.257 * r[row][col])
                              + (0.504 * g[row][col])
                              + (0.098 * b[row][col]));
            
            // Only sample chroma on even numbered rows/columns
            if (!((row % 2) || (col % 2))) {
                c_col = col / 2;
                cb[c_row][c_col] = (128 - (0.148 * r[row][col])
                                        - (0.291 * g[row][col])
                                        + (0.439 * b[row][col]));
                cr[c_row][c_col] = (128 + (0.439 * r[row][col])
                                        - (0.368 * g[row][col])
                                        - (0.071 * b[row][col]));
            }
        }
    }
    
    printf("RGB to YCC conversion completed\n");
    
    /*** STARTING POINT FOR OPTIMIZATION ***/
    
    // Perform quick floating-point inverse conversion for testing
    // Upsampling of cb/cr by replication
    c_row = 0;
    c_col = 0;
    for (row = 0; row < HEIGHT; row++) {
        c_row = row / 2;
        for (col = 0; col < WIDTH; col++) {
            c_col = col / 2;
            r[row][col] = (1.164 * (y[row][col] - 16))
                        + (1.596 * (cr[c_row][c_col] - 128));
            g[row][col] = (1.164 * (y[row][col] - 16))
                        - (0.813 * (cr[c_row][c_col] - 128))
                        - (0.391 * (cb[c_row][c_col] - 128));
            b[row][col] = (1.164 * (y[row][col] - 16))
                        + (2.018 * (cb[c_row][c_col] - 128));
        }
    }
    
    /*** ENDING POINT FOR OPTIMIZATION ***/
    
    printf("YCC to RGB conversion completed\n");
    
    // Interleave the r, g, and b components back into the pixel data array
    i = 0;
    for (row = 0; row < HEIGHT; row++) {
        for (col = 0; col < WIDTH; col++) {
            px_data[i++] = r[row][col];
            px_data[i++] = g[row][col];
            px_data[i++] = b[row][col];
        }
    }
    
    printf("RGB arrays interleaved into one array for writing output\n");
    
    stbi_write_png(OUTPUT_FILE, WIDTH, HEIGHT, STBI_rgb, px_data,
                   (WIDTH * STBI_rgb));
    
    printf("Output file written\n");
    
    return 0;
}
