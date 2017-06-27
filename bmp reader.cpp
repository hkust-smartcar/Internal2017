#include <iostream>
#include <fstream>
#include <stdio.h>

using namespace std;

int width,height;
unsigned char* map;

unsigned char* readBMP(char* filename)
{
    int i;
    FILE* f = fopen(filename, "rb");
    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

    // extract image height and width from header
    width = *(int*)&info[18];
    height = *(int*)&info[22];

    int size = 3 * width * height;
    unsigned char* data = new unsigned char[size]; // allocate 3 bytes per pixel
    fread(data, sizeof(unsigned char), size, f); // read the rest of the data at once
    fclose(f);

    for(i = 0; i < size; i += 3)
    {
            unsigned char tmp = data[i];
            data[i] = data[i+2];
            data[i+2] = tmp;
    }

    return data;
}

bool readbit(int x, int y){
	return map[3 * width * height - (y*width+x)*3 - 1];
}

int main(){
	map = readBMP("input.bmp");
	for (int i =0; i<height ; i++){
		for (int j=0; j<width; j++)
			printf("%d",(readbit(j,i)?1:0));
		printf("\n");
	}
	}