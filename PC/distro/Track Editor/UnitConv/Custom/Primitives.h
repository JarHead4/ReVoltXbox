#ifndef _PRIMITIVES_H
#define _PRIMITIVES_H

#include "structs.h"
#include "MappedVector.h"
#include "NamedLists.h"
#include <vector>

using namespace std;

typedef struct
{
    #ifdef _PSX
        unsigned short Red:5;
        unsigned short Green:5;
        unsigned short Blue:5;
        unsigned short Transparent:1;
    #else
        U8 Red;
        U8 Green;
        U8 Blue;
        U8 Transparent;
    #endif
}RevoltRGB;


//-------------------------------------------------------------------------------------------------
// RevoltVertexCompare
//
// Function object used by the (MappedVector based) VertexMap as a 'less<>' predicate
// Returns a bool to indicate whether 'left' is deemed to be less than 'right'
// The X coord is used as the major key followed by the Y & Z coords respectively.
//-------------------------------------------------------------------------------------------------
class RevoltVertexCompare
{
	public:
		bool operator()(const RevoltVertex* left, const RevoltVertex* right) const
		{
			if(left->X == right->X)
			{
				if(left->Y == right->Y)
				{
					if(left->Z == right->Z)
					{
						return false;
					}
					else
					{
						return (left->Z < right->Z);
					}
				}
				else
				{
					return (left->Y < right->Y);
				}
			}
			else
			{
				return (left->X < right->X);
			}
		}
};

//Define a class to contain a unique set of RevoltVertex objects
typedef MappedVector<RevoltVertex, RevoltVertexCompare> VertexMap;

//-------------------------------------------------------------------------------------------------
// RevoltPolygon
//
// Represents a polygon as a list of indices into a vector of vertices
//-------------------------------------------------------------------------------------------------
typedef SmallIndexFirstVector RevoltPolygon;

typedef struct 
{
    U16 Width;
    U16 Height;
    U8  BitDepth;
    U8  PaletteSize;
}RevoltTPageHeader;

typedef struct
{
    RevoltTPageHeader Header;
    RevoltRGB		  Palette[256];
    U8                Image[1];
}RevoltTPage;

//-------------------------------------------------------------------------------------------------
// RevoltUVPolygon
//
// Represents a UV polygon as a list of indices into a vector of UV coords
//-------------------------------------------------------------------------------------------------
typedef SmallIndexFirstVector RevoltUVPolygon;

typedef MappedVector<RevoltUVPolygon, IndexVectorCompare> UVPolygonMap;

//-------------------------------------------------------------------------------------------------
// RevoltUVCoordCompare
//
// Function object used by the (MappedVector based) UVPolygonMap as a 'less<>' predicate
// Returns a bool to indicate whether 'left' is deemed to be less than 'right'
// Sorts by 'u' component, then by 'v' component
//-------------------------------------------------------------------------------------------------
class RevoltUVCoordCompare
{
	public:
		bool operator()(const RevoltUVCoord* left, const RevoltUVCoord* right) const
		{
			if(left->U == right->U)
			{
				return (left->V < right->V);
			}
			return (left->U < right->U);
		}
};

typedef MappedVector<RevoltUVCoord, RevoltUVCoordCompare> UVCoordMap;

typedef struct
{
	INDEX   TPageID;
	INDEX   PolyID;
	U8	    Rotation;
	bool	Reversed;
}UVPolyInstance;

typedef PointerVector<UVPolyInstance> UVPolyInstanceVector;
typedef vector<U32> SurfaceVector;

class RevoltTrackUnit
{
	public:
	    INDEX MeshID;  /* Index into the Mesh Array */
		UVPolyInstanceVector UVPolySet; /* Index into TrackUnitSet::UVPolygonSets */
		SurfaceVector		 SurfaceSet; /* list of surface materials for each collision poly */
};

typedef PointerVector<TRACKZONE> TrackZoneVector;

class RevoltModule : public NamedPointerVector<RevoltTrackUnitInstance>
{
	public:
		RevoltModule(const string& name);
		TrackZoneVector Zones;
};

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
typedef MappedVector<RevoltPolygon, IndexVectorCompare> PolygonMap;
typedef NamedVector<U32> RevoltPolySet;
typedef NamedPointerVector<RevoltPolySet> RevoltMesh;
typedef PointerVector<RevoltMesh> MeshVector;
typedef PointerVector<RevoltTrackUnit> RevoltTrackUnitSet;
typedef PointerVector<RevoltModule> RevoltModuleSet;

class U32Compare
{
	public:
		bool operator()(const U32* left, const U32* right) const
		{
			return (*left < *right);
		}
};

typedef MappedVector<U32, U32Compare> TPageRecnumMap;

class RevoltTheme
{
	protected:
		string				TheName;
		VertexMap			VertexPool;
		PolygonMap			PolygonPool;
		MeshVector			MeshPool;
		RevoltModuleSet		ModulePool;
		RevoltTrackUnitSet  UnitPool;
		UVCoordMap			UVCoordPool;
		UVPolygonMap		UVPolygonPool;
		TPageRecnumMap		TPageRecnumPool;

	public:
		RevoltTheme(const string& name){TheName = name;};

		U32 VertexCount(void){return VertexPool.size();};
		U32 PolygonCount(void){return PolygonPool.size();};
		U32 MeshCount(void){return MeshPool.size();};
		U32 ModuleCount(void){return ModulePool.size();};
		U32 UnitCount(void){return UnitPool.size();};
		U32 UVCoordCount(void){return UVCoordPool.size();};
		U32 UVPolyCount(void){return UVPolygonPool.size();};
		U32 TPageRecnumCount(void){return TPageRecnumPool.size();};
		
		const RevoltVertex*       Vertex(U32 index){return VertexPool[index];};
		const RevoltPolygon*      Polygon(U32 index){return PolygonPool[index];};
		const RevoltMesh*		  Mesh(U32 index){return MeshPool[index];};
		const RevoltModule*	      Module(U32 index){return ModulePool[index];};
		const RevoltTrackUnit*    Unit(U32 index){return UnitPool[index];};
		const RevoltUVCoord*	  UVCoord(U32 index){return UVCoordPool[index];};
		const RevoltUVPolygon*	  UVPolygon(U32 index){return UVPolygonPool[index];};
		U32						  TPageRecnum(U32 index){return *(TPageRecnumPool[index]);};

		U32 InsertVertex(const RevoltVertex& vertex){return VertexPool.Insert(vertex);};
		U32 InsertPolygon(const RevoltPolygon& polygon){return PolygonPool.Insert(polygon);};
		U32 InsertMesh(const RevoltMesh* mesh){return MeshPool.Insert(mesh);};
		U32 InsertModule(const RevoltModule* module){return ModulePool.Insert(module);};
		U32 InsertUnit(const RevoltTrackUnit* unit){return UnitPool.Insert(unit);};
		U32 InsertUVCoord(const RevoltUVCoord& uvcoord){return UVCoordPool.Insert(uvcoord);};
		U32 InsertUVPolygon(const RevoltUVPolygon& uvpoly){return UVPolygonPool.Insert(uvpoly);};
		U32 InsertTPageRecnum(U32 recnum){return TPageRecnumPool.Insert(recnum);};
};

#endif