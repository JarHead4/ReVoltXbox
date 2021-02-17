/*-----------------------------------------------------------------------------------------------------------
	rv_ObjAnimation.h

	Written by Harvey Gilpin (Rv), Feb 97.

	Copyright Probe Entertainment Ltd,
	All rights reserved.
-----------------------------------------------------------------------------------------------------------*/
#ifndef _rv_ObjAnimation_H_
#define _rv_ObjAnimation_H_
#ifdef RV_TRACK_INCLUDES
	#pragma message( "********Including - " __FILE__ " ********") 
#endif
//-----------------------------------------------------------------------------------------------------------
#include "jcclasses.h"
#include "rv_ObjTransformation.h"
//-----------------------------------------------------------------------------------------------------------
//. rv_AnimatedModel - Base class for handling run time part of animations
//-----------------------------------------------------------------------------------------------------------
class JCCLASS_DLL rv_AnimationEngine
{

public:

	rv_AnimationEngine(PS_Database * db);
	~rv_AnimationEngine();

	void					SetOwner(rv_Model *owner);

	U32						CalcFrame(const JC_Anchor *anchor, float start, float now, PS_CompressRecordList *dirty_list = NULL);

	void					ResetAnimationData(const JC_Anchor *anchor);
	const JC_Anchor&		PreCalcAnchor(void) const		{ return m_PreCalcAnchor;				}
	rv_PreCalcVectArray*	PreCalcVertexList()				{ return &m_PreCalcVectArray;			}
	rv_PreCalcVectArray*	PreCalcBackVertexList()			{ return &m_PreCalcBackVectArray;		}

protected:

	//. Run time data
	JC_Anchor				m_PreCalcAnchor;			//. Anchor for translate/rotate transformations
	rv_PreCalcVectArray		m_PreCalcVectArray;			//. For use with morphs
	rv_PreCalcVectArray		m_PreCalcBackVectArray;		// same for the back mash
	PS_GouraudSetMap		m_GouraudSetMap;			//. For use with Gouraud animation
	JC_Template				*m_MorphBase;				//. Current morph keyframe template.
	rv_Model				*m_Owner;					//. Obj/Template that we are attached to.

	U32						CalcFrameForChildren(JC_RecordRefList& child_list, float start, float now, PS_CompressRecordList *dirty_list);
	void					FindCurrentMorphBase(float time_so_far);
	void					SetMorphBase(JC_Template *tmpl);
	void					ResetVertexList();

	virtual void ResolveRefs(JC_UpgradeContext& upgrade);
};
//-----------------------------------------------------------------------------------------------------------
//. rv_AnimatedTemplate - Template for animtated 3D world objects
//-----------------------------------------------------------------------------------------------------------
class JCCLASS_DLL rv_AnimatedTemplate : public JC_Template
{
private:
	//. Saved data:
	JC_RecordRef			m_StaticNum;
	rv_ObjTransList			m_TransformList;		//. List of transformations for this template
	float					m_Duration;				//. Duration in seconds

	//. Runtime
	rv_StaticTemplate		*m_Static;
	rv_AnimationEngine		m_AnimEngine;			//. Animation handler

protected:
public:
	rv_AnimatedTemplate(PS_Database * db, U32 recnum);
	virtual ~rv_AnimatedTemplate();
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);

	//. Rerouting functions
	void					AttachFace(JC_Face * f)		{ m_Static->AttachFace(f);		}
	void					AttachVertex(JC_Vertex * v)	{ m_Static->AttachVertex(v);	}

	void GetTextureList(PS_UniqueRecordList * list) const	{ m_Static->GetTextureList(list);	}

	JC_Pin*					FindPin(U32 pinid);
	JC_Vertex*				FindVertex(U32 vertid)		{ return FindVertex(vertid);		}


	JC_Data*	CopyPropertyData(const CString& name)	{ return m_Static->CopyPropertyData(name);	}

	void		ModifyProperty(const CString& name, JC_Data * data, JC_Property::Switch sw)
	{
		m_Static->ModifyProperty(name, data, sw);
	}

	JC_FaceList*	FaceList(void) const				{ return m_Static->FaceList();		}
	JC_VertexList*	VertexList(void) const				{ return m_Static->VertexList();	}

	//. Animation specific functions
	void					SetStaticTemplate(U32 num);
	rv_StaticTemplate*		StaticTemplatePtr()			{ if (m_Static) m_Static->AddRef(); return m_Static; }
	rv_ObjTransList*		TransList()					{ return &m_TransformList;			}
	void					SortTransList();
	float					Duration()					{ return m_Duration;				}
	void					Duration(float new_time)	{ m_Duration = new_time;			}

	void					CascadeDurationChange(float new_time, float scale);
	
	U32						CalcFrame(float start, float now, PS_CompressRecordList *dirty_list = NULL);

	const JC_Anchor&		RealAnchor(void) const		{ return rv_Model::Anchor();				}
	const JC_Anchor&		Anchor(void) const			{ return m_AnimEngine.PreCalcAnchor();		}
	rv_PreCalcVectArray*	PreCalcVertexList()			{ return m_AnimEngine.PreCalcVertexList();	}
	rv_PreCalcVectArray*	PreCalcBackVertexList()		{ return m_AnimEngine.PreCalcBackVertexList(); }

	void					PinTo(rv_Model* parent, U32 pinid, const JC_Vector& up, const JC_Vector& fwd);

	void					Save() const;
	void					SaveTree() const;

