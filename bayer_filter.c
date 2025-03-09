#include<stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define IMAGE_DATA_OFFSET 122 

uint32_t parseWidth(FILE *input_image){
    uint8_t wBytes[4];
    uint32_t width = 0;
    fseek(input_image, 18, SEEK_SET);
    fread(wBytes, sizeof(uint8_t), 4, input_image); 
    rewind(input_image);

    width |= wBytes[0];
    width |= (wBytes[1] << 8);
    width |= (wBytes[2] << 16);
    width |= (wBytes[3] << 24);
    printf("BMP width: %u \n", width);

    return width;
}

int main(int argc, char* argv[]) {

    //char input_fn[] = "test.bmp";
    char *input_fn = argv[1];
    char *buffer, ch;

    FILE *input_image=fopen(input_fn,"rb");

    fseek(input_image, 0, SEEK_END);
    long filesize = ftell(input_image);
    rewind(input_image);

    // allocate memory
    buffer = calloc(filesize, sizeof(uint8_t)); 

    // go to first data byte 
    rewind(input_image);
    fread(buffer, 1, filesize, input_image);

    // parse width
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
   
    int i,j; 
    int ind = 0;
    int k = 0; // int to keep track of column 

    int row_type = 0; // 0 - RG, 1 - GB
    for(i=0;i<height;i++){ // for each row
        int base_index = IMAGE_DATA_OFFSET + (i * bytes_per_row); 
         
        for (int j = 0; j < bytes_per_row; j += 4) {

            ind = base_index + j; 
    
            /*
            printf("\n index:%d ", ind);
            printf("\nB:%d ", buffer[ind]);
            printf("G:%d ", (unsigned char)buffer[ind+1]);
            printf("R:%d ", (unsigned char)buffer[ind+2]);
            */
       
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
            

            /*
            printf("\n index:%d ", ind);
            printf("\nB:%d ", buffer[ind]);
            printf("G:%d ", (unsigned char)buffer[ind+1]);
            printf("R:%d ", (unsigned char)buffer[ind+2]);
            */
            // Skips buffer[i + 3] (padding)
            
        }

        // toggle row type
        row_type ^= 1;
    
        
    }

    printf("\n");

    // go to beginning of data    
    rewind(input_image);

    // Allocate a buffer to hold data temporarily
    size_t bytesWritten = 0;
    size_t chunkSize = 1024; // 1 KB
    size_t totalChunks = filesize / chunkSize;
    size_t remainingBytes = filesize % chunkSize;
        
    printf("filesize: %lu\n", filesize);
    printf("totalChunks: %lu\n", totalChunks);

    for (size_t m = 0; m < totalChunks; m++) {

        bytesWritten = fwrite(buffer + (m * chunkSize), 1, chunkSize, input_image);
        if (bytesWritten != chunkSize) {
            perror("Error writing to file");
            break;
        }
    }
    printf("bytesWritten:%lu \n", bytesWritten);

   // Write remaining bytes
    if (remainingBytes > 0) {
        bytesWritten = fwrite(buffer + (totalChunks * chunkSize), 1, remainingBytes, input_image);
        if (bytesWritten != remainingBytes) {
            perror("Error writing remaining bytes");
        }
    } 
    
    printf("bytesWritten:%lu \n", bytesWritten);

    fclose(input_image);
    free(buffer);

    return 0;
}
