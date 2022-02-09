/**
 * write a program that applies a filter to an image. The image will be loaded
 * from a local BMP file, and then transformed using one of two image filters.
 * The two image filters blur filter and swiss cheese filter. The resulting file
 * will be saved back to a BMP file that can be viewed on the host system.
 *
 * Completion time: 24 hours
 *
 * @author BRETT PERRY, PROFESSOR ACUNA
 * @version 2.7.22
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
#define THREAD_COUNT 4

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

typedef struct Instruction {
    Image *img;
    Pixel** filtered; // write to this with filter applied
    // int first_col;
    // int last_col;
    int start;
    int finish;
} Instruction;

typedef struct Circle {
    int x;
    int y;
    int radius;
} Circle;


////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS

/**
 * Read Pixels from BMP file based on width and height.
 *
 * @param  file: A pointer to the file being read
 * @param  pArr: Pixel array to store the pixels being read
 * @param  width: Width of the pixel array of this image
 * @param  height: Height of the pixel array of this image
 */
void readPixelsBMP(FILE* file, struct Pixel** pArr, int width, int height) {
    int pitch, padding;
    int x, y;

    //get padding
    pitch = width*3;
    if (pitch % 4 != 0) {
        pitch += 4 - (pitch % 4);
    }
    padding = pitch - (width * 3);

    //iterate scanlines
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            fread(&pArr[y][x].b, sizeof(unsigned char), 1, file);
            fread(&pArr[y][x].g, sizeof(unsigned char), 1, file);
            fread(&pArr[y][x].r, sizeof(unsigned char), 1, file);
        }

        // skip padding
        fseek(file, padding, SEEK_CUR);
    }
}

/**
 * Write Pixels from BMP file based on width and height.
 *
 * @param  file: A pointer to the file being read or written
 * @param  pArr: Pixel array of the image to write to the file
 * @param  width: Width of the pixel array of this image
 * @param  height: Height of the pixel array of this image
 */
void writePixelsBMP(FILE* file, struct Pixel** pArr, int width, int height) {
    int pitch, padding;
    int x, y, p;

    //get padding
    pitch = width*3;
    if (pitch % 4 != 0) {
        pitch += 4 - (pitch % 4);
    }
    padding = pitch - (width * 3);

    //iterate scanlines
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            fwrite(&pArr[y][x].b, sizeof(unsigned char), 1, file);
            fwrite(&pArr[y][x].g, sizeof(unsigned char), 1, file);
            fwrite(&pArr[y][x].r, sizeof(unsigned char), 1, file);
        }

        // skip padding write for loop to add chars
        for (p = 0; p < padding; p++) {
            fwrite(" ", sizeof(unsigned char), 1, file);
        }
    }
}

/* Creates a new image and returns it.
*
 * @param  pArr: Pixel array of this image.
 * @param  width: Width of this image.
 * @param  height: Height of this image.
 * @return A pointer to a new image.
*/
Image* image_create(struct Pixel** pArr, int width, int height) {

    //allocate mem
    Image* img = malloc(sizeof(struct Image));
    //assign height
    img->height = height;
    //assign width
    img->width = width;
    //assign pixel array
    img->pArr = pArr;

    //return new image
    return img;
}


