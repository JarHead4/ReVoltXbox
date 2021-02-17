#ifndef _BA_3DVIEWSTYLE_H
#define _BA_3DVIEWSTYLE_H

#ifdef BA_TRACK_INCLUDES
	#pragma message( "********Including - " __FILE__ " ********") 
#endif

#include <BA_3DViewFilter.h>

class JCCLASS_DLL BA_3DViewStyle : public CArray<BA_3DViewFilter, BA_3DViewFilter&>
{
public:
	void	ChangeKids(BA_3DViewFilter::CHANGETYPE changetype);
	enum	{CURRENT_VERSION = 1};
	//Construction
	BA_3DViewStyle();
	BA_3DViewStyle(CString& description, BA_FilterType defaultmode = GOURAUDSHADED);
	BA_3DViewStyle(const BA_3DViewStyle& other);
	//Operators
	BA_3DViewStyle& operator=(const BA_3DViewStyle& other);
	//Attributes
	const CString&	Description(void) const {return m_Description;};
	void			Description(const CString& newdescription);
	BA_FilterType	DefaultMode(void) const;
	void			DefaultMode(BA_FilterType mode){m_DefaultMode = mode;};
	
	//database support functions
	void			Dependencies(PS_CompressRecordList* list) const;
	bool			RemovePropertyNameReference(const JC_PropertyName * name);
	void			Extract(JC_Buffer& buf);
	void			Insert(JC_Buffer& buf) const;
	friend JC_Buffer& operator << (JC_Buffer& buf, const BA_3DViewStyle& style)	{ style.Insert(buf); return buf; }
	friend JC_Buffer& operator >> (JC_Buffer& buf, BA_3DViewStyle& style)		{ style.Extract(buf); return buf; }

protected:
	//attributes
	CString				m_Description;
	BA_FilterType		m_DefaultMode;
};

class JCCLASS_DLL BA_3DViewStyleCollection : public CArray<BA_3DViewStyle, BA_3DViewStyle&>
{
public:
	void ChangeKids(BA_3DViewFilter::CHANGETYPE changetype);
	enum	{CURRENT_VERSION = 3};
	//constructors
	BA_3DViewStyleCollection();
	BA_3DViewStyleCollection(BA_3DViewStyleCollection& other);
	//operators
	BA_3DViewStyleCollection&	operator = (const BA_3DViewStyleCollection& other);
	BA_3DViewStyleCollection&	operator += (const BA_3DViewStyleCollection& other);
	//access functions
	BA_FilterTypeArray&			Filters(void) {return m_Filters;};
	S32							DefaultStyleNumber(void){return m_Default;};
	BOOL						DefaultStyleNumber(S32 value);
	const BA_3DViewStyle&		GetStyleFromIndex(S32 index) const;
	void						Dependencies(PS_CompressRecordList* list) const;
	bool RemovePropertyNameReference(const JC_PropertyName * name);

	void Extract(JC_Buffer& buf);
	void Insert(JC_Buffer& buf) const;

	friend JC_Buffer& operator << (JC_Buffer& buf, const BA_3DViewStyleCollection& styles)	{ styles.Insert(buf); return buf; }
	friend JC_Buffer& operator >> (JC_Buffer& buf, BA_3DViewStyleCollection& styles)		{ styles.Extract(buf); return buf; }
protected:
	void AddStandardStyles(void);
	//helper functions
	void ReadStyles(JC_Buffer& buf);
	void SkipFilters(JC_Buffer& buf);
	//attributes
	S32						m_Default;
	BA_FilterTypeArray		m_Filters;
	const BA_3DViewStyle	m_DefaultStyle;
};

#endif
// NOTE - These classes are almost useable as base classes for other styles, such as 
// 'Which objects should be scanned for textures when doing drag-drop operations onto
// a Texture Set Editor window'
// The following is a working summary of the changes needed to produce the base classes
// which would be used to derive the new classes from. (And for that matter, the new
// versions of BA_3DViewStyle & BA_3DViewStyleCollection
//
//
//----------------------------
// BA_3DViewStyle
//----------------------------
// Atributes/functions which will be standard to all derived classes.
//
// Constructor(s)....
// BA_BaseStyle(void)											//default constructor (needed for CArray)
// BA_BaseStyle(CString& description, BA_FilterType default);	//construct a specifically named style, with a default
// BA_BaseStyle(const BA_3DViewStyle& other);					//copy constructor
// 
// CString			m_Description;								//all styles should have a description
// const CString&	Description(void) const ;					//the description should be accessible
// void				Description(const CString& newdescription); //and it should be changeable
//
// BA_FilterType	m_Default;									//there should be a default course of action
//																//for those objects which don't match any of the styles' filters
// BA_FilterType	DefaultMode(void) const ;					//the default should be accessible
// void				DefaultMode(BA_FilterType mode);			//and it should be changeable
//