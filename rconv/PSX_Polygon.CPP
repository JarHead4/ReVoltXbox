#include "stdafx.h"
#include "PSX_Polygon.H"
#include "math.h"

// The calculations to find the scan lines and horizontal pixel positions
// are rounded down on the left and top, and up and - 1 on the right and bottom.
// This gives the result that the pixels identified are those between but not
// on the floating point coordinates identified.
// eg to define a polygon that encloses exactly one pixel you would use the
// following coordinates:

// 1.0, 1.0
// 1.0, 2.0
// 2.0, 2.0
// 2.0, 1.0

// If the calculations were treated the same for left and right, top and bottom
// you would have to define your coordinates as follows to get the same result:

// 1.0,		1.0
// 1.0,		1.99999
// 1.99999, 1.99999
// 1.99999, 1.0

// By treating left / right and top / bottom as separate calculations we can keep
// the data neat by complicating the code slightly.

double PS_Point::Distance(const PS_Point & point1, const PS_Point & point2)
{
	double d1 = point1.X() - point2.X();
	double d2 = point1.Y() - point2.Y();
	return sqrt((d1 * d1) + (d2 * d2));
}

IMPLEMENT_SERIAL(PS_Point, CObject, 1);

void PS_Point::Serialize(CArchive& ar) 
{ 
	if (ar.IsStoring())
	{
		ar << m_X << m_Y;
	}
	else
	{
		ar >> m_X >> m_Y;
	}
}

IMPLEMENT_SERIAL(PS_Line, CObject, 1);

void PS_Line::Serialize(CArchive& ar)
{
	m_Start.Serialize(ar);
	m_End.  Serialize(ar);
}

PS_Point PS_Line::GetTopPos(void) const
{
	if (m_Start.Y() <= m_End.Y())
	{
		return m_Start;
	}
	else 
	{
		return m_End;
	}
}

PS_Point PS_Line::GetBottomPos(void) const
{
	if (m_Start.Y() > m_End.Y())
	{
		return m_Start;
	}
	else
	{
		return m_End;
	}
}

int PS_Line::GetTopScanLine(void) const
{
	// round down the floating point value to get the top scan line
	return (int)(floor(GetTopPos().Y()));
}

int PS_Line::GetBottomScanLine(void) const
{
	// round up the floating point value to get the bottom scan line + 1
	return (int)(ceil(GetBottomPos().Y()) - 1);
}

double PS_Line::Gradient(void) const
{
	return (GetBottomPos().X() - GetTopPos().X()) / (GetBottomPos().Y() - GetTopPos().Y());
} 

int PS_Line::GetTopLeftPixel(const int scan_line) const
{
	// round down to get the left hand side of the pixel
	ASSERT(scan_line >= GetTopScanLine());
	ASSERT(scan_line <= GetBottomScanLine());
	int pixel;
	if (scan_line == GetTopScanLine())
	{
		pixel = (int)(floor(GetTopPos().X()));
	}
	else
	{
		pixel = (int)(floor(GetTopPos().X() + (Gradient() * (scan_line - GetTopPos().Y()))));
	}
	return pixel;
}

int PS_Line::GetTopRightPixel(const int scan_line) const
{
	// round up to get the right hand side of the pixel + 1
	ASSERT(scan_line >= GetTopScanLine());
	ASSERT(scan_line <= GetBottomScanLine());
	int pixel;
	if (scan_line == GetTopScanLine())
	{
		pixel = (int)(ceil(GetTopPos().X()) - 1);
	}
	else
	{
		pixel = (int)(ceil(GetTopPos().X() + (Gradient() * (scan_line - GetTopPos().Y()))) - 1);
	}
	return pixel;
}

int PS_Line::GetBottomLeftPixel(const int scan_line) const
{
	// round down to get the left hand side of the pixel
	ASSERT(scan_line >= GetTopScanLine());
	ASSERT(scan_line <= GetBottomScanLine());
	int pixel;
	if (scan_line == GetBottomScanLine())
	{
		pixel = (int)(floor(GetBottomPos().X()));
	}
	else
	{
		pixel = (int)(floor(GetTopPos().X() + (Gradient() * ((scan_line + 1) - GetTopPos().Y()))));
	}
	return pixel;
}

int PS_Line::GetBottomRightPixel(const int scan_line) const
{
	// round up to get the right hand side of the pixel + 1
	ASSERT(scan_line >= GetTopScanLine());
	ASSERT(scan_line <= GetBottomScanLine());
	int pixel;
	if (scan_line == GetBottomScanLine())
	{
		pixel = (int)(ceil(GetBottomPos().X()) - 1);
	}
	else
	{
		pixel = (int)(ceil(GetTopPos().X() + (Gradient() * ((scan_line + 1) - GetTopPos().Y()))) - 1);
	}
	return pixel;
}

int PS_Line::GetLeftPixel(const int scan_line) const
{
	return min(GetTopLeftPixel(scan_line), GetBottomLeftPixel(scan_line));
}

int PS_Line::GetRightPixel(const int scan_line) const
{
	return max(GetTopRightPixel(scan_line), GetBottomRightPixel(scan_line));
}

IMPLEMENT_SERIAL(PSX_Polygon, CObject, 1);


void PSX_Polygon::DeleteAllPoints(void)
{
	int num_of_points = m_Points.GetSize();
	for (int i = 0; i < num_of_points; i++)
	{
		delete m_Points[i];
	}
	m_Points.RemoveAll();

	ClearEdgeTable();
}

