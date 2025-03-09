#include<stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

    fclose(input_image);
    free(buffer);

    return 0;
}
