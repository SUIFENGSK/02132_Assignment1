// To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
// To run (linux/mac): ./main.out example.bmp example_inv.bmp

// To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
// To run (win): main.exe example.bmp example_inv.bmp
//
// Shuokai
// cd "g:\OneDrive_PRIVAT\OneDrive\Uni_DTU\3.semester\02132 Computersystemer E22\02132_workspace\02132_Assignment1\" ; if ($?) { gcc cbmp.c  main.c -o main } ; if ($?) { .\main example.bmp example_out.bmp}
// cd "d:\OneDrive_PRIVAT\OneDrive\Uni_DTU\3.semester\02132 Computersystemer E22\02132_workspace\02132_Assignment1\" ; if ($?) { gcc cbmp.c  main.c -o main } ; if ($?) { .\main example.bmp example_out.bmp}

// Mathias
// cd "C:\Users\mathi\Documents\DTU\Computersystemer\Assignment 1\02132_Assignment1" ; if ($?) { gcc cbmp.c  main.c -o main } ; if ($?) { .\main example.bmp example_inv.bmp}

/* Imports */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "cbmp.h"
#include <math.h>

/* Macro definitions */
#define THRESHOLD 90
#define BMP_SIZE 950
#define DETECTION_FRAME 14
#define CELLS_MAX 300
#define CROSS_SIZE 5
#define SE_SIZE 5

/* Prototypes */
void convert_RGB_to_GS_and_apply_BT(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char (*output_image_buffer)[BMP_SIZE]);
void convert_2dim_to_3dim(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);
void erode_image(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char (*output_image_buffer)[BMP_SIZE]);
unsigned char check_white_points(unsigned char (*output_image_buffer)[BMP_SIZE]);
void swap_arrays(unsigned char (**arr1)[BMP_SIZE], unsigned char (**arr2)[BMP_SIZE]);
void detect_cells(unsigned char (*input_image_buffer)[BMP_SIZE]);
void draw_cross_and_print_results(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned int (*cells_pos_p)[2], unsigned char print_coordinates);
void run_detection(char *input_image_path, char *output_image_path, int print_coordinates);
void print_test(char *arg, char *arg2);
unsigned char otsu(unsigned short int (*input_image_buffer)[BMP_SIZE]);

// Used for testing
FILE *output;

// Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned int cells_pos[CELLS_MAX][2]; // only x and y

unsigned char image1[BMP_SIZE][BMP_SIZE];
unsigned char image2[BMP_SIZE][BMP_SIZE];

unsigned char (*output_image_buffer)[BMP_SIZE] = image1;
unsigned char (*output_image_buffer_temp)[BMP_SIZE] = image2;
unsigned int (*cells_pos_p)[2] = cells_pos;

unsigned int detected_cells = 0;

// Main function
int main(int argc, char **argv)
{
  /* 
  argc counts how many arguments are passed
  argv[0]: Name of the program
  argv[1]: First line argument (either the input image or "test")
  argv[2]: Second line argument (either the output image or the difficulty)
  argv[3]: Third (optional) line argument to print everything (used for testing) 
  */

  // Checking that either 2 or 3 arguments are passed
  if (argc < 2 || argc > 4)
  {
    fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
    exit(1);
  }

  // If the first argument is "test", run cell detection on all images in a folder
  if (strcmp(argv[1], "test") == 0)
  {
    // Output file name
    char text_file_name[40];
    snprintf(text_file_name, sizeof text_file_name, "%s-test-output.txt", argv[2]);

    // Write output to text_file_name
    output = fopen(text_file_name, "w");
    if (output == NULL)
    {
      printf("Could not open output file");
      return 0;
    }

    // Determine whether to print coordinates or not
    unsigned char *print_coordinates = argv[3] != NULL ? argv[3] : "";
    
    print_test(argv[2], print_coordinates);
  }
  else
  {
    // Write output to console
    output = stdout;
    run_detection(argv[1], argv[2], 1); // 1 = print coordinates and output image
  }

  
  fclose(output);
  return 0;
}

void run_detection(char *input_image_path, char *output_image_path, int print_coordinates) {
  
  // Time execution
  clock_t start, end;
  double cpu_time_used;
  start = clock();

  // Load image from file
  read_bitmap(input_image_path, input_image);

  // Run conversion to greyscale and apply binary threshold
  convert_RGB_to_GS_and_apply_BT(input_image, output_image_buffer);

  
  while (check_white_points(output_image_buffer))
  {
    erode_image(output_image_buffer, output_image_buffer_temp);
    swap_arrays(&output_image_buffer, &output_image_buffer_temp);
    detect_cells(output_image_buffer);
  }

  // If print_coordinates is true, then print out all the coordinates for detected cells and save the image to a file
  draw_cross_and_print_results(input_image, cells_pos_p, print_coordinates);
  if (print_coordinates)
  {
    write_bitmap(input_image, output_image_path);
  }

  fprintf(output, "Found %d cells\n", detected_cells);
  fprintf(output, "Done!\n");

  // Stop timing
  end = clock();
  cpu_time_used = end - start;

  fprintf(output, "Total time: %f ms\n\n\n", cpu_time_used * 1000.0 / CLOCKS_PER_SEC);
}

