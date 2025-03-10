#include<stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "utils.h"


int main(int argc, char* argv[]) {

    char *input_fn = argv[1];
    char *buffer, ch;

    printf("Creating Bayer pattern image with file: %s\n", input_fn);

    // open with ability to read filesize
    FILE *input_image=fopen(input_fn,"rb");

    // obtain filesize
    fseek(input_image, 0, SEEK_END);
    long filesize = ftell(input_image);
    rewind(input_image);

    // allocate memory
    buffer = calloc(filesize, sizeof(uint8_t)); 

    // copy input_image bytes into buffer 
    rewind(input_image);
    fread(buffer, 1, filesize, input_image);

    // parse header information 
    struct headerInfo hi = get_header_info(input_image);

    printf("BMP width: %u \n", hi.width);
    printf("BMP height: %d \n", hi.height);
    printf("BMP data offset: %u \n", hi.offset);

    // reopen for writing
    fclose(input_image);
    input_image=fopen(input_fn,"wb+");
    rewind(input_image);
 
    int bytes_per_row = hi.width * 4; // number of bytes * ( R, G, B, Padding)
   
    int i,j; // ints for row index and column index 
    int ind = 0; // int for buffer index value
    int k = 0; // int to keep track of column 

    int row_type = 0; // 0 - RG, 1 - GB
    for(i=0;i<hi.height;i++){ // for each row
        int base_index = hi.offset + (i * bytes_per_row); 
         
        for (int j = 0; j < bytes_per_row; j += 4) {

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

