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
#define N_CHANNELS 4 // The source image file has an additional alpha channel

int main()
{
    int x = WIDTH;
    int y = HEIGHT;
    int n = N_CHANNELS;
    
    unsigned char* r_px = (unsigned char*)malloc(N_PIXELS);
    unsigned char* g_px = (unsigned char*)malloc(N_PIXELS);
    unsigned char* b_px = (unsigned char*)malloc(N_PIXELS);
    
    float* y_px = (float*)malloc(N_PIXELS * sizeof(float));
    float* cb_px_ds = (float*)malloc((N_PIXELS / 4) * sizeof(float));
    float* cr_px_ds = (float*)malloc((N_PIXELS / 4) * sizeof(float));
    float* cb_px_us = (float*)malloc(N_PIXELS * sizeof(float));
    float* cr_px_us = (float*)malloc(N_PIXELS * sizeof(float));
    
    // Ignore the alpha channel and just return RGB pixel data
    unsigned char* px_data = stbi_load(INPUT_FILE, &x, &y, &n, STBI_rgb);
    
    // Parse red, green, and blue into seperate arrays
    for (int i = 0; i < N_PIXELS; i++)
    {
        r_px[i] = px_data[3 * i];
        g_px[i] = px_data[(3 * i) + 1];
        b_px[i] = px_data[(3 * i) + 2];
    }
    
    // Calculate luma (no downsampling)
    for (int i = 0; i < N_PIXELS; i++)
    {
        y_px[i] = 16 + (0.257 * r_px[i]) + (0.504 * g_px[i])
                  + (0.098 * b_px[i]);
    }
    
    // Calculate chroma (4:2:0 downsampling)
    for (int i = 0, j = 0; i < N_PIXELS; i++)
    {
        // Only sample on even lines
        if ((i % (WIDTH * 2)) < WIDTH)
        {
            // Only sample every second pixel
            if ((i % 2) == 0)
            {
                cb_px_ds[j] = 128 - (0.148 * r_px[i]) - (0.291 * g_px[i])
                           + (0.439 * b_px[i]);
                cr_px_ds[j] = 128 + (0.439 * r_px[i]) - (0.368 * g_px[i])
                           - (0.071 * b_px[i]);
                j++;
            }
        }
    }
    
    // Upsample the chroma by replication
    for (int j = 0; j < (N_PIXELS / 4); j++)
    {
        int LINE_OFFSET = (j / (WIDTH / 2)) * (WIDTH * 2);
        int ROW_OFFSET = 2 * (j % (WIDTH / 2));
        int BASE = LINE_OFFSET + ROW_OFFSET;
        
        cb_px_us[BASE] = cb_px_ds[j];
        cb_px_us[BASE + 1] = cb_px_ds[j];
        cb_px_us[BASE + WIDTH] = cb_px_ds[j];
        cb_px_us[BASE + WIDTH + 1] = cb_px_ds[j];
        
        cr_px_us[BASE] = cr_px_ds[j];
        cr_px_us[BASE + 1] = cr_px_ds[j];
        cr_px_us[BASE + WIDTH] = cr_px_ds[j];
        cr_px_us[BASE + WIDTH + 1] = cr_px_ds[j];
    }
    
    // Perform quick inverse conversion to RGB to see if this works
    for (int i = 0; i < N_PIXELS; i++)
    {
        r_px[i] = (unsigned char)((1.164 * (y_px[i] - 16))
                                  + (1.596 * (cr_px_us[i] - 128)));
        g_px[i] = (unsigned char)((1.164 * (y_px[i] - 16))
                                  - (0.813 * (cr_px_us[i] - 128))
                                  - (0.391 * (cb_px_us[i] - 128)));
        b_px[i] = (unsigned char)((1.164 * (y_px[i] - 16))
                                  + (2.018 * (cb_px_us[i] - 128)));
    }
    
    // Interleave the r, g, and b components back into the pixel data array
    for (int i = 0; i < N_PIXELS; i++)
    {
        px_data[3 * i] = r_px[i];
        px_data[(3 * i) + 1] = g_px[i];
        px_data[(3 * i) + 2] = b_px[i];
    }
    
    // Write the output file
    stbi_write_png(OUTPUT_FILE, WIDTH, HEIGHT, STBI_rgb, px_data,
                   (WIDTH * STBI_rgb));
    
    free(r_px);
    free(g_px);
    free(b_px);
    free(y_px);
    free(cb_px_ds);
    free(cr_px_ds);
    free(cb_px_us);
    free(cr_px_us);
    
    return 0;
}

// This is just a test to dump the pixel data to the console
void dump_pixel_data(unsigned char* px_data)
{
    for (int i = 0; i < (N_PIXELS * STBI_rgb); i++)
    {
        if ((i % 3) == 0)
        {
            printf(" ");
            if ((i % 24) == 0)
            {
                printf("\n");
            }
        }
        printf("%02X", *(px_data++));
    }
    printf("\nTotal number of RGB bytes: %d\n", N_PIXELS * STBI_rgb);
}
