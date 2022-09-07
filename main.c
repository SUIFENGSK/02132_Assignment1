// To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
// To run (linux/mac): ./main.out example.bmp example_inv.bmp

// To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
// To run (win): main.exe example.bmp example_inv.bmp
// 
// Shuokai
// cd "d:\OneDrive_PRIVAT\OneDrive\Uni_DTU\3.semester\02132 Computersystemer E22\02132_workspace\02132_Assignment1\" ; if ($?) { gcc cbmp.c  main.c -o main } ; if ($?) { .\main example.bmp example_inv.bmp}

// Mathias
// cd "C:\Users\mathi\Documents\DTU\Computersystemer\Assignment 1\02132_Assignment1" ; if ($?) { gcc cbmp.c  main.c -o main } ; if ($?) { .\main example.bmp example_inv.bmp}

#include <stdlib.h>
#include <stdio.h>
#include "cbmp.h"

#define THRESHOLD 90

void convert_to_binary_threshold(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image_binary_threshold[BMP_WIDTH][BMP_HEIGTH]);

// Function to invert pixels of an image (negative)
void invert(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      for (int c = 0; c < BMP_CHANNELS; c++)
      {
        output_image[x][y][c] = 255 - input_image[x][y][c];
      }
    }
  }
}

void convert_RGB_to_gray(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image_gray[BMP_WIDTH][BMP_HEIGTH]);
void convert_3dim_to_2dim(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);

// Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_image_gray[BMP_WIDTH][BMP_HEIGTH];

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

  // Run inversion
  // invert(input_image, output_image);
  convert_RGB_to_gray(input_image, output_image_gray);
  convert_3dim_to_2dim(output_image_gray,output_image);

  // Save image to file
  write_bitmap(output_image, argv[2]);

  printf("Done!\n");
  return 0;
}

void convert_RGB_to_gray(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image_gray[BMP_WIDTH][BMP_HEIGTH])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      output_image_gray[x][y] = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) / 3;
    }
  }
}
void convert_3dim_to_2dim(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      output_image[x][y][0] = input_image[x][y];
      output_image[x][y][1] = input_image[x][y];
      output_image[x][y][2] = input_image[x][y];
    }
  }
}
