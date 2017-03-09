#include <iostream>
#include <fstream>
#include <stdio.h>

using namespace std;

int width,height;
unsigned char* map;
int vanish_x=0;
int vanish_y=0;

int y1=0,y2=0;

int max(int a, int b){return (a>b?a:b);}
int min(int a, int b){return (a<b?a:b);}

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
void autocal(){
	
	int a=height-1,b=0,c=width-1,d=height-1;
	const bool WHITE = readbit(width/2,height-1);
	printf("white is defined as %d\n" , WHITE);
	
	while(readbit(0,a--)==WHITE);
	while(readbit(++b,0)!=WHITE); 
	while(readbit(--c,0)!=WHITE); 
	while(readbit(width-1,d--)==WHITE);
	
	printf("a,b,c,d : %d,%d,%d,%d\n", a,b,c,d);
	const int A=a,B=b,C=a*b,D=d,E=c-width+1,F=c*d;
	int det = A*E-B*D;
	
	vanish_x = -(B*F-C*E)/det;
	vanish_y = (A*F-C*D)/det;
	
	printf("(%d,%d)\n",vanish_x,vanish_y);
	
	y1=a;y2=d;
}

/**
*
*@param ry: reference_level
*/
bool getfixbit(int ry, int x, int y){
	int H = vanish_y - ry;
	int R = width/2 - x;
	int h = ry - y;
	int r = R*h/H;
	readbit(max(0,min(width-1,x-r)),y);
}


int main(){
	map = readBMP("input.bmp");
	for (int i =0; i<height ; i++){
		for (int j=0; j<width; j++)
			printf("%d",(readbit(j,i)?1:0));
		printf("\n");
	}
	autocal();
	printf("done\n");
	
	for (int i =0; i<height ; i++){
		for (int j=0; j<width; j++)
			printf("%d",(getfixbit(max(y1,y2),j,i)?1:0));
		printf("\n");
	}
	printf("done\n");
}
