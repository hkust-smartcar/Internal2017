#include <cassert>
#include <cstring>
#include <libbase/k60/mcg.h>
#include <libsc/system.h>

#include <string>
#include <sstream>

//Random Number Header File-------------------------------------------------------------------------------------------------
#include<stdlib.h>
#include<libbase/k60/rand_generator.h>

//Button Header File--------------------------------------------------------------------------------------------------------
#include<libsc/button.h>

//Joy Stick Header File-----------------------------------------------------------------------------------------------------
#include<libsc/joystick.h>

//led Header File-----------------------------------------------------------------------------------------------------------
#include <libsc/led.h>

//LCD Header File-----------------------------------------------------------------------------------------------------------
#include<libsc/st7735r.h>
#include<libsc/lcd_console.h>

//Required DON'T DELETE IT !!!----------------------------------------------------------------------------------------------
namespace libbase
{
	namespace k60
	{

		Mcg::Config Mcg::GetMcgConfig()
		{
			Mcg::Config config;
			config.external_oscillator_khz = 50000;
			config.core_clock_khz = 150000;
			return config;
		}

	}


}

//namespace-------------------------------------------------------------------------------------------------------
using namespace libsc;
using namespace libsc::k60;
using namespace libbase::k60;
using namespace std;

//Global variable-------------------------------------------------------------------------------------------------

class COORD{
public:
	int X;
	int Y;
};

class GameBoard{
	int length;
	int width;
public:
	GameBoard():length(32),width(32){}//With 1 unit boundary
	int getlength(){return length;};
	int getwidth(){return width;};
	void print(St7735r *lcd, LcdConsole *console){
		for(int y=0;y<length;y++){
			for(int x=0;x<width;x++){
				if(x==0||x==width-1||y==0||y==length-1){
					lcd->SetRegion(Lcd::Rect(x*4,y*4,4,4));
					lcd->FillColor(0xFFFF);
				}
			}
		}
		console->WriteString("Your Length: /n");
	}
};

GameBoard board;

class Snake{
	int length;
	int direction;
	int Position[32][2];
public:
	Snake():length(2){}
	void ini(){
		Position[0][0]=board.getwidth()/2;
		Position[0][1]=board.getlength()/2;
		Position[1][0]=board.getwidth()/2;
		Position[1][1]=board.getlength()/2+1;
		Position[2][0]=board.getwidth()/2;
		Position[2][1]=board.getlength()/2+2;
	}
	void print(St7735r *lcd,LcdConsole *console){
		for(int y=1;board.getlength()-1;y++){
			for(int x=1;x<board.getwidth()-1;x++){
				if(x==Position[0][0]&&y==Position[0][1]){
					lcd->SetRegion(Lcd::Rect(x*4,y*4,4,4));
					lcd->FillColor(0x07E0);
				}else if(x==Position[1][0]&&y==Position[1][1]){
					lcd->SetRegion(Lcd::Rect(x*4,y*4,4,4));
					lcd->FillColor(0x07E0);
				}else if(x==Position[length][0]&&y==Position[length][1]){
					lcd->SetRegion(Lcd::Rect(x*4,y*4,4,4));
					lcd->FillColor(0x0000);
				}
			}
		}
		console->SetCursorRow(1);
		char buff[50];
		sprintf(buff,"%d",length);
		console->WriteString(buff);
	}
	void control(Joystick *FiveWaySwitch){
		if (FiveWaySwitch->GetState()==Joystick::State::kUp && direction != 3) {
		direction = 1;
		}
		if (FiveWaySwitch->GetState()==Joystick::State::kLeft && direction != 4) {
			direction = 2;
		}
		if (FiveWaySwitch->GetState()==Joystick::State::kDown && direction != 1) {
			direction = 3;
		}
		if (FiveWaySwitch->GetState()==Joystick::State::kRight && direction != 2) {
			direction = 4;
		}
	}
	void move(){
		for(int i=31;i>0;i--){
			Position[i][0]=Position[i-1][0];
			Position[i][1]=Position[i-1][1];
		}
		if (direction == 1) {
			Position[0][1] = Position[0][1] - 1;
		}
		if (direction == 2) {
			Position[0][0] = Position[0][0] - 1;
		}
		if (direction == 3) {
			Position[0][1] = Position[0][1] + 1;
		}
		if (direction == 4) {
			Position[0][0] = Position[0][0] + 1;
		}
	}
	void setDirection(int x){
		direction=x;
	}
	int getPosition(int x,int y){return Position[x][y];};
	int getLength(){return length;};
	int getDirection(){return direction;};
	void AddLength(){length=length+1;}
};

Snake snake;

