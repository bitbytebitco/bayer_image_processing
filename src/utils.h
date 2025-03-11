#ifndef UTILS_H
#define UTILS_H

/*  
 * Filename: utils.h  
 * Description: Utility functions for Bayer pattern generation and interpolation 
 * Author: Zachary Becker  
 */

#include "types.h"

/* Function Declarations */

    
// Parses BMP header for width, height, data offset
struct headerInfo get_header_info(FILE *input_image);

// generates a map of the byte count indexes needed to generate a 3x3 matrix of a section of the image (handles mirroring)
struct indexMap generateIndexMap(int i, int j, int base_index_last, int base_index, int base_index_next, int bytes_per_row, int height);

// Applies a 3x3 convolution kernel to a 3x3 image matrix
int applyKernel(int image[3][3], float kernel[3][3]);

// Arbitrary interpolation algorithm
void runBilinearInterpolation(char * buffer, char * new_buf, struct headerInfo hi);

// write buffer to output file
void write_output_to_file(FILE *input_image, long filesize, char *buffer);

#endif 
