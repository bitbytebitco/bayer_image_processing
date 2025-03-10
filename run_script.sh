#!/bin/bash

## change directory to src
cd src

## Compile 
make clean && make

cd ..

## Convert PNG to BMP
convert $1 -depth 8 -type TrueColorAlpha -define bmp:format=bmp4 -colorspace sRGB output/input.bmp

# Copy input image to bayer output file 
cp output/input.bmp output/bayer.bmp 

# Run script to create Bayer Pattern (RGGB)
./src/bayer_filter output/bayer.bmp 

# Copy bayer pattern to final output file
cp output/bayer.bmp output/demosaic.bmp 

# Run script to create final output
./src/demosaic output/demosaic.bmp

xdg-open output/demosaic.bmp
