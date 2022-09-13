// To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
// To run (linux/mac): ./main.out example.bmp example_inv.bmp

// To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
// To run (win): main.exe example.bmp example_inv.bmp
//
// Shuokai
// cd "g:\OneDrive_PRIVAT\OneDrive\Uni_DTU\3.semester\02132 Computersystemer E22\02132_workspace\02132_Assignment1\" ; if ($?) { gcc cbmp.c  main.c -o main } ; if ($?) { .\main example.bmp example_inv.bmp}
// cd "d:\OneDrive_PRIVAT\OneDrive\Uni_DTU\3.semester\02132 Computersystemer E22\02132_workspace\02132_Assignment1\" ; if ($?) { gcc cbmp.c  main.c -o main } ; if ($?) { .\main example.bmp example_inv.bmp}

// Mathias
// cd "C:\Users\mathi\Documents\DTU\Computersystemer\Assignment 1\02132_Assignment1" ; if ($?) { gcc cbmp.c  main.c -o main } ; if ($?) { .\main example.bmp example_inv.bmp}

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "cbmp.h"

#define THRESHOLD 90
#define BMP_SIZE 950
#define DETECTION_FRAME 25
#define CELLS_MAX 500
#define CROSS_SIZE 5

void convert_RGB_to_GS_and_apply_BT(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char (*output_image_buffer)[BMP_SIZE]);
void convert_2dim_to_3dim(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);
void erode_image(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char (*output_image_buffer)[BMP_SIZE]);
int check_white_points(unsigned char (*output_image_buffer)[BMP_SIZE]);
void swap_arrays(unsigned char (**arr_1)[BMP_SIZE], unsigned char (**arr_2)[BMP_SIZE]);
void copy_array(unsigned char (*arr1)[BMP_SIZE][BMP_CHANNELS], unsigned char (*arr2)[BMP_SIZE][BMP_CHANNELS]);

void detect_cells(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char (*output_image_buffer)[BMP_SIZE]);
// void delete_pixels(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image_temp[BMP_WIDTH][BMP_HEIGTH], unsigned char start_x, unsigned char start_y);

void draw_cross_and_print_results(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned int (*cells_pos_p)[2], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);

// Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned int cells_pos[CELLS_MAX][2]; // only x and y

unsigned char image1[BMP_SIZE][BMP_SIZE];
unsigned char image2[BMP_SIZE][BMP_SIZE];
unsigned char image3[BMP_SIZE][BMP_SIZE];

unsigned char (*output_image_buffer)[BMP_SIZE] = image1;
unsigned char (*output_image_buffer_temp)[BMP_SIZE] = image2;
unsigned char (*output_image_buffer_final)[BMP_SIZE] = image3;
unsigned int (*cells_pos_p)[2] = cells_pos;

// Main function
int main(int argc, char **argv)
{
  // argc counts how may arguments are passed
  // argv[0] is a string with the name of the program
  // argv[1] is the first command line argument (input image)
  // argv[2] is the second command line argument (output image)

  clock_t start, end;
  double cpu_time_used;
  start = clock(); /* The code that has to be measured. */

  // Checking that 2 arguments are passed
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
    exit(1);
  }

  printf("Example program - 02132 - A1\n");

  // Load image from file
  read_bitmap(argv[1], input_image);

  // Run
  convert_RGB_to_GS_and_apply_BT(input_image, output_image_buffer);
  int i = 0;
  // Erode image
  while (check_white_points(output_image_buffer))
  {
    erode_image(output_image_buffer, output_image_buffer_temp);
    swap_arrays(&output_image_buffer, &output_image_buffer_temp);
    detect_cells(output_image_buffer, output_image_buffer_final);
    swap_arrays(&output_image_buffer, &output_image_buffer_final);
    // convert_2dim_to_3dim(output_image_buffer, output_image);
    // char str[100];
    // sprintf(str, "erode_%d.bmp", i);
    // i++;
    // write_bitmap(output_image, str);
  }
  // convert_2dim_to_3dim(output_image_buffer, output_image);

  
  draw_cross_and_print_results(input_image, cells_pos_p, output_image);
  // convert_2dim_to_3dim(output_image_buffer_final, output_image);

  // Save image to file
  write_bitmap(output_image, argv[2]);

  printf("Done!\n");

  end = clock();
  cpu_time_used = end - start;
  printf("Total time: %f ms\n", cpu_time_used * 1000.0 / CLOCKS_PER_SEC);
  return 0;
}

