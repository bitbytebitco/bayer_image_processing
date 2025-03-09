#include<stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint32_t parseHeader(FILE *input_image, uint8_t byte_offset, uint8_t len){
    uint8_t bytes[len];
    uint32_t out = 0;
    fseek(input_image, byte_offset, SEEK_SET);
    fread(bytes, sizeof(uint8_t), len, input_image); 
    rewind(input_image);

    out |= bytes[0];
    out |= (bytes[1] << 8);
    out |= (bytes[2] << 16);
    out |= (bytes[3] << 24);

    return out;
}

int main(int argc, char* argv[]) {

    //char input_fn[] = "demosaic.bmp";
    char *input_fn = argv[1];
    char *buffer,*new_buf, ch;

    FILE *input_image=fopen(input_fn,"rb");

    fseek(input_image, 0, SEEK_END);
    long filesize = ftell(input_image);
    rewind(input_image);

    // allocate memory
    buffer = calloc(filesize, sizeof(uint8_t)); 
    new_buf = calloc(filesize, sizeof(uint8_t)); 

    // go to first data byte 
    rewind(input_image);
    fread(buffer, 1, filesize, input_image);
    rewind(input_image);
    fread(new_buf, 1, filesize, input_image);
    rewind(input_image);

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

    // take 2's complement to remove the negative sign
    height = ~h+1;

    printf("BMP width: %u \n", width);
    printf("BMP height: %d \n", height);
    printf("BMP data offset: %u \n", offset);

    // reopen for writing
    fclose(input_image);
    input_image=fopen(input_fn,"wb+");
    rewind(input_image);
 
    int bytes_per_row = width * 4; // number of bytes * ( R, G, B, Padding)
  
    int base_index_last, base_index, base_index_next; 
    int i,j; 
    int ind = 0;

    int row_type = 0; // 0 - RG, 1 - GB
    //for(i=0;i<4;i++){ // for each row
    for(i=0;i<height;i++){ // for each row

        int k = 0; // int to keep track of column 

        // TEMP avoid edges
        if(i>=2){
            
            //printf("\n\nRow %d :\n", i);

            if(i>0){
                base_index_last = offset + ((i-1) * bytes_per_row); 
            }
            base_index = offset + (i * bytes_per_row); 
            if(i<768){
                base_index_next = offset + ((i+1) * bytes_per_row); 
            }
             
            //for (int j = 0; j < 512; j += 4) {
            //for (int j = 0; j < 9; j += 4) {
            for (int j = 0; j < (bytes_per_row-(4*2)); j += 4) {

                if(j>(3*2)-1){

                    int ind_ul = base_index_last + j - 4; 
                    int ind_u = base_index_last + j; 
                    int ind_ur = base_index_last + j + 4; 
                    
                    int ind_l = base_index + j - 4; 
                    int ind = base_index + j; 
                    int ind_r = base_index + j + 4; 

                    int ind_dl = base_index_next + j - 4; 
                    int ind_d = base_index_next + j; 
                    int ind_dr =base_index_next + j + 4; 


                    /*
                    printf("\nind:%d ", ind);
                    printf("\nind_l:%d ", ind_l);
                    printf("\nind_r:%d ", ind_r);
                    printf("\nind_u:%d ", ind_u);
                    printf("\nind_d:%d \n", ind_d);

                    printf("\nB:%d , ind:%u ", buffer[ind], ind);
                    printf("G:%d , ind:%u ", (unsigned char)buffer[ind+1], ind+1);
                    printf("R:%d , ind:%u ", (unsigned char)buffer[ind+2], ind+2);
    
                    */

                    int B_ul, B_ur, B_dl, B_dr, G_l, G_r, G_u, G_d, B_u, B_d, R_l, R_r, R_ul, R_ur, R_dl, R_dr, B_l, B_r, R_u, R_d;

                    if(row_type == 0){ // RG
                        if(k==0){ // Red column

                            // Calulate B
                            int B_ul = (unsigned char)buffer[ind_ul]; 
                            int B_ur = (unsigned char)buffer[ind_ur]; 
                            int B_dl = (unsigned char)buffer[ind_dl]; 
                            int B_dr = (unsigned char)buffer[ind_dr]; 
                            new_buf[ind] = (B_ul + B_ur + B_dl + B_dr)>>2; // bitshift to divide by 4
        
                            // Calculate G 
                            int G_l = (unsigned char)buffer[ind_l+1];
                            int G_r = (unsigned char)buffer[ind_r+1];
                            int G_u = (unsigned char)buffer[ind_u+1];   
                            int G_d = (unsigned char)buffer[ind_d+1];   
                            new_buf[ind+1] = (G_l + G_r + G_u + G_d)>>2; // bitshift to divide by 4

                            k = 1;
                        } else { // Green column
                            // Calulate B
                            int B_u = (unsigned char)buffer[ind_u]; 
                            int B_d = (unsigned char)buffer[ind_d]; 
                            new_buf[ind] = (B_u + B_d)>>1; // bitshift to divide by 2 

                            // calculate R
                            int R_l = (unsigned char)buffer[ind_l+2];
                            int R_r = (unsigned char)buffer[ind_r+2];
                            new_buf[ind+2] = (R_l + R_r)>>1;

                            k = 0;
                        }
                    } else { // GB

                        if(k==0){ // Green Column
                            // Calulate B

                            /*
                            printf("\n###\n");
                            printf("\nB_l:%u , ind_l:%u", B_l, ind_l);
                            printf("\nv:%u , ind_l+1:%u", buffer[ind_l+1], ind_l+1);
                            printf("\nB_r:%u , ind_r:%u", B_r, ind_r);
                            */
                            B_l = (unsigned char)buffer[ind_l]; 
                            B_r = (unsigned char)buffer[ind_r]; 
                            new_buf[ind] = (B_l + B_r)>>1; // bitshift to divide by 2 

                            // calculate R
                            R_u = (unsigned char)buffer[ind_u+2];
                            R_d = (unsigned char)buffer[ind_d+2];

                            /*
                            printf("\nR_u:%u , ind_u:%u", R_u, ind_u);
                            printf("\nR_d:%u , ind_d,", B_r, ind_d);
                            */
                            new_buf[ind+2] = (R_u + R_d)>>1;
                            k = 1;
                        } else { // Blue Column
                            // Calculate G 
                            G_l = (unsigned char)buffer[ind_l+1];
                            G_r = (unsigned char)buffer[ind_r+1];
                            G_u = (unsigned char)buffer[ind_u+1];   
                            G_d = (unsigned char)buffer[ind_d+1];   
                            new_buf[ind+1] = (G_l + G_r + G_u + G_d)>>2; // bitshift to divide by 4

                            // calculate R
                            R_ul = (unsigned char)buffer[ind_ul+2];
                            R_ur = (unsigned char)buffer[ind_ur+2];
                            R_dl = (unsigned char)buffer[ind_dr+2];
                            R_dr = (unsigned char)buffer[ind_dr+2];
                            new_buf[ind+2] = (R_ul + R_ur + R_dl + R_dr)>>2;

                            k = 0;
                        }

                    }

                    /*
                    printf("\nB_new:%u ", new_buf[ind]);
                    printf("G_new:%u ", (unsigned char)new_buf[ind+1]);
                    printf("R_new:%u ", (unsigned char)new_buf[ind+2]);
                    */
                    // Skips buffer[i + 3] (padding)
                } 
            }


        // toggle row type
        row_type ^= 1;
    
        }    
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

        bytesWritten = fwrite(new_buf + (m * chunkSize), 1, chunkSize, input_image);
        if (bytesWritten != chunkSize) {
            perror("Error writing to file");
            break;
        }
    }
    printf("bytesWritten:%lu \n", bytesWritten);

   // Write remaining bytes
    if (remainingBytes > 0) {
        bytesWritten = fwrite(new_buf + (totalChunks * chunkSize), 1, remainingBytes, input_image);
        if (bytesWritten != remainingBytes) {
            perror("Error writing remaining bytes");
        }
    } 
    
    printf("bytesWritten:%lu \n", bytesWritten);

    fclose(input_image);
    free(buffer);
    free(new_buf);

    return 0;
}
