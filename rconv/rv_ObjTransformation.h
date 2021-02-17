/*-----------------------------------------------------------------------------------------------------------
	rv_ObjTransformation.h

	Written by Harvey Gilpin (Rv), Feb 97.

	Copyright Probe Entertainment Ltd,
	All rights reserved.
-----------------------------------------------------------------------------------------------------------*/
#ifndef _rv_ObjTransformation_H_
#define _rv_ObjTransformation_H_
#ifdef RV_TRACK_INCLUDES
	#pragma message( "********Including - " __FILE__ " ********") 
#endif
#include "jcclasses.h"
#include "PS_Undo.h"
//-----------------------------------------------------------------------------------------------------------
struct JCCLASS_DLL PS_FaceVertexGouraud
{
	bool		m_IsSet;
	U32			m_Index;
	JC_Colour	m_Colour;
};
//-----------------------------------------------------------------------------------------------------------
typedef CArray < PS_FaceVertexGouraud, const PS_FaceVertexGouraud& > PS_FaceVertexGouraudArray;
//-----------------------------------------------------------------------------------------------------------
class BA_PickedVertex;
class JCCLASS_DLL PS_AnimGouraudFaceUndo
{
	BOOL					m_Valid;
	JC_Point *				m_Point;
	PS_FaceVertexGouraudArray	m_Colours;
public:
	PS_AnimGouraudFaceUndo(void){ m_Point = NULL; }
	PS_AnimGouraudFaceUndo(const PS_AnimGouraudFaceUndo& undo); // copy constructor
	PS_AnimGouraudFaceUndo(JC_Point * point, const PS_FaceVertexGouraudArray& array);
	~PS_AnimGouraudFaceUndo(void);
	PS_AnimGouraudFaceUndo& operator = (const PS_AnimGouraudFaceUndo& undo); // equals operator
	JC_Point *						Point(void) { return m_Point; }
	PS_FaceVertexGouraudArray&		GouraudArray(void) { return m_Colours; }
};
//-----------------------------------------------------------------------------------------------------------
class JCCLASS_DLL PS_AnimGouraudUndo: public CArray < PS_AnimGouraudFaceUndo, const PS_AnimGouraudFaceUndo& >
{
public:
	PS_AnimGouraudUndo& operator = (const PS_AnimGouraudUndo& undo);
	PS_AnimGouraudUndo(const PS_AnimGouraudUndo& undo);
	PS_AnimGouraudUndo(void){}
};
//-----------------------------------------------------------------------------------------------------------
class JCCLASS_DLL rv_PreCalcVect
{
private:

	float	m_X;
	float	m_Y;
	float	m_Z;

	float	m_U;
	float	m_V;

	U8		m_R;
	U8		m_G;
	U8		m_B;

public:

	rv_PreCalcVect()		{}
	~rv_PreCalcVect()		{}

	float	X()	const		{ return m_X;	}
	float	Y()	const		{ return m_Y;	}
	float	Z()	const		{ return m_Z;	}

	U8		R()	const		{ return m_R;	}
	U8		G()	const		{ return m_G;	}
	U8		B()	const		{ return m_B;	}

	float	U()	const		{ return m_U;	}
	float	V()	const		{ return m_V;	}

	void	X(float val)	{ m_X = val;	}
	void	Y(float val)	{ m_Y = val;	}
	void	Z(float val)	{ m_Z = val;	}

	void	R(U8 val)		{ m_R = val;	}
	void	G(U8 val)		{ m_G = val;	}
	void	B(U8 val)		{ m_B = val;	}

	void	U(float val)	{ m_U = val;	}
	void	V(float val)	{ m_V = val;	}
	
	rv_PreCalcVect& operator = (const rv_PreCalcVect& src)
	{	X(src.X()); Y(src.Y()); Z(src.Z());
		R(src.R()); G(src.G()); B(src.B());
		U(src.U()); V(src.V());
		return *this;
	}

};
//-----------------------------------------------------------------------------------------------------------
typedef CArray<rv_PreCalcVect, rv_PreCalcVect>	rv_PreCalcVectArray;
typedef CMap <U32, U32, JC_Colour, JC_Colour&> PS_GouraudSetMap;
//-----------------------------------------------------------------------------------------------------------
class JCCLASS_DLL rv_ObjTransBase : public JC_ListElement
{
public:
	rv_ObjTransBase(PS_Database* db);
	
