
// types
struct headerInfo {
    uint32_t width;
    uint32_t height;
    uint32_t offset;
};

struct indexMap {
    int ind_ul; // up left
    int ind_u;  // up 
    int ind_ur; // up right
    int ind_l;  // left
    int ind;    // center
    int ind_r;  // right 
    int ind_dl; // bottom left
    int ind_d;  // bottom
    int ind_dr; // bottom right
};
