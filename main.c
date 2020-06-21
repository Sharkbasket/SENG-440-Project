#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

#define FILE_NAME "1024x768-Jerry-TestPattern.png"
#define WIDTH 1024
#define HEIGHT 768
#define CHANNELS 4

int main()
{
    int x = WIDTH;
    int y = HEIGHT;
    int n = CHANNELS;
    unsigned char* pixel_data = stbi_load(FILE_NAME, &x, &y, &n, STBI_rgb);
    
    for (int i = 0; i < 1024; i++)
    {
        if ((i % 3) == 0)
        {
            printf("\n");
        }
        printf("%2x ", *(pixel_data++));
    }
    
    return 0;
}
