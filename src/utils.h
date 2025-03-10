// my_header.h
#ifndef UTILS_H
#define UTILS_H

#include "types.h"

// prototypes 
struct headerInfo get_header_info(FILE *input_image);

struct indexMap generateIndexMap(int i, int j, int base_index_last, int base_index, int base_index_next, int bytes_per_row, int width, int height);

void write_output_to_file(FILE *input_image, long filesize, char *buffer);

#endif 