void *blur_runner(void *args) {

    Instruction *params = (Instruction *) args;
    int red, blue, green, counter;
    int height, width;

    red = 0;
    blue = 0;
    green = 0;
    counter = 0;
    height = params->img->height;
    width = params->img->width;
    Pixel** filtered = params->filtered;

    //box blur algo
    for (int x = params->start; x < params->finish; x++) {
        for (int y = 0; y < height; y++) {

            red = 0;
            blue = 0;
            green = 0;
            counter = 0;

            // top left corner
            if (x == 0 && y == 0) {
                red += params->img->pArr[y][x].r;
                blue += params->img->pArr[y][x].b;
                green += params->img->pArr[y][x].g;
                red += params->img->pArr[y][x + 1].r;
                blue += params->img->pArr[y][x + 1].b;
                green += params->img->pArr[y][x + 1].g;
                red += params->img->pArr[y + 1][x].r;
                blue += params->img->pArr[y + 1][x].b;
                green += params->img->pArr[y + 1][x].g;
                red += params->img->pArr[y + 1][x + 1].r;
                blue += params->img->pArr[y + 1][x + 1].b;
                green += params->img->pArr[y + 1][x + 1].g;
                counter = 4;
            } else if (y == 0 && x == width - 1) { // top right
                red += params->img->pArr[y][x].r;
                blue += params->img->pArr[y][x].b;
                green += params->img->pArr[y][x].g;
                red += params->img->pArr[y][x - 1].r;
                blue += params->img->pArr[y][x - 1].b;
                green += params->img->pArr[y][x - 1].g;
                red += params->img->pArr[y + 1][x].r;
                blue += params->img->pArr[y + 1][x].b;
                green += params->img->pArr[y + 1][x].g;
                red += params->img->pArr[y + 1][x - 1].r;
                blue += params->img->pArr[y + 1][x - 1].b;
                green += params->img->pArr[y + 1][x - 1].g;
                counter = 4;
            } else if (y == height - 1 && x == 0) { // bottom left
                red += params->img->pArr[y][x].r;
                blue += params->img->pArr[y][x].b;
                green += params->img->pArr[y][x].g;
                red += params->img->pArr[y][x + 1].r;
                blue += params->img->pArr[y][x + 1].b;
                green += params->img->pArr[y][x + 1].g;
                red += params->img->pArr[y - 1][x].r;
                blue += params->img->pArr[y - 1][x].b;
                green += params->img->pArr[y - 1][x].g;
                red += params->img->pArr[y - 1][x + 1].r;
                blue += params->img->pArr[y - 1][x + 1].b;
                green += params->img->pArr[y - 1][x + 1].g;
                counter = 4;
            } else if (y == height - 1 && x == width - 1) { // bottom right
                red += params->img->pArr[y][x].r;
                blue += params->img->pArr[y][x].b;
                green += params->img->pArr[y][x].g;
                red += params->img->pArr[y][x - 1].r;
                blue += params->img->pArr[y][x - 1].b;
                green += params->img->pArr[y][x - 1].g;
                red += params->img->pArr[y - 1][x].r;
                blue += params->img->pArr[y - 1][x].b;
                green += params->img->pArr[y - 1][x].g;
                red += params->img->pArr[y - 1][x - 1].r;
                blue += params->img->pArr[y - 1][x - 1].b;
                green += params->img->pArr[y - 1][x - 1].g;
                counter = 4;
            } else if (y == 0) { // top row
                red += params->img->pArr[y][x].r;
                blue += params->img->pArr[y][x].b;
                green += params->img->pArr[y][x].g;
                red += params->img->pArr[y][x - 1].r;
                blue += params->img->pArr[y][x - 1].b;
                green += params->img->pArr[y][x - 1].g;
                red += params->img->pArr[y][x + 1].r;
                blue += params->img->pArr[y][x + 1].b;
                green += params->img->pArr[y][x + 1].g;
                red += params->img->pArr[y + 1][x].r;
                blue += params->img->pArr[y + 1][x].b;
                green += params->img->pArr[y + 1][x].g;
                red += params->img->pArr[y + 1][x - 1].r;
                blue += params->img->pArr[y + 1][x - 1].b;
                green += params->img->pArr[y + 1][x - 1].g;
                red += params->img->pArr[y + 1][x + 1].r;
                blue += params->img->pArr[y + 1][x + 1].b;
                green += params->img->pArr[y + 1][x + 1].g;
                counter = 6;
            } else if (y == height - 1) { // bottom row
                red += params->img->pArr[y][x].r;
                blue += params->img->pArr[y][x].b;
                green += params->img->pArr[y][x].g;
                red += params->img->pArr[y][x - 1].r;
                blue += params->img->pArr[y][x - 1].b;
                green += params->img->pArr[y][x - 1].g;
                red += params->img->pArr[y][x + 1].r;
                blue += params->img->pArr[y][x + 1].b;
                green += params->img->pArr[y][x + 1].g;
                red += params->img->pArr[y - 1][x].r;
                blue += params->img->pArr[y - 1][x].b;
                green += params->img->pArr[y - 1][x].g;
                red += params->img->pArr[y - 1][x - 1].r;
                blue += params->img->pArr[y - 1][x - 1].b;
                green += params->img->pArr[y - 1][x - 1].g;
                red += params->img->pArr[y - 1][x + 1].r;
                blue += params->img->pArr[y - 1][x + 1].b;
                green += params->img->pArr[y - 1][x + 1].g;
                counter = 6;
            } else if (x == 0) { // left column
                red += params->img->pArr[y][x].r;
                blue += params->img->pArr[y][x].b;
                green += params->img->pArr[y][x].g;
                red += params->img->pArr[y][x + 1].r;
                blue += params->img->pArr[y][x + 1].b;
                green += params->img->pArr[y][x + 1].g;
                red += params->img->pArr[y - 1][x].r;
                blue += params->img->pArr[y - 1][x].b;
                green += params->img->pArr[y - 1][x].g;
                red += params->img->pArr[y - 1][x + 1].r;
                blue += params->img->pArr[y - 1][x + 1].b;
                green += params->img->pArr[y - 1][x + 1].g;
                red += params->img->pArr[y + 1][x].r;
                blue += params->img->pArr[y + 1][x].b;
                green += params->img->pArr[y + 1][x].g;
                red += params->img->pArr[y + 1][x + 1].r;
                blue += params->img->pArr[y + 1][x + 1].b;
                green += params->img->pArr[y + 1][x + 1].g;
                counter = 6;
            } else if (x == width - 1) { // right column
                red += params->img->pArr[y][x].r;
                blue += params->img->pArr[y][x].b;
                green += params->img->pArr[y][x].g;
                red += params->img->pArr[y][x - 1].r;
                blue += params->img->pArr[y][x - 1].b;
                green += params->img->pArr[y][x - 1].g;
                red += params->img->pArr[y - 1][x].r;
                blue += params->img->pArr[y - 1][x].b;
                green += params->img->pArr[y - 1][x].g;
                red += params->img->pArr[y - 1][x - 1].r;
                blue += params->img->pArr[y - 1][x - 1].b;
                green += params->img->pArr[y - 1][x - 1].g;
                red += params->img->pArr[y + 1][x].r;
                blue += params->img->pArr[y + 1][x].b;
                green += params->img->pArr[y + 1][x].g;
                red += params->img->pArr[y + 1][x - 1].r;
                blue += params->img->pArr[y + 1][x - 1].b;
                green += params->img->pArr[y + 1][x - 1].g;
                counter = 6;
            } else { // interior
                red += params->img->pArr[y][x].r;
                blue += params->img->pArr[y][x].b;
                green += params->img->pArr[y][x].g;
                red += params->img->pArr[y][x - 1].r;
                blue += params->img->pArr[y][x - 1].b;
                green += params->img->pArr[y][x - 1].g;
                red += params->img->pArr[y][x + 1].r;
                blue += params->img->pArr[y][x + 1].b;
                green += params->img->pArr[y][x + 1].g;
                red += params->img->pArr[y - 1][x].r;
                blue += params->img->pArr[y - 1][x].b;
                green += params->img->pArr[y - 1][x].g;
                red += params->img->pArr[y - 1][x - 1].r;
                blue += params->img->pArr[y - 1][x - 1].b;
                green += params->img->pArr[y - 1][x - 1].g;
                red += params->img->pArr[y - 1][x + 1].r;
                blue += params->img->pArr[y - 1][x + 1].b;
                green += params->img->pArr[y - 1][x + 1].g;
                red += params->img->pArr[y + 1][x].r;
                blue += params->img->pArr[y + 1][x].b;
                green += params->img->pArr[y + 1][x].g;
                red += params->img->pArr[y + 1][x - 1].r;
                blue += params->img->pArr[y + 1][x - 1].b;
                green += params->img->pArr[y + 1][x - 1].g;
                red += params->img->pArr[y + 1][x + 1].r;
                blue += params->img->pArr[y + 1][x + 1].b;
                green += params->img->pArr[y + 1][x + 1].g;
                counter = 9;
            }

            //write average value of pixels
            filtered[y][x].r = red / counter;
            filtered[y][x].g = green / counter;
            filtered[y][x].b = blue / counter;
        }
    }

    pthread_exit(NULL);
}

