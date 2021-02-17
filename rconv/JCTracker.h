//-------------------------------------------------------------------------------------------------------------------------
#ifndef jctracker_h
#define jctracker_h
//-------------------------------------------------------------------------------------------------------------------------
#include "jcgenericundo.h"
//-------------------------------------------------------------------------------------------------------------------------
// JC_Tracker
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Tracker
{
public:
	enum Hits { hitNothing = -1, hitTopLeft = 0, HitTop, HitTopRight, hitRight,
				hitBottomRight, hitBottom, hitBottomLeft, hitLeft, hitMiddle };
private:
	int m_Hit;
	int m_HandleSize;
	CWnd * m_TrackWnd;
	CRect m_TrackRect;
	CRect m_TrackRectOld;
	CRect m_TrackRectTemp;
	CSize m_TrackMinSize;
	BOOL  m_MoveEnabled;
	BYTE  m_HandleEnable;

private: // Static
	static int m_InitCnt;
	static BOOL m_SizeEnabled;
	static CBitmap * m_Bitmap;
	static int m_HitTable[9];
	static HCURSOR m_Cursors[9];

public:
	JC_Tracker();
	virtual ~JC_Tracker();

	void Draw(CDC * dc) const;
	int  HitTest(CPoint pt) const;
	BOOL SetCursor(CWnd * wnd, UINT nHitTest) const;
	void SetMinSize(CSize size)				{ m_TrackMinSize = size; }
	void SetHandleSize(int size)			{ m_HandleSize = size; }
	int  GetHandleSize(void)	const		{ return m_HandleSize; }
	void SetHandleEnable(BYTE mask)			{ m_HandleEnable = mask; }
	void EnableMove(BOOL flag)				{ m_MoveEnabled = flag; }
	const CRect& GetPixelRect(void) const	{ return m_TrackRect; }
	void SetPixelRect(const CRect& rect)	{ m_TrackRect = rect; }
	void TrackBegin(CWnd * wnd, CPoint pt, CDC * dc);
	void TrackMove(CSize delta, CDC * dc);
	void TrackEnd(CDC * dc);
	void TrackSize(void);

public: // Static functions
	static void EnableSize(BOOL flag)	{ m_SizeEnabled = flag; }
	static BOOL SizeIsEnabled(void)		{ return m_SizeEnabled; }

private:
	CRect GetTrueRect(void)		 const;
	CRect GetHandleRect(int num) const;
	BOOL  HandleExists(int num)  const	{ return (BOOL)(GetHandleMask() & (1 << num)); }
	BOOL  HandleEnabled(int num) const	{ return (BOOL)(m_SizeEnabled && (m_HandleEnable & (1 << num))); }

public: // Virtual functions
	virtual void OnChangedRect(CRect&) { }
	virtual BYTE GetHandleMask(void) const { return 0xFF; }
	virtual void DrawTrackerRect(const CRect& rect, CDC * dc, CWnd * wnd);
};
//-------------------------------------------------------------------------------------------------------------------------
#endif
//-------------------------------------------------------------------------------------------------------------------------
