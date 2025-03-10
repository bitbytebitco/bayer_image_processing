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

    char *input_fn = argv[1];
    char *buffer;

    printf("\nCreating Bayer pattern image with file: %s\n", input_fn);

    // open with ability to read filesize
    FILE *input_image;
    input_image = fopen(input_fn,"rb");

    // obtain filesize
    fseek(input_image, 0, SEEK_END);
    size_t filesize = ftell(input_image);
    rewind(input_image);

    // allocate memory
    buffer = calloc(filesize, sizeof(uint8_t)); 

    // copy input_image bytes into buffer 
    rewind(input_image);
    size_t res = fread(buffer, 1, filesize, input_image);
    if(res != filesize){
        fprintf(stderr, "Error reading file. Expected %ld bytes, but read %ld bytes.\n", filesize, res);
        exit(1);  
    }

    // parse header information 
    struct headerInfo hi = get_header_info(input_image);

    printf("BMP width: %u \n", hi.width);
    printf("BMP height: %d \n", hi.height);
    printf("BMP data offset: %u \n", hi.offset);

    // reopen for writing
    fclose(input_image);
    input_image=fopen(input_fn,"wb+");
    rewind(input_image);
 
    uint32_t bytes_per_row = hi.width * 4; // number of bytes * ( R, G, B, Padding)
   
    // ints for row index and column index 
    uint32_t i;
    uint32_t j; 
    uint32_t ind = 0; // int for buffer index value
    uint8_t k = 0; // int to keep track of column 

    uint8_t row_type = 0; // 0 - RG, 1 - GB
    for(i=0;i<hi.height;i++){ // for each row
        uint32_t base_index = hi.offset + (i * bytes_per_row); 
         
        for (j = 0; j < bytes_per_row; j += 4) {

            ind = base_index + j; 
    
            if(row_type == 0){ 
                if(k==0){
                    // Zero B,G
                    buffer[ind] = 0;
                    buffer[ind+1] = 0;
                    k = 1;
                } else {
                    // Zero B,R
                    buffer[ind] = 0;
                    buffer[ind+2] = 0;
                    k = 0;
                }
            } else {
                if(k==0){
                    // Zero B,R
                    buffer[ind] = 0;
                    buffer[ind+2] = 0;
                    k = 1;
                } else {
                    // Zero B,R
                    buffer[ind+1] = 0;
                    buffer[ind+2] = 0;
                    k = 0;
                }

            }
            
        }

        // toggle row type
        row_type ^= 1;
        
    }

    // overwrite input file with buffer contents
    write_output_to_file(input_image, filesize, buffer); 
    
    fclose(input_image);
    free(buffer);

    return 0;
}

