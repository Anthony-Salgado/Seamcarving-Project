#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "seamcarving.h"
#include "c_img.h"
// #include <opencv2/opencv.hpp>
// #include <iostream>

uint8_t wrap_around_height( struct rgb_img *im, int i, int j, int k){
    int height = im -> height;
    uint8_t rgb_elm;
    if (i<0){
        rgb_elm = get_pixel(im, height-1, j, k);
    } else if (i >= height) {
        rgb_elm = get_pixel(im, 0, j, k);
    } else {
        rgb_elm = get_pixel(im, i, j, k);
    }
    return rgb_elm;
}

uint8_t wrap_around_width(struct rgb_img *im, int i, int j, int k){
    int width = im -> width;
    uint8_t rgb_elm;
    if (j<0){
        rgb_elm = get_pixel(im, i, width-1, k);
    } else if (j >= width) {
        rgb_elm = get_pixel(im, i, 0, k);
    } else {
        rgb_elm = get_pixel(im, i, j, k);
    }
    return rgb_elm;
}


void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    int height = im->height;
    int width = im->width;
    create_img(grad, height, width);
    int col = 3;
    uint8_t energy = 0;
    // int final_engery = 0;
    int total = 0;
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            //compute each column
            for (int k = 0; k < col; k++) {
                uint8_t left_r = wrap_around_width(im, i, j-1, k);
                int test_left_r = (uint8_t) left_r;
                uint8_t right_r = wrap_around_width(im, i, j+1, k);
                int test_right_r = (uint8_t) right_r;
                uint8_t up_r = wrap_around_height(im, i-1, j, k);
                int test_up_r = (uint8_t) up_r;
                uint8_t down_r = wrap_around_height(im, i+1, j, k);
                int test_down_r = (uint8_t) down_r;
                int dx = abs(left_r - right_r);
                int dy = abs(up_r - down_r);
                total += pow(dx,2) + pow(dy,2);
            }
            int final_energy = sqrt(total)/10;
            energy = (uint8_t) final_energy;
            set_pixel(*grad, i, j, energy, energy, energy);
            total = 0;
        }
    }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr){
    int height = grad->height;
    int width = grad->width;

    //allocate memeory for best_arr
    *best_arr = (double*)malloc(height*width*sizeof(double));

    //first row stays the same
    for(int i = 0; i < width; i++){
        (*best_arr)[i] = get_pixel(grad, 0, i, 0);
    }

    for(int i = 1; i < height; i++){
        for(int j = 0; j < width; j++){
            int ind_above = (i-1)*width+j;
            // (*best_arr)[i*width + j] = get_pixel(grad, i, j, 0); //the first element of the row
            double min_val = (*best_arr)[ind_above];
            
            //run if it's not the first element of the row
            if (ind_above % width != 0){ 
                if((*best_arr)[ind_above - 1] < min_val){
                    min_val = (*best_arr)[ind_above -1 ];
                }
            }

            //run if it's not the last element of the row
            if (ind_above % width != (width - 1)) { 
                if((*best_arr)[ind_above + 1] < min_val){
                    min_val = (*best_arr)[ind_above + 1];
                }
            }

            (*best_arr)[i*width+j] = min_val + get_pixel(grad, i, j, 0);
        }
    }
}

void recover_path(double *best, int height, int width, int **path) {
    // Allocate memory for path
    *path = malloc(height * sizeof(int));
    int min_idx = 0;
    double min_val = best[(height-1)*width];
    
    // Find the index with the minimum value in the last row of best
    for (int j = 1; j < width; j++) {
        if (best[(height-1)*width+j] < min_val) {
            min_idx = j;
            min_val = best[(height-1)*width+j];
        }
    }
    
    (*path)[height-1] = min_idx;
    
    // Traverse the matrix from bottom to top to recover the path
    for (int i = height-2; i >= 0; i--) {
        int j = (*path)[i+1];
        
        // Check for boundary cases
        if (j == 0) {
            if (best[i*width+j] < best[i*width+j+1]) {
                (*path)[i] = j;
            } else {
                (*path)[i] = j+1;
            }
        } else if (j == width-1) {
            if (best[i*width+j-1] < best[i*width+j]) {
                (*path)[i] = j-1;
            } else {
                (*path)[i] = j;
            }
        } else {
            double left = best[i*width+j-1];
            double center = best[i*width+j];
            double right = best[i*width+j+1];
            double min = center;
            
            if (left < min) {
                min = left;
                (*path)[i] = j-1;
            }
            if (right < min) {
                min = right;
                (*path)[i] = j+1;
            }
            if (center == min) {
                (*path)[i] = j;
            }
        }
    }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path) {
    int height = src->height;
    int width = src->width - 1;

    // create destination image
    create_img(dest, height, width);

    // copy pixels from source image to destination image, skipping pixels in path
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int index = i * width + j;
            int src_index = i * src->width + (j + (path[i] <= j)); // skip pixel if it's in path
            (*dest)->raster[index * 3] = src->raster[src_index * 3];
            (*dest)->raster[index * 3 + 1] = src->raster[src_index * 3 + 1];
            (*dest)->raster[index * 3 + 2] = src->raster[src_index * 3 + 2];
        }
    }
}

void print_best_array(double *best_arr, int height, int width) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printf("%.2f ", best_arr[i*width+j]);
        }
        printf("\n");
    }
}

void print_seam(int *path, int height) {
    for (int i = 0; i<height; i++) {
        printf("%d, ", path[i]);
    }
}


// int main() {
//     struct rgb_img *im;
//     struct rgb_img *cur_im;
//     struct rgb_img *grad;
//     double *best;
//     int *path;

//     read_in_img(&im, "6x5.bin");
//     calc_energy(im,  &grad);
//     dynamic_seam(grad, &best);
//     recover_path(best, grad->height,grad->width, &path);
//     print_grad(grad);
//     print_best_array(best, im->height, im->width);
//     print_seam(path, im->height);
    
    // for(int i = 0; i < 5; i++){
    //     printf("i = %d\n", i);
    //     calc_energy(im,  &grad);
    //     dynamic_seam(grad, &best);
    //     recover_path(best, grad->height, grad->width, &path);
    //     remove_seam(im, &cur_im, path);

    //     // cv::Mat img(cur_im->height, cur_im->width, CV_8UC3, cur_im->raster);
    //     // cv::imshow("Seam Carving", img);
    //     // cv::waitKey(0);


    //     char filename[200];
    //     sprintf(filename, "img%d.bin", i);
    //     write_img(cur_im, filename);


    //     destroy_image(im);
    //     destroy_image(grad);
    //     free(best);
    //     free(path);
    //     im = cur_im;
    // }
    // destroy_image(im);
    
    // return 0;
// }