int PSX_Polygon::GetTopScanLine(void) const
{
	// finding the top so round down
	int num_of_points = m_Points.GetSize();
	int top_scan_line, scan_line;
	for (int i = 0; i < num_of_points; i++)
	{
		scan_line = static_cast <int> (floor(m_Points[i]->Y()));
		if (i == 0)
		{
			top_scan_line = scan_line;
		}
		else
		{
			top_scan_line = min(top_scan_line, scan_line);
		}
	}
	return top_scan_line;
}

int PSX_Polygon::GetBottomScanLine(void) const
{
	// finding the bottom so round up and - 1
	int num_of_points = m_Points.GetSize();
	int bottom_scan_line, scan_line;
	for (int i = 0; i < num_of_points; i++)
	{
		scan_line = static_cast <int> (ceil(m_Points[i]->Y()) - 1);
		if (i == 0)
		{
			bottom_scan_line = scan_line;
		}
		else
		{
			bottom_scan_line = max(bottom_scan_line, scan_line);
		}
	}
	return bottom_scan_line;
}

int PSX_Polygon::NearestPoint(const PS_Point point) const
{
	// find the nearest point to allow for mouse dragging of coordinates
	const int num_of_points = m_Points.GetSize();
	int nearest_index = 0;
	double nearest_distance = 10;
	for (int i = 0; i < num_of_points; i++)
	{
		const double distance = PS_Point::Distance(*m_Points[i], point);
		if (nearest_distance > distance)
		{
			nearest_index = i;
			nearest_distance = distance;
		}
	}
	return nearest_index;
}

void PSX_Polygon::Serialize(CArchive& ar)
{
	int points = m_Points.GetSize();
	if (ar.IsStoring())
	{
		ar << points;
	}
	else
	{
		ar >> points;
		m_EdgeTableValid = false;
	}
	for (int i = 0; i < points; i++)
	{
		m_Points[i]->Serialize(ar);
	}
}

void PSX_Polygon::GetPSXPoint(const unsigned int index, unsigned int & u, unsigned int & v) const
{
	const PS_Point & point = GetPoint(index);
#if 0
	u = static_cast <unsigned int> (point.X());
	v = static_cast <unsigned int> (point.Y());
#else
	const int top_edge = GetTopScanLine();
	const int top_line = static_cast <int> (ceil(point.Y())) - (top_edge + 1);
	const int bottom_line = static_cast <int> (ceil(point.Y())) - top_edge;
	if (IsScanLineInEdgeTable(top_line))
	{
		v = top_line + top_edge;
		u = FindNearestPointOnScanLine(point, top_line);
	}
	else if (IsScanLineInEdgeTable(bottom_line))
	{
		v = bottom_line + top_edge;
		u = FindNearestPointOnScanLine(point, bottom_line);
	}
	else
	{
		u = static_cast <int> (floor(point.X()));
		v = static_cast <int> (floor(point.Y()));
	}
#endif
}

void PSX_Polygon::ClearEdgeTable(void) const
{
	const unsigned int num_of_lines = m_EdgeTable.GetSize();
	for (unsigned int i = 0; i < num_of_lines; i++)
	{
		delete m_EdgeTable[i];
	}
	m_EdgeTable.RemoveAll();
	m_EdgeTableValid = false;
}

void PSX_Polygon::CreateEdgeTable(void) const
{
	ClearEdgeTable();

	const int top = GetTopScanLine();
	const int span = (GetBottomScanLine() - top) + 1;

	for (int i = 0; i < span; i++)
	{
		m_EdgeTable.Add(new PS_EdgeTableEntry);
	}

	// fill the edge table
	const int num_of_points = GetNumberOfPoints();
	if (num_of_points)
	{
		int i = num_of_points - 1;
		PS_Point start = GetPoint(i);
		PS_Point end;
		for (i = 0; i < num_of_points; i++)
		{
			end = GetPoint(i);
			const PS_Line edge(start, end);
			for (int line_index = 0; line_index < span; line_index++)
			{
				const int scan_line = line_index + top;
				if (scan_line >= edge.GetTopScanLine() && scan_line <= edge.GetBottomScanLine())
				{
					const int left_pixel = edge.GetLeftPixel(scan_line);
					const int right_pixel = edge.GetRightPixel(scan_line);
					m_EdgeTable[line_index]->Set(left_pixel, right_pixel);
				}
			}
			start = end;
		}
	}

	m_EdgeTableValid = true;
}

int PSX_Polygon::GetLeftEdge(const unsigned int index) const
{
	if (!m_EdgeTableValid)
	{
		CreateEdgeTable();
	}
	return m_EdgeTable[index]->GetLeftEdge();
}

int PSX_Polygon::GetRightEdge(const unsigned int index) const
{
	if (!m_EdgeTableValid)
	{
		CreateEdgeTable();
	}
	return m_EdgeTable[index]->GetRightEdge();
}

bool PSX_Polygon::IsScanLineInEdgeTable(const unsigned int index) const
{
	if (!m_EdgeTableValid)
	{
		CreateEdgeTable();
	}
	return (index < static_cast <unsigned int>(m_EdgeTable.GetSize()));
}

int PSX_Polygon::FindNearestPointOnScanLine(const PS_Point& point, const unsigned int index) const
{
	if (!m_EdgeTableValid)
	{
		CreateEdgeTable();
	}
	int x = static_cast<int>(ceil(point.X() - 1.0));
	
	return max(min(x, GetRightEdge(index)), GetLeftEdge(index));
}

void PS_EdgeTableEntry::Set(const int left, const int right)
{
	if (m_IsSet == false)
	{
		m_IsSet = true;
		m_Left = left;
		m_Right = right;
	}
	else
	{
		m_Left = min(m_Left, left);
		m_Right = max(m_Right, right);
	}
}