/**
 * Box Blur Filter
 *
 * @param img: image for pixel array
 */
void blur_filter(Image* img) {

    // create new pixel array to write to
    Pixel** filtered = malloc(sizeof(Pixel*) * img->height);
    for (int i = 0; i < img->height; i++) {
        filtered[i] = malloc(sizeof(Pixel) * img->width);
    }

    //create an array of threads
    pthread_t thread[THREAD_COUNT];
    //create an array of params
    Instruction params[THREAD_COUNT];

    //create the instructions
    for (int i = 0; i < THREAD_COUNT; i++) {
        params[i].img = img;
        params[i].filtered = filtered;
        params[i].start = (img->width / THREAD_COUNT) * i;
        if (i < THREAD_COUNT - 1) params[i].finish = (i + 1) * (img->width / THREAD_COUNT);
        else params[i].finish = img->width;
    }

    //create the threads
    for (int n = 0; n < THREAD_COUNT; n++) {
        pthread_create(&thread[n], NULL, blur_runner, &params[n]);
    }

    //join the threads
    for (int n = 0; n < THREAD_COUNT; n++) {
        pthread_join(thread[n], NULL);
    }

    //free the old array
    Pixel** temp = img->pArr;
    img->pArr = filtered;
    for (int i = 0; i < img->height; i++) {
        free(temp[i]);
    }
    free(temp);
}

