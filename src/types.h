#ifndef TYPES_H
#define TYPES_H

/*  
 * Filename: types.h  
 * Description: File for structure definitions 
 * Author: Zachary Becker  
 */

struct headerInfo {
    uint32_t width;
    uint32_t height;
    uint32_t offset;
};

struct indexMap {
    uint32_t ind_ul; // up left
    uint32_t ind_u;  // up 
    uint32_t ind_ur; // up right
    uint32_t ind_l;  // left
    uint32_t ind;    // center
    uint32_t ind_r;  // right 
    uint32_t ind_dl; // bottom left
    uint32_t ind_d;  // bottom
    uint32_t ind_dr; // bottom right
};

#endif 
