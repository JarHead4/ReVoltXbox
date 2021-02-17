




#ifndef _AD_LINK_
#define _AD_LINK_

#include <JCINCLUDES.H>

class JCCLASS_DLL AD_Link : public JC_PropertyRecord
{
	public:			
		enum DirectionType { StartEnd = 0, BiDirectional = 1};
	 
		AD_Link(PS_Database *db, U32 recnum);
		~AD_Link();

		virtual bool Delete(void);
		virtual void ReadBody(void);
		virtual void WriteBody(void);
		virtual U16 Type(void) const;
		virtual void ResolveRefs(JC_UpgradeContext& upgrade);
		virtual void Dependencies(PS_CompressRecordList *List) const;
		virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
		virtual CString TypeString(void) const	{ return CString("Link"); }

		void SetLink(rv_Model *NewStartObject, rv_Model *NewEndObject);
		void SetStartObject(rv_Model *NewStartObject);
		void SetEndObject(rv_Model *NewEndObject);

		void SetLinkDirection(DirectionType NewLinkDirection);
		DirectionType GetLinkDirection(void) const;

		U32 GetStartObjectRecordNumber(void) const;
		U32 GetEndObjectRecordNumber(void) const;
		
		BOOL IsDualLink(void);
		void ReverseForwardLink(void);

	private:
		U32 RecordNumber;

		DirectionType LinkDirection;
		JC_RecordRef StartObject;
		JC_RecordRef EndObject;				
};

#endif