/**
* Runner for Swiss Cheese Filter multi-thread
*
* @param args: image for pixel array
*/
void *cheese_runner(void *args) {
    Instruction *params = (Instruction *) args;
    int red, blue, green, counter;
    int height, width;
    int x, y;

    red = 0;
    blue = 0;
    green = 0;
    counter = 0;
    height = params->img->height;
    width = params->img->width;
    Pixel** filtered = params->filtered;

    //filter image to yellow
    for (int x = params->start; x < params->finish; x++) {
        for (int y = 0; y < height; y++) {

            //assign current pixel value to local var
            red = params->img->pArr[y][x].r;
            green = params->img->pArr[y][x].g;
            blue = 0;

            //filter image to yellow by eliminating blue
            filtered[y][x].r = red;
            filtered[y][x].g = green;
            filtered[y][x].b = blue;
        }
    }

    /* This section of code doensn't work properly :(
    //get total holes
    int total_holes, max_radius, min_radius;

    if (params->img->width <= params->img->height) {
        total_holes = rand() % (params->img->width / 12); //~8%
        max_radius = params->img->width / 8; //~12%
        min_radius = params->img->width / 16; //~4%
    } else {
        total_holes = rand() % (params->img->height / 12); //~8%
        max_radius = params->img->height / 8; //~12%
        min_radius = params->img->height / 16; //~4%
    }

    //create random points
    Circle *circle = malloc(sizeof(Circle) * total_holes);
    for (int i = 0; i < total_holes; i++) {
        int radius = (rand() % max_radius) + min_radius;
        int rand_x = rand() % params->img->width;
        int rand_y = rand() % params->img->height;

        circle[i].x = rand_x;
        circle[i].y = rand_y;
        circle[i].radius = radius;
    }

    //turn pixels inside radius to black if pixel is inside radius
    //i from y-r to y+r
    int i = 0, j = 0;
    int r = circle[i].radius;
    for (i = 0; i < params->img->width; i++) {
        //j from x-r to x+r
        for (j = 0; j < params->img->height; j++) {
            if ((i-circle[i].x)^2 + (j-circle[i].y)^2 <= circle[i].radius^2) {
                //turn pixel black
                filtered[j][i].r = 0;
                filtered[j][i].b = 0;
                filtered[j][i].g = 0;
            }
        }
    }*/
}


