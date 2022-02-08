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

typedef struct inst {
    Image *img;
    Pixel** filtered; // write to this with filter applied
    // int first_col;
    // int last_col;
    int start;
    int finish;
} inst;


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

    inst *params = (inst *) args;
    int y, x;
    int red, blue, green, counter;
    int height, width;
    //int first_col, last_col;

    // x = 0;
    // y = 0;
    red = 0;
    blue = 0;
    green = 0;
    counter = 0;
    height = params->img->height;
    width = params->img->width;
    Pixel** filtered = params->filtered;
    // first_col = params->first_col;
    // last_col = params->last_col;

    // i width, j height
    for (int i = params->start; i < params->finish; i++) {
        for (int j = 0; j < height; j++) {

            red = 0;
            blue = 0;
            green = 0;
            counter = 0;

            // top left corner
            if (i == 0 && j == 0) {
                red += params->img->pArr[j][i].r;
                blue += params->img->pArr[j][i].b;
                green += params->img->pArr[j][i].g;
                red += params->img->pArr[j][i + 1].r;
                blue += params->img->pArr[j][i + 1].b;
                green += params->img->pArr[j][i + 1].g;
                red += params->img->pArr[j + 1][i].r;
                blue += params->img->pArr[j + 1][i].b;
                green += params->img->pArr[j + 1][i].g;
                red += params->img->pArr[j + 1][i + 1].r;
                blue += params->img->pArr[j + 1][i + 1].b;
                green += params->img->pArr[j + 1][i + 1].g;
                counter = 4;
            } else if (j == 0 && i == width -1) { // top right
                red += params->img->pArr[j][i].r;
                blue += params->img->pArr[j][i].b;
                green += params->img->pArr[j][i].g;
                red += params->img->pArr[j][i - 1].r;
                blue += params->img->pArr[j][i - 1].b;
                green += params->img->pArr[j][i - 1].g;
                red += params->img->pArr[j + 1][i].r;
                blue += params->img->pArr[j + 1][i].b;
                green += params->img->pArr[j + 1][i].g;
                red += params->img->pArr[j + 1][i - 1].r;
                blue += params->img->pArr[j + 1][i - 1].b;
                green += params->img->pArr[j + 1][i - 1].g;
                counter = 4;
            } else if (j == height - 1 && i == 0) { // bottom left
                red += params->img->pArr[j][i].r;
                blue += params->img->pArr[j][i].b;
                green += params->img->pArr[j][i].g;
                red += params->img->pArr[j][i + 1].r;
                blue += params->img->pArr[j][i + 1].b;
                green += params->img->pArr[j][i + 1].g;
                red += params->img->pArr[j - 1][i].r;
                blue += params->img->pArr[j - 1][i].b;
                green += params->img->pArr[j - 1][i].g;
                red += params->img->pArr[j - 1][i + 1].r;
                blue += params->img->pArr[j - 1][i + 1].b;
                green += params->img->pArr[j - 1][i + 1].g;
                counter = 4;
            } else if (j == height - 1 && i == width - 1) { // bottom right
                red += params->img->pArr[j][i].r;
                blue += params->img->pArr[j][i].b;
                green += params->img->pArr[j][i].g;
                red += params->img->pArr[j][i - 1].r;
                blue += params->img->pArr[j][i - 1].b;
                green += params->img->pArr[j][i - 1].g;
                red += params->img->pArr[j - 1][i].r;
                blue += params->img->pArr[j - 1][i].b;
                green += params->img->pArr[j - 1][i].g;
                red += params->img->pArr[j - 1][i - 1].r;
                blue += params->img->pArr[j - 1][i - 1].b;
                green += params->img->pArr[j - 1][i - 1].g;
                counter = 4;
            } else if (j == 0) { // top row
                red += params->img->pArr[j][i].r;
                blue += params->img->pArr[j][i].b;
                green += params->img->pArr[j][i].g;
                red += params->img->pArr[j][i - 1].r;
                blue += params->img->pArr[j][i - 1].b;
                green += params->img->pArr[j][i - 1].g;
                red += params->img->pArr[j][i + 1].r;
                blue += params->img->pArr[j][i + 1].b;
                green += params->img->pArr[j][i + 1].g;
                red += params->img->pArr[j + 1][i].r;
                blue += params->img->pArr[j + 1][i].b;
                green += params->img->pArr[j + 1][i].g;
                red += params->img->pArr[j + 1][i - 1].r;
                blue += params->img->pArr[j + 1][i - 1].b;
                green += params->img->pArr[j + 1][i - 1].g;
                red += params->img->pArr[j + 1][i + 1].r;
                blue += params->img->pArr[j + 1][i + 1].b;
                green += params->img->pArr[j + 1][i + 1].g;
                counter = 6;
            } else if (j == height - 1) { // bottom row
                red += params->img->pArr[j][i].r;
                blue += params->img->pArr[j][i].b;
                green += params->img->pArr[j][i].g;
                red += params->img->pArr[j][i - 1].r;
                blue += params->img->pArr[j][i - 1].b;
                green += params->img->pArr[j][i - 1].g;
                red += params->img->pArr[j][i + 1].r;
                blue += params->img->pArr[j][i + 1].b;
                green += params->img->pArr[j][i + 1].g;
                red += params->img->pArr[j - 1][i].r;
                blue += params->img->pArr[j - 1][i].b;
                green += params->img->pArr[j - 1][i].g;
                red += params->img->pArr[j - 1][i - 1].r;
                blue += params->img->pArr[j - 1][i - 1].b;
                green += params->img->pArr[j - 1][i - 1].g;
                red += params->img->pArr[j - 1][i + 1].r;
                blue += params->img->pArr[j - 1][i + 1].b;
                green += params->img->pArr[j - 1][i + 1].g;
                counter = 6;
            } else if (i == 0) { // left column
                red += params->img->pArr[j][i].r;
                blue += params->img->pArr[j][i].b;
                green += params->img->pArr[j][i].g;
                red += params->img->pArr[j][i + 1].r;
                blue += params->img->pArr[j][i + 1].b;
                green += params->img->pArr[j][i + 1].g;
                red += params->img->pArr[j - 1][i].r;
                blue += params->img->pArr[j - 1][i].b;
                green += params->img->pArr[j - 1][i].g;
                red += params->img->pArr[j - 1][i + 1].r;
                blue += params->img->pArr[j - 1][i + 1].b;
                green += params->img->pArr[j - 1][i + 1].g;
                red += params->img->pArr[j + 1][i].r;
                blue += params->img->pArr[j + 1][i].b;
                green += params->img->pArr[j + 1][i].g;
                red += params->img->pArr[j + 1][i + 1].r;
                blue += params->img->pArr[j + 1][i + 1].b;
                green += params->img->pArr[j + 1][i + 1].g;
                counter = 6;
            } else if (i == width - 1) { // right column
                red += params->img->pArr[j][i].r;
                blue += params->img->pArr[j][i].b;
                green += params->img->pArr[j][i].g;
                red += params->img->pArr[j][i - 1].r;
                blue += params->img->pArr[j][i - 1].b;
                green += params->img->pArr[j][i - 1].g;
                red += params->img->pArr[j - 1][i].r;
                blue += params->img->pArr[j - 1][i].b;
                green += params->img->pArr[j - 1][i].g;
                red += params->img->pArr[j - 1][i - 1].r;
                blue += params->img->pArr[j - 1][i - 1].b;
                green += params->img->pArr[j - 1][i - 1].g;
                red += params->img->pArr[j + 1][i].r;
                blue += params->img->pArr[j + 1][i].b;
                green += params->img->pArr[j + 1][i].g;
                red += params->img->pArr[j + 1][i - 1].r;
                blue += params->img->pArr[j + 1][i - 1].b;
                green += params->img->pArr[j + 1][i - 1].g;
                counter = 6;
            } else { // interior
                red += params->img->pArr[j][i].r;
                blue += params->img->pArr[j][i].b;
                green += params->img->pArr[j][i].g;
                red += params->img->pArr[j][i - 1].r;
                blue += params->img->pArr[j][i - 1].b;
                green += params->img->pArr[j][i - 1].g;
                red += params->img->pArr[j][i + 1].r;
                blue += params->img->pArr[j][i + 1].b;
                green += params->img->pArr[j][i + 1].g;
                red += params->img->pArr[j - 1][i].r;
                blue += params->img->pArr[j - 1][i].b;
                green += params->img->pArr[j - 1][i].g;
                red += params->img->pArr[j - 1][i - 1].r;
                blue += params->img->pArr[j - 1][i - 1].b;
                green += params->img->pArr[j - 1][i - 1].g;
                red += params->img->pArr[j - 1][i + 1].r;
                blue += params->img->pArr[j - 1][i + 1].b;
                green += params->img->pArr[j - 1][i + 1].g;
                red += params->img->pArr[j + 1][i].r;
                blue += params->img->pArr[j + 1][i].b;
                green += params->img->pArr[j + 1][i].g;
                red += params->img->pArr[j + 1][i - 1].r;
                blue += params->img->pArr[j + 1][i - 1].b;
                green += params->img->pArr[j + 1][i - 1].g;
                red += params->img->pArr[j + 1][i + 1].r;
                blue += params->img->pArr[j + 1][i + 1].b;
                green += params->img->pArr[j + 1][i + 1].g;
                counter = 9;
            }

            // //params->img->pArr[i][j].r
            // // top left corner pixel of 3x3
            // if (y + 1 < width && x - 1 >= width) { // first_col) {
            //     red += params->img->pArr[y + 1][x - 1].r;
            //     green += params->img->pArr[y + 1][x - 1].g;
            //     blue += params->img->pArr[y + 1][x - 1].b;
            //     counter++;
            // }

            // // top middle pixel of 3x3
            // if (y + 1 < width) {
            //     red += params->img->pArr[y + 1][x].r;
            //     green += params->img->pArr[y + 1][x].g;
            //     blue += params->img->pArr[y + 1][x].b;
            //     counter++;
            // }

            // // top right corner pixel of 3x3
            // if (y + 1 < width && x + 1 < width) { // last_col) {
            //     red += params->img->pArr[y + 1][x + 1].r;
            //     green += params->img->pArr[y + 1][x + 1].g;
            //     blue += params->img->pArr[y + 1][x + 1].b;
            //     counter++;
            // }

            // // middle left pixel of 3x3
            // if (x - 1 >= first_col) {
            //     red += params->img->pArr[y][x - 1].r;
            //     green += params->img->pArr[y][x - 1].g;
            //     blue += params->img->pArr[y][x - 1].b;
            //     counter++;
            // }

            // // current pixel aka middle pixel of 3x3
            // red += params->img->pArr[y][x].r;
            // green += params->img->pArr[y][x].g;
            // blue += params->img->pArr[y][x].b;
            // counter++;

            // // middle right pixel of 3x3
            // if (x + 1 < last_col) {
            //     red += params->img->pArr[y][x + 1].r;
            //     green += params->img->pArr[y][x + 1].g;
            //     blue += params->img->pArr[y][x + 1].b;
            //     counter++;
            // }

            // // bottom left pixel of 3x3
            // if (y - 1 >= 0 && x - 1 >= first_col) {
            //     red += params->img->pArr[y - 1][x - 1].r;
            //     green += params->img->pArr[y - 1][x - 1].g;
            //     blue += params->img->pArr[y - 1][x - 1].b;
            //     counter++;
            // }

            // // bottom middle pixel of 3x3
            // if (y - 1 >= 0) {
            //     red += params->img->pArr[y - 1][x].r;
            //     green += params->img->pArr[y - 1][x].g;
            //     blue += params->img->pArr[y - 1][x].b;
            //     counter++;
            // }

            // // bottom right pixel of 3x3
            // if (y - 1 >= 0 && x + 1 < last_col) {
            //     red += params->img->pArr[y - 1][x + 1].r;
            //     green += params->img->pArr[y - 1][x + 1].g;
            //     blue += params->img->pArr[y - 1][x + 1].b;
            //     counter++;
            // }

            //write average value of pixels
            filtered[j][i].r = red / counter;
            filtered[j][i].g = green / counter;
            filtered[j][i].b = blue / counter;

            // params->img->pArr[y][x].b = blue / counter;
            // params->img->pArr[y][x].g = green / counter;
            // params->img->pArr[y][x].r = red / counter;
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

    // Image* result = malloc(sizeof(Image));
    // result->pArr = (Pixel**) malloc(sizeof(Pixel*) * img->height);  // (img->height * img->width));
    // for (int i = 0; i < img->height; i++) {
    //     result->pArr[i] = malloc(sizeof(Pixel) * img->width);
    // }

    // create new pixel array to write to
    Pixel** filtered = malloc(sizeof(Pixel*) * img->height);
    for (int i = 0; i < img->height; i++) {
        filtered[i] = malloc(sizeof(Pixel) * img->width);
    }

    //create an array of threads
    pthread_t thread[THREAD_COUNT];
    //create an array of params
    inst params[THREAD_COUNT];

    //create the instructions
    for (int i = 0; i < THREAD_COUNT; i++) {
        params[i].img = img;
        params[i].filtered = filtered;
        params[i].start = (img->width / THREAD_COUNT) * i;
        //params[i].finish = params[i].start + (img->width / THREAD_COUNT);
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

    // return result;
}

/**
* Swiss Cheese Filter
*
* @param img: image for pixel array
*/
void *cheese_filter(Image* img) {

    int r, g, b;
    int x, y, h, w;
    int width, height, radius;

    for (y = 0; y < img->height; y++) {
        for (x = 0; x < img->width; x++) {

            b = 0;
            g = img->pArr[y][x].g;
            r = img->pArr[y][x].r;

            //find pixels for each swiss cheese hole
            /*
            width = img->width;
            height = img->height;
            h = img->height / THREAD_COUNT;
            w = img->width / THREAD_COUNT;
            for (int i = 0; i < width; i + w) {
                for (int j = 0; j < height; j + h) {

                }
            }
             */

            //write average value of pixels
            img->pArr[y][x].b = b;
            img->pArr[y][x].g = g;
            img->pArr[y][x].r = r;
        }
    }
    pthread_exit(0);
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

    struct thread_info** infos = (struct thread_info**)malloc(sizeof(struct thread_info*) * THREAD_COUNT);


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
    //free(result);


    printf("Success! Your filtered image has been saved to the root folder.\n");
    return EXIT_SUCCESS;
}