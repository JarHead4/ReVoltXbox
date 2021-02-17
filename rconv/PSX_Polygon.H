#ifndef PSX_POLYGON_H
#define PSX_POLYGON_H

#include <afxtempl.h>

class PS_Point : public CObject
{
	DECLARE_SERIAL(PS_Point)
	double m_X;
	double m_Y;
public:
	static double Distance(const PS_Point & p1, const PS_Point & p2);
	PS_Point(void) { m_X = 0.0; m_Y = 0.0; }
	PS_Point(const double x, const double y) { m_X = x; m_Y = y; }
	PS_Point(const PS_Point& src) { Assign(src); }
	double X(void) const { return m_X; }
	double Y(void) const { return m_Y; }
	void Serialize(CArchive& ar);
	PS_Point& operator = (const PS_Point& src) { return Assign(src); }
	PS_Point& Assign(const PS_Point& src) { m_X = src.m_X; m_Y = src.m_Y; return *this; }
};

class PS_Line : public CObject
{
	DECLARE_SERIAL(PS_Line)
	PS_Point m_Start;
	PS_Point m_End;
public:
	PS_Line(void){}
	PS_Line(const double x1, const double y1, const double x2, const double y2) 
	{ 
		m_Start = PS_Point(x1, y1); 
		m_End = PS_Point(x2, y2); 
	}
	PS_Line(const PS_Line& src) { Assign(src); }
	PS_Line(const PS_Point& start, const PS_Point & end) { m_Start = start; m_End = end; }
	PS_Line& operator = (const PS_Line& src) { return Assign(src); }
	PS_Line& Assign(const PS_Line& src) { m_Start = src.m_Start; m_End = src.m_End; return *this; }
	~PS_Line(void){}
	PS_Point StartPos(void) const { return m_Start; }
	PS_Point EndPos  (void) const { return m_End; }
	void Serialize(CArchive& ar);
	int GetLeftPixel(const int scan_line) const;
	int GetRightPixel(const int scan_line) const;
	int GetTopLeftPixel(const int scan_line) const;
	int GetBottomLeftPixel(const int scan_line) const;
	int GetTopRightPixel(const int scan_line) const;
	int GetBottomRightPixel(const int scan_line) const;
	int GetTopScanLine(void) const;
	int GetBottomScanLine(void) const;
	PS_Point GetTopPos(void) const;
	PS_Point GetBottomPos(void) const;
	double Gradient(void) const;
};

class PS_EdgeTableEntry : public CObject
{
	int m_Left;
	int m_Right;
	bool m_IsSet;
public:
	PS_EdgeTableEntry(void) { m_IsSet = false; }
	int GetLeftEdge(void) const { return m_Left; }
	int GetRightEdge(void) const { return m_Right; }
	void Set(const int left, const int right);
};

typedef CTypedPtrArray <CObArray, PS_Point*> PSX_PointArray;
typedef CTypedPtrArray <CObArray, PS_EdgeTableEntry*> PS_EdgeTable;

class PSX_Polygon : public CObject
{
	DECLARE_SERIAL(PSX_Polygon)
	PSX_PointArray m_Points;
	mutable PS_EdgeTable m_EdgeTable;
	mutable bool m_EdgeTableValid;
	void CreateEdgeTable(void) const;
	void ClearEdgeTable(void) const;
public:
	void GetPSXPoint(const unsigned int index, unsigned int & u, unsigned int & v) const;
	unsigned int GetNumberOfPoints() const { return m_Points.GetSize(); }
	const PS_Point& GetPoint(const unsigned int index) const { return *m_Points[index]; }
	PSX_Polygon(void){ m_EdgeTableValid = false; }
	virtual ~PSX_Polygon(void){ DeleteAllPoints(); }
	void SetPoint(const int index, const PS_Point point){ *m_Points[index] = point; m_EdgeTableValid = false; }
	void DeleteAllPoints(void);
	int GetTopScanLine(void) const;
	int GetBottomScanLine(void) const;
	double Gradient(const PS_Line line) const { return line.Gradient(); }
	int NearestPoint(const PS_Point point) const;
	void Serialize(CArchive& ar);
	void AddPoint(const PS_Point& point) { m_Points.Add(new PS_Point(point)); m_EdgeTableValid = false;}
	int GetLeftEdge(const unsigned int index) const;
	int GetRightEdge(const unsigned int index) const;
	bool IsScanLineInEdgeTable(const unsigned int index) const;
	int FindNearestPointOnScanLine(const PS_Point& point, const unsigned int index) const;
};

#endif