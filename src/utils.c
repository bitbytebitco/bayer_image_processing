/*  
 * Filename: utils.c  
 * Description: Utility functions for Bayer pattern generation and interpolation 
 * Author: Zachary Becker  
 */

#include<stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "types.h"

// Convolution Kernels (arbitrary algorithm kernels)
float greenKernel[3][3] = {
    {0, 0.25, 0},
    {0.25, 0, 0.25},
    {0, 0.25, 0}
};

float redBlueKernel[3][3] = {
    {0.25, 0, 0.25},
    {0, 0, 0},
    {0.25, 0, 0.25}
};

// Parses BMP header for width, height, data offset
struct headerInfo get_header_info(FILE *input_image){
    int32_t h;
    uint32_t width;
    uint32_t height;
    uint32_t offset;

    struct headerInfo hi;

    // parse offset
    fseek(input_image, 10, SEEK_SET);
    size_t res = fread(&offset, sizeof(offset), 1, input_image); 

    // parse image width
    fseek(input_image, 18, SEEK_SET);
    res = fread(&width, sizeof(width), 1, input_image); 
    if(res != 1){
        fprintf(stderr, "Error reading file. Expected 1 byte, but read %ld bytes.\n", res);
        exit(1);  
    }

    // parse image height 
    fseek(input_image, 22, SEEK_SET);
    res = fread(&h, sizeof(h), 1, input_image); 
    if(res != 1){
        fprintf(stderr, "Error reading file. Expected 1 byte, but read %ld bytes.\n", res);
        exit(1);  
    }
    rewind(input_image);

    // using two's complement to obtain positive value
    if(h && (1 << 31)){
        height = ~h+1;
        height = ~height+1;
    } else {
        height = h;
    }

    // filling output struct
    hi.width = width;
    hi.height = height;
    hi.offset = offset;

    return hi;
}

// generates a map of the byte count indexes needed to generate a 3x3 matrix of a section of the image (handles mirroring)
struct indexMap generateIndexMap(int i, int j, int base_index_last, int base_index, int base_index_next, int bytes_per_row, int height){

    struct indexMap im;

    // 3x3 matrix of indexes
    // ul, u, ur
    // l, ind, r
    // dl, d, dr 

    // top row
    im.ind_ul = base_index_last + j - 4; 
    im.ind_u = base_index_last + j; 
    im.ind_ur = base_index_last + j + 4; 
   
    // middle row 
    im.ind_l = base_index + j - 4; 
    im.ind = base_index + j; 
    im.ind_r = base_index + j + 4; 

    // bottom row
    im.ind_dl = base_index_next + j - 4; 
    im.ind_d = base_index_next + j; 
    im.ind_dr =base_index_next + j + 4; 

    // mirroring values
    if(j == 0){ // if on the first column 
        if(i == 0){ // if first row (top left corner)
            im.ind_ul = im.ind_dr; 
            im.ind_u = im.ind_d;
            im.ind_ur = im.ind_dr;
            im.ind_l = im.ind_r;
            im.ind_dl = im.ind_dr;
                 
        } else if(i == (height-1)) { // bottom row (bottom left corner) 

            im.ind_ul = im.ind_ur; 
            im.ind_l = im.ind_r;
            im.ind_dl = im.ind_ur;
            im.ind_d = im.ind_u;
            im.ind_dr = im.ind_ur;
                                        
        } else { // not on the first or last row (middle)
       
            // mirror values  
            im.ind_ul = im.ind_ur; 
            im.ind_l = im.ind_r;
            im.ind_dl = im.ind_dr;

        } 
        
    } else if(j >= bytes_per_row - 4){ // if last column

        if(i == 0){ // if first row (top right corner)

            im.ind_ul = im.ind_dl;
            im.ind_u = im.ind_d;
            im.ind_ur = im.ind_dl;
            im.ind_r = im.ind_l;
            im.ind_dr = im.ind_dl;

        } else if(i == (height-1)){ // bottom row (bottom right corner) 

            im.ind_ur = im.ind_ul;
            im.ind_r = im.ind_l;
            im.ind_dr = im.ind_ul;
            im.ind_d = im.ind_u;
            im.ind_dl = im.ind_ul; 

        } else { // not on the first or last row (middle)

            im.ind_ur = im.ind_ul;
            im.ind_r = im.ind_l;
            im.ind_dr = im.ind_dl;

        }

    } else { // if middle columns 

        if(i == 0){ // first row

            im.ind_ul = im.ind_dl;
            im.ind_u = im.ind_d;
            im.ind_ur = im.ind_dr;

        } else if(i == (height-1)) { // last row

            im.ind_dl = im.ind_ul;
            im.ind_d = im.ind_u;
            im.ind_dl = im.ind_ur;

        }
            
    }

    return im;

}

// Applies a 3x3 convolution kernel to a 3x3 image value matrix
int applyKernel(int image[3][3], float kernel[3][3]) {
    int result = 0;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            result += image[i][j] * kernel[i][j];
        }
    }

    return result;
}

