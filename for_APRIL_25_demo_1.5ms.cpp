void Moving::NormalMovingTestingVersionOld(FutabaS3010& servo, St7735r& lcd, AlternateMotor & motor_right, AlternateMotor& motor_left) {
////	 initialize LCD console
//	  LcdConsole::Config console_config;
//	  console_config.lcd = &lcd;
//	  LcdConsole console(console_config);

	int layer = 55;
	int LayerCount = 0;
	const int NormalRoadWidth_UpperBound = 66; // Set this value for decelerate before roundabout
	const int CrossingRoadWidth_LowerBound = 76; // Do not use 3 seconds delay when it is a crossing
	bool R_Edge_is_found;
	bool L_Edge_is_found;
	bool EncounterCrossing = false;
	static Timer::TimerInt RoundRoad_StartTime;
	static bool RoundRoadNow;
	int UpdatedDegree;
	int Middle_xcor = 37;
	int ServoP;
	const int ServoP_Straight = 10;
	const int ServoP_NormalTurning = 15;
	const int ServoP_SharpTurning = 25;
	const int ServoP_Roundabout = 25;

	// Find the origin (y=1). (Assume the original layer is accurate every time)-------------------------------------------
	for (int x = W / 2; x > 1; x--) {
		if (ext_camptr[x][layer] != ext_camptr[x - 1][layer]) {
			Left_edge[layer] = x;
			ext_camptr[x][layer] = 2;
			break;
		}
	}
	for (int x = W / 2; x < W; x++) {
		if (ext_camptr[x][layer] != ext_camptr[x + 1][layer]) {
			Right_edge[layer] = x;
			ext_camptr[x][layer] = 2;
			break;
		}
	}
	// Update the center point
	LayerCount++;
	Center[layer] = (Left_edge[layer] + Right_edge[layer]) / 2;
	/*ROUNDABOUT JUDGEMENT*/
	if ((ext_camptr[Center[layer] - 1][layer]
									  || ext_camptr[Center[layer]][layer]
																   || ext_camptr[Center[layer] + 1][layer]) == true) {

		/*Double check*/
		//    if (DoubleCheckRound(Center[layer], layer)) {
		//      /*ROUNDABOUT HANDLING*/
//		      string s = "Status: Roundabout\n";
//		      console.SetCursorRow(1);
//		      console.WriteString(s.c_str());

		for (int x = Center[layer]; x > 0; x--) {
			if (ext_camptr[x][layer] == false) {
				Right_edge[layer] = x;
				break;
			}
		}
		for (int x = Right_edge[layer]; x > 0; x--) {
			Left_edge[layer] = x;
			if (ext_camptr[x][layer] == true) {
				break;
			}
		}
		Center[layer] = (Left_edge[layer] + Right_edge[layer]) / 2; // Update new center point
		//      lcd.SetRegion(Lcd::Rect(Center[layer], layer, 5, 5));
		//      lcd.FillColor(Lcd::kRed);
		ServoP = ServoP_Roundabout;
		if(Center[layer] - Middle_xcor < 0){
			/*Motor Control*/
			motor_right.SetPower(300);
			motor_left.SetPower(200);
		}
		else{
			motor_right.SetPower(200);
			motor_left.SetPower(300);
		}

		if ((ServoStraightDegree - (Center[layer] - Middle_xcor) * ServoP) > ServoLeftBoundary) {
			UpdatedDegree = ServoLeftBoundary;
		} else if ((ServoStraightDegree - (Center[layer] - Middle_xcor) * ServoP) < ServoRightBoundary) {
			UpdatedDegree = ServoRightBoundary;
		} else {
			UpdatedDegree = ServoStraightDegree - (Center[layer] - Middle_xcor) * ServoP;
		}
		servo.SetDegree(UpdatedDegree); //Action immediately for this special case
		return;
		//    }
	}
	//ext_camptr[Center[layer]][layer] = 3;
	// Find the rest (Choose the start point based on last layer midpoint)-------------------------------------------
	for (; --layer > 25;) {
		R_Edge_is_found = false;
		L_Edge_is_found = false;
		//LEFT
		for (int x = Center[layer + 1]; x > 1; x--) {
			//Found when change from white to black && new edge is not far away from last edge
			if ((ext_camptr[x][layer] != ext_camptr[x - 1][layer]) && (abs(x - Left_edge[layer + 1]) <= 5)) {
				Left_edge[layer] = x;
				ext_camptr[x][layer] = 2;
				L_Edge_is_found = true;
				break;
			}
		}
		if (L_Edge_is_found == false) {
			//DO CANT FIND THING HERE
			Left_edge[layer] = Left_edge[layer + 1];
			ext_camptr[Left_edge[layer]][layer] = 2;
		}
		//Right
		for (int x = Center[layer + 1]; x < W; x++) {
			if ((ext_camptr[x][layer] != ext_camptr[x + 1][layer]) && (abs(x - Right_edge[layer + 1]) <= 5)) {
				Right_edge[layer] = x;
				ext_camptr[x][layer] = 2;
				R_Edge_is_found = true;
				break;
			}
		}
		if (R_Edge_is_found == false) {
			//DO CANT FIND THING HERE
			Right_edge[layer] = Right_edge[layer + 1];
			ext_camptr[Right_edge[layer]][layer] = 2;
		}
		// Update the center point
		Center[layer] = (Left_edge[layer] + Right_edge[layer]) / 2;
		/*Special Case Judgement----------------------------------------------------------*/
		//		if(R_Edge_is_found && L_Edge_is_found){
		/*ROUNDABOUT JUDGEMENT*/
		if ((ext_camptr[Center[layer] - 1][layer]
										   || ext_camptr[Center[layer]][layer]
																			  || ext_camptr[Center[layer] + 1][layer]) == true) {
//			/*Double check*/
//	        string s = "Status: Roundabout\n";
//	        console.SetCursorRow(1);
//	        console.WriteString(s.c_str());

			for (int x = Center[layer]; x > 0; x--) {
				if (ext_camptr[x][layer] == false) {
					Right_edge[layer] = x;
					break;
				}
			}
			for (int x = Right_edge[layer]; x > 0; x--) {
				Left_edge[layer] = x;
				if (ext_camptr[x][layer] == true) {
					break;
				}
			}
			Center[layer] = (Left_edge[layer] + Right_edge[layer]) / 2; // Update new center point

			//        lcd.SetRegion(Lcd::Rect(Center[layer], layer, 5, 5));
			//        lcd.FillColor(Lcd::kRed);
			ServoP = ServoP_Roundabout;
			if(Center[layer] - Middle_xcor < 0){
				/*Motor Control*/
				motor_right.SetPower(300);
				motor_left.SetPower(200);
			}
			else{
				motor_right.SetPower(200);
				motor_left.SetPower(300);
			}

			if ((ServoStraightDegree - (Center[layer] - Middle_xcor) * ServoP) > ServoLeftBoundary) {
				UpdatedDegree = ServoLeftBoundary;
			} else if ((ServoStraightDegree - (Center[layer] - Middle_xcor) * ServoP) < ServoRightBoundary) {
				UpdatedDegree = ServoRightBoundary;
			} else {
				UpdatedDegree = ServoStraightDegree - (Center[layer] - Middle_xcor) * ServoP;
			}
			servo.SetDegree(UpdatedDegree); //Action immediately for this special case
			return;
			//      }
		}
		ext_camptr[Center[layer]][layer] = 3;
		LayerCount++;
		//		}
		/*CROSSING JUDGEMENT*/
//		if ((L_Edge_is_found || R_Edge_is_found) == false) {
////			/*CROSSING DOUBLE CHECK*/
////			if (DoubleCheckCrossing(Center[layer], layer)) {
////				/*HAS ROAD - GO STRAIGHT*/
////				motor_right.SetPower(350);
////				motor_left.SetPower(350);
//		        string s = "Status: Crossing\n";
//		        console.SetCursorRow(1);
//		        console.WriteString(s.c_str());
////			    EncounterCrossing = true;
////				break;
////			}
//		}
	}

	/*CROSSING HANDLING*/
	//	if (HasRoad() && EncounterCrossing == true){
	//		servo.SetDegree(ServoStraightDegree);
	//		return;
	//	}
	int sum = 0; // Initially 50 - 20
	for (int L = 55; L > 55 - LayerCount; L--) {
		sum += Center[L];
	}
	int Average = sum / LayerCount;
//	  	string s = "Average: " + to_string(Average) + "\n";
//	  	console.WriteString(s.c_str());
	int Error = Average -Middle_xcor;
//
//	string s_1 = "Error: " + to_string(Error) + "\n";
//    console.SetCursorRow(2);
//	console.WriteString(s_1.c_str());

/*---------------------------------------------Not Round Road : Straight + Normal Turning + Sharp Turning*/
	if(!RoundRoadNow){
		//Round Road Reminder
		int CurrentRoadWidth = abs(abs(Left_edge[55 - LayerCount + 1]) - abs(Right_edge[55 - LayerCount + 1]));
		if( (CurrentRoadWidth >= NormalRoadWidth_UpperBound)
				&& (CurrentRoadWidth <= CrossingRoadWidth_LowerBound) // Not the crossing && ((L_Edge_is_found && R_Edge_is_found) == true)
																		){

//			string s = "Round Reminder\n";
//            console.SetCursorRow(5);
//			console.WriteString(s.c_str());

			motor_right.SetPower(150);
			motor_left.SetPower(150);
			RoundRoadNow = true;
			RoundRoad_StartTime = System::Time();
		}
		/*Servo P value setting*/
		/*Straight*/
		if(abs(Error) < 5){
			int right_area_sum = 0;
			int left_area_sum = 0;
			for(int y=30; y>20;y--){
				for(int x = Middle_xcor; x>Middle_xcor-30 ;x--){
					(!ext_camptr[x][y])? left_area_sum++:left_area_sum;
				}
				for(int x = Middle_xcor; x<Middle_xcor+30; x++){
					(!ext_camptr[x][y])? right_area_sum++:right_area_sum;
				}
			}

//			string s_2 = "Straight: Area Difference is " + to_string(abs(left_area_sum - right_area_sum)) + "\n";
//            console.SetCursorRow(3);
//			console.WriteString(s_2.c_str());

			/*Motor Control*/
			// Reduce speed in advance
			if(abs(left_area_sum - right_area_sum) > 100){
				motor_right.SetPower(150);
				motor_left.SetPower(150);
			}
			else{
				motor_right.SetPower(350);
				motor_left.SetPower(350);
			}
			/*Servo Control*/
			ServoP = ServoP_Straight;
			if ((ServoStraightDegree - Error * ServoP) > ServoLeftBoundary) {
				UpdatedDegree = ServoLeftBoundary;
			} else if ((ServoStraightDegree - Error * ServoP) < ServoRightBoundary) {
				UpdatedDegree = ServoRightBoundary;
			} else {
				UpdatedDegree = ServoStraightDegree - Error * ServoP;
			}
			servo.SetDegree(UpdatedDegree);
		}
		/*Normal Turning*/
		else if((abs(Error) >= 5) && (abs(Error) <=10)){

//			string s_2 = "Normal Turning\n ";
//            console.SetCursorRow(3);
//			console.WriteString(s_2.c_str());

			if(Error < 0){
				/*Motor Control*/
				motor_right.SetPower(350);
				motor_left.SetPower(250);
			}
			else{
				motor_right.SetPower(250);
				motor_left.SetPower(350);
			}

			/*Servo Control*/
			ServoP = ServoP_NormalTurning;
			if ((ServoStraightDegree - Error * ServoP) > ServoLeftBoundary) {
				UpdatedDegree = ServoLeftBoundary;
			} else if ((ServoStraightDegree - Error * ServoP) < ServoRightBoundary) {
				UpdatedDegree = ServoRightBoundary;
			} else {
				UpdatedDegree = ServoStraightDegree - Error * ServoP;
			}
			servo.SetDegree(UpdatedDegree);
		}
		/*Sharp Turning*/
		else{
			if(Error < 0){
				/*Motor Control*/
				motor_right.SetPower(350);
				motor_left.SetPower(250);
			}
			else{
				motor_right.SetPower(250);
				motor_left.SetPower(350);
			}

//			string s_2 = "Sharp Turning\n ";
//            console.SetCursorRow(3);
//			console.WriteString(s_2.c_str());

			/*Servo Control*/
			ServoP = ServoP_SharpTurning;
			if ((ServoStraightDegree - Error * ServoP) > ServoLeftBoundary) {
				UpdatedDegree = ServoLeftBoundary;
			} else if ((ServoStraightDegree - Error * ServoP) < ServoRightBoundary) {
				UpdatedDegree = ServoRightBoundary;
			} else {
				UpdatedDegree = ServoStraightDegree - Error * ServoP;
			}
			servo.SetDegree(UpdatedDegree);
		}
	}
/*---------------------------------------------During Round Road : Straight + Normal Turning + Sharp Turning*/
	else{
		if(Error < 0){
			/*Motor Control*/
			motor_right.SetPower(350);
			motor_left.SetPower(250);
		}
		else{
			motor_right.SetPower(250);
			motor_left.SetPower(350);
		}
		/*Servo Control*/
		ServoP = ServoP_Roundabout;
		if ((ServoStraightDegree - Error * ServoP) > ServoLeftBoundary) {
			UpdatedDegree = ServoLeftBoundary;
		} else if ((ServoStraightDegree - Error * ServoP) < ServoRightBoundary) {
			UpdatedDegree = ServoRightBoundary;
		} else {
			UpdatedDegree = ServoStraightDegree - Error * ServoP;
		}

		servo.SetDegree(UpdatedDegree);
		if((System::Time() - RoundRoad_StartTime) > 3000){
			//ServoP = ;
			RoundRoadNow = false;
		}
	}
}