public:	// Debugging
	virtual void ReportRecordRef(JC_RecordRefList& l)	{ m_Static->ReportRecordRef(l);		}

public:	// Virtual overrides
	virtual U16				Type(void)		const		{ return TypeAnimatedTemplate;	}
	virtual BOOL			IsAnimated()	const		{ return TRUE;					}
	virtual bool			Initialise(void); // Called after ReadBody to perform any required initialisation
	virtual void			WriteBody(void);
	virtual void			ReadBody(void);
	virtual void			Dependencies(PS_CompressRecordList * list) const;
	virtual void			Copy(const JC_Template * src_tmp);
	virtual void			CopyTree(const JC_Template * src_tmp);
	virtual U32				NumberOfRefs(const JC_PropertyName * name)			{ return m_Static->NumberOfRefs(name);	}
	virtual bool			RemovePropertyNameReference(const JC_PropertyName * name);
	virtual void			AddPropertyNameUsage(JC_RecordRefList& list) const	{ m_Static->AddPropertyNameUsage(list);	}
	virtual U32				Synchronise();
	virtual JC_PropertyList * CreateBackgroundPropertyList(void) const;
	virtual void			ResolveRefs(JC_UpgradeContext& upgrade);
	virtual void			CopyAcross(JC_UpgradeContext& upgrade);
	virtual bool			CreateCopyRecordList(CDWordArray& list) const;
	virtual bool			Delete(void);

public:	// Debugging
	bool AppendAnimation(rv_AnimatedTemplate * source_template);
	virtual CString TypeString(void) const { return CString("AnimatedTemplate"); }
protected:
	virtual BOOL			IsInstancedByRecord(U32 record_number) const;
};
//-----------------------------------------------------------------------------------------------------------
//. rv_AnimatedObject - Objects that use the AnimatedTemplates
//-----------------------------------------------------------------------------------------------------------
class JCCLASS_DLL rv_AnimatedObject : public JC_Object
{

private:

	//. Member vairables
	float					m_Speed;				//. Amount to scale template's animation by. (not used)

	rv_AnimationEngine		m_AnimEngine;			//. Animation handler

public:

	rv_AnimatedObject(PS_Database * db, U32 recnum);
	virtual					~rv_AnimatedObject() {}


	U32						CalcFrame(float start, float now, PS_CompressRecordList *dirty_list = NULL);

	const JC_Anchor&		RealAnchor(void) const		{ return rv_Model::Anchor();				}
	const JC_Anchor&		Anchor(void) const			{ return m_AnimEngine.PreCalcAnchor();		}
	rv_PreCalcVectArray*	PreCalcVertexList()			{ return m_AnimEngine.PreCalcVertexList();	}
	rv_PreCalcVectArray*	PreCalcBackVertexList()		{ return m_AnimEngine.PreCalcBackVertexList(); }

	void					PinTo(rv_Model* parent, U32 pinid, const JC_Vector& up, const JC_Vector& fwd);

public:	// Virtual overrides

	virtual void			WriteBody(void);
	virtual void			ReadBody(void);
	virtual BOOL			IsAnimated()	const		{ return TRUE;							}
	virtual U16				Type()			const		{ return TypeAnimatedObject;			}
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);

public:	// Debugging

	virtual CString			TypeString(void)			{ return CString("AnimatedObject");		}

};
//-----------------------------------------------------------------------------------------------------------
#endif // Bouncer