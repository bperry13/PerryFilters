/**
 * write a program that applies a filter to an image. The image will be loaded
 * from a local BMP file, and then transformed using one of two image filters.
 * The two image filters blur filter and swiss cheese filter. The resulting file
 * will be saved back to a BMP file that can be viewed on the host system.
 *
 * Completion time: 20 hours
 *
 * @author BRETT PERRY, PROFESSOR ACUNA
 * @version 2.6.22
 */


////////////////////////////////////////////////////////////////////////////////
//INCLUDES
#include <stdio.h>

//TODO: finish me
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <pthread.h>

//UNCOMMENT BELOW LINE IF USING SER334 LIBRARY/OBJECT FOR BMP SUPPORT
//#include "BmpProcessor.h"

////////////////////////////////////////////////////////////////////////////////
//MACRO DEFINITIONS

//problem assumptions
#define BMP_HEADER_SIZE 14
#define BMP_DIB_HEADER_SIZE_BYTES 40
#define MAX_IMAGE_SIZE 4096

//TODO: finish me
//bmp compression methods
//none:
#define BI_RGB 0

////////////////////////////////////////////////////////////////////////////////
//DATA STRUCTURES

typedef struct BMP_Header {
    char signature[2];		//ID field
    int size;		//Size of the BMP file
    short reserved1;		//Application specific
    short reserved2;		//Application specific
    int offset_pixel_array;  //Offset where the pixel array (bitmap data) can be found
} BMP_Header;

//40 bytes information header
typedef struct DIB_Header {
    int size;
    int width;
    int height;
    short planes;
    short bitsPerPixel;
    int compression;
    int imgSizeBytes;
    int pixPerMeterHorizontal;
    int pixPerMeterVertcal;
    int colorUsed;
    int colorImp;
} DIB_Header;

typedef struct Image {
    struct Pixel** pArr;
    int width;
    int height;
} Image;

typedef struct Pixel {
    unsigned char r,g,b;
} Pixel;

Pixel rgb_index[MAX_IMAGE_SIZE][MAX_IMAGE_SIZE];
Pixel blur_rgb_index[MAX_IMAGE_SIZE][MAX_IMAGE_SIZE];

typedef enum pixel_colors {
    BLUE = 0,
    GREEN = 1,
    RED = 2
} PIXEL_COLORS;

//apply blur filter
unsigned char blur_filter(int i, int j, PIXEL_COLORS color);

//global data to hold height & width of the image data
int img_height = 0;
int img_width = 0;

//thread functions
void* l_upper_blur (void* unused)
{
    int i,j;
    int height = (img_height + 1) / 2;
    int width = (img_width + 1) / 2;
    for (j = 0;j < height;j++)
    {
        for (i = 0;i < width;i++)
        {
            blur_rgb_index[i][j].b = blur_filter(i, j, BLUE);
            blur_rgb_index[i][j].g = blur_filter(i, j, GREEN);
            blur_rgb_index[i][j].r = blur_filter(i, j, RED);
        }
    }
}

void* l_lower_blur (void* unused)
{
    int i,j;
    int height = img_height;
    int width = (img_width + 1) / 2;
    for (j = (img_height / 2); j < height; j++)
    {
        for (i = 0;i < width;i++)
        {
            blur_rgb_index[i][j].b = blur_filter(i, j, BLUE);
            blur_rgb_index[i][j].g = blur_filter(i, j, GREEN);
            blur_rgb_index[i][j].r = blur_filter(i, j, RED);
        }
    }
}

void* r_upper_blur (void* unused)
{
    int i,j;
    int height = (img_height + 1) / 2;
    int width = img_width;
    for (j = 0;j < height;j++)
    {
        for (i = (img_width / 2); i < width; i++)
        {
            blur_rgb_index[i][j].b = blur_filter(i, j, BLUE);
            blur_rgb_index[i][j].g = blur_filter(i, j, GREEN);
            blur_rgb_index[i][j].r = blur_filter(i, j, RED);
        }
    }
}

