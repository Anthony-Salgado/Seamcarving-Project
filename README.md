# Seamcarving-Project
# Seam Carving Algorithm
## Authors: Anthony Salgado & Harry Nguyen
## Date Completed: April 9, 2023
## Overview
This project implements the seam carving algorithm, which is used for content-aware image resizing. Seam carving removes or inserts seams of pixels in an image, effectively resizing the image while preserving important content.

### Features:
- Calculates energy of the image based on pixel gradients.
- Uses dynamic programming to find the least energy seam to remove.
- Handles wrap-around edge cases for pixels at the image borders.
- Seam removal for reducing image size.

## Code Structure

### Functions

1. **wrap_around_height(struct rgb_img *im, int i, int j, int k)**:
   - Handles pixel wrapping around the image’s height.
   - Fetches a pixel value at coordinates `(i, j)` for the channel `k`. If `i` is out of bounds, it wraps around the image height.

2. **wrap_around_width(struct rgb_img *im, int i, int j, int k)**:
   - Similar to `wrap_around_height()`, but wraps around the image width.

3. **calc_energy(struct rgb_img *im, struct rgb_img **grad)**:
   - Calculates the energy map of the image using the difference in pixel values between neighboring pixels (both horizontally and vertically).
   - The result is stored as a grayscale image representing the energy.

4. **dynamic_seam(struct rgb_img *grad, double **best_arr)**:
   - Uses dynamic programming to find the seam with the least energy.
   - The function stores the cumulative minimum energy at each pixel, considering the seam path from the top to the current pixel.

5. **recover_path(double *best, int height, int width, int **path)**:
   - Recovers the optimal seam path from the bottom of the image to the top using the `best` array.
   - The result is an array of column indices representing the seam path in each row.

6. **remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path)**:
   - Removes the identified seam from the image.
   - A new image is created, copying all pixels except those on the seam.

7. **print_best_array(double *best_arr, int height, int width)**:
   - Utility function to print the best seam energy array (for debugging).

8. **print_seam(int *path, int height)**:
   - Utility function to print the identified seam path (for debugging).

## How to Compile
Make sure to include the required header files and libraries when compiling. Example using `gcc`:

```bash
gcc -o seam_carving seam_carving.c -lm

## How to Run

1. Load an image in binary format using `read_in_img()`.
2. Calculate the energy of the image using `calc_energy()`.
3. Use `dynamic_seam()` to find the optimal seam to remove.
4. Call `recover_path()` to get the seam path and then remove it using `remove_seam()`.

Example main function (commented out in the provided code):

```c
int main() {
    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    // Load input image
    read_in_img(&im, "6x5.bin");

    // Calculate energy
    calc_energy(im, &grad);

    // Find the dynamic seam
    dynamic_seam(grad, &best);

    // Recover the optimal seam path
    recover_path(best, grad->height, grad->width, &path);

    // Remove the seam from the image
    remove_seam(im, &cur_im, path);

    // Save the new image
    write_img(cur_im, "output.bin");

    // Free resources
    destroy_image(im);
    destroy_image(grad);
    free(best);
    free(path);

    return 0;
}

Run this in your C development environment and ensure that all dependencies (like the c_img and seamcarving headers) are correctly set up.
