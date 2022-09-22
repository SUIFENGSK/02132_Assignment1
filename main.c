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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "cbmp.h"
#include <math.h>

#define THRESHOLD 90
#define BMP_SIZE 950
#define DETECTION_FRAME 14
#define CELLS_MAX 300
#define CROSS_SIZE 5

// Size of structuring element
#define SE_SIZE 5

void convert_RGB_to_GS_and_apply_BT(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char (*output_image_buffer)[BMP_SIZE]);
void convert_2dim_to_3dim(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);
void erode_image(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char (*output_image_buffer)[BMP_SIZE]);
unsigned char check_white_points(unsigned char (*output_image_buffer)[BMP_SIZE]);
void swap_arrays(unsigned char (**arr1)[BMP_SIZE], unsigned char (**arr2)[BMP_SIZE]);
void detect_cells(unsigned char (*input_image_buffer)[BMP_SIZE]);
void draw_cross_and_print_results(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned int (*cells_pos_p)[2], unsigned char print_coordinates);
void print_test(char *arg, char *arg2);
unsigned char otsu(unsigned short int (*input_image_buffer)[BMP_SIZE]);

// Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned int cells_pos[CELLS_MAX][2]; // only x and y

// Used for testing
FILE *output;

unsigned char image1[BMP_SIZE][BMP_SIZE];
unsigned char image2[BMP_SIZE][BMP_SIZE];

unsigned char (*output_image_buffer)[BMP_SIZE] = image1;
unsigned char (*output_image_buffer_temp)[BMP_SIZE] = image2;
unsigned int (*cells_pos_p)[2] = cells_pos;

unsigned int detected_cells = 0;

// Main function
int main(int argc, char **argv)
{
  // argc counts how may arguments are passed
  // argv[0] is a string with the name of the program
  // argv[1] is the first command line argument (input image)
  // argv[2] is the second command line argument (output image)
  // argv[3] is the third (optional) command line argument (test text file)

  // Checking that 2 or 3 arguments are passed
  if (argc < 2 || argc > 4)
  {
    fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
    exit(1);
  }

  // If a third argument is passed, generate a test file with all the inputs
  if (strcmp(argv[1], "test") == 0)
  {
    char text_file_name[40];
    snprintf(text_file_name, sizeof text_file_name, "%s-test-output.txt", argv[2]);

    output = fopen(text_file_name, "w");
    if (output == NULL)
    {
      printf("Could not open output file");
      return 0;
    }

    unsigned char *print_coordinates = argv[3] != NULL ? argv[3] : "";

    print_test(argv[2], print_coordinates);
    return 0;
  }
  else
  {
    output = stdout;
  }

  clock_t start, end;
  double cpu_time_used;
  start = clock(); /* The code that has to be measured. */

  fprintf(stdout, "Program start!\n");

  // Load image from file
  read_bitmap(argv[1], input_image);

  // Run
  convert_RGB_to_GS_and_apply_BT(input_image, output_image_buffer);

  // int i = 0;
  // Erode image
  while (check_white_points(output_image_buffer))
  {
    erode_image(output_image_buffer, output_image_buffer_temp);
    swap_arrays(&output_image_buffer, &output_image_buffer_temp);
    detect_cells(output_image_buffer);
    // convert_2dim_to_3dim(output_image_buffer, output_image);
    // char str[100];
    // sprintf(str, "erode_%d.bmp", i);
    // i++;
    // write_bitmap(output_image, str);
  }
  draw_cross_and_print_results(input_image, cells_pos_p, 1); // 1 = print coordinates

  // Save image to file
  write_bitmap(input_image, argv[2]);
  fprintf(output, "Found %d cells\n", detected_cells);
  fprintf(output, "Done!\n");

  end = clock();
  cpu_time_used = end - start;
  fprintf(output, "Total time: %f ms\n", cpu_time_used * 1000.0 / CLOCKS_PER_SEC);
  fclose(output);
  return 0;
}

// Runs program on all images in the folder (used for testing purposes)
void print_test(char *arg, char *arg2)
{

  unsigned char no_images;
  unsigned char print_coordinates;

  no_images = arg != "impossible" ? 10 : 5;
  print_coordinates = strcmp(arg2, "all") == 0 ? 1 : 0; // If second argument is "all", print all coordinates
  fprintf(stdout, "%d", print_coordinates);

  for (int i = 1; i <= no_images; i++)
  {

    // Resets variables
    detected_cells = 0;
    memset(cells_pos, 0, sizeof(cells_pos));

    fprintf(output, "Image %d%s.\n", i, arg);

    clock_t start, end;
    double cpu_time_used;
    start = clock(); /* The code that has to be measured. */

    char file_path[40];
    snprintf(file_path, sizeof file_path, "samples/%s/%d%s.bmp", arg, i, arg);

    // Load image from file
    read_bitmap(file_path, input_image);

    // Grey scale and binary thresholding
    convert_RGB_to_GS_and_apply_BT(input_image, output_image_buffer);

    // Erode image and detect cells
    while (check_white_points(output_image_buffer))
    {
      erode_image(output_image_buffer, output_image_buffer_temp);
      swap_arrays(&output_image_buffer, &output_image_buffer_temp);
      detect_cells(output_image_buffer);
    }
    draw_cross_and_print_results(input_image, cells_pos_p, print_coordinates);

    // Output image name
    char output_image_name[40];
    snprintf(output_image_name, sizeof output_image_name, "%d%s-output.bmp", i, arg);

    // Save image to file
    if (print_coordinates)
    {
      write_bitmap(input_image, output_image_name);
    }

    fprintf(output, "Found %d cells\n", detected_cells);
    fprintf(output, "Done!\n");

    end = clock();
    cpu_time_used = end - start;
    fprintf(output, "Total time for image %d%s: %f ms\n\n\n\n", i, arg, cpu_time_used * 1000.0 / CLOCKS_PER_SEC);
  }

  fclose(output);
}

