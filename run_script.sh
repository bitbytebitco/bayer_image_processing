#!/bin/bash

## Compile 
make clean && make

## Convert PNG to BMP
convert $1 -depth 8 -type TrueColorAlpha -define bmp:format=bmp4 -colorspace sRGB input.bmp

# Copy input image to bayer output file 
cp input.bmp bayer.bmp 

# Run script to create Bayer Pattern (RGGB)
./bayer_filter bayer.bmp 

# Copy bayer pattern to final output file
cp bayer.bmp output/demosaic.bmp 

# Run script to create final output
./demosaic output/demosaic.bmp

xdg-open output/demosaic.bmp
