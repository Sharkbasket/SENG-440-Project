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

// Define fixed point constant coefficients (coded in 16 bits)
#define SCALE_FACTOR 14
#define DESCALE(x) (x >> SCALE_FACTOR)

#define RGB_TO_YCC_COEFF_1_1 ((uint16_t)0x1073) // 0.257 * 2^14
#define RGB_TO_YCC_COEFF_1_2 ((uint16_t)0x2042) // 0.504 * 2^14
#define RGB_TO_YCC_COEFF_1_3 ((uint16_t)0x0646) // 0.098 * 2^14
#define RGB_TO_YCC_COEFF_2_1 ((uint16_t)0x0979) // 0.148 * 2^14
#define RGB_TO_YCC_COEFF_2_2 ((uint16_t)0x12A0) // 0.291 * 2^14
#define RGB_TO_YCC_COEFF_2_3 ((uint16_t)0x1C19) // 0.439 * 2^14
#define RGB_TO_YCC_COEFF_3_1 ((uint16_t)0x1C19) // 0.439 * 2^14
#define RGB_TO_YCC_COEFF_3_2 ((uint16_t)0x178D) // 0.368 * 2^14
#define RGB_TO_YCC_COEFF_3_3 ((uint16_t)0x048B) // 0.071 * 2^14

#define YCC_TO_RGB_COEFF_1_1 ((uint16_t)0x4A7F) // 1.164 * 2^14
#define YCC_TO_RGB_COEFF_1_2 ((uint16_t)0x0000) // Unused
#define YCC_TO_RGB_COEFF_1_3 ((uint16_t)0x6625) // 1.596 * 2^14
#define YCC_TO_RGB_COEFF_2_1 ((uint16_t)0x4A7F) // 1.164 * 2^14
#define YCC_TO_RGB_COEFF_2_2 ((uint16_t)0x1906) // 0.391 * 2^14
#define YCC_TO_RGB_COEFF_2_3 ((uint16_t)0x3408) // 0.813 * 2^14
#define YCC_TO_RGB_COEFF_3_1 ((uint16_t)0x4A7F) // 1.164 * 2^14
#define YCC_TO_RGB_COEFF_3_2 ((uint16_t)0x8127) // 2.018 * 2^14
#define YCC_TO_RGB_COEFF_3_3 ((uint16_t)0x0000) // Unused

