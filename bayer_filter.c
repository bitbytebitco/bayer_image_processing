#include<stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "utils.h"

int main(int argc, char* argv[]) {

    char *input_fn = argv[1];
    char *buffer, ch;

    // open with ability to read filesize
    FILE *input_image=fopen(input_fn,"rb");

    fseek(input_image, 0, SEEK_END);
    long filesize = ftell(input_image);
    rewind(input_image);

    // allocate memory
    buffer = calloc(filesize, sizeof(uint8_t)); 

    // go to first data byte 
    rewind(input_image);
    fread(buffer, 1, filesize, input_image);

    // parse header information 
    uint32_t width;
    int32_t h;
    uint32_t height;
    uint32_t offset;
    fseek(input_image, 10, SEEK_SET);
    fread(&offset, sizeof(offset), 1, input_image); 
    fseek(input_image, 18, SEEK_SET);
    fread(&width, sizeof(width), 1, input_image); 
    fseek(input_image, 22, SEEK_SET);
    fread(&h, sizeof(h), 1, input_image); 
    rewind(input_image);

    if(1 && (1 << 31)){
        height = ~h+1;
    }
    //height = height * -1;

    printf("BMP width: %u \n", width);
    printf("BMP height: %d \n", height);
    printf("BMP data offset: %u \n", offset);

    // reopen for writing
    fclose(input_image);
    input_image=fopen(input_fn,"wb+");
    rewind(input_image);
 
    int bytes_per_row = width * 4; // number of bytes * ( R, G, B, Padding)
   
    int i,j; // ints for row index and column index 
    int ind = 0; // int for buffer index value
    int k = 0; // int to keep track of column 

    int row_type = 0; // 0 - RG, 1 - GB
    for(i=0;i<height;i++){ // for each row
        int base_index = offset + (i * bytes_per_row); 
         
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

