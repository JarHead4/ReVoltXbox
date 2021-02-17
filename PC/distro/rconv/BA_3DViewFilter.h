#ifndef _BA_3DVIEWFILTERS_H
#define _BA_3DVIEWFILTERS_H

#ifdef BA_TRACK_INCLUDES
	#pragma message( "********Including - " __FILE__ " ********") 
#endif

enum BA_FilterType {DEFAULTFILTER, HIDDEN, WIREFRAME, FLATSHADED, GOURAUDSHADED, FLATTEXTURED, PLAINGOURAUD,
					NUMBEROFFILTERS,
					 HIDDEN_C = HIDDEN | 0x8000, WIREFRAME_C, FLATSHADED_C, GOURAUDSHADED_C, FLATTEXTURED_C, PLAINGOURAUD_C};

typedef CArray<BA_FilterType, BA_FilterType> BA_FilterTypeArray;

class JCCLASS_DLL BA_3DViewFilter
{
public:
	enum {CURRENT_VERSION = 3};
	typedef enum CHANGETYPE {CHANGE_SET, CHANGE_CLEAR, CHANGE_INVERT};
	BA_3DViewFilter();
	inline U32				Property(void) const {return m_Property;};
	inline U32				Value(void) const {return m_Value;};
	inline BA_FilterType	Type(void) const {return m_Type;};
	inline void				Property(U32 data) {m_Property = data;};
	inline void				Value(U32 data) {m_Value = data;};
	inline void				Type(BA_FilterType data) {m_Type = data;};
	void					Dependencies(PS_CompressRecordList* list) const;
	bool					RemovePropertyNameReference(const JC_PropertyName * name);
	
	static bool				NameFromType(BA_FilterType type, CString& name);
	void					ChangeKids(BA_3DViewFilter::CHANGETYPE changetype);

	void Extract(JC_Buffer& buf);
	void Insert(JC_Buffer& buf) const;

	friend JC_Buffer& operator << (JC_Buffer& buf, const BA_3DViewFilter& filter)	{ filter.Insert(buf); return buf; }
	friend JC_Buffer& operator >> (JC_Buffer& buf, BA_3DViewFilter& filter)		{ filter.Extract(buf); return buf; }

protected:
	static const char *		Names[NUMBEROFFILTERS];
	U32				m_Property;
	U32				m_Value;
	BA_FilterType	m_Type;
};


#endif