// Runs program on all images in the folder (used for testing purposes)
void print_test(char *arg, char *arg2)
{
  unsigned char no_images;
  unsigned char print_coordinates;

  no_images = arg != "impossible" ? 10 : 5;
  print_coordinates = strcmp(arg2, "all") == 0 ? 1 : 0; // If second argument is "all", print all coordinates to output

  for (int i = 1; i <= no_images; i++)
  {
    // Resets variables
    detected_cells = 0;
    memset(cells_pos, 0, sizeof(cells_pos));

    fprintf(output, "Image %d%s.\n", i, arg);

    // Create input image file path
    char input_file_path[40];
    snprintf(input_file_path, sizeof input_file_path, "samples/%s/%d%s.bmp", arg, i, arg);

    // Create output image file path
    char output_image_path[40];
    snprintf(output_image_path, sizeof output_image_path, "%d%s-output.bmp", i, arg);

    // Run detection on the i'th image
    run_detection(input_file_path, output_image_path, print_coordinates);
  
  }
}

// Greyscale conversion and binary threshold
void convert_RGB_to_GS_and_apply_BT(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char (*output_image_buffer)[BMP_SIZE])
{
  unsigned short int img_temp[BMP_SIZE][BMP_SIZE];
  unsigned short int(*img_temp_p)[BMP_SIZE] = img_temp;

  // Convert to greyscale
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      // Use multiplication to avoid division by 3. (2^(n-bit)=3*factor)
      img_temp_p[x][y] = ((input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) * 86) >> 8;
    }
  }

  // Apply Otsu thresholding
  unsigned char new_threshold = otsu(img_temp_p);

  // Apply binary threshold
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      output_image_buffer[x][y] = img_temp_p[x][y] > new_threshold ? 255 : 0;
    }
  }
}

// Helper function. Convert 2-dimensional array to 3-dimensional array
void convert_2dim_to_3dim(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      output_image[x][y][0] = input_image_buffer[x][y];
      output_image[x][y][1] = input_image_buffer[x][y];
      output_image[x][y][2] = input_image_buffer[x][y];
    }
  }
}

// Erodes image
void erode_image(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char (*output_image_buffer)[BMP_SIZE])
{
  // Structuring element (1's are the pixels that will get checked)
    unsigned char se[SE_SIZE][SE_SIZE] = {
    {0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 1, 1, 1, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0}
  };

  // Offset for structuring element
  unsigned char offset = SE_SIZE / 2;

  // Loop through image
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {

      // If the pixel is white, check it. If not paint output image black
      if (input_image_buffer[x][y] == 255)
      {

        // Loop through the structuring element
        for (int se_x = 0; se_x < SE_SIZE; se_x++)
        {
          for (int se_y = 0; se_y < SE_SIZE; se_y++)
          {

            // Checks if the pixels around the pixel in question is within the image
            if (((x - offset) + se_x >= 0 && (x - offset) + se_x < BMP_WIDTH) && ((y - offset) + se_y >= 0 && (y - offset) + se_y < BMP_HEIGTH))
            {

              // If structuring element is 1, and the corresponding pixel is black, paint the pixel black and continue
              if (se[se_x][se_y] == 1 && input_image_buffer[(x - offset) + se_x][(y - offset) + se_y] == 0)
              {

                output_image_buffer[x][y] = 0;
                goto breakout;
              }
            }

            // If the nested if-statements didn't go to breakout, copy pixel
            output_image_buffer[x][y] = input_image_buffer[x][y];
          }
        }
      }
      else
      {
        output_image_buffer[x][y] = 0;
      }

    breakout:
      continue;
    }
  }
}

// Checks whether there's still white pixels in the image
unsigned char check_white_points(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      if (input_image[x][y] == 255)
      {
        return 1;
      }
    }
  }
}

// Helper function. Swaps two arrays
void swap_arrays(unsigned char (**arr1)[BMP_SIZE], unsigned char (**arr2)[BMP_SIZE])
{
  unsigned char(*temp)[BMP_SIZE];
  temp = *arr1;
  *arr1 = *arr2;
  *arr2 = temp;
}