int main() {
    // Array index/loop counters
    uint32_t i = 0;     // Range: [0, N_PIXELS*3)
    uint16_t row = 0;   // Range: [0, HEIGHT)
    uint16_t col = 0;   // Range: [0, WIDTH)
    uint16_t c_row = 0; // Range: [0, HEIGHT/2)
    uint16_t c_col = 0; // Range: [0, WIDTH/2)
    
    // Dynamically allocate arrays for holding pixel data
    int16_t* r[HEIGHT];
    int16_t* g[HEIGHT];
    int16_t* b[HEIGHT];
    int16_t* y[HEIGHT];
    int16_t* cb[HEIGHT / 2];
    int16_t* cr[HEIGHT / 2];
    for (row = 0; row < HEIGHT; row++) {
        r[row] = (int16_t*)malloc(WIDTH * sizeof(int16_t));
        g[row] = (int16_t*)malloc(WIDTH * sizeof(int16_t));
        b[row] = (int16_t*)malloc(WIDTH * sizeof(int16_t));
        y[row] = (int16_t*)malloc(WIDTH * sizeof(int16_t));
        if (row < (HEIGHT / 2)) {
            cb[row] = (int16_t*)malloc(WIDTH / 2 * sizeof(int16_t));
            cr[row] = (int16_t*)malloc(WIDTH / 2 * sizeof(int16_t));
        }
    }
    
    // Extract pixel data from input file
    int X = WIDTH;
    int Y = HEIGHT;
    int N = N_CHANNELS;
    uint8_t* px_data = stbi_load(INPUT_FILE, &X, &Y, &N, STBI_rgb);
    
    printf("Pixel data extracted\n");
    
    // Parse the extracted data into separate R, G, and B arrays
    for (row = 0, i = 0; row < HEIGHT; row++) {
        for (col = 0; col < WIDTH; col++) {
            r[row][col] = (int16_t)px_data[i++];
            g[row][col] = (int16_t)px_data[i++];
            b[row][col] = (int16_t)px_data[i++];
        }
    }
    
    printf("RGB arrays populated\n");
    
    // Calculate YCC (4:2:0 chroma subsampling)
    for (row = 0, c_row = 0; row < HEIGHT; row++) {
        c_row = row / 2;
        for (col = 0, c_col = 0; col < WIDTH; col++) {
            // Calculate luma with full resolution (no downsampling)
            y[row][col] = 16
                        + DESCALE(RGB_TO_YCC_COEFF_1_1 * r[row][col])
                        + DESCALE(RGB_TO_YCC_COEFF_1_2 * g[row][col])
                        + DESCALE(RGB_TO_YCC_COEFF_1_3 * b[row][col]);
            
            // Only sample chroma on even numbered rows/columns
            if (!((row % 2) || (col % 2))) {
                c_col = col / 2;
                cb[c_row][c_col] =
                    128
                  - DESCALE(RGB_TO_YCC_COEFF_2_1 * r[row][col])
                  - DESCALE(RGB_TO_YCC_COEFF_2_2 * g[row][col])
                  + DESCALE(RGB_TO_YCC_COEFF_2_3 * b[row][col]);
                cr[c_row][c_col] =
                    128
                  + DESCALE(RGB_TO_YCC_COEFF_3_1 * r[row][col])
                  - DESCALE(RGB_TO_YCC_COEFF_3_2 * g[row][col])
                  - DESCALE(RGB_TO_YCC_COEFF_3_3 * b[row][col]);
            }
        }
    }
    
    printf("RGB to YCC conversion completed\n");
    
    /*** STARTING POINT FOR OPTIMIZATION ***/
    
    // Perform quick floating-point inverse conversion for testing
    // Upsampling of cb/cr by replication
    for (row = 0, c_row = 0; row < HEIGHT; row++) {
        c_row = row / 2;
        for (col = 0, c_col = 0; col < WIDTH; col++) {
            c_col = col / 2;
            r[row][col] =
                DESCALE(YCC_TO_RGB_COEFF_1_1 * (y[row][col] - 16))
              + DESCALE(YCC_TO_RGB_COEFF_1_3 * (cr[c_row][c_col] - 128));
            if (r[row][col] < 0) {
                r[row][col] = 0;
            } else if (r[row][col] > 0xFF) {
                r[row][col] = 0xFF;
            }
            g[row][col] = 
                DESCALE(YCC_TO_RGB_COEFF_2_1 * (y[row][col] - 16))
              - DESCALE(YCC_TO_RGB_COEFF_2_2 * (cb[c_row][c_col] - 128))
              - DESCALE(YCC_TO_RGB_COEFF_2_3 * (cr[c_row][c_col] - 128));
            if (g[row][col] < 0) {
                g[row][col] = 0;
            } else if (g[row][col] > 0xFF) {
                g[row][col] = 0xFF;
            }
            b[row][col] =
                DESCALE(YCC_TO_RGB_COEFF_3_1 * (y[row][col] - 16))
              + DESCALE(YCC_TO_RGB_COEFF_3_2 * (cb[c_row][c_col] - 128));
            if (b[row][col] < 0) {
                b[row][col] = 0;
            } else if (b[row][col] > 0xFF) {
                b[row][col] = 0xFF;
            }
        }
    }
    
    /*** ENDING POINT FOR OPTIMIZATION ***/
    
    printf("YCC to RGB conversion completed\n");
    
    // Interleave the r, g, and b components back into the pixel data array
    for (row = 0, i = 0; row < HEIGHT; row++) {
        for (col = 0; col < WIDTH; col++) {
            px_data[i++] = (uint8_t)r[row][col];
            px_data[i++] = (uint8_t)g[row][col];
            px_data[i++] = (uint8_t)b[row][col];
        }
    }
    
    printf("RGB arrays interleaved into one array for writing output\n");
    
    stbi_write_png(OUTPUT_FILE, WIDTH, HEIGHT, STBI_rgb, px_data,
                   (WIDTH * STBI_rgb));
    
    printf("Output file written\n");
    
    return 0;
}
