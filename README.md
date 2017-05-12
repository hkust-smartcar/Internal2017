# Internal2017
This is a repository for all groups in our internal.

Each Person please open a new branch for storing your program. You are encouraged to discuss the algorithm used.

### code that can deal with world view matrix

```C++
//get bit value from camerabuf using camera coordinate system
bool getBit(int i_x, int i_y){
	if (i_x<=0 || i_x>CameraSize.w-1 || i_y <= 0 || i_y > CameraSize.h-1) return -1;
	return CameraBuf[i_y*CameraSize.w/8 + i_x/8] >> (7 - (i_x%8)) & 1;
}


bool getWorldBit(int w_x, int w_y){
	int i_x,i_y;
	i_x = transformMatrix[w_x][w_y][0];
	i_y = transformMatrix[w_x][w_y][1];
	return getBit(i_x,i_y);
}

void PrintWorldImage(){
	Byte temp[128/8];
	for (int i=0; i<160; i++){
		for (int j=0; j<128; j++){
			temp[j/8]<<=1;
			temp[j/8]+=getWorldBit(j,i);
			//WorldBuf[i*128/8+j/8]<<=1;
			//WorldBuf[i*128/8+j/8]+=getWorldBit(j,i);
		}
		pLcd->SetRegion(Lcd::Rect(0,i,128,1));
		pLcd->FillBits(0x0000,0xFFFF,temp,128);
		//pLcd->FillColor(getWorldBit(j,i)?Lcd::kBlack:Lcd::kWhite);
	}
	return;
}
```
