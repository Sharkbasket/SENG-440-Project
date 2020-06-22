#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

#define FILE_NAME "1024x768-Jerry-TestPattern.png"
#define WIDTH 1024
#define HEIGHT 768
#define CHANNELS 4 // The source image file has an additional alpha channel

int main()
{
    int x = WIDTH;
    int y = HEIGHT;
    int n = CHANNELS;
    // Ignore the alpha channel and just return RGB pixel data
    unsigned char* pixel_data = stbi_load(FILE_NAME, &x, &y, &n, STBI_rgb);
    
    // This is just a test to see some of the pixel data printed out
    for (int i = 0; i < 1200; i++)
    {
        if ((i % 3) == 0)
        {
            printf(" ");
            
            if ((i % 24) == 0)
            {
                printf("\n");
            }
        }
        printf("%02X", *(pixel_data++));
    }
    
    return 0;
}