// Arbitrary interpolation algorithm
void runBilinearInterpolation(char * buffer, char * new_buf, struct headerInfo hi){

    // number of bytes * ( R, G, B, Padding)
    uint32_t bytes_per_row = hi.width * 4; 

    // buffer index value for the first byte in a row  
    uint32_t base_index_last, base_index, base_index_next; 
    
    // ints for row index and column index 
    uint32_t i,j; 

    // int to keep track of column (0 - R/G or 1 - G/B)
    int k;

     // row type (0 - RG, 1 - GB)
    int row_type = 1;
   
    // vars to make simple averaging operations easier to read             
    uint8_t B_u, B_d, R_l, R_r, B_l, B_r, R_u, R_d;
    
    for(i=0;i<hi.height;i++){ // for each row

        // set column type to R/G
        k = 0; 

        // calculate base indexes
        // previous row
        if(i>0){
            base_index_last = hi.offset + ((i-1) * bytes_per_row);  
        } else {
            base_index_last = -1;
        }

        // current row 
        base_index = hi.offset + (i * bytes_per_row);  

        // next row
        if(i<hi.height-1){
            base_index_next = hi.offset + ((i+1) * bytes_per_row); 
        } else {
            base_index_next = -1;
        }
        
        // iterate through bytes (skipping every fourth, which is padding) 
        for (j = 0; j < (bytes_per_row); j += 4) {

                // generate 3x3 buffer index map
                struct indexMap im = generateIndexMap(i, j, base_index_last, base_index, base_index_next, bytes_per_row, hi.height);

                if(row_type == 0){ // Red/Green
                    if(k==0){ // Red column

                        // generate image matrix
                        int image[3][3] = {    
                            {(unsigned char)buffer[im.ind_ul], (unsigned char)buffer[im.ind_u+1] , (unsigned char)buffer[im.ind_ur]},
                            {(unsigned char)buffer[im.ind_l+1], 0 , (unsigned char)buffer[im.ind_r+1]},
                            {(unsigned char)buffer[im.ind_dl], (unsigned char)buffer[im.ind_d+1] , (unsigned char)buffer[im.ind_dr]}
                        };

                        // Calulate B
                        new_buf[im.ind] = applyKernel(image, redBlueKernel);
   
                        // Calculate G 
                        new_buf[im.ind+1] = applyKernel(image, greenKernel);

                        k = 1;
                    } else { // Green column
                        // Calulate B
                        B_u = (unsigned char)buffer[im.ind_u]; 
                        B_d = (unsigned char)buffer[im.ind_d]; 
                        new_buf[im.ind] = (B_u + B_d)>>1; // bitshift to divide by 2 

                        // calculate R
                        R_l = (unsigned char)buffer[im.ind_l+2];
                        R_r = (unsigned char)buffer[im.ind_r+2];
                        new_buf[im.ind+2] = (R_l + R_r)>>1; // bitshift to divide by 2 

                        k = 0;
                    }
                } else { // Green/Blue

                    if(k==0){ // Green Column
                        // Calulate B
                        B_l = (unsigned char)buffer[im.ind_l]; 
                        B_r = (unsigned char)buffer[im.ind_r]; 
                        new_buf[im.ind] = (B_l + B_r)>>1; // bitshift to divide by 2 

                        // calculate R
                        R_u = (unsigned char)buffer[im.ind_u+2];
                        R_d = (unsigned char)buffer[im.ind_d+2];
                        new_buf[im.ind+2] = (R_u + R_d)>>1; // bitshift to divide by 2 

                        k = 1;
                    } else { // Blue Column

                        // generate image matrix
                        int image[3][3] = {    
                            {(unsigned char)buffer[im.ind_ul+2], (unsigned char)buffer[im.ind_u+1] , (unsigned char)buffer[im.ind_ur+2]},
                            {(unsigned char)buffer[im.ind_l+1], 0 , (unsigned char)buffer[im.ind_r+1]},
                            {(unsigned char)buffer[im.ind_dl+2], (unsigned char)buffer[im.ind_d+1] , (unsigned char)buffer[im.ind_dr+2]}
                        };

                        // Calculate G 
                        new_buf[im.ind+1] = applyKernel(image, greenKernel);

                        // calculate R
                        new_buf[im.ind+2] = applyKernel(image, redBlueKernel);

                        k = 0;
                    }

                }

        }

    // toggle row type
    row_type ^= 1;

    }
}

// write buffer to output file
void write_output_to_file(FILE *input_image, long filesize, char *buffer){

    // go to beginning of data    
    rewind(input_image);

    // Allocate a buffer to hold data temporarily
    size_t bytes_written = 0;
    size_t chunk_size = 1024; // 1 KB
    size_t total_chunks = filesize / chunk_size;
    size_t remaining_bytes = filesize % chunk_size;
        
    printf("filesize: %lu\n", filesize);
    printf("total_chunks: %lu\n", total_chunks);

    for (size_t m = 0; m < total_chunks; m++) {
        bytes_written = fwrite(buffer + (m * chunk_size), 1, chunk_size, input_image);
        if (bytes_written != chunk_size) {
            perror("Error writing to file");
            break;
        }
    }
    printf("bytes_written:%lu \n", bytes_written);

   // Write remaining bytes
    if (remaining_bytes > 0) {
        bytes_written = fwrite(buffer + (total_chunks * chunk_size), 1, remaining_bytes, input_image);
        if (bytes_written != remaining_bytes) {
            perror("Error writing remaining bytes");
        }
    } 
    
    printf("bytes_written:%lu \n", bytes_written);
}
