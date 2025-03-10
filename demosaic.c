#include<stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "utils.h"


int main(int argc, char* argv[]) {
    
    printf("\nGenerating output image using bilinear interpolation.\n");

    char *input_fn = argv[1];
    char *buffer,*new_buf, ch;

    // open with ability to read filesize
    FILE *input_image=fopen(input_fn,"rb");

    // obtain filesize
    fseek(input_image, 0, SEEK_END);
    long filesize = ftell(input_image);
    rewind(input_image);

    // allocate memory
    buffer = calloc(filesize, sizeof(uint8_t)); 
    new_buf = calloc(filesize, sizeof(uint8_t)); 

    // copy input_image bytes into buffers 
    rewind(input_image);
    fread(buffer, 1, filesize, input_image);
    rewind(input_image);
    fread(new_buf, 1, filesize, input_image);
    rewind(input_image);

    // parse header information 
    struct headerInfo hi = get_header_info(input_image);

    // reopen for writing
    fclose(input_image);
    input_image=fopen(input_fn,"wb+");
    rewind(input_image);
 
    int bytes_per_row = hi.width * 4; // number of bytes * ( R, G, B, Padding)
  
    int base_index_last, base_index, base_index_next; 
    int i,j; 
    int ind = 0;
    int k = 0; // int to keep track of column (R/G or G/B)

    int row_type = 0; // 0 - RG, 1 - GB
    
    for(i=0;i<hi.height;i++){ // for each row

        k = 0; 

        // calculate base indexes
        if(i>0){
            base_index_last = hi.offset + ((i-1) * bytes_per_row);  // previous row
        }
        
        base_index = hi.offset + (i * bytes_per_row);  // current row 

        if(i<hi.height-1){
            base_index_next = hi.offset + ((i+1) * bytes_per_row); // next row
        }
         
        for (int j = 0; j < (bytes_per_row); j += 4) {

                
                struct indexMap im = generateIndexMap(new_buf, i, j, base_index_last, base_index, base_index_next, bytes_per_row, hi.width, hi.height);

                int B_ul, B_ur, B_dl, B_dr, G_l, G_r, G_u, G_d, B_u, B_d, R_l, R_r, R_ul, R_ur, R_dl, R_dr, B_l, B_r, R_u, R_d;

                if(row_type == 0){ // Red/Green
                    if(k==0){ // Red column

                        // Calulate B
                        int B_ul = (unsigned char)buffer[im.ind_ul]; 
                        int B_ur = (unsigned char)buffer[im.ind_ur]; 
                        int B_dl = (unsigned char)buffer[im.ind_dl]; 
                        int B_dr = (unsigned char)buffer[im.ind_dr]; 
                        new_buf[im.ind] = (B_ul + B_ur + B_dl + B_dr)>>2; // bitshift to divide by 4
    
                        // Calculate G 
                        int G_l = (unsigned char)buffer[im.ind_l+1];
                        int G_r = (unsigned char)buffer[im.ind_r+1];
                        int G_u = (unsigned char)buffer[im.ind_u+1];   
                        int G_d = (unsigned char)buffer[im.ind_d+1];   
                        new_buf[im.ind+1] = (G_l + G_r + G_u + G_d)>>2; // bitshift to divide by 4

                        k = 1;
                    } else { // Green column
                        // Calulate B
                        int B_u = (unsigned char)buffer[im.ind_u]; 
                        int B_d = (unsigned char)buffer[im.ind_d]; 
                        new_buf[im.ind] = (B_u + B_d)>>1; // bitshift to divide by 2 

                        // calculate R
                        int R_l = (unsigned char)buffer[im.ind_l+2];
                        int R_r = (unsigned char)buffer[im.ind_r+2];
                        new_buf[im.ind+2] = (R_l + R_r)>>1;

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
                        new_buf[im.ind+2] = (R_u + R_d)>>1;

                        k = 1;
                    } else { // Blue Column
                        // Calculate G 
                        G_l = (unsigned char)buffer[im.ind_l+1];
                        G_r = (unsigned char)buffer[im.ind_r+1];
                        G_u = (unsigned char)buffer[im.ind_u+1];   
                        G_d = (unsigned char)buffer[im.ind_d+1];   
                        new_buf[im.ind+1] = (G_l + G_r + G_u + G_d)>>2; // bitshift to divide by 4

                        // calculate R
                        R_ul = (unsigned char)buffer[im.ind_ul+2];
                        R_ur = (unsigned char)buffer[im.ind_ur+2];
                        R_dl = (unsigned char)buffer[im.ind_dr+2];
                        R_dr = (unsigned char)buffer[im.ind_dr+2];
                        new_buf[im.ind+2] = (R_ul + R_ur + R_dl + R_dr)>>2;

                        k = 0;
                    }

                }

        }


    // toggle row type
    row_type ^= 1;

    }
    

    // overwrite input file with buffer contents
    write_output_to_file(input_image, filesize, new_buf); 

    fclose(input_image);
    free(buffer);
    free(new_buf);

    return 0;
}