	// Getcher oats ear!
	enum TransformType { OAT_NONE, OAT_MORPH, OAT_TRANSLATE, OAT_ROTATE, OAT_TRIGGER, OAT_GOURAUD };
	enum TransformApplyBits
	{
		OAD_NONE				= 0x0000,
		OAD_MORE_TO_DO			= 0x0001,
		OAD_MORPH_APPLIED		= 0x0002,
		OAD_TRANSLATE_APPLIED	= 0x0004,
		OAD_ROTATE_APPLIED		= 0x0008,
		OAD_TRIGGER_APPLIED		= 0x0010,
		OAD_INACTIVE_MORPH		= 0x0020,
		OAD_GOURAUD_APPLIED		= 0x0040,
	};
	enum INTERPOLATION_TYPE
	{
		IT_RAMP,
		IT_SINE_WAVE,
		IT_FLICKER,
	};

	// Member variable access functions
	virtual U8		ListElementType(void) const		{ return OAT_NONE;			}
	virtual void	Copy(const JC_ListElement * source);
	void			Name(CString& Name)				{ m_Name = Name;			}
	void			Owner(JC_RecordRef& Owner)		{ m_Owner = Owner;			}
	void			StartTime(float Start)			{ m_StartTime = Start;		}
	INTERPOLATION_TYPE InterpolationType(void) const { return m_InterpolationType; }
	void			InterpolationType(const INTERPOLATION_TYPE type) { m_InterpolationType = type; }
	float			ExtraAmount(void) const			{ return m_ExtraAmount;	}
	float			InterpolationAmount(const float amount) const;
	void			ExtraAmount(const float amount) { m_ExtraAmount = amount; }
	void			Duration(float Duration)		{ m_Duration = Duration;	}
	void			Disable(BOOL tof)				{ m_Disabled = tof;			}
	CString			Name() const					{ return m_Name;			}
	JC_RecordRef&	Owner()							{ return m_Owner;			}
	float			StartTime()	const				{ return m_StartTime;		}
	float			Duration() const				{ return m_Duration;		}	
	BOOL			Disabled() const				{ return m_Disabled;		}
	PS_Database*	Database() const				{ return m_Database;		}	


	virtual void Insert(JC_Buffer &buf) const;
	virtual void Extract(JC_Buffer &buf);
	virtual void ResolveRefs(JC_UpgradeContext& upgrade, int item, JC_List * parentlist);

protected:
	// Member variables
	U16				m_Version;			
	CString			m_Name;				
	JC_RecordRef	m_Owner;			
	float			m_StartTime;		
	float			m_Duration;			
	float			m_ExtraAmount;
	INTERPOLATION_TYPE m_InterpolationType;

	//. Run time member variables
	BOOL			m_Disabled;
	PS_Database*	m_Database;
};
//-----------------------------------------------------------------------------------------------------------
class JCCLASS_DLL rv_ObjTransList : public JC_List
{
private:
	PS_Database*	m_Database;

public:
	rv_ObjTransList()			{ m_Database = NULL;}	
	virtual ~rv_ObjTransList()	{ }

	PS_Database*	Database()					{	return m_Database;	}
	void			Database(PS_Database *db)	{	m_Database = db;	}

public:	// Virtual overrides
	JC_ListElement * NewType(U8 Type);

};
//-----------------------------------------------------------------------------------------------------------
class JCCLASS_DLL rv_ObjTransTranslate : public rv_ObjTransBase
{
public:
	rv_ObjTransTranslate(PS_Database* db) : rv_ObjTransBase(db)	{ m_ApplyLocally = FALSE; }

	virtual U8		ListElementType(void) const		{ return OAT_TRANSLATE;		}

	void			Offset(D3DVECTOR &V)			{ m_Offset = V;				}
	void			ApplyLocally(BOOL tof)			{ m_ApplyLocally = tof;		}

	D3DVECTOR&		Offset()						{ return m_Offset;			}
	BOOL			ApplyLocally()					{ return m_ApplyLocally;	}

	virtual void	Insert(JC_Buffer &buf) const;
	virtual void	Extract(JC_Buffer &buf);
	virtual void Copy(const JC_ListElement * source);

	U32				Apply(float time_now, JC_Anchor& anchor);

protected:
	D3DVECTOR		m_Offset;
	BOOL			m_ApplyLocally;
};
//-----------------------------------------------------------------------------------------------------------
class JCCLASS_DLL rv_ObjTransRotate : public rv_ObjTransBase
{
public:
	rv_ObjTransRotate(PS_Database* db) : rv_ObjTransBase(db)	{ m_Version = 0x0003; m_ApplyLocally = FALSE;}

	virtual U8		ListElementType(void) const		{ return OAT_ROTATE;		}
	void			RotationPoint(D3DVECTOR &V)		{ m_RotationPoint = V;		}
	void			RotationAxis(D3DVECTOR &V)		{ m_RotationAxis = V;		}
	void			RotationValue(float Val)		{ m_RotationVal = Val;		}
	void			ApplyLocally(BOOL tof)			{ m_ApplyLocally = tof;		}

