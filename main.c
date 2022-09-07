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
#include "cbmp.h"

#define THRESHOLD 90
#define BMP_SIZE 950
#define DETECTION_FRAME 25

void convert_RGB_to_GS_and_apply_BT(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char (*output_image_buffer)[BMP_SIZE]);
void convert_3dim_to_2dim(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);
void erode_image(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char (*output_image_buffer)[BMP_SIZE]);
int check_white_points(unsigned char (*output_image_buffer)[BMP_SIZE]);
void swap_arrays(unsigned char (**arr_1)[BMP_SIZE], unsigned char (**arr_2)[BMP_SIZE]);

void detect_cells(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char (*output_image_buffer)[BMP_SIZE]);
// void delete_pixels(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image_temp[BMP_WIDTH][BMP_HEIGTH], unsigned char start_x, unsigned char start_y);

// Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

unsigned char image1[BMP_SIZE][BMP_SIZE];
unsigned char image2[BMP_SIZE][BMP_SIZE];
unsigned char image3[BMP_SIZE][BMP_SIZE];

unsigned char (*output_image_buffer)[BMP_HEIGTH] = image1;
unsigned char (*output_image_buffer_temp)[BMP_HEIGTH] = image2;
unsigned char (*output_image_buffer_final)[BMP_HEIGTH] = image3;


// Main function
int main(int argc, char **argv)
{
  // argc counts how may arguments are passed
  // argv[0] is a string with the name of the program
  // argv[1] is the first command line argument (input image)
  // argv[2] is the second command line argument (output image)

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
  // while (check_white_points(output_image_buffer))
  // {
  //   erode_image(output_image_buffer, output_image_buffer_temp);
  //   swap_arrays(&output_image_buffer, &output_image_buffer_temp);
  //   convert_3dim_to_2dim(output_image_buffer, output_image);
  //   char str[100];
  //   sprintf(str, "erode_%d.bmp", i);
  //   i++;
  //   write_bitmap(output_image, str);
  // }
  // convert_3dim_to_2dim(output_image_buffer, output_image);

  detect_cells(output_image_buffer, output_image_buffer_final);
  convert_3dim_to_2dim(output_image_buffer_final, output_image);

  // Save image to file
  write_bitmap(output_image, argv[2]);

  printf("Done!\n");
  return 0;
}

void convert_RGB_to_GS_and_apply_BT(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char (*output_image_buffer)[BMP_SIZE])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      output_image_buffer[x][y] = ((input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) / 3) > THRESHOLD ? 255 : 0;
      ;
    }
  }
}
void convert_3dim_to_2dim(unsigned char (*input_image_buffer)[BMP_SIZE], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS])
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
  printf("%s", "start");
  unsigned char frame[DETECTION_FRAME][DETECTION_FRAME];
  unsigned char radius = DETECTION_FRAME / 2;

  unsigned char detected_cells = 0;

  unsigned char invalid = 0;
  unsigned char found_white = 0;

  for (int image_x = 0; image_x < BMP_WIDTH; image_x++)
  {
    for (int image_y = 0; image_y < BMP_HEIGTH; image_y++)
    {
      // Copy input image pixel over to output image
      output_image_buffer[image_x][image_y] = input_image_buffer[image_x][image_y];

      // Reset invalid-boolean
      invalid = 0;

      // Loop through every x-coordinate
      for (int x = image_x - radius; x < image_x + radius; x++)
      {

        // If previous pixel was invalid, break out of loop
        if (invalid)
        {
          break;
        }

        // Loop through every y-coordinate
        for (int y = image_y - radius; y < image_y + radius; y++)
        {

          // If a white pixel is found on the border (outer pixel) of the detection frame, then the cell is invalid
          if ((x == 0 || x == radius - 1 || y == 0 || y == radius - 1) && input_image_buffer[x][y] != 0)
          {
            invalid = 1;
            break;
          }
          else if ((x != 0 && y != 0) && input_image_buffer[x][y] != 0)
          { // Else, if a white cell is found within the detection frame, note it
            found_white = 1;
            continue;
          }
        }
      }

      // If the previous loop wasn't invalid and it found atleast one white pixel, then make every pixel within the frame black,
      // and add one to amount of detected cells
      if (!invalid && found_white)
      {
        for (int x = image_x - radius; x < image_x + radius; x++)
        {
          for (int y = image_y - radius; y < image_y + radius; y++)
          {
            output_image_buffer[x][y] = 0;
          }
        }
        detected_cells++;
      }
    }
  }
  printf("Found %d cells", detected_cells);
}
