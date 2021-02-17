#ifndef _MAPPEDVECTOR_H
#define _MAPPEDVECTOR_H

#pragma warning(disable:4786)
#pragma warning(disable:4663)

#include <map>
#include <vector>
#include <algorithm>

using namespace std;

//-------------------------------------------------------------------------------------------------
// MappedVector template class
//
// This template class provides a container (a method for maintaining a collection of objects),
// within which no two objects have the same value. In addition to this functionality, which could
// provided by the 'map' template class, it also provides a means of accessing those objects via
// a vector style operator[] function.
// The container manages its own internal copies of the objects passed to it and consequently
// deletes them as part of its destruction process.
// 
// Usage:-
// 
// Contains(); - return true or false depending on whether the specified object matches one of the
//				 contained objects
//
// Insert(); - checks if the object is already in the container and, if not, adds a copy of it
//			   in both cases the index which can be used to access a pointer to the object concerned
//
// operator[] - will return a pointer to the specified object 
//
// Size(); - returns the number of object in the container
//
//-------------------------------------------------------------------------------------------------
template <class COMMODITY, class COMPAROR> class MappedVector
{
	protected:
		typedef map<COMMODITY*, U32, COMPAROR> CommodityMap; //define a type for a map of COMMODITY pointers
		typedef vector<COMMODITY*> CommodityVector;			 //define a type for a vector of COMMODITY pointers

		CommodityMap TheMap;		//instantiate one of each
		CommodityVector TheVector;
	public:
		~MappedVector();			//we need a destructor to handle deletion of contained objects
		bool Contains(const COMMODITY& target);	//specifies whether a given object is in the container
		U32 Insert(const COMMODITY& target);	//
		const COMMODITY* operator[] (U32 index){return TheVector[index];};
		U32 size(void){return TheVector.size();};
};

template <class COMMODITY, class COMPAROR> MappedVector<COMMODITY, COMPAROR>::~MappedVector()
{
	CommodityVector::iterator i = TheVector.begin();
	CommodityVector::iterator end = TheVector.end();

	while(i !=end)
	{
		delete (*i);
		i++;
	};
}

template <class COMMODITY, class COMPAROR> U32 MappedVector<COMMODITY, COMPAROR>::Insert(const COMMODITY& target)
{
	U32 index;				//variable for return value

	CommodityMap::iterator i;
	
	i = TheMap.find((COMMODITY*)&target); //check to see if we already control the object
	
	if(i != TheMap.end())	//if we do
	{
		index = (*i).second;	//then obtain the index into 'TheVector'
	}
	else
	{
		index = TheVector.size();	//index will be to the next object added to 'TheVector'
		COMMODITY* inserted = new COMMODITY(target);	//create a new object
		TheVector.push_back(inserted);					//add its address to 'TheVector'
		TheMap[inserted] = index;						//map its value to its index
	}
	return index;	//return result to caller
}

template <class COMMODITY, class COMPAROR> bool MappedVector<COMMODITY, COMPAROR>::Contains(const COMMODITY& target)
{
	CommodityMap::iterator i;
	
	i = TheMap.find((COMMODITY*)&target); //check to see if we already control the object
	
	return (i != TheMap.end());	//return result to caller
}

#endif