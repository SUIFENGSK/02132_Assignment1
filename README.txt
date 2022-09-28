  ///////////////////////////////////////////////////
 ////////////////// Instructions ///////////////////
///////////////////////////////////////////////////

If you use the terminal, compile 'main.c' as follows: 

Linux/Mac:
- To compile: gcc cbmp.c main.c -o main.out -std=c99

Windows:
- To compile: gcc cbmp.c main.c -o main.exe -std=c99


How to run the program:

To run the program on a single image:

Linux/Mac:
- To run: ./main.out samples/easy/1easy.bmp 1easy-output.bmp

Windows:
- To run: main.exe samples/easy/1easy.bmp 1easy-output.bmp


It is also possible to run the program on a whole folder. 
This outputs to a .txt-file called [difficulty]-test-output.txt.
Note: this only outputs the amount of found cells and the timing. 
To output everything, include the argument "all".

Linux/Mac:
- Limited output: ./main.out test easy
- Output everything: ./main.out test easy all

Windows:
- Limited output: main.exe test easy
- Output everything: main.exe test easy all
