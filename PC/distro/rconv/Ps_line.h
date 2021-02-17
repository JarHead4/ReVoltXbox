//-------------------------------------------------------------------------------------------------------------------------
#pragma once
//-------------------------------------------------------------------------------------------------------------------------
class PS_Point : public CObject
{
	DECLARE_SERIAL(PS_Point);

public:
	float x;
	float y;

public:
	PS_Point(const float ix=0.0, const float iy=0.0) { x = ix; y = iy; }
	PS_Point(CPoint point) { x = (float)point.x;  y = (float)point.y; }
	PS_Point(CSize  point) { x = (float)point.cx; y = (float)point.cy; }
	PS_Point(const PS_Point& src) { Assign(src); }
	float X(void) const { return x; }
	float Y(void) const { return y; }
	PS_Point& operator = (const PS_Point& src) { return Assign(src); }
	PS_Point operator + (const PS_Point& src) { return PS_Point(x+src.x, y+src.y); }
	void Delta(const PS_Point& delta) { x += delta.x; y += delta.y; }
	PS_Point operator - (const PS_Point& src) { return PS_Point(x-src.x, y-src.y); }
	PS_Point& Assign(const PS_Point& src) { x = src.x; y = src.y; return *this; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JC_Polygon : public CArray<PS_Point, const PS_Point&> { };
//-------------------------------------------------------------------------------------------------------------------------
class PS_Line : public CObject
{
	DECLARE_SERIAL(PS_Line);

private:
	PS_Point m_Start;
	PS_Point m_End;

public:
	PS_Line(void){}
	PS_Line(const float x1, const float y1, const float x2, const float y2) 
	{ 
		m_Start = PS_Point(x1, y1); 
		m_End = PS_Point(x2, y2); 
	}
	PS_Line(const PS_Line& src) { Assign(src); }
	PS_Line(const PS_Point start, const PS_Point end) { m_Start = start; m_End = end; }
	PS_Line& operator = (const PS_Line& src) { return Assign(src); }
	PS_Line& Assign(const PS_Line& src) { m_Start = src.m_Start; m_End = src.m_End; return *this; }
	~PS_Line(void){}
	PS_Point StartPoint(void) const { return m_Start; }
	PS_Point EndPoint(void) const { return m_End; }
	void DrawRealLine(CDC * dc, const int scale) const;
	void DrawPixelLine(CDC * dc, const int scale) const;
	int GetLeftPixel(const int scan_line) const;
	int GetRightPixel(const int scan_line) const;
	int GetTopLeftPixel(const int scan_line) const;
	int GetBottomLeftPixel(const int scan_line) const;
	int GetTopRightPixel(const int scan_line) const;
	int GetBottomRightPixel(const int scan_line) const;
	int GetTopScanLine(void) const;
	int GetBottomScanLine(void) const;
	PS_Point GetTopPoint(void) const;
	PS_Point GetBottomPoint(void) const;
	float Gradient(void) const;
};
//-------------------------------------------------------------------------------------------------------------------------
class PS_Rect
{
public:
	float left;
	float top;
	float right;
	float bottom;

public:
	PS_Rect() { /* Random filled */ }
	PS_Rect(float l, float t, float r, float b) { left = l; top = t; right = r; bottom = b; }
	PS_Rect(const CRect& rect) { left = (float)rect.left; top = (float)rect.top; right = (float)rect.right; bottom = (float)rect.bottom; }

	float Width(void)  const { return right - left; }
	float Height(void) const { return bottom - top; }
	PS_Point CenterPoint(void) const { return PS_Point((right+left)*0.5f, (bottom+top)*0.5f); }
};
//-------------------------------------------------------------------------------------------------------------------------
