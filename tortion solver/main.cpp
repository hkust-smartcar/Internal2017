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

/*unsigned char getdisbit(int eyelevel, int reference,int x, int y){
	return readbit()
}*/

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
	int centerx= -(B*F-C*E)/det;
	int centery= (A*F-C*D)/det;
	
	printf("(%d,%d)\n",centerx,centery);
	
}



int main(){
	map = readBMP("input.bmp");
	/*
	int i=0;
	while (i<3 * width * height){
		printf("%d",(map[i]?1:0));
		i+=3;
		if (i%(width*3)==0) printf("\n");
	}*/
	for (int i =0; i<height ; i++){
		for (int j=0; j<width; j++)
			printf("%d",(readbit(j,i)?1:0));
		printf("\n");
	}
	autocal();
	printf("done\n");
}
