#ifndef _NAMEDLISTS_H
#define _NAMEDLISTS_H

//-------------------------------------------------------------------------------------------------
// IndexVector
//
// Simply a pseudonym for a vector of U32 objects, for use in those circumstances when a list
// of indices is required
//-------------------------------------------------------------------------------------------------
typedef vector<U32> IndexVector;

//-------------------------------------------------------------------------------------------------
// IndexVectorCompare
//
// Function object usable as a sorting callback function
// Returns (left < right)
//-------------------------------------------------------------------------------------------------
class IndexVectorCompare
{
	public:
		bool operator()(const IndexVector* left, const IndexVector* right) const
		{
			if(left->size() == right->size())
			{
				U32 pos = 0;
				while(pos < left->size() && ((*left)[pos] == (*right)[pos]))
				{
					pos++;
				}
				if(pos >= left->size())
				{
					return false;
				}
				return ((*left)[pos] < (*right)[pos]);
			}
			return (left->size() < right->size());
		}
};

//-------------------------------------------------------------------------------------------------
// SmallIndexFirstVector
//
// A version IndexVector which also has the facility to rotate its vector circularly until
// the first member is the smallest
//-------------------------------------------------------------------------------------------------
class SmallIndexFirstVector : public IndexVector
{
	public:
		U32 SmallestFirst(void);
};

//-------------------------------------------------------------------------------------------------
// NamedVector
//
// template class for a container of objects
// the class also has a name attribute which can only be set during construction
// but which can be accessed at any time
//-------------------------------------------------------------------------------------------------
template <class COMMODITY> class NamedVector: public vector<COMMODITY>
{
	public:
		NamedVector(const string& name){TheName = name;};
		const string& Name(void) const {return TheName;};
	protected:
		string TheName;
};

//-------------------------------------------------------------------------------------------------
// PointerVector
//
// template class for a container of pointers (the memory pointed to is deemed to be owned by this class)
//
// Note the template should be instantiated with...
// PointerVector<CLASS>
// ...not with...
// PointerVector<CLASS*>
// ...unless you actually want to store pointers to pointers
//-------------------------------------------------------------------------------------------------
template <class COMMODITY> class PointerVector: public vector<COMMODITY*>
{
	public:
		~PointerVector();
		U32 Insert(const COMMODITY* insertion);
};

//-------------------------------------------------------------------------------------------------
// PointerVector destructor
//
// Simply disposes of all the contained objects
//-------------------------------------------------------------------------------------------------
template <class COMMODITY> PointerVector<COMMODITY>::~PointerVector()
{
	vector<COMMODITY*>::iterator i = begin();
	vector<COMMODITY*>::iterator finish = end();

	while(i != finish)
	{
		delete (*i);
		i++;
	};
}

template <class COMMODITY> U32 PointerVector<COMMODITY>::Insert(const COMMODITY* insertion)
{
	U32 index;				//variable for return value

	index = size();			//index will be to the next object added to the vector
	COMMODITY* inserted = (COMMODITY*) insertion;	//create a new object
	push_back(inserted);	//add the inserted item to the 

	return index;	//return result to caller
}

//-------------------------------------------------------------------------------------------------
// NamedPointerVector
//
// Based on PointerVector the class also has a name attribute which can only be set during construction
// but which can be accessed at any time
//
// Note the template should be instantiated with...
// NamedPointerVector<CLASS>
// ...not with...
// NamedPointerVector<CLASS*>
// ...unless you actually want to store pointers to pointers
//-------------------------------------------------------------------------------------------------
template <class COMMODITY> class NamedPointerVector: public PointerVector<COMMODITY>
{
	public:
		NamedPointerVector(const string& name){TheName = name;};
		const string& Name(void) const {return TheName;};
	protected:
		string TheName;
};

#endif