/*
 * Edge_finding.cpp
 *
 *  Created on: Feb 9, 2017
 *      Author: Mr.King
 */

//#define H 60
//#define W 80
//struct Coor{
//	int x;
//	int y;
//};
//Coor Left_edge[20]; // 19 - 39
//Coor Right_edge[20];
//
///*
// * Step one: Find the edges on both sides
// * Step two: Find the "white area" on both sides according to edge
// * @Left_edge: array to store the left edge coordinate
// * @Right_edge: array to store the right edge coordinate
// * @L_sum_white: the sum of white points on the left side
// * @R_sum_white: the sum of white points on the right side
// *
// */
//
//
//void Find_left_edge(){
//    // To store the position of current layer, when layer=0 means y-cor=0
//    int layer = 19;
//    // Find the origin (y=20). (Assume the original layer is accurate every time)
//    for (int x = W/2; x>0 ;x--){
//  	  if (ext_camptr[layer][x]!=ext_camptr[layer][x-1]){
//  		  Left_edge[layer].y = layer;
//  		  Left_edge[layer].x = x;
//  		  break;
//  	  }
//    layer++;
//    // Find the rest.
//    for(;layer< H-20 ;layer++){
//  	  // LEFT
//		  for (int x = W/2; x>0 ;x--){
//			  // Tolerance = 15
//			  if (ext_camptr[layer][x]!=ext_camptr[layer][x-1] && x <= Left_edge[x-1].x + 15){
//				  Left_edge[layer].y = layer;
//				  Left_edge[layer].x = x;
//				  break;
//			  }
//		  }
//    	}
//    }
//}
//
//void Find_right_edge(){
//    // To store the position of current layer, when layer=0 means y-cor=0
//    int layer = 19;
//    // Find the origin (y=20). (Assume the original layer is accurate every time)
//    for (int x = W/2; x<W ;x++){
//  	  if (ext_camptr[layer][x]!=ext_camptr[layer][x+1]){
//  		  Right_edge[0].y = layer;
//  		  Right_edge[0].x = x;
//  		  break;
//  	  }
//    }
//    layer++;
//    // Find the rest.
//    for(;layer< H-20 ;layer++){
//	  // RIGHT
//	  for (int x = W/2; x<W ; x++){
//	  if (ext_camptr[layer][x]!=ext_camptr[layer][x+1] && x >= Right_edge[x-1].x - 15){
//		  Right_edge[layer].y = layer;
//		  Right_edge[layer].x = x;
//		  break;
//	  	  }
//	  }
//    }
//}
//
//
