#ifndef JC_TEXTURE_SET_H
#define JC_TEXTURE_SET_H
//-------------------------------------------------------------------------------------------------------------------------
#pragma once
//-------------------------------------------------------------------------------------------------------------------------
#include "JC_Dib.h"
#include "PS_Line.h"
#include "JC_TextureSetGridDlg.h"
//-------------------------------------------------------------------------------------------------------------------------
class JC_TextureSet;
class JC_TextureGroup;
class JC_PaletteGroup;
class JC_PaletteGroupList;
class JC_TextureSetItem;
class JC_TextureSetItemList;
class JC_TextureSetDragList;
class JC_TextureSetEditContext;
class JC_TextureSetRenderContext;
class JC_TextureSetCollisionContext;
class PS_ExtractRenderContext;
//-------------------------------------------------------------------------------------------------------------------------
typedef CArray <CPoint, CPoint&> PS_PointArray;
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Damage : public CObject
{
	DECLARE_DYNAMIC(JC_Damage)

protected:
	BOOL m_Erase;	// TRUE if damaged area must be erased.
	CRect m_Rect;	// Workspace rect of damaged area.

public:
	JC_Damage() { m_Erase = FALSE; m_Rect.SetRectEmpty(); }
	JC_Damage(CRect rect, BOOL erase=FALSE)	{ m_Erase = erase; m_Rect = rect; m_Rect.NormalizeRect(); }
	JC_Damage(const JC_Damage& d)			{ m_Erase = d.m_Erase; m_Rect = d.m_Rect; }
	CRect GetRect(void)		const			{ return m_Rect; }
	BOOL MustErase(void)	const			{ return m_Erase; }
	void operator |= (const JC_Damage& d)	{ m_Erase |= d.m_Erase; m_Rect |= d.m_Rect; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_ImportBitmap // A 4 or 8 bit bitmap.
{
protected:
	CSize m_Size;				// Dimensions of bitmap.
	CString m_Path;				// Path of associated BMP file.
	int m_Nextline;				// Offset to next line in bytes.
	BOOL m_IsBottomUp;			// TRUE if 'bottom-up' bitmap.
	BOOL m_Is8Bit;				// TRUE if 8-bit picture, FALSE if 4-bit picture.
	CPalette * m_Palette;		// Ptr to palette.
	BYTE * m_Buffer;			// Ptr to buffer file was loaded into.
	BYTE * m_ImageBits;			// Ptr to image bits within buffer.
	BITMAPINFO * m_BitmapInfo;	// Ptr to bitmap info within buffer.
	U32 m_BufferSize;

public:
	JC_ImportBitmap(void);
	JC_ImportBitmap(const CString& path);
	virtual ~JC_ImportBitmap();

	bool GetPalette(CDWordArray& palette) const;
	bool Insert(JC_Buffer& buf);
	bool Extract(JC_Buffer& buf);
	void Unload(void);
	BOOL Load(CSize requiredsize);
	BOOL LoadFromBMI(BITMAPINFO * bitmap_info, CSize& requiredsize, CString& error);
	BYTE GetPixelColourIndex(int x, int y) const;
	JC_Pixel GetPixel(int x, int y) const;
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureSetGrid
{
protected:
	int  m_Type;		// Type of grid, see JC_TextureSetGridDlg class.
	BOOL m_Snap;		// If TRUE, snap to grid is active.
	BOOL m_ShowSnap;	// If TRUE, snap grid is visible.
	BOOL m_ShowPixel;	// If TRUE, pixel grid is visible when zoomed.
	CSize m_Custom;		// Custom size to be used when type is 'Custom'.
	CPoint m_Origin;	// Origin of grid.

public:
	JC_TextureSetGrid();

	BOOL Edit(void);
	void Insert(JC_Buffer& buf) const;
	void Extract(JC_Buffer& buf);
	void SnapToGrid(CPoint& workpos) const;
	void SetOrigin(CPoint origin) { m_Origin = origin; }
	void Render(JC_TextureSetRenderContext& rc) const;
	BOOL SnapGridActive(void)  const { return m_Snap     && (m_Type != JC_TextureSetGridDlg::FixedNone); }
	BOOL SnapGridVisible(void) const { return m_ShowSnap && (m_Type != JC_TextureSetGridDlg::FixedNone); }
	BOOL PixelGridVisible(void)const { return m_ShowPixel; }

protected:
	CSize GetGrid(void) const;
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureSetRenderContext
{
protected:
	enum
	{
		None         = 0x00,
		Pixels       = 0x01,
		Outlines     = 0x02,
		UserOutlines = 0x04,
	};

protected:
	int m_Zoom;					// Zoom factor, 1 to 20.
	CDC * m_DC;					// DC for using GDI with the dib.
	JC_Dib m_Dib;				// Dib that handles the bitmap, drawing and blitting.
	CRect m_DibRect;			// Logical rectangle of the dib within the destination device context.
	CRect m_ClipRect;			// Logical rectangle of the clipping area.
	JC_TextureSetGrid * m_Grid;	// Ptr to grid to use.
	BYTE m_Flags;				// Flags (see above).

public:
	JC_TextureSetRenderContext(COLORREF background);

	BOOL Zoom(int direction);
	CPoint GetAutoScroll(CPoint workpos, CPoint client) const;
	int GetZoom(void) const { return m_Zoom; }
	CPoint LogicalToWorkspace(CPoint logpos) const;
	CRect  LogicalToWorkspace(CRect  logrect) const;
	CPoint WorkspaceToLogical(CPoint workpos) const;
	CRect  WorkspaceToLogical(CRect  workrect) const;
	PS_Point WorkspaceToLogical(const PS_Point& workpos) const;
	BOOL BeginRender(CDC * dc, CRect client, CRect clip);
	void EndRender(CDC * dc);
	void Clear(void) { m_Dib.Clear(); }
	CDC& GetDC(void) { return *m_DC; }
	virtual BOOL IsClipped(CRect logrect) const;
	void SnapToGrid(CRect& workrect) const;
	void SnapToGrid(CPoint& workpos) const;
	void SetGrid(JC_TextureSetGrid * grid) { m_Grid = grid; }
	void DrawGridLines(CSize step, JC_Pixel col, CPoint origin) const;
	BOOL SnapGridActive(void)  const { return m_Grid->SnapGridActive(); }
	void OnlyPixels(void)   { m_Flags |= Pixels; m_Flags &= ~Outlines; }
	void OnlyOutlines(void) { m_Flags |= Outlines; m_Flags &= ~Pixels; }
	void ToggleUserOutlines(void) { m_Flags ^= UserOutlines; }
	virtual void WriteBMP(const CString& path) const { m_Dib.WriteBMP(path); }
	const CPoint& Origin(void) const { return m_DibRect.TopLeft(); }
	void Rectangle(const CRect& log, JC_Pixel col) const { m_Dib.Rectangle(log-Origin(), col); }
	BOOL UserWantsOutlines(void) const { return m_Flags & UserOutlines; }

	virtual void Pixel(const CPoint& workpos, const JC_Pixel& col)
	{
		CPoint logpos = WorkspaceToLogical(workpos);
		CRect logrect(logpos.x, logpos.y, logpos.x+m_Zoom, logpos.y+m_Zoom);
		Rectangle(logrect, col);
	}

public:
	virtual BOOL WantsPixels(void)     const { return  m_Flags & Pixels; }
	virtual BOOL WantsOutlines(void)   const { return (m_Flags & Outlines) && UserWantsOutlines(); }
	virtual BOOL WantsSelections(void) const { return TRUE; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureSetExportContext : public JC_TextureSetRenderContext
{
public:
	JC_TextureSetExportContext(CRect bmrect, COLORREF background);
	bool CreateBitmapInfo(LPBITMAPINFO& bitmap_info);

public:
	virtual BOOL WantsPixels(void)     const { return TRUE;  }
	virtual BOOL WantsOutlines(void)   const { return FALSE; }
	virtual BOOL WantsSelections(void) const { return FALSE; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureSetDataSource : public COleDataSource
{
public:
	JC_TextureSetDataSource(JC_TextureSetItemList * list);

	static CLIPFORMAT GetClipboardFormat(void) { return (CLIPFORMAT)RegisterClipboardFormat("HB_TextureSetInternalData"); }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureSetDropSource : public COleDropSource
{
private:
	BOOL m_Multiple;							// Flag TRUE when multiple items are being dragged.

public:
	JC_TextureSetDropSource(JC_TextureSetEditContext& ec);

protected:
	virtual SCODE GiveFeedback(DROPEFFECT effect);
};
//-------------------------------------------------------------------------------------------------------------------------
class JC_Texture;
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureSetItemList : public CTypedPtrList<CPtrList, JC_TextureSetItem*>
{
public:
	bool UsesSamePalette(void) const;
	CRect LogicalBound(void) const;
	void Insert(JC_Buffer& buf) const;
	void Extract(JC_Buffer& buf);
	void DeleteAllItems(void);
	BOOL Contains(U32 recnum) const;
	void OffsetPosition(CSize delta) const;
	void ModifyOrientation(int action) const;
	JC_TextureSetItem * HitTest(CPoint workpos) const;
	JC_TextureSetItemList * HitTest(CRect workpos) const;
	void SnapToGrid(JC_TextureSetRenderContext& rc) const;
	void Normalise(void) const;
	void BuildInternal(void) const;
	void RebuildInternal(void) const;
	void Move(CSize delta) const;
	void MoveInternal(CSize delta) const;
	void BuildWorkspaceEdgeTable(void) const;
	void BuildRenderArrays(void) const;
	void SetTextureSet(JC_TextureSet * set) const;
	void FindPaletteGroup(JC_PaletteGroupList * pglist);
	void Render(JC_TextureSetRenderContext& rc) const;
	UINT Collision(JC_TextureSetCollisionContext& cc) const;
	void GetUnselectedItems(const JC_TextureSetItemList& input);
	void Nudge(CSize delta, CRect workspace, JC_TextureSetCollisionContext& cc) const;
	void DrawOutline(CDC& dc, JC_TextureSetRenderContext& rc, BOOL snap, CPoint grab) const;
	void SetCommonProperties(JC_PaletteGroup * pg, float bleed, int bleedstate, BOOL bleedok) const;
	JC_TextureSetItem * FindOverlappingTexture(JC_TextureGroup * group, CSize& offset);
	JC_PaletteGroup * GetCommonProperties(float& bleed, int& bleedstate, BOOL& bleedok) const;
	void AddOutlinePoints(CDC& dc, JC_TextureSetRenderContext& rc, BOOL snap, CPoint grab, PS_PointArray& start_pos, PS_PointArray& end_pos) const;
	JC_Texture * GetTextureFromRecord(U32 recnum) const;
	void ResolveRefs(JC_UpgradeContext& upgrade);
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureSetDragList : public JC_TextureSetItemList
{
protected:
	CPoint m_Grab;		// Position for dragging feedback.
	PS_PointArray m_RubberBandStartArray;
	PS_PointArray m_RubberBandEndArray;

public:
	void BeginFeedback(CDC& dc, JC_TextureSetRenderContext& rc, CPoint logical);
	void UpdateFeedback(CDC& dc, JC_TextureSetRenderContext& rc, CPoint logical);
	void EndFeedback(CDC& dc, JC_TextureSetRenderContext& rc);
	BOOL CanBeDroppedHere(CRect workspace, const JC_TextureSetItemList& allitems) const;

protected:
	void DrawRubberBand(CDC& dc, JC_TextureSetRenderContext& rc) const;
	void GetRubberBandArrays(CDC& dc, JC_TextureSetRenderContext& rc);
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureSetGroupRequest
{
private:
	JC_TextureSetItemList m_Items;		// List of items to be included in the group.

public:
	JC_TextureSetGroupRequest(JC_TextureSetItem * primary) { AddItem(primary); }

	JC_TextureSetItemList& GetItemList(void) { return m_Items; }
	void AddItem(JC_TextureSetItem * item) { m_Items.AddTail(item); }
	void AppendTo(JC_TextureSetGroupRequest * that);
	BOOL Contains(const JC_TextureSetItem * item) const { return (BOOL)(m_Items.Find((JC_TextureSetItem*)item) != NULL); }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureSetGroupRequestList : public CTypedPtrList<CPtrList, JC_TextureSetGroupRequest*>
{
public:
	virtual ~JC_TextureSetGroupRequestList() { while (!IsEmpty()) delete RemoveHead(); }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureSetCollisionContext
{
public:
	enum
	{
		NoCollision   = 0x00,		// No collision.
		SoftCollision = 0x01,		// A collision in which items can be grouped; legal.
		HardCollision = 0x02,		// A collision in which items cannot be grouped; illegal.
	};

protected:
	BOOL m_Grouping;									// Flag TRUE if auto-grouping is required.
	BOOL m_ForceOverlap;								// Flag TRUE if force-overlap is required.
	const JC_TextureSetItemList * m_ItemList;			// Ptr to list of items to check against.
	JC_TextureSetGroupRequestList m_GroupRequestList;	// List of new group requests for auto-grouping.

public:
	JC_TextureSetCollisionContext(const JC_TextureSetItemList * list, BOOL grouping=FALSE, BOOL forceoverlap=FALSE);

	CRect DoGrouping(JC_TextureSet * set);
	BOOL WantsGrouping(void) const { return m_Grouping; }
	BOOL ForceOverlap(void)  const { return m_ForceOverlap; }
	const JC_TextureSetItemList& GetItemList(void) const { return *m_ItemList; }
	void RequestGroup(JC_TextureSetItem * primary, JC_TextureSetItem * secondary);
	void DisableGrouping(void) { m_Grouping = FALSE; m_ForceOverlap = FALSE; }

protected:
	JC_TextureSetGroupRequest * FindGroupRequestFor(JC_TextureSetItem * item) const;
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureSetEditContext : public CObject
{
public:
	enum // Flags for ModifySelection.
	{
		Clear	= 0x01,
		Add		= 0x02,
		Remove	= 0x04,
		Toggle	= 0x08,
	};

protected:
	JC_TextureSetItemList m_Selection;	// List of selected texture set items.

public:
	void Forget(const JC_TextureSetItem * item);
	void Forget(const JC_TextureSetItemList * list);
	JC_TextureSetItemList * CopySelection(void) const;
	JC_TextureSetItemList * GetSelection(void) { return &m_Selection; }
	const JC_TextureSetItemList * GetSelection(void) const { return &m_Selection; }
	POSITION FindInSelection(const JC_TextureSetItem * item) const;
	JC_Damage ModifySelection(UINT selflags, const JC_TextureSetItemList * list=NULL);
	JC_Damage DeleteSelection(void);
	JC_Damage NormaliseSelection(void) const;
	JC_Damage OrientSelection(int action) const;
	void NudgeSelection(CSize delta, CRect workspace, JC_TextureSetCollisionContext& cc) const;
	BOOL EditSelectionProperties(int outputformat, float defaultbleed, JC_PaletteGroupList * pglist);
	CRect BoundSelection(void)	 const { return m_Selection.LogicalBound(); }
	int CountSelectedItems(void) const { return m_Selection.GetCount(); }

protected:
	JC_Damage ClearSelection(void);
	JC_Damage AddToSelection(const JC_TextureSetItemList * list);
	JC_Damage RemoveFromSelection(const JC_TextureSetItemList * list);
	JC_Damage ToggleSelection(const JC_TextureSetItemList * list);
	JC_Damage InvertSelection(const JC_TextureSetItemList * list);
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureBitmap		// 32-bit bitmap created from texture bitmap record and used for rendering.
{
private:
	CSize m_Size;						// Size of bitmap.
	int m_Nextline;						// Offset to next line.
	JC_Pixel * m_Pixels;				// Ptr to JC_Pixel image data.
	BITMAPINFO * m_BitmapInfo;			// Ptr to bitmap info containing pixels.

public:
	JC_TextureBitmap(const JC_Bitmap * bitmap);
	virtual ~JC_TextureBitmap();

	JC_Pixel GetPixel(int x, int y) const
	{
		y = (m_Size.cy - 1) - y;
		if (x < 0)			x += m_Size.cx;
		else
		if (x >= m_Size.cx)	x -= m_Size.cx;

		if (y < 0)			y += m_Size.cy;
		else
		if (y >= m_Size.cy)	y -= m_Size.cy;

		return *(m_Pixels + x + (y * m_Nextline));
	}
};
//-------------------------------------------------------------------------------------------------------------------------
class JC_TextureBitmapMap : public CMap<U32, const U32&, JC_TextureBitmap*, JC_TextureBitmap*&>
{
private:
	int m_RefCnt;

public:
	void AddRef(void)  { m_RefCnt++; }
	void Release(void) { if (--m_RefCnt == 0) DeleteAll(); }

protected:
	void DeleteAll(void);
};
//-------------------------------------------------------------------------------------------------------------------------
class JC_Edge
{
public:
	int left;		// Left pixel position.
	int right;		// Right pixel position (inclusive).

	JC_Edge() { left = right = 0; };
};
//-------------------------------------------------------------------------------------------------------------------------
class JC_EdgeTable : public CArray<JC_Edge, const JC_Edge&>
{
	DECLARE_DYNAMIC(JC_EdgeTable)

public: // Edge table bounds (exclusive).
	int left;
	int top;
	int right;
	int bottom;

public:
	void Move(CSize delta);
	int Width(void)  const { return right - left; }
	int Height(void) const { return bottom - top; }
	void CopyRect(const CRect& rect) { left = rect.left; top = rect.top; right = rect.right; bottom = rect.bottom; }
	BOOL Overlaps(const JC_EdgeTable * that) const { CSize temp; return CheckCommonPixels(that, temp); }

public:
	virtual BOOL CheckCommonPixels(const JC_EdgeTable * that, CSize& offset) const;
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_BitmapEdgeTable : public JC_EdgeTable
{
	DECLARE_DYNAMIC(JC_BitmapEdgeTable)

protected:
	U32 m_BitmapId; // Unique id of bitmap that pixels originate from.

public:
	JC_BitmapEdgeTable() { m_BitmapId = 0; }
	void SetBitmapId(U32 id) { m_BitmapId = id; }

public:
	virtual BOOL CheckCommonPixels(const JC_EdgeTable * that, CSize& offset) const;
};
//-------------------------------------------------------------------------------------------------------------------------
class JC_EdgeTableList : public CTypedPtrList<CPtrList, JC_EdgeTable*> { };
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureSetItem : public CObject
{
	DECLARE_DYNAMIC(JC_TextureSetItem)

public:
	enum // Type.
	{
		Texture,
		TextureGroup,
	};

	enum // Action for ModifyOrientation.
	{
		RotateLeft,
		RotateRight,
		FlipHori,
		FlipVert,
	};

protected:
	enum // Orientation.
	{
		Rot0	 = 0,		// Not rotated.
		Rot90	 = 1,		// Rotated clockwise by  90 degrees.
		Rot180	 = 2,		// Rotated clockwise by 180 degrees.
		Rot270	 = 3,		// Rotated clockwise by 270 degrees.
		RotMask	 = 0x03,	// Mask for rotation value.
		HFlip	 = 0x04,	// Flipped horizontally.
		VFlip	 = 0x08,	// Flipped vertically.
	};

	enum // EdgeTable list types.
	{
		WorkList, BitmapList,
	};

protected: // Serialized.
	float m_Bleed;						// Bleed pixel value to be used if m_UseBleed is TRUE.
	BOOL m_UseBleed;					// Flag TRUE when m_Bleed is to be used instead of texture set default.
	BOOL m_Selected;					// Flag TRUE if item is selected.
	int m_Orientation;					// Orientation, see above.
	mutable CPoint m_Position;			// Workspace pos of the top-left of the bounding rectangle within the parent.
	mutable S16 m_PaletteGroupSequence;	// Temporary; used only by serialization.

protected: // Not serialized.
	BOOL m_IsInTextureSetList;			// TRUE when the item is in a texture set list.
	JC_TextureSetItem * m_Parent;		// Ptr to parent item or NULL if this is a root item.
	JC_TextureSet * m_TextureSet;		// Ptr to the texture set that contains this item (if at top-level).
	JC_PaletteGroup * m_PaletteGroup;	// Ptr to palette group, or NULL if none.
	mutable bool m_LogicalBoundIsValid;
	mutable bool m_PhysicalBoundIsValid;
	mutable bool m_WorkspacePosIsValid;
	mutable CPoint m_WorkspacePos;
	mutable CRect m_LogicalBound;
	mutable CRect m_PhysicalBound;

public:
	JC_TextureSetItem(CPoint pos=CPoint(0, 0));
	virtual ~JC_TextureSetItem();
	virtual void InvalidateWorkspacePos(bool recurse = true) = 0;

	float GetRealBleed(void) const;
	CRect LogicalBound(void) const;
	CPoint GetWorkspacePos(void) const;
	float GetBleed(void) const { return m_Bleed; }
	BOOL GetUseBleed(void) const { return m_UseBleed; }
	BOOL IsChildOf(JC_TextureSetItem * item) const;
	void SetPosition(CPoint pos) { m_Position = pos; InvalidateWorkspacePos(); }
	CPoint GetPosition(void) const { return m_Position; }
	void OffsetPosition(CSize delta) { m_Position += delta; InvalidateWorkspacePos(); }
	void SetBleed(float bleed) { m_Bleed = bleed; }
	void SetUseBleed(BOOL usebleed) { m_UseBleed = usebleed; }
	void SetParent(JC_TextureSetItem * parent);
	JC_TextureSetItem * GetParent(void) const { return m_Parent; }
	void SnapToGrid(JC_TextureSetRenderContext& rc);
	void ModifyOrientation(int action);
	void FindPaletteGroup(JC_PaletteGroupList * pglist);
	JC_TextureSetItem * GetTopmostItem(void);
	void IsInTextureSetList(BOOL state) { m_IsInTextureSetList = state; }
	BOOL IsSelected(void) const;
	void Selected(BOOL state) { m_Selected = state; }
	void Normalise(void);
	void ApplyOrientationTree(CRect& rect) const;
	void ApplyOrientationTree(CPoint& point) const;
	void ApplyOrientationTree(PS_Point& point) const;
	void ApplyOrientation(int orient, const PS_Point& centre, CRect& rect) const;
	void ApplyOrientation(int orient, const PS_Point& centre, PS_Point& point) const;
	void DrawRubberBand(CDC& dc, JC_TextureSetRenderContext& rc, CPoint grab) const;
	void SetPaletteGroup(JC_PaletteGroup * pg);
	JC_PaletteGroup * GetPaletteGroup(void) const { return m_PaletteGroup; }
	JC_PaletteGroup * GetRealPaletteGroup(void) const;
	void GetEdgeTableLists(JC_EdgeTableList*& work, JC_EdgeTableList*& bitmap) const;
	BOOL HasImportBitmap(void) const;
	void Move(CSize delta) { OffsetPosition(delta); MoveInternal(delta); }
	void GetOrientedVectors(CPoint& x_inc, CPoint& y_inc) const;
	void GetOrientedVectors(PS_Point& x_inc, PS_Point& y_inc) const;

protected:
	void Rotate(int dir);
	BOOL IsKeyPressed(int vKey) const { return ((GetAsyncKeyState(vKey) & 0x8000) > 0); };
	BOOL FullyOverlapping(void) const { return IsKeyPressed(VK_MENU); }

public:
	virtual BOOL IsGroup(void) const = 0;
	virtual BOOL CanUngroup(void) const = 0;
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual CRect PhysicalBound(void) const = 0;
	virtual BYTE GetType(void) const = 0;
	virtual BOOL HitTest(CRect workrect) const = 0;
	virtual BOOL HitTest(CPoint workpos) const = 0;
	virtual void MoveInternal(CSize delta) = 0;
	virtual void BuildInternal(void) = 0;
	virtual void RebuildInternal(void) = 0;
	virtual BOOL Contains(U32 recnum) const = 0;
	virtual void BuildWorkspaceEdgeTable(void) = 0;
	virtual void BuildRenderArrays(void) = 0;
	virtual JC_EdgeTableList * GetEdgeTableList(int which) const = 0;
	virtual BOOL IsInGroup(void) const { return (BOOL)(m_Parent != NULL); }
	virtual void SetTextureSet(JC_TextureSet * set) { m_TextureSet = set; }
	virtual void Render(JC_TextureSetRenderContext& rc) const = 0;
	virtual JC_TextureSetItemList * GetChildList(void) = 0;
	virtual JC_TextureSetItemList * GetTextureList(void) = 0;
	virtual void Ungroup(JC_TextureSetItemList * parentlist) = 0;
	virtual UINT Collision(JC_TextureSetCollisionContext& cc) = 0;
	virtual BOOL GetOverlapOffset(JC_TextureGroup*, CSize&) const { return FALSE; }
	virtual void DrawOutline(CDC& dc, JC_TextureSetRenderContext& rc, BOOL snap, CPoint grab) const = 0;
	virtual void AddOutlinePoints(CDC& dc, JC_TextureSetRenderContext& rc, BOOL snap, CPoint grab, PS_PointArray& start_pos, PS_PointArray& end_pos) const = 0;
	virtual JC_Texture * GetTextureFromRecord(U32 recnum) = 0;
	virtual void ResolveRefs(JC_UpgradeContext& upgrade) = 0;
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Texture : public JC_TextureSetItem		// A single texture.
{
	enum { TopLeft, TopRight, BottomLeft, BottomRight, TotalPositions };

protected:
	CRect m_Bound;							// Bounding rectangle with top-left as (0, 0).
	CRect m_BitmapBound;					// Bounding rectangle within bitmap.
	U32 m_BitmapRecordNum;					// Record number of associated JC_Bitmap.
	JC_RecordRef m_Texture;					// Reference to associated PS_Texture record.
	JC_Polygon m_InnerPolygon;				// Array of inner points.
	JC_Polygon m_BleedPolygon;				// Array of bleed points.
	JC_EdgeTable m_WorkspaceEdgeTable;		// Edge-table of workspace positions on each scan line, from the bleed polygon.
	JC_BitmapEdgeTable m_BitmapEdgeTable;	// Edge-table of bitmap positions on each scan line, from the bleed polygon.
	CArray<CPoint, const CPoint&> m_RenderPoints;		// Array of points for fast rendering.
	CArray<JC_Pixel, const JC_Pixel&> m_RenderPixels;	// Array of pixels for fast rendering.

protected: // Static.
	static JC_TextureBitmapMap m_Map;		// Map of bitmap record number to texture bitmaps.

public:
	JC_Texture();							// Construct from serialization only.
	JC_Texture(CPoint pos, const JC_RecordRef& rr, JC_TextureSet * set);
	virtual ~JC_Texture();

protected:
	void Construct(void);
	void ProcessRecord(void);
	CRect BoundAndBuildBitmapEdgeTable(void);
	JC_TextureBitmap * BuildTextureBitmap(const JC_Bitmap * bitmap) const;
	void CalculateBleedPolygon(const JC_Polygon& source, JC_Polygon& dest);
	PS_Point CalculateBleedPoint(const PS_Point& point, int position);
	void DrawOutlinePolygon(CDC& dc, JC_TextureSetRenderContext& rc, const JC_Polygon& polygon, BOOL snap) const;

public:
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);
	U32 GetPaletteRecordNumber(void) const;
	CPoint GetOriginalPos(void) const;
	virtual void InvalidateWorkspacePos(bool recurse = true);
	virtual BOOL IsGroup(void) const { return FALSE; }
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual CRect PhysicalBound(void) const;
	virtual BOOL HitTest(CRect workrect) const;
	virtual BOOL HitTest(CPoint workpos) const;
	virtual BYTE GetType(void) const { return Texture; }
	virtual void BuildInternal(void);
	virtual void RebuildInternal(void);
	virtual BOOL Contains(U32 recnum) const;
	virtual void SetTextureSet(JC_TextureSet * set);
	virtual void MoveInternal(CSize delta);
	virtual void BuildWorkspaceEdgeTable(void);
	virtual void BuildRenderArrays(void);
	virtual BOOL CanUngroup(void) const { return FALSE; }
	virtual void Render(JC_TextureSetRenderContext& rc) const;
	virtual JC_TextureSetItemList * GetChildList(void) { return NULL; }
	virtual JC_TextureSetItemList * GetTextureList(void);
	virtual void Ungroup(JC_TextureSetItemList*)  { ASSERT(FALSE); } // Can't ungroup a texture.
	virtual JC_EdgeTableList * GetEdgeTableList(int which) const;
	virtual UINT Collision(JC_TextureSetCollisionContext& cc);
	virtual BOOL GetOverlapOffset(JC_TextureGroup * group, CSize& offset) const;
	virtual void DrawOutline(CDC& dc, JC_TextureSetRenderContext& rc, BOOL snap, CPoint grab) const;
	virtual void AddOutlinePoints(CDC& dc, JC_TextureSetRenderContext& rc, BOOL snap, CPoint grab, PS_PointArray& start_pos, PS_PointArray& end_pos) const;
	bool GetFloatUVArray(PS_FloatUVArray& array) const;
	virtual JC_Texture * GetTextureFromRecord(U32 recnum) { if (m_Texture.RecordNum() == recnum) return this; else return NULL; }
	U32 GetTextureRecordNumber(void) const { return m_Texture.RecordNum(); }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureGroup : public JC_TextureSetItem	// A group of textures.
{
private:
	JC_TextureSetItemList m_ItemList;	// List of items in the group.

public:
	JC_TextureGroup() { } // Construct from serialization only.
	JC_TextureGroup(CPoint pos, float bleed, BOOL usebleed);
	virtual ~JC_TextureGroup();

	void AddItem(JC_TextureSetItem * item, CSize offset);
	BOOL Contains(const JC_TextureSetItem * item) const { return (BOOL)(m_ItemList.Find((JC_TextureSetItem*)item) != NULL); }
	bool RemoveTextureFromGroup(const PS_Texture * texture);

public:
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);
	virtual void InvalidateWorkspacePos(bool recurse = true);
	virtual BOOL IsGroup(void) const { return TRUE; }
	virtual BOOL HitTest(CRect workrect) const;
	virtual BOOL HitTest(CPoint workpos) const;
	virtual BYTE GetType(void) const { return TextureGroup; }
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual CRect PhysicalBound(void) const;
	virtual void BuildInternal(void);
	virtual void RebuildInternal(void);
	virtual BOOL Contains(U32 recnum) const;
	virtual void MoveInternal(CSize delta);
	virtual void SetTextureSet(JC_TextureSet * set);
	virtual void BuildWorkspaceEdgeTable(void);
	virtual void BuildRenderArrays(void);
	virtual BOOL CanUngroup(void) const { return TRUE; }
	virtual void Render(JC_TextureSetRenderContext& rc) const;
	virtual JC_EdgeTableList * GetEdgeTableList(int which) const;
	virtual UINT Collision(JC_TextureSetCollisionContext& cc);
	virtual JC_TextureSetItemList * GetChildList(void) { return &m_ItemList; }
	virtual JC_TextureSetItemList * GetTextureList(void);
	virtual void Ungroup(JC_TextureSetItemList * parentlist);
	virtual void DrawOutline(CDC& dc, JC_TextureSetRenderContext& rc, BOOL snap, CPoint grab) const;
	virtual void AddOutlinePoints(CDC& dc, JC_TextureSetRenderContext& rc, BOOL snap, CPoint grab, PS_PointArray& start_pos, PS_PointArray& end_pos) const;
	virtual JC_Texture * GetTextureFromRecord(U32 recnum) { return m_ItemList.GetTextureFromRecord(recnum); }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_PaletteGroup : public CObject
{
	DECLARE_DYNAMIC(JC_PaletteGroup)

private: // Serialized.
	CString m_Name;							// Name of the palette group.
	CString m_Bmp;							// File name of the associated import/export BMP file.
	JC_ImportBitmap * m_ImportBitmap;		// Bitmap imported from BMP file.

private: // Not serialized.
	U32 m_RefCnt;							// Number of users of this group.
	JC_TextureSetItemList m_ItemList;		// List of items in the palette group.
	JC_PaletteGroupList * m_ParentList;		// Ptr to list containing this palette group.

public:
	JC_PaletteGroup(const CString& name, JC_PaletteGroupList * parentlist);
	virtual ~JC_PaletteGroup();

	bool GetPalette(CDWordArray& palette) const;
	void Edit(void);
	void Insert(JC_Buffer& buf) const;
	void Extract(JC_Buffer& buf);
	void Extract(JC_Buffer& buf, U16 version);
	void AddItem(JC_TextureSetItem * item);
	void RemItem(JC_TextureSetItem * item);
	void Import(CSize requiredsize);
	void Export(PS_ExtractRenderContext& exc) const;
	S16  GetSequence(void) const;
	void AddRef(void)								{ m_RefCnt++; }
	void Release(void)								{ m_RefCnt--; }
	CString GetName(void)		const				{ return m_Name; }
	CString GetBitmapFile(void)	const				{ return m_Bmp; }
	int GetNumUsers(void)		const				{ return (int)m_RefCnt; }
	int GetNumItems(void)		const				{ return m_ItemList.GetCount(); }
	JC_TextureSetItemList& GetItemList(void)		{ return m_ItemList; }
	JC_ImportBitmap * GetImportBitmap(void) const	{ return m_ImportBitmap; }
	BOOL HasImport(void)		const				{ return (BOOL)(m_ImportBitmap != NULL); }
	void ClearImport(void);

protected:
	void FreeImport(void);
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_PaletteGroupList : public CTypedPtrList<CPtrList, JC_PaletteGroup*>
{
public:
	void Insert(JC_Buffer& buf) const;
	void Extract(JC_Buffer& buf, U16 version);
	S16  GetSequence(const JC_PaletteGroup * pg) const;
	JC_PaletteGroup * FindPaletteGroup(int sequence) const;
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_TextureSet : public JC_PropertyRecord
{
public:
	enum // Output formats, same order as properties combo box.
	{
		OF_4 = 0,	//  4-bit.
		OF_8,		//  8-bit with palette.
		OF_16,		// 16-bit.
		OF_24,		// 24-bit.
		OF_32,		// 32-bit.
	};

protected: // Serialized.
	CRect m_BitmapPad;						// Rectangle of the bitmap 'pad' which is the main bitmap workspace area.
	int m_OutputFormat;						// Output format, see above.
	float m_DefaultBleed;					// Default bleed pixel value.
	JC_TextureSetGrid m_Grid;				// Grid.
	COLORREF m_BackgroundColour;			// Colour of bitmap background.
	JC_TextureSetItemList m_ItemList;		// List of items in the texture set.
	JC_PaletteGroupList m_PaletteGroupList;	// List of palette groups in the texture set.
	JC_RecordRef m_OutputBitmap;			// Output bitmap reference, starts NULL but is assigned when created.
	JC_TextureSetItemList * m_TempItemList; // Used during the load procedure

protected: // Not serialized.
	JC_TextureSetItemList m_AcceptList;		// List of items that were just accepted; used for tiling.
	JC_TextureSetEditContext m_EditContext;	// Edit context for this texture set.
	JC_PaletteGroup * m_FilterPaletteGroup;	// Palette group to draw, or NULL to draw all items.
	bool m_OutputBitmapUpdated;				// flag set when bitmap has been recreated.

public:
	JC_TextureSet(PS_Database * db, U32 recnum);
	virtual ~JC_TextureSet();

	virtual bool Initialise(void); // Called after ReadBody to perform any required initialisation
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	virtual bool RemoveTextureFromSet(const PS_Texture * texture);
	U32 GetTextureGroupIDFromTexture(const U32 texture_recnum) const;
	U32 GetOutputBitmap(void);
	bool ContainsTexture(U32 recnum) { return (m_ItemList.Contains(recnum) == TRUE); }
	bool GetPalette(const U32 texture_recnum, CDWordArray& palette) const;
	BOOL EditGrid(void);
	BOOL EditProperties(void);
	CSize GetWorkspaceSize(void) const;
	CRect GetWorkspaceRect(void) const;
	void AttachItem(JC_TextureSetItem * item);
	void DetachItem(JC_TextureSetItem * item);
	CString GetFilterStatus(void) const;
	CString GetExportMenuString(void) const;
	CString GetImportMenuString(void) const;
	CString GetCounts(void) const;
	JC_TextureSetEditContext& GetEditContext(void) { return m_EditContext; }
	const JC_TextureSetItemList& GetItemList(void) const { return m_ItemList; }
	JC_TextureSetGrid * GetGrid(void) { return &m_Grid; }
	JC_Damage AttachList(JC_TextureSetItemList * input, BOOL forceoverlap=FALSE);
	CRect GetBitmapPad(void) const { return m_BitmapPad; }
	int GetOutputFormat(void) const { return m_OutputFormat; }
	float GetDefaultBleed(void) const { return m_DefaultBleed; }
	void Render(JC_TextureSetRenderContext& rc) const;
	CString GetName(void) const { return JC_PropertyRecord::Name(); }
	BOOL CanUngroup(void) const;
	void AcceptRecords(const CDWordArray& recarray, CPoint workpos);
	BOOL CanAcceptRecords(const CDWordArray& recarray) const;
	BOOL EditSelectionProperties(void);
	void ExportPaletteGroup(void);
	BOOL CanExportPaletteGroup(void) const { return (BOOL)((m_FilterPaletteGroup != NULL) && (m_OutputFormat == OF_8 || m_OutputFormat == OF_4)); }
	void ExportAllPaletteGroups(void);
	BOOL CanExportAllPaletteGroups(void) const { return (BOOL)(m_OutputFormat == OF_8 || m_OutputFormat == OF_4); }
	void ImportPaletteGroup(void);
	BOOL CanImportPaletteGroup(void) const { return (BOOL)((m_FilterPaletteGroup != NULL) && (m_OutputFormat == OF_8 || m_OutputFormat == OF_4)); }
	void ImportAllPaletteGroups(void);
	BOOL CanImportAllPaletteGroups(void) const { return (BOOL)(m_OutputFormat == OF_8 || m_OutputFormat == OF_4); }
	BOOL CanClearImports(void) const;
	BOOL ClearImports(void);
	JC_TextureSetItem * HitTest(CPoint workpos) const;
	JC_TextureSetItemList * HitTest(CRect workrect) const;
	JC_PaletteGroupList * GetPaletteGroupList(void) const { return &(((JC_TextureSet*)this)->m_PaletteGroupList); }
	BOOL CheckOverlap(const CString& action);
	CRect Group(JC_TextureSetGroupRequest * greq);
	void Ungroup(JC_TextureSetItemList * list);
	JC_Damage UngroupSelection(void);
	JC_Damage SelectAll(void);
	JC_Damage InvertSelection(void);
	JC_Damage NudgeSelection(CSize delta);
	JC_Damage DeleteSelection(void)			{ return m_EditContext.DeleteSelection(); }
	JC_Damage NormaliseSelection(void)		{ return m_EditContext.NormaliseSelection(); }
	JC_Damage OrientSelection(int action)	{ return m_EditContext.OrientSelection(action); }
	int CountSelectedItems(void) const		{ return m_EditContext.CountSelectedItems(); }
	bool GetFloatUVArray(U32 texture_record, PS_FloatUVArray& array);

protected:
	int AcceptBitmap(JC_Record * record, CPoint workpos);
	int AcceptTexture(JC_Record * record, CPoint workpos);
	int AcceptModel(JC_Record * record, CPoint workpos);
	int AcceptTextureArray(const CDWordArray& recarray, CPoint workpos);
	int AcceptTextureList(PS_UniqueRecordList& reclist, CPoint workpos);
	int AcceptTexture(U32 recnum, CPoint workpos);
	const JC_TextureSetItemList& GetActiveList(void) const;
	void AddItemToGroup(JC_TextureSetItem * item, JC_TextureGroup * group, CSize offset);
	void ArrangeNewTextures(bool is_bitmap_drop);

protected:	// JC_PropertyRecord overrides
	virtual void WriteBody(void);
	virtual void ReadBody(void);
	virtual U16  Type(void) const { return TypeNewTextureSet; }
	virtual CString TypeString(void) const { return CString("New Texture Set"); }
	virtual void Dependencies(PS_CompressRecordList * list) const;
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);
};
//-------------------------------------------------------------------------------------------------------------------------
#endif