	D3DVECTOR&		RotationPoint()					{ return m_RotationPoint;	}
	D3DVECTOR&		RotationAxis()					{ return m_RotationAxis;	}
	float			RotationValue()					{ return m_RotationVal;		}
	BOOL			ApplyLocally()					{ return m_ApplyLocally;	}

	U32				Apply(float time_now, JC_Anchor& anchor);

	virtual void	Insert(JC_Buffer &buf) const;
	virtual void	Extract(JC_Buffer &buf);
	virtual void Copy(const JC_ListElement * source);

protected:
	D3DVECTOR		m_RotationPoint;
	D3DVECTOR		m_RotationAxis;
	float			m_RotationVal;
	BOOL			m_ApplyLocally;
};
//-----------------------------------------------------------------------------------------------------------
class JCCLASS_DLL rv_ObjTransTrigger : public rv_ObjTransBase
{
public:
	rv_ObjTransTrigger(PS_Database *db);
	~rv_ObjTransTrigger();

	virtual U8		ListElementType(void) const		{ return OAT_TRIGGER;		}

	JC_Property*	Property()						{ return m_Property;		}
	void			Property(JC_Property *p)		{ delete m_Property; m_Property = p; }

	virtual void	Insert(JC_Buffer &buf) const;
	virtual void	Extract(JC_Buffer &buf);
	virtual void Copy(const JC_ListElement * source);

protected:

	JC_Property*	m_Property;
};
//-----------------------------------------------------------------------------------------------------------
class JCCLASS_DLL rv_ObjTransMorph : public rv_ObjTransBase
{
public:
	void MoveStartTimeForward(float time);
	rv_ObjTransMorph(PS_Database *db) : rv_ObjTransBase(db), m_MorphObject(db) {}

	virtual U8		ListElementType(void) const		{ return OAT_MORPH;			}

	void			MorphObject(JC_RecordRef& M)	{ m_MorphObject = M;		}
	JC_RecordRef& 	MorphObject()					{ return m_MorphObject;		}

	virtual void	Insert(JC_Buffer &buf) const;
	virtual void	Extract(JC_Buffer &buf);

	U32				Apply(float time_now, rv_PreCalcVectArray *vert_array, rv_PreCalcVectArray *back_vert_array, JC_Template *base_tmpl, JC_Anchor& anchor);

	virtual void ResolveRefs(JC_UpgradeContext& upgrade, int item, JC_List * parentlist);
	virtual void Copy(const JC_ListElement * source);

protected:
	JC_RecordRef	m_MorphObject;
	rv_ObjTransList	m_OtherKeyFrames;
};
//-----------------------------------------------------------------------------------------------------------
typedef CMap<U32, U32, JC_Colour, JC_Colour&> PS_ColourMap;
typedef PS_UndoArray < PS_AnimGouraudUndo > PS_AnimGouraudUndoArray;
class JCCLASS_DLL rv_ObjTransGouraud : public rv_ObjTransBase
{
	PS_ColourMap m_PointMap;
	PS_AnimGouraudUndoArray m_Undo;
public:
	rv_ObjTransGouraud(PS_Database * db) : rv_ObjTransBase(db) {}
	virtual U8 ListElementType(void) const { return OAT_GOURAUD; }

	U32 Apply(float time_now, rv_PreCalcVectArray *vert_array, PS_GouraudSetMap * set_map);
	bool GetVertexColour(JC_Point& point, JC_Colour& colour) const;
	void SetVertexColour(JC_Point& point, JC_Colour& colour);
	void RemoveVertexColour(JC_Point& point);

	virtual void Insert(JC_Buffer &buf) const;
	virtual void Extract(JC_Buffer & buf);
	virtual void Copy(const JC_ListElement * source);

	bool CanRedo(void) const { return (m_Undo.CanRedo() == TRUE); }
	bool CanUndo(void) const { return (m_Undo.CanUndo() == TRUE); }
	PS_AnimGouraudUndo * GetRedo(void) { return m_Undo.GetRedo(); }
	PS_AnimGouraudUndo * GetUndo(void) { return m_Undo.GetUndo(); }
	void Undo(PS_AnimGouraudUndo * undo) { m_Undo.Undo(undo); }
	void Redo(PS_AnimGouraudUndo * redo) { m_Undo.Redo(redo); }
	void SetUndo(PS_AnimGouraudUndo * undo) { m_Undo.SetUndo(undo); }
};
//-----------------------------------------------------------------------------------------------------------
#endif // Bouncer