// Detects cells that fulfill detection criteria
void detect_cells(unsigned char (*input_image_buffer)[BMP_SIZE])
{

  // Detection frame used for detecting cells
  unsigned char df[DETECTION_FRAME][DETECTION_FRAME] = {
    {0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0}
  };

  unsigned char offset = DETECTION_FRAME / 2;
  unsigned char invalid;

  // Loop through every pixel in the image
  for (int image_x = 0; image_x < BMP_WIDTH; image_x++)
  {
    for (int image_y = 0; image_y < BMP_HEIGTH; image_y++)
    {
      // Reset invalid variable
      invalid = 0;

      // If the pixel is white, check if it fulfills detection frame
      if (input_image_buffer[image_x][image_y] == 255)
      {

        // Loop through detection frame
        for (int df_x = 0; df_x < DETECTION_FRAME; df_x++)
        {
          for (int df_y = 0; df_y < DETECTION_FRAME; df_y++)
          {

            // Checks whether the border pixels is within the image
            if (((image_x - offset) + df_x >= 0 && (image_x - offset) + df_x < BMP_WIDTH) && ((image_y - offset) + df_y >= 0 && (image_y - offset) + df_y < BMP_HEIGTH))
            {

              // If the detection frame element is 1, and the pixel in the image is white, the cell is invalid
              if (df[df_x][df_y] == 1 && input_image_buffer[(image_x - offset) + df_x][(image_y - offset) + df_y] == 255)
              {
                invalid = 1;
                goto breakout;
              }
            }
          }
        }

      // Used to breakout of the nested for-loops
      breakout:
        // If the cell wasn't invalid, note it and erase the cell
        if (invalid != 1)
        {
          cells_pos[detected_cells][0] = image_x;
          cells_pos[detected_cells][1] = image_y;
          detected_cells++;
          for (int x = image_x - offset; x <= image_x + offset; x++)
          {
            for (int y = image_y - offset; y <= image_y + offset; y++)
            {
              if ((x >= 0 && x < BMP_WIDTH) && (y >= 0 && y < BMP_HEIGTH))
              {
                input_image_buffer[x][y] = 0;
              }
            }
          }
        }
      }
    }
  }
}

void draw_cross_and_print_results(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned int (*cells_pos_p)[2], unsigned char print_coordinates)
{
  // Loop through every detected cell
  for (int i = 0; i < detected_cells; i++)
  {
    int pos_x = cells_pos_p[i][0];
    int pos_y = cells_pos_p[i][1];

    if (print_coordinates != 0)
    {
      fprintf(output, "Nr.%d : [%d,%d]\n", i + 1, pos_x, pos_y);
    }

    // Draw red cross
    // Start with x-direction
    for (int x = -CROSS_SIZE; x <= CROSS_SIZE; x++)
    {
      if (pos_x + x > 0 && pos_x + x < BMP_WIDTH)
      {
        // Draw red x-line
        input_image[pos_x + x][pos_y][0] = 255;
        input_image[pos_x + x][pos_y][1] = 0;
        input_image[pos_x + x][pos_y][2] = 0;
      }
    }
    // Then y-direction
    for (int y = -CROSS_SIZE; y <= CROSS_SIZE; y++)
    {
      if (pos_y + y > 0 && pos_y + y < BMP_HEIGTH)
      {
        // Draw red y-line
        input_image[pos_x][pos_y + y][0] = 255;
        input_image[pos_x][pos_y + y][1] = 0;
        input_image[pos_x][pos_y + y][2] = 0;
      }
    }
  }
}

// Otsu method
unsigned char otsu(unsigned short int (*input_image_buffer)[BMP_SIZE])
{
  unsigned int histogram[256] = {0};
  unsigned int total_pixel = BMP_WIDTH * BMP_HEIGTH;

  unsigned int n_b = 0; // total background pixel which is less than optimal_threshold
  unsigned int n_f = 0; // total foreground pixel which is greater than optimal_threshold

  float omega_b = 0; // The proportion of pixels in the background to the entire image
  float omega_f = 0; // The proportion of pixels in the foreground to the entire image

  unsigned int sum = 0;   // total img value
  unsigned int sum_b = 0; // total background value
  unsigned int sum_f = 0; // total foreground value

  float mu_b = 0; // average of background value
  float mu_f = 0; // average of foreground value

  unsigned char optimal_threshold = 0;
  float inter_var = 0; // intra-class variance
  float max = 0;

  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      histogram[input_image_buffer[x][y]]++;
    }
  }

  for (int i = 0; i < 256; i++)
  {
    sum += i * histogram[i]; // total img value
  }
  for (int i = 0; i < 256; i++)
  {
    n_f += histogram[i];     // the number of pixels gray value is less than the optimal_threshold in the image (foreground)
    n_b = total_pixel - n_f; // the number of pixels gray value is greater than the optimal_threshold in the image (background)
    omega_f = (float)n_f / total_pixel;
    omega_b = 1 - omega_f;
    sum_f += i * histogram[i]; // total foreground img value
    sum_b = sum - sum_f;       // total background img value
    if (n_f == 0 || n_b == 0)  // 0 check
      continue;
    mu_f = sum_f / n_f;
    mu_b = sum_b / n_b;
    inter_var = omega_f * omega_b * pow(mu_f - mu_b, 2); // ostu formula
    if (inter_var > max)
    {
      max = inter_var;
      optimal_threshold = i;
    }
  }
  return optimal_threshold;
}