void convert_RGB_to_GS_and_apply_BT(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char (*output_image_buffer)[BMP_SIZE])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      output_image_buffer[x][y] = ((input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) / 3) > THRESHOLD ? 255 : 0;
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
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      // printf("%d%s%d%s%d\n", x, " ", y, " ", input_image[x][y]);
      int pos_left = 0, pos_right = 0, pos_top = 0, pos_down = 0;
      if (x - 1 >= 0 && input_image_buffer[x - 1][y] == 0)
        pos_left = 1;
      if (x + 1 < BMP_WIDTH && input_image_buffer[x + 1][y] == 0)
        pos_right = 1;
      if (y - 1 >= 0 && input_image_buffer[x][y - 1] == 0)
        pos_top = 1;
      if (y + 1 < BMP_HEIGTH && input_image_buffer[x][y + 1] == 0)
        pos_down = 1;
      if (pos_left || pos_right || pos_top || pos_down)
      {
        output_image_buffer[x][y] = 0;
      }
      else
      {
        output_image_buffer[x][y] = input_image_buffer[x][y];
      }
    }
  }
}

int check_white_points(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH])
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

void swap_arrays(unsigned char (**arr_1)[BMP_SIZE], unsigned char (**arr_2)[BMP_SIZE])
{
  unsigned char(*temp)[BMP_SIZE];
  temp = *arr_1;
  *arr_1 = *arr_2;
  *arr_2 = temp;
}

void detect_cells(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char (*output_image_buffer)[BMP_SIZE])
{
  printf("%s", "Start!\n");
  unsigned char frame[DETECTION_FRAME][DETECTION_FRAME];
  unsigned char radius = DETECTION_FRAME / 2;

  unsigned int detected_cells = 0;

  unsigned char invalid = 0;
  unsigned char found_white = 0;

  for (int image_x = 0; image_x < BMP_WIDTH; image_x++) {
    for (int image_y = 0; image_y < BMP_HEIGTH; image_y++) {
      // Copy input image pixel over to output image
      output_image_buffer[image_x][image_y] = input_image_buffer[image_x][image_y];
    }
  }


  for (int image_x = 0; image_x < BMP_WIDTH; image_x++) {
    for (int image_y = 0; image_y < BMP_HEIGTH; image_y++) {
      
      invalid = 0;

      if (output_image_buffer[image_x][image_y] == 255) {

        for (int border_x = image_x - radius; border_x <= image_x + radius; border_x++) {

          if ((border_x >= 0) && output_image_buffer[border_x][image_y - radius] != 0) {
            invalid = 1;
          } else if ((border_x < BMP_HEIGTH) && output_image_buffer[border_x][image_y + radius] != 0) {
            invalid = 1;
          }

        }

        for (int border_y = image_y - radius; border_y <= image_y + radius; border_y++ ) {
          if ((border_y >= 0) && output_image_buffer[image_x - radius][border_y] != 0) {
            invalid = 1;
          } else if ((border_y < BMP_WIDTH) && output_image_buffer[image_x + radius][border_y] != 0) {
            invalid = 1;
          }
        }

        if (invalid != 1) {
          cells_pos[detected_cells][0] = image_x;
          cells_pos[detected_cells][1] = image_y;
          detected_cells++;
          // printf("x: %d\ny: %d\n\n", image_x, image_y);


          for (int x = image_x - radius; x <= image_x + radius; x++) {
            for (int y = image_y - radius; y <= image_y + radius; y++) {
              output_image_buffer[x][y] = 0;
            }
          }

        }
      }
    }
  }

  printf("Found %d cells\n", detected_cells);
}

void copy_array(unsigned char (*arr1)[BMP_SIZE][BMP_CHANNELS], unsigned char (*arr2)[BMP_SIZE][BMP_CHANNELS])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      for (int z = 0; z < BMP_CHANNELS; z++)
      {
        arr2[x][y][z] = arr1[x][y][z];
      }
    }
  }
}

void draw_cross_and_print_results(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned int (*cells_pos_p)[2], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS])
{
  copy_array(input_image, output_image);
  // test drawing
  // cells_pos_p[0][0]=100;
  // cells_pos_p[0][1]=100;
  for (int i = 0; i < CELLS_MAX; i++)
  {
    int pos_x = cells_pos_p[i][0];
    int pos_y = cells_pos_p[i][1];
    if (pos_x == 0 && pos_y == 0)
    {
      printf("%s\n", "jump");
      break;
    }
    else
    {
      printf("Nr.%d : [%d,%d]\n", i + 1, pos_x, pos_y);
    }
    // draw red cross
    // start x-direction
    for (int x = -CROSS_SIZE; x <= CROSS_SIZE; x++)
    {
      if (pos_x + x > 0 && pos_x + x < BMP_WIDTH)
      {
        // draw red x-line
        output_image[pos_x + x][pos_y][0] = 255;
        output_image[pos_x + x][pos_y][1] = 0;
        output_image[pos_x + x][pos_y][2] = 0;
      }
    }
    // Then y-direction
    for (int y = -CROSS_SIZE; y <= CROSS_SIZE; y++)
    {
      if (pos_y + y > 0 && pos_y + y < BMP_HEIGTH)
      {
        // draw red y-line
        output_image[pos_x][pos_y + y][0] = 255;
        output_image[pos_x][pos_y + y][1] = 0;
        output_image[pos_x][pos_y + y][2] = 0;
      }
    }
  }
}