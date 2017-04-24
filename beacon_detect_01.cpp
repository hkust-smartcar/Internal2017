//============================================================================
// Name        : beacon.cpp
// Author      : TSE SK Avin
// Version     : 1 (23/4/2017)

//============================================================================

#include <iostream>
using namespace std;

struct circle
{
	int x;
	int y;
	int distance;
};

class Doubly_linked_list
{
	public:
	// constructor initialize the prevPtr and nextPtr
	Doubly_linked_list()
	{
		found = false;
		number = 0;
		x = 0;
		y = 0;
		dist = 0;
		prevPtr = 0; // points to null at the beginning
		nextPtr = 0; // points to null at the beginning
	}

	void SetFound( bool fd )
	{	found = fd;	}
	bool GetFound()
	{	return found;	}

	// get a number
	int GetNum()
	{	return number;	}
	// set a number
	void SetNum( int num )
	{	number = num;	}

	int GetX()
	{	return x;	}
	void SetX( int a )
	{	x = a;	}

	int GetY()
	{	return y;	}
	void SetY( int b )
	{	y = b;	}

	int GetDist()
	{	return dist;	}
	void SetDist( int s )
	{	dist = s;	}

	// get the prev pointer
	Doubly_linked_list *GetPrev()
	{	return prevPtr;	}

	// set the prev pointer
	void SetPrev (Doubly_linked_list *ptr)
	{	prevPtr = ptr;	}

	// get the next pointer
	Doubly_linked_list *GetNext()
	{	return nextPtr;	}

	// set the next pointer
	void SetNext( Doubly_linked_list *ptr )
	{	nextPtr = ptr;	}

private:
	bool found;
	int	number;
	int x;
	int y;
	int dist;
	Doubly_linked_list *prevPtr;
	Doubly_linked_list *nextPtr;
};


// 1. create a doubly linked list
Doubly_linked_list *Create_Doubly_linked_list(int cx, int cy, int d, bool f)	// class *object (input int)
{
	Doubly_linked_list *tempPtr, *firstPtr, *lastPtr, *currentPtr;
//	int		i = 0;
//	do
//	{
		tempPtr = new Doubly_linked_list;			// new object
//		tempPtr ->SetNum(i);
		tempPtr ->SetX(cx);
		tempPtr ->SetY(cy);
		tempPtr ->SetDist(d);
		tempPtr ->SetFound(f);
//		if ( i == 0 )
//		{
			firstPtr	= tempPtr;					// first = temp
			lastPtr		= tempPtr;
			currentPtr	= tempPtr;
/*			i++;
		}
		else
		{
			lastPtr ->SetNext(tempPtr);				// head pointer of lastPtr points to next object
																			// e.g. ( last at 1, next points to 2 )
			lastPtr = tempPtr;						// last = temp			// ( last becomes 2 )
			lastPtr ->SetPrev(currentPtr);			// tail pointer of lastPtr points to previous object
																			// ( last at 2, prev points to 1 )
			currentPtr = tempPtr;					// current = temp		// ( current becomes 2 )
			i++;
		}
	} while ( i < n );
*/
	return firstPtr;
}

/*
// 2. print a doubly linked list
void Print_Doubly_linked_list( Doubly_linked_list *ptr )
{
	while ( ptr != 0 )
	{
		cout << "[" << ptr ->GetNum() << "]->"; // print pointer number
		ptr = ptr ->GetNext();					// next pointer
	}
	cout << "Null" << endl;						// ptr = 0
	cout << endl;
}
*/

// 4. insert a node at the front
Doubly_linked_list *Insert_node_at_front ( Doubly_linked_list *ptr, Doubly_linked_list *firstPtr,
											int cx, int cy, int d, bool f)
{
	Doubly_linked_list *tempPtr;

	tempPtr = firstPtr;								// temp  = first
	firstPtr = ptr;									// first = ptr
	ptr ->SetNext(tempPtr);							// ptr head points to temp
	tempPtr ->SetPrev(ptr);						// temp tail points back ptr

	ptr ->SetX(cx);
	ptr ->SetY(cy);
	ptr ->SetDist(d);
	ptr ->SetFound(f);
	return ptr;										// (next)
}