void convert_RGB_to_GS_and_apply_BT(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char (*output_image_buffer)[BMP_SIZE])
{
  unsigned short int img_temp[BMP_SIZE][BMP_SIZE];
  unsigned short int(*img_temp_p)[BMP_SIZE] = img_temp;
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      // Use multiplying to avoid division by 3. 2^(n-bit)=3*factor
      img_temp_p[x][y] = ((input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) * 86) >> 8;
      //img_temp_p[x][y] = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) / 3;
    }
  }
  // Apply Otsu thresholding
  unsigned char new_threshold = otsu(img_temp_p);
  printf("%s%d\n", "newTS: ", new_threshold);
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      output_image_buffer[x][y] = img_temp_p[x][y] > new_threshold ? 255 : 0;
    }
  }
}
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

void erode_image(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char (*output_image_buffer)[BMP_SIZE])
{
  // Structuring element (1's are the pixels that will get checked)
  //   unsigned char se[SE_SIZE][SE_SIZE] = {
  //   {0, 0, 0, 0, 0},
  //   {0, 0, 1, 0, 0},
  //   {0, 1, 1, 1, 0},
  //   {0, 0, 1, 0, 0},
  //   {0, 0, 0, 0, 0}
  // };
  unsigned char se[SE_SIZE][SE_SIZE] = {
    {0, 0, 0, 0, 0},
    {0, 1, 1, 1, 0},
    {0, 1, 1, 1, 0},
    {0, 1, 1, 1, 0},
    {0, 0, 0, 0, 0}
  };

  unsigned char offset = SE_SIZE / 2;

  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {

      if (input_image_buffer[x][y] == 255)
      {

        for (int se_x = 0; se_x < SE_SIZE; se_x++)
        {
          for (int se_y = 0; se_y < SE_SIZE; se_y++)
          {

            // printf("x: %d, y: %d\n", x-offset+se_x, y-offset+se_y);

            if (((x - offset) + se_x >= 0 && (x - offset) + se_x < BMP_WIDTH) && ((y - offset) + se_y >= 0 && (y - offset) + se_y < BMP_HEIGTH))
            {

              if (se[se_x][se_y] == 1 && input_image_buffer[(x - offset) + se_x][(y - offset) + se_y] == 0)
              {

                output_image_buffer[x][y] = 0;
                goto breakout;
              }
            }

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

void swap_arrays(unsigned char (**arr1)[BMP_SIZE], unsigned char (**arr2)[BMP_SIZE])
{
  unsigned char(*temp)[BMP_SIZE];
  temp = *arr1;
  *arr1 = *arr2;
  *arr2 = temp;
}

void detect_cells(unsigned char (*input_image_buffer)[BMP_SIZE])
{

  // Detection frame (1's are the pixels that will be checked)
  // unsigned char df[DETECTION_FRAME][DETECTION_FRAME] = {
  //     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  //     {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  //     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
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

  for (int image_x = 0; image_x < BMP_WIDTH; image_x++)
  {
    for (int image_y = 0; image_y < BMP_HEIGTH; image_y++)
    {
      invalid = 0;

      if (input_image_buffer[image_x][image_y] == 255)
      {

        for (int df_x = 0; df_x < DETECTION_FRAME; df_x++)
        {
          for (int df_y = 0; df_y < DETECTION_FRAME; df_y++)
          {

            if (((image_x - offset) + df_x >= 0 && (image_x - offset) + df_x < BMP_WIDTH) && ((image_y - offset) + df_y >= 0 && (image_y - offset) + df_y < BMP_HEIGTH))
            {

              if (df[df_x][df_y] == 1 && input_image_buffer[(image_x - offset) + df_x][(image_y - offset) + df_y] == 255)
              {

                invalid = 1;
                goto breakout;
              }
            }
          }
        }

      breakout:
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
  for (int i = 0; i < detected_cells; i++)
  {
    int pos_x = cells_pos_p[i][0];
    int pos_y = cells_pos_p[i][1];

    if (print_coordinates != 0)
    {
      fprintf(output, "Nr.%d : [%d,%d]\n", i + 1, pos_x, pos_y);
    }
    // draw red cross
    // start x-direction
    for (int x = -CROSS_SIZE; x <= CROSS_SIZE; x++)
    {
      if (pos_x + x > 0 && pos_x + x < BMP_WIDTH)
      {
        // draw red x-line
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
        // draw red y-line
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
  float var = 0;
  float max = 0;

  printf("%s\n", "start otsu");

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
    var = omega_f * omega_b * pow(mu_f - mu_b, 2); // ostu formula
    if (var > max)
    {
      max = var;
      optimal_threshold = i;
    }
  }
  return optimal_threshold;
}