class GameManager{
	bool GameOver;
	bool GameWin;
	bool GameStart;
	bool Fruit;
	COORD FruitPosition;
public:
	GameManager():GameOver(false),GameWin(false),GameStart(false),Fruit(false){}
	bool getGameOver(){
		return GameOver;
	};
	void setGameOver(bool x){
		GameOver=x;
	}
	bool getGameStart(){
		return GameStart;
	};
	void setGameStart(bool x){
		GameStart=x;
	}
	bool getGameWin(){return GameWin;};
	void FruitGenerate(St7735r *lcd){
		if(Fruit==false){
			FruitPosition.X=rand()%(board.getwidth()-2)+1;
			FruitPosition.Y=rand()%(board.getlength()-2)+1;
			Fruit=true;
			for(int i=0;i<snake.getLength();i++){
				if(snake.getPosition(i,0)==FruitPosition.X&&snake.getPosition(i,1)==FruitPosition.Y){
					FruitGenerate(lcd);
					break;
				}
			}
			if(FruitPosition.X==0||FruitPosition.Y==0||FruitPosition.X==board.getwidth()-1||FruitPosition.Y==board.getlength()-1){
				FruitGenerate(lcd);
			}
			for(int y=0;y<board.getlength()+2;y++){
			for(int x=0;x<board.getwidth()+2;x++){
				if(x==FruitPosition.X&&y==FruitPosition.Y){
					lcd->SetRegion(Lcd::Rect(x*4,y*4,4,4));
					lcd->FillColor(0xF800);
				}
			}
		}
		}
	}
	void FruitEaten(){
		if(snake.getPosition(0,0)==FruitPosition.X&&snake.getPosition(0,1)==FruitPosition.Y){
			Fruit=false;
			snake.AddLength();
			if(snake.getLength()==32){
				GameWin=true;
			}
		}
	}
};

GameManager game;

void GameOverCheck(){
	if(snake.getPosition(0,0)==0||snake.getPosition(0,1)==0||snake.getPosition(0,0)==board.getwidth()+1||snake.getPosition(0,1)==board.getlength()+1){
		game.setGameOver(true);
	}
	for(int i=1;i<snake.getLength();i++){
		if(snake.getPosition(i,0)==snake.getPosition(0,0)&&snake.getPosition(i,1)==snake.getPosition(0,1)){
			game.setGameOver(true);
		}
	}
}


//Main Program--------------------------------------------------------------------------------------------------------------
int main(void)
{
	System::Init();

//Button Configuration------------------------------------------------------------------------------------------------------
	Button::Config ConfigButton1;
	ConfigButton1.id = 0;
	ConfigButton1.is_active_low = true;
	Button button1(ConfigButton1);

//Joy Stick Configuration---------------------------------------------------------------------------------------------------
	Joystick::Config ConfigJoystick;
	ConfigJoystick.id=0;
	ConfigJoystick.is_active_low=true;
	Joystick FiveWaySwitch(ConfigJoystick);

//LCD Configuration---------------------------------------------------------------------------------------------------------
	St7735r::Config ConfigLCD;
	ConfigLCD.is_revert=true;
	ConfigLCD.is_bgr=false;
	ConfigLCD.fps=60;
	St7735r LCD(ConfigLCD);

	LcdConsole::Config ConfigConsole;
	ConfigConsole.lcd=&LCD;
	ConfigConsole.region=Lcd::Rect(0,130,128,30);
	LcdConsole Console(ConfigConsole);

 	srand((unsigned)System::Time());
 	snake.ini();
 	snake.print(&LCD,&Console);
 	game.FruitGenerate(&LCD);
 	board.print(&LCD,&Console);
 	while(game.getGameOver()==false&&game.getGameWin()==false){
 		if(game.getGameStart()==true){
			snake.control(&FiveWaySwitch);
			snake.move();
			GameOverCheck();
			game.FruitEaten();
			snake.print(&LCD,&Console);
			game.FruitGenerate(&LCD);
			int PrevDirection=snake.getDirection();
			while(System::Time()%200!=0){
				if((FiveWaySwitch.GetState()==Joystick::State::kUp)&&(PrevDirection!=3)){
					snake.setDirection(1);
				}else if((FiveWaySwitch.GetState()==Joystick::State::kLeft)&&(PrevDirection!=4)){
					snake.setDirection(1);
				}else if((FiveWaySwitch.GetState()==Joystick::State::kDown)&&(PrevDirection!=1)){
					snake.setDirection(1);
				}else if((FiveWaySwitch.GetState()==Joystick::State::kRight)&&(PrevDirection!=2)){
					snake.setDirection(1);
				}
			}
		}else if(FiveWaySwitch.GetState()==Joystick::State::kUp) {
			snake.setDirection(1);
			game.setGameStart(true);
		}else if (FiveWaySwitch.GetState()==Joystick::State::kLeft) {
			snake.setDirection(2);
			game.setGameStart(true);
		}else if (FiveWaySwitch.GetState()==Joystick::State::kRight) {
			snake.setDirection(4);
			game.setGameStart(true);
		}
	 }
	 if(game.getGameOver()==true){
		Console.Clear(true);
		Console.WriteString("You Lose!!");
		while(System::Time()%5000!=0){
		}
	 }
	 if(game.getGameWin()==true){
		Console.Clear(true);
		Console.WriteString("You Lose!!");
		while(System::Time()%5000!=0){
		}
	 }
}
