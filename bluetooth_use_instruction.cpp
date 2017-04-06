/*
 * bluetooth_use_instruction.cpp
 *
 *  Created on: Apr 6, 2017
 *      Author: lzhangbj
 */



/*
 * put this in global positions before main to use as change constant function
 */



string inputStr;  //global
bool tuning = false;  //global
vector<double> constVector;  //global
bool bluetoothListener(const Byte *data, const size_t size) {
	if (data[0] == 't') {
		tuning = 1;
		inputStr = "";
	} else if (tuning && data[0] != 't') {
		if (data[0] != '\n') {
			inputStr += (char)data[0];
		} else {
			tuning = 0;
			constVector.clear();
			char * pch;
			pch = strtok(&inputStr[0], " ,");
			while (pch != NULL){
			int constant;
			stringstream(pch) >> constant;
			constVector.push_back(constant);
			pch = strtok (NULL, " ,");
		}
//		ANGLE = constVector[0];
//		SPEED = constVector[1];
		a = constVector[0];
		b = constVector[1];
		BC_KP = constVector[2];
		BC_KD = constVector[3];
		SC_KP = constVector[4];
		SC_KD = constVector[5];
		DC_KP = constVector[6];
		DC_KD = constVector[7];
		LForOffSet = constVector[8];
		RForOffSet = constVector[9];
		LAntiForOffSet = constVector[10];
		RAntiForOffSet = constVector[11];
		/*
		 *       assume constants you need to change are:  constant1, constant2...
		 *
		 *        format:    constant1 = constVector[0];
		 *        			 constant2 = constVector[1];
		 *        			 .....
		 *        			 the maximum number is 12
		 */

    }
  }
  return 1;
}







/*
 * put this in main with bluetooth config to change constant
 */

bluetooth_config.rx_isr = &bluetoothListener;
/*
 * put this in main to use as buffer to send data
 */
char Buffer[100];

/*
 *    put this in while loop to show data
 */
sprintf(Buffer, "%.1lf,%.1lf,%.1lf\n",1.0,(float)number1,(float)number2);		//	the first number should be 1.0, the other numbers are numbers you need to show
const Byte speedByte = 85;
bt->SendBuffer(&speedByte, 1);
bt->SendStr(Buffer);