void* r_lower_blur (void* unused)
{
    int i,j;
    int height = img_height;
    int width = img_width;
    for (j = (img_height / 2); j < height; j++)
    {
        for (i = (img_width / 2); i < width; i++)
        {
            blur_rgb_index[i][j].b = blur_filter(i, j, BLUE);
            blur_rgb_index[i][j].g = blur_filter(i, j, GREEN);
            blur_rgb_index[i][j].r = blur_filter(i, j, RED);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//MAIN PROGRAM CODE


int main(int argc, char* argv[]) {

    //input parameters
    char *input = argv[optind];
    char *output = argv[optind + 1];

    BMP_Header bmp_header;
    DIB_Header dib_header;
    int i, j;

    //open input file
    FILE* file = fopen(input, "rb");
    if(file == NULL) {
        printf("ERROR: File does not exist in your directory");
        return EXIT_FAILURE;
    }

    //read bitmap bmp header
    fread(&bmp_header.signature, sizeof(char) * 2, 1, file);
    fread(&bmp_header.size, sizeof(int), 1, file);
    fread(&bmp_header.reserved1, sizeof(short), 1, file);
    fread(&bmp_header.reserved2, sizeof(short), 1, file);
    fread(&bmp_header.offset_pixel_array, sizeof(int), 1, file);

    //test to see bmp header in console
    printf("signature: %c%c\n", bmp_header.signature[0], bmp_header.signature[1]);
    printf("size: %d\n", bmp_header.size);
    printf("reserved1: %d\n", bmp_header.reserved1);
    printf("reserved2: %d\n", bmp_header.reserved2);
    printf("offset_pixel_array: %d\n", bmp_header.offset_pixel_array);

    //read bitmap dib header
    fread(&dib_header, sizeof(DIB_Header), 1, file);

    //test to see image width and height
    printf("Image size = %d x %d\n", dib_header.width, dib_header.height);


    img_height = dib_header.height;
    img_width = dib_header.width;

    //read image data
    fseek(file, bmp_header.offset_pixel_array, SEEK_SET);

    //
    for (j=0; j < dib_header.height; j++) {
        for (i=0; i < dib_header.width; i++) {
            fread(&rgb_index[i][j].b, sizeof(unsigned char), 1, file);
            fread(&rgb_index[i][j].g, sizeof(unsigned char), 1, file);
            fread(&rgb_index[i][j].r, sizeof(unsigned char), 1, file);
        }
    }
    fclose(file);

    //create threads to process the image
    pthread_t l_upper_tid,l_lower_tid,r_upper_tid,r_lower_tid;

    pthread_create(&l_upper_tid, NULL, &l_upper_blur, NULL);
    pthread_join(l_upper_tid, NULL);

    pthread_create(&l_lower_tid, NULL, &l_lower_blur, NULL);
    pthread_join(l_lower_tid, NULL);

    pthread_create(&r_upper_tid, NULL, &r_upper_blur, NULL);
    pthread_join(r_upper_tid, NULL);

    pthread_create(&r_lower_tid, NULL, &r_lower_blur, NULL);
    pthread_join(r_lower_tid, NULL);

    //open output file
    FILE* output_file = fopen(output, "wb");
    if (output_file == NULL) {
        printf("ERROR: Output file not created.");
        return EXIT_FAILURE;
    }

    //write bmp header to output file
    fwrite(&bmp_header.signature, sizeof(char) * 2, 1, output_file);
    fwrite(&bmp_header.size, sizeof(int), 1, output_file);
    fwrite(&bmp_header.reserved1, sizeof(short), 1, output_file);
    fwrite(&bmp_header.reserved2, sizeof(short), 1, output_file);
    fwrite(&bmp_header.offset_pixel_array, sizeof(int), 1, output_file);

    //write dib header to output file
    fwrite(&dib_header, sizeof(DIB_Header), 1, output_file);

    //write the filtered pixels to the array
    for (j=0; j < dib_header.height; j++) {
        for (i=0; i < dib_header.width; i++) {
            fwrite(&blur_rgb_index[i][j].b, sizeof(unsigned char), 1, output_file);
            fwrite(&blur_rgb_index[i][j].g, sizeof(unsigned char), 1, output_file);
            fwrite(&blur_rgb_index[i][j].r, sizeof(unsigned char), 1, output_file);
        }
    }
    fclose(output_file);
    printf("Success! Your filtered image has been saved to the root folder.\n");
}

unsigned char blur_filter(int i, int j, PIXEL_COLORS color)
{
    int q, r;
    switch(color)
    {
        case BLUE:
        {
            int sum = 0;
            int count = 0;
            int avg = 0;
            for(q = i-1; q <= i+1 ; q++)
            {
                if(q < 0 || q >= img_height)
                {
                    continue;
                }
                for(r = j-1; r <= j+1; r++)
                {
                    if(r < 0 || r >= img_width)
                    {
                        continue;
                    }
                    sum = sum + rgb_index[q][r].b;
                    count++;
                }
            }
            if(count)
            {
                avg = sum/count;
            }
            return avg;
        }
            break;

        case GREEN:
        {
            int sum = 0;
            int count = 0;
            int avg = 0;
            for(q = i-1; q <= i+1 ; q++)
            {
                if(q < 0 || q >= img_height)
                    continue;
                for(r = j-1; r<= j+1; r++)
                {
                    if(r < 0 || r >= img_width)
                    {
                        continue;
                    }
                    sum = sum + rgb_index[q][r].g;
                    count++;
                }
            }
            if(count)
            {
                avg = sum/count;
            }
            return avg;
        }
            break;

        case RED:
        {
            int sum = 0;
            int count = 0;
            int avg = 0;
            for(q = i-1; q <= i+1 ; q++)
            {
                if(q < 0 || q >= img_height)
                {
                    continue;
                }
                for(r = j-1; r<= j+1; r++)
                {
                    if(r < 0 || r >= img_width)
                    {
                        continue;
                    }
                    sum = sum + rgb_index[q][r].r;
                    count++;
                }
            }
            if(count)
            {
                avg = sum/count;
            }
            return avg;
        }
            break;

        default:
        {
            //do nothing
        }
    }
}