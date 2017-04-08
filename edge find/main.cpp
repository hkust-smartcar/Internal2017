#include <iostream>
#include <fstream>
#include <stdio.h>
#include <conio.h>
#define WIDTH width
#define HEIGHT height

using namespace std;

#include <windows.h>

void gotoxy( int x, int y ){
COORD p = { x, y };
SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), p );
}

int width,height;
unsigned char* map;

int max(int a, int b){return (a>b?a:b);}
int min(int a, int b){return (a<b?a:b);}

unsigned char* readBMP(char* filename){
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

    for(i = 0; i < size; i += 3){
            unsigned char tmp = data[i];
            data[i] = data[i+2];
            data[i+2] = tmp;
    }

    return data;
}

bool get_bit(int x, int y){
	if (x<0||x>=width||y<0||y>=height) return true;
	return map[3 * width * height - (y*width+x)*3 - 1];
}

void load_picture(){
	map = readBMP("image.bmp");
}

void print_raw(){
	for (int i =0; i<height ; i++){
		for (int j=0; j<width; j++)
			printf("%d",(get_bit(j,i)?1:0));
		printf("\n");
	}
}


class Point{
	public:
	int x,y;
	Point(int x, int y):x(x),y(y){
	}
	Point Set(int x, int y){
		this->x=x;this->y=y;
		return *this;
	}
	/******
	 * 107
	 * 2X6
	 * 345
	 * */
	Point NeiPt(int d){
			d%=8;
			if(d==0)return Point(x,y-1);
			if(d==1)return Point(x-1,y-1);
			if(d==2)return Point(x-1,y);
			if(d==3)return Point(x-1,y+1);
			if(d==4)return Point(x,y+1);
			if(d==5)return Point(x+1,y+1);
			if(d==6)return Point(x+1,y);
			return Point(x+1,y-1);
	}
};

void colorxy(Point);
bool get_bit(Point pt){
	return get_bit(pt.x,pt.y);
}

class BufferManager{
public:
	//const Byte* buff;
	//BufferManager(const Byte* buff):buff(buff){}
	BufferManager(){}
	bool get(int x, int y){
		return map[3 * width * height - (y*width+x)*3 - 1];//buff[y*WIDTH/8+x/8] & 0x80>>x%8;
	}
	bool get(Point pt){
		return get(pt.x,pt.y);
	}
	public:
	Point search(Point base,int from){
		//const bool BLACK=true,WHITE=false;
		if(base.x<0||base.y<0||base.x>WIDTH||base.y>HEIGHT) return base;

		if(get(base)){
			return search(base.NeiPt((from+4)%8),from);
		}
		else{
			colorxy(base);
			int target=from;
			if(get(base.NeiPt(++target))){//do anti clock wise
				while(get(base.NeiPt(++target)));
				return search(base.NeiPt(target),from);
			}
			else if(get(base.NeiPt(--target))){
				while(get(base.NeiPt(--target)));
				return search(base.NeiPt(target),from);

			}
			else {
				int sum=0;
				for (int i=0;i<8;i++){
					sum+=get(base.NeiPt(i));
				}
				if (sum==0) return search(base.NeiPt(0),4);
				else return base;
			}
		}
	}
	
	bool isEdge(Point base, int from_direction){
		return get_bit(base.NeiPt((from_direction+4)%8))!=get_bit(base);
	}
};

bool isEdge(Point base, int from_direction){
		return get_bit(base.NeiPt((from_direction+4)%8))!=get_bit(base);
	}

void colorxy(Point pt){
	gotoxy(pt.x,pt.y);
	cout<<"*";
}

void colorxy(int x, int y){
	gotoxy(x,y);
	cout<<"*";
}

void printxy(int x, int y, char* c){
	gotoxy(x,y);
	cout<<c;
}

int main(){
	getch();
	load_picture();
	print_raw();
	Point base(0,height-1);
	BufferManager buff;
	int from=4;
	if (get_bit(base)){
		while (get_bit(base)){
			base=base.NeiPt(6);
			cout<<"shift";
		}
		from=2;
	}
	cout<<"hi";
	while (1){
		printxy(base.x,base.y,"*");
		for(int i=from; i>=from-8; --i){
			if(!get_bit(base.NeiPt(i+1))&&get_bit(base.NeiPt(i))) {
			base=base.NeiPt(i+1);
			from=(i+5)%8;
			break;
			}
		}
		/*
		if(!get_bit(base.NeiPt(from-2))&&get_bit(base.NeiPt(from-1))) {
		base=base.NeiPt(from-2);
		from+=2;
		}
		else if(!get_bit(base.NeiPt(from-4))&&(get_bit(base.NeiPt(from-3))||get_bit(base.NeiPt(from-5)))){
			base=base.NeiPt(from-4);
		}*/
	}
	
}
