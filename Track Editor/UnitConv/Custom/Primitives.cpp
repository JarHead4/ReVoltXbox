#include "stdafx.h"
#include "primitives.h"
#include <algorithm>

//-------------------------------------------------------------------------------------------------
// SmallIndexFirstVector::SmallestFirst
//
// Rotates the list of indices so that the one with the smallest value is at the head of the list
// This allows us to compare objects for equality more quickly by simply checking for equality of
// each entry with the entry at the corresponding index in the object being compared
//
// Returns:- number of positions that the vector was rotated (clockwise)
//-------------------------------------------------------------------------------------------------
U32 SmallIndexFirstVector::SmallestFirst(void)
{
	U32 index_of_smallest = 0;	//assume the first entry is the smallest (the list has already be adjusted)
	if(size() > 1)				//only adjust the list if there is more than 1 entry
	{
		U32 which = 1;			//start looking at the second entry
		while(which < size())	//as long as we are not at the end of the list
		{
			if((*this)[which] < (*this)[index_of_smallest])	//if this entry is smaller than the previous smallest
			{
				index_of_smallest = which;	//make a note of it
			}
			which++;	//next entry
		}
		if(index_of_smallest != 0)	//if the smallest entry is not the first
		{
			RevoltPolygon::iterator start;	
			RevoltPolygon::iterator finish;
			RevoltPolygon::iterator middle;
			start  = begin();
			finish = end();
			middle = start + index_of_smallest;
			rotate(start, middle, finish) ;	//shift the list so that the smallest comes first
		}
	}
	return index_of_smallest;
}

