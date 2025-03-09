#include<stdio.h>

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