/**
* Swiss Cheese Filter
*
* @param img: image for pixel array
*/
void cheese_filter(Image* img) {

    // create new pixel array to write to
    Pixel** filtered = malloc(sizeof(Pixel*) * img->height);
    for (int i = 0; i < img->height; i++) {
        filtered[i] = malloc(sizeof(Pixel) * img->width);
    }

    //create an array of threads
    pthread_t thread[THREAD_COUNT];
    //create an array of params
    Instruction params[THREAD_COUNT];

    //create the instructions
    for (int i = 0; i < THREAD_COUNT; i++) {
        params[i].img = img;
        params[i].filtered = filtered;
        params[i].start = (img->width / THREAD_COUNT) * i;
        if (i < THREAD_COUNT - 1) params[i].finish = (i + 1) * (img->width / THREAD_COUNT);
        else params[i].finish = img->width;
    }

    //create the threads
    for (int n = 0; n < THREAD_COUNT; n++) {
        pthread_create(&thread[n], NULL, cheese_runner, &params[n]);
    }

    //join the threads
    for (int n = 0; n < THREAD_COUNT; n++) {
        pthread_join(thread[n], NULL);
    }

    //free the old array
    Pixel** temp = img->pArr;
    img->pArr = filtered;
    for (int i = 0; i < img->height; i++) {
        free(temp[i]);
    }
    free(temp);
}

////////////////////////////////////////////////////////////////////////////////
//MAIN PROGRAM CODE


int main(int argc, char* argv[]) {

    //input parameters
    char *input = argv[optind + 1];
    char *output = argv[optind + 2];
    char *filters;
    filters = "bc";
    int filter, bflag = 0, cflag = 0;

    //header functionality
    BMP_Header bmp_header;
    DIB_Header dib_header;

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

    //allocate mem for pixels
    struct Pixel** pixels = (struct Pixel**)malloc(sizeof(struct Pixel*) * dib_header.height);
    for (int p = 0; p < dib_header.height; p++) {
        pixels[p] = (struct Pixel*)malloc(sizeof(struct Pixel) * dib_header.width);
    }

    //read pixel array
    readPixelsBMP(file, pixels, dib_header.width, dib_header.height);

    //create the new image
    Image* img = image_create(pixels, dib_header.width, dib_header.height);


    //apply blur or swiss cheese filter based on the flag
    while ((filter = getopt(argc, argv, filters)) != -1) {
        switch(filter) {
            case 'b':
                bflag = 1;
                printf("applying blur filter...\n");
                //TODO: apply blur filter
                blur_filter(img);
                break;
            case 'c':
                cflag = 1;
                printf("applying swiss cheese filter...\n");
                //TODO: apply swiss cheese filter
                cheese_filter(img);
                break;
            case '?':
                printf("ERROR: unknown filter -%c.\n", optopt);
                break;
            default:
                printf("ERROR: default switch case\n");
        }
    }

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

    //write pixels to output file
    pixels = img->pArr;
    writePixelsBMP(output_file, pixels, dib_header.width, dib_header.height);
    fclose(output_file);

    //dealloc mem
    free(img);
    free(pixels);
    //free(circle);

    printf("Success! Your filtered image has been saved to the root folder.\n");
    return EXIT_SUCCESS;
}