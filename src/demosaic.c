/*  
 * Filename: demosaic.c  
 * Description: Script that runs the image interpolation from a Bayer pattern image
 * Author: Zachary Becker  
 */

#include<stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "utils.h"

int main(int argc, char* argv[]) {

    // check for input file
    if(argc < 1){
        printf("Please add an input file as the first argument (e.g. ./run_script [filename.png]");
        return 1;
    }
   
    // user output 
    printf("\nGenerating output image using bilinear interpolation.\n");

    // buffers
    char *input_fn = argv[1];
    char *buffer,*new_buf;

    // open with ability to read filesize
    FILE *input_image;
    input_image = fopen(input_fn,"rb");

    // obtain filesize
    fseek(input_image, 0, SEEK_END);
    size_t filesize = ftell(input_image);
    rewind(input_image);

    // allocate memory
    buffer = calloc(filesize, sizeof(uint8_t)); 
    new_buf = calloc(filesize, sizeof(uint8_t)); 

    // copy input_image bytes into buffers 
    rewind(input_image);
    size_t res = fread(buffer, 1, filesize, input_image);
    if(res != filesize){
        fprintf(stderr, "Error reading file. Expected %ld bytes, but read %ld bytes.\n", filesize, res);
        exit(1);  
    }

    // copy file contents  
    rewind(input_image);
    res = fread(new_buf, 1, filesize, input_image);
    rewind(input_image);

    // parse header information 
    struct headerInfo hi = get_header_info(input_image);

    // reopen for writing
    fclose(input_image);
    input_image=fopen(input_fn,"wb+");
    rewind(input_image);

    // run an arbitrary interpolation algorithm
    runBilinearInterpolation(buffer, new_buf, hi); 

    // overwrite input file with buffer contents
    write_output_to_file(input_image, filesize, new_buf); 

    // clean up
    fclose(input_image);
    free(buffer);
    free(new_buf);

    return 0;
}