// 5. remove a node at the front
Doubly_linked_list *Remove_node_at_front ( Doubly_linked_list *firstPtr )
{
	Doubly_linked_list *tempPtr;

	tempPtr = firstPtr;								// temp  = first
	firstPtr = firstPtr ->GetNext();				// first = next of first
	firstPtr ->SetPrev(0);
	delete tempPtr;
	return firstPtr;
}


// 6. insert a node at the back
Doubly_linked_list *Insert_node_at_back ( Doubly_linked_list *ptr, Doubly_linked_list *firstPtr,
											int cx, int cy, int d, bool f)
{
	Doubly_linked_list *prevPtr, *currPtr;

	currPtr = firstPtr;								// current  = first
	while ( currPtr != 0 )
	{
		prevPtr = currPtr;							// prev = current
		currPtr = currPtr->GetNext();				// current = next of current
	}
	prevPtr ->SetNext(ptr);
	ptr ->SetPrev(prevPtr);

	ptr ->SetX(cx);
	ptr ->SetY(cy);
	ptr ->SetDist(d);
	ptr ->SetFound(f);
	return firstPtr;
}

// 7. remove a node at the back
Doubly_linked_list *Remove_node_at_back ( Doubly_linked_list *firstPtr )
{
	Doubly_linked_list *prevPtr, *currPtr;

	currPtr = firstPtr;								// current  = first
	while ( currPtr ->GetNext() != 0 )
	{
		prevPtr = currPtr;							// prev = current
		currPtr = currPtr->GetNext();				// current = next of current
	}
	prevPtr ->SetNext(0);
	delete currPtr;
	return firstPtr;
}




circle circle_detection(int image[80][60], int length, int width)
{
//	bool found = false;
	circle sphere;
	sphere.distance = 0;
	sphere.x = 0;
	sphere.y = 0;
	int count = 0;
	int x_temp = 0;
	int y_temp = 0;
//	int x_match = 0;
	int y_match = 0;

//	if (found != false){
	for (int y = 0; y < width; y++)
		for (int x = 0; x < length; x++)
		{
			if (image[x][y] == 1)					// find dark pixel from 2d image
			{
				if (count == 0)
				{	x_temp = x;
					y_temp = y;}
				count++;
			}
			else									// a circle must able to form a square inside
			{
				if ( count > 2)						// ignore a dot
				{
					for (int j = y_temp; j < width ; j++)
					{
							if (image[x_temp][j] == 1)
							{	y_match++;	}
							else break;
					}
					if (y_match == count)// && (image[x_temp +count -1][y_temp +count -1] == 1))				// if x = y, it is a square/circle
					{
						sphere.x = x_temp;			// start pt. of x
						sphere.y = y_temp;			// start pt. of y
						sphere.distance = count;
				//		found = true;
						break;
					}
				}
				count = 0;
				y_match = 0;
			}
	}
	return sphere;
}




int main()
{
	circle test;
	bool first_run = true;
	bool temp_found = false;
	Doubly_linked_list *headPtr;
	Doubly_linked_list *newPtr;

	int map[80][60];
	for (int j = 0; j<60; j++)
	{
		for (int i = 0; i < 80; i++)
		{	map[i][j] = 0;	}
	}

// Test set 1
/*
	for (int count = 0; count < 4; count++)
	{
		map[5+count][5] = 1;
		map[5][5+count] = 1;
	}
*/
//	map[9][5] = 1;							// not a square
//	map[5][9] = 1;							// not a square

	test = circle_detection(map, 80, 60);
	if (test.distance == 0)
	{ temp_found = false; }
	else
	{ temp_found = true; }

	if (first_run == true)
	{
		headPtr = Create_Doubly_linked_list(test.x, test.y, test.distance, temp_found);
		first_run = false;
	}
	else
	{
//-------------------------
//		node_check
		headPtr = Insert_node_at_back(newPtr, headPtr, test.x, test.y, test.distance, temp_found);
/*------------------------
 *    if found
 *    Remove_node_at_back
 * 	  Remove_node_at_back
 *
 */

	}

	return 0;
}

