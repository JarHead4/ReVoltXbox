#include "stdafx.h"
#include "MappedVector.h"

MappedVector::~MappedVector()
{
	VERTEXVECTOR::iterator i   = VertexVector.begin();	//start at the beggining
	VERTEXVECTOR::iterator end = VertexVector.end();	//find the end
	while(i != end)		//while not at the end of the collection
	{
		delete (*i);	//delete an item
		i++;			//then move onto the next one
	};
}

U32 MappedVector::ObtainIndexFor(const RevoltVertex& target)
{
	U32 index;
	VERTEXMAP::iterator i;
	RevoltVertex localvert = target;
	i = VertexMap.find(&localvert);
	if(i != VertexMap.end())
	{
		index = (*i).second;
	}
	else
	{
		index = VertexVector.size();
		RevoltVertex* inserted = new RevoltVertex(target);
		VertexVector.push_back(inserted);
		VertexMap[inserted] = index;
	}
	return index;
}
