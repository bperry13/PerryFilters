# Blur and Swiss Cheese Filters
SER334 Module 6: Create bitmap filters in c using multiple threads

The command line parameters should be entered in the order below.
1: the filter 2: the file you want to load 3: the filename for the output image

To compile from root directory:
gcc src/BMPHandler.c src/Image.c src/PerryFilters.c -o PerryFilters

To run blur:
./PerryFilters -b ./res/test1wonderbread.bmp test1_b_output.bmp


To run swiss cheese:
./PerryFilters -c ./res/test1wonderbread.bmp test1_c_output.bmp