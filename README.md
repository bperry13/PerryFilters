# Blur and Swiss Cheese Filters
SER334 Module 6: Create bitmap filters in c using multiple threads
-
The command line parameters should be entered in the order below
-
1: the filter 2: the file you want to load 3: the filename for the output image

To compile from root directory:
-
gcc PerryFilters.c -o PerryFilters -pthread

To run blur:
-
./PerryFilters -b test1wonderbread.bmp test1_b_output.bmp
./PerryFilters -b test2.bmp test2_b_output.bmp
./PerryFilters -b test3.bmp test3_b_output.bmp

To run swiss cheese:
-
./PerryFilters -c test1wonderbread.bmp test1_c_output.bmp
./PerryFilters -c test2.bmp test2_c_output.bmp
./PerryFilters -c test3.bmp test3_c_output.bmp
Note: The code for the holes doesn't work so it's commented out. I gave it
my best shot :(