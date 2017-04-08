#include <iostream>
#include <fstream>
#include <stdio.h>
#include <conio.h>
#include <time.h> 

using namespace std;

#include <windows.h>

void gotoxy( int x, int y ){
COORD p = { x, y };
SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), p );
}

#define WIDTH 80
#define HEIGHT 60

int map[HEIGHT][WIDTH];
int max(int a, int b){return (a>b?a:b);}
int min(int a, int b){return (a<b?a:b);}

bool get_bit(int x,int y){
	return map[min(max(y,0),HEIGHT)][min(max(x,0),WIDTH)];
}

void printpicture();

void getpicture(){
	srand(time(NULL));
	for (int j=0;j<WIDTH;j++){
		for (int i=0;i<HEIGHT;i++){
			map[i][j]=rand()%2;
		}
	}
	printpicture();
	bool flag=1;
	int x;
	do{
		flag=0;
		for (int j=1;j<WIDTH-1;j++){
			for (int i=1;i<HEIGHT-1;i++){
				int val=0,cnt=0;
				for(int k=-1;k<2;k++){
				
					for(int l=-1;l<2;l++){
						if(l==0&&k==0)continue;
						val+=map[i+k][j+l];
						cnt++;
					}
				}
				//val/=4;//cnt/2;
				cout<<val<<cnt;
				if(val>=4!=map[i][j]){
					map[i][j]=val>=4;
					flag=1;
				}
			}
		}
	}while(0);
}

void printpicture(){
	for (int i=0;i<HEIGHT;i++){
		for(int j=0;j<WIDTH;j++)
			cout<<map[i][j];
		cout<<endl;
	}
}

int main(){
	getpicture();
	printpicture();
}
