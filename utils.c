#include<stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "types.h"


struct headerInfo get_header_info(FILE *input_image){
    int32_t h;
    uint32_t width;
    uint32_t height;
    uint32_t offset;

    struct headerInfo hi;

    fseek(input_image, 10, SEEK_SET);
    size_t res = fread(&offset, sizeof(offset), 1, input_image); 

    fseek(input_image, 18, SEEK_SET);
    res = fread(&width, sizeof(width), 1, input_image); 
    if(res != 1){
        fprintf(stderr, "Error reading file. Expected 1 byte, but read %ld bytes.\n", res);
        exit(1);  
    }

    fseek(input_image, 22, SEEK_SET);
    res = fread(&h, sizeof(h), 1, input_image); 
    if(res != 1){
        fprintf(stderr, "Error reading file. Expected 1 byte, but read %ld bytes.\n", res);
        exit(1);  
    }
    rewind(input_image);

    if(h && (1 << 31)){
        height = ~h+1;
        height = ~height+1;
        //height = height * -1;
    } else {
        height = h;
    }

    hi.width = width;
    hi.height = height;
    hi.offset = offset;

    return hi;
}

struct indexMap generateIndexMap(int i, int j, int base_index_last, int base_index, int base_index_next, int bytes_per_row, int width, int height){

    struct indexMap im;

    im.ind_ul = base_index_last + j - 4; 
    im.ind_u = base_index_last + j; 
    im.ind_ur = base_index_last + j + 4; 
    
    im.ind_l = base_index + j - 4; 
    im.ind = base_index + j; 
    im.ind_r = base_index + j + 4; 

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
        
    } else if(j == (width-1)){ // if last column
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
