// FontMakerMFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FontMakerMFC.h"
#include "FontMakerMFCDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




//-----------------------------------------------------------------------------
// Name: struct GLYPH_ATTR
// Desc: A structure to hold attributes for one glpyh. The left, right, etc.
//       values are texture coordinate offsets into the resulting texture image
//       (which ends up in the .tga file). The offset, width, etc. values are
//       spacing information, used when rendering the font.
//-----------------------------------------------------------------------------
struct FILE_GLYPH_ATTR
{
    FLOAT fLeft, fTop, fRight, fBottom;
    SHORT wOffset;
    SHORT wWidth;
    SHORT wAdvance;
    SHORT wPad;
};

struct GLYPH_ATTR : public FILE_GLYPH_ATTR
{
    int   a, b, c;
    int   x, y, w, h;
};


BYTE*       g_ValidGlyphs     = NULL;
WCHAR       g_cMaxGlyph       = 0;
WORD*       g_TranslatorTable = NULL;
WCHAR*      g_strGlyphString  = NULL;
DWORD       g_dwNumGlyphs     = 0;
GLYPH_ATTR* g_pGlyphs         = NULL;

DWORD       g_dwVersion   = 0x00000005; // Texture file properties
DWORD       g_dwTexWidth   = 256;
DWORD       g_dwTexHeight  = 256;
DWORD       g_dwTexBPP    = 16;
DWORD       g_dwFontEffectiveHeight = 20;

BOOL m_bShadow = FALSE;
BOOL m_bBorder = FALSE;




//-----------------------------------------------------------------------------
// Name: ExtractValidGlyphsFromRange()
// Desc: Set global variables to indicate we will be drawing all glyphs in the
//       range specified.
//-----------------------------------------------------------------------------
HRESULT ExtractValidGlyphsFromRange( WORD wStart, WORD wEnd )
{
    // Cleanup any previous entries
    if( g_ValidGlyphs )
        delete g_ValidGlyphs;
    if( g_TranslatorTable )
        delete g_TranslatorTable;
    if( g_strGlyphString )
        delete g_strGlyphString;
    g_cMaxGlyph       = 0;
    g_dwNumGlyphs     = 0;
    g_ValidGlyphs     = NULL;
    g_TranslatorTable = NULL;
    g_strGlyphString  = NULL;

    // Allocate memory for the array of vaild glyphs
    g_ValidGlyphs = new BYTE[65536];
    ZeroMemory( g_ValidGlyphs, 65536 );

    for( DWORD c=(DWORD)wStart; c<=(DWORD)wEnd; c++ )
    {
        g_ValidGlyphs[c]++;
        g_dwNumGlyphs++;
    }

    g_cMaxGlyph = wEnd;

    // Insure the \0 is there
    if( 0 == g_ValidGlyphs['\0'] ) g_dwNumGlyphs++, g_ValidGlyphs['\0'] = 1;

    // Fill the string of all valid glyphs and build the translator table
    g_strGlyphString  = new WCHAR[g_dwNumGlyphs+1];
    g_TranslatorTable = new WORD[g_cMaxGlyph+1];
    ZeroMemory( g_TranslatorTable, sizeof(WORD)*(g_cMaxGlyph+1) );
    DWORD dwGlyph = 0;
    for( DWORD i=0; i<65536; i++ )
    {
        if( g_ValidGlyphs[i] )
        {
            g_strGlyphString[dwGlyph] = (WCHAR)i;
            g_TranslatorTable[i] = (WORD)dwGlyph;
            dwGlyph++;
        }
    }
    g_strGlyphString[dwGlyph] = (WCHAR)0;
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ExtractValidGlyphsFromRange()
// Desc: Set global variables to indicate we will be drawing all glyphs that
//       are present in the specified text file.
//-----------------------------------------------------------------------------
HRESULT ExtractValidGlyphsFromFile( const CHAR* strFileName )
{
    // Open the file
    FILE* file = fopen( strFileName, "rb" );
    if( NULL == file )
        return E_FAIL;

    // Cleanup any previous entries
    if( g_ValidGlyphs )
        delete g_ValidGlyphs;
    if( g_TranslatorTable )
        delete g_TranslatorTable;
    if( g_strGlyphString )
        delete g_strGlyphString;
    g_cMaxGlyph       = 0;
    g_dwNumGlyphs     = 0;
    g_ValidGlyphs     = NULL;
    g_TranslatorTable = NULL;
    g_strGlyphString  = NULL;

    // Allocate memory for the array of vaild glyphs
    g_ValidGlyphs = new BYTE[65536];
    ZeroMemory( g_ValidGlyphs, 65536 );


    // Skip the unicode marker
    BOOL bIsUnicode = (fgetwc(file) == 0xfeff) ? TRUE : FALSE;

    if( bIsUnicode == FALSE )
        rewind( file );

    // Record which glyphs are valid
    WCHAR c;
    while( (WCHAR)EOF != ( c = bIsUnicode ? fgetwc(file) : fgetc(file) ) )
    {
        while( c == L'\\' )
        {
            c = bIsUnicode ? fgetwc(file) : fgetc(file);

            // Handle octal-coded characters
            if( isdigit(c) )
            {
                int code = (c - L'0');
                c = bIsUnicode ? fgetwc(file) : fgetc(file);
                
                if( isdigit(c) )
                {
                    code = code*8 + (c - L'0');
                    c = bIsUnicode ? fgetwc(file) : fgetc(file);

                    if( isdigit(c) )
                    {
                        code = code*8 + (c - L'0');
                        c = bIsUnicode ? fgetwc(file) : fgetc(file);
                    }
                }

                if( g_ValidGlyphs[code] == 0 )
                    g_dwNumGlyphs++;
                g_ValidGlyphs[code] = 255;
            }
        }

        if( c > g_cMaxGlyph )
            g_cMaxGlyph = c;

        if( g_ValidGlyphs[c] == 0 )
            g_dwNumGlyphs++;

        if( g_ValidGlyphs[c] < 100 )
            g_ValidGlyphs[c]++;
    }

    // Done with the file
    fclose( file );

    // Reject certain unprintable characters
    if( g_ValidGlyphs['\n'] ) g_dwNumGlyphs--, g_ValidGlyphs['\n'] = 0;
    if( g_ValidGlyphs['\r'] ) g_dwNumGlyphs--, g_ValidGlyphs['\r'] = 0;

    // Insure the \0 is there
    if( 0 == g_ValidGlyphs['\0'] ) g_dwNumGlyphs++, g_ValidGlyphs['\0'] = 1;

    // Fill the string of all valid glyphs and build the translator table
    g_strGlyphString  = new WCHAR[g_dwNumGlyphs+1];
    g_TranslatorTable = new WORD[g_cMaxGlyph+1];
    ZeroMemory( g_TranslatorTable, sizeof(WORD)*(g_cMaxGlyph+1) );
    DWORD dwGlyph = 0;
    for( DWORD i=0; i<65536; i++ )
    {
        if( g_ValidGlyphs[i] )
        {
            g_strGlyphString[dwGlyph] = (WCHAR)i;
            g_TranslatorTable[i] = (WORD)dwGlyph;
            dwGlyph++;
        }
    }
    g_strGlyphString[dwGlyph] = (WCHAR)0;
    
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: class CFontMakerMFCDlg
// Desc: Main dialog for the app.
//-----------------------------------------------------------------------------
CFontMakerMFCDlg::CFontMakerMFCDlg(CWnd* pParent /*=NULL*/)
    : CDialog( CFontMakerMFCDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CFontMakerMFCDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFontMakerMFCDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFontMakerMFCDlg)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFontMakerMFCDlg, CDialog)
    //{{AFX_MSG_MAP(CFontMakerMFCDlg)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_CBN_SELCHANGE(IDC_FONTNAME, OnSelchangeFontname)
    ON_CBN_EDITUPDATE(IDC_FONTNAME, OnEditupdateFontname)
    ON_CBN_SELCHANGE(IDC_FONTSTYLE, OnSelchangeFontstyle)
    ON_CBN_SELCHANGE(IDC_FONTSIZE, OnSelchangeFontsize)
    ON_CBN_KILLFOCUS(IDC_FONTSIZE, OnKillfocusFontsize)
    ON_NOTIFY(HDN_ITEMCLICK, IDC_GLYPH_LIST, OnItemclickGlyphList)
    ON_NOTIFY(NM_CLICK, IDC_GLYPH_LIST, OnClickGlyphList)
    ON_NOTIFY(HDN_ITEMCHANGED, IDC_GLYPH_LIST, OnItemchangedGlyphList)
    ON_BN_CLICKED(IDC_FIXED_RANGE_RADIO, OnFixedRangeRadio)
    ON_BN_CLICKED(IDC_EXTRACT_FROM_FILE, OnExtractFromFile)
    ON_BN_CLICKED(IDC_FILESELECTOR, OnFileselector)
    ON_EN_KILLFOCUS(IDC_GLYPHRANGE_FROM, OnKillfocusGlyphrangeFrom)
    ON_EN_KILLFOCUS(IDC_GLYPHRANGE_TO, OnKillfocusGlyphrangeTo)
    ON_EN_KILLFOCUS(IDC_GLYPHFILENAME, OnKillfocusGlyphfilename)
	ON_EN_KILLFOCUS(IDC_BITMAP_WIDTH, OnKillfocusBitmapSize)
	ON_EN_KILLFOCUS(IDC_BITMAP_HEIGHT, OnKillfocusBitmapSize)
    ON_WM_SYSCOMMAND()
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_GLYPH_LIST, OnItemchangedGlyphList)
	ON_BN_CLICKED(IDC_EFFECTS_BORDER, OnEffects)
	ON_BN_CLICKED(IDC_EFFECTS_SHADOW, OnEffects)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()





#define MAX_FONTS 200
LOGFONT    g_LogFont[MAX_FONTS];
DWORD      g_dwNumFonts;

//-----------------------------------------------------------------------------
// Name: EnumFontsCallback()
// Desc: Callback function for enumerating fonts
//-----------------------------------------------------------------------------
int CALLBACK EnumFontsCallback( const LOGFONT* plf, const TEXTMETRIC* ptm, DWORD dwType, LPARAM lpData )
{
    if( g_dwNumFonts >= MAX_FONTS )
        return FALSE;

    if( plf->lfCharSet == 0 && plf->lfFaceName[0]!='@' )
    {
        memcpy( &g_LogFont[g_dwNumFonts], plf, sizeof(LOGFONT) );
        g_LogFont[g_dwNumFonts].lfWidth = 0;
        g_dwNumFonts++;
    }

    return TRUE;
}



//-----------------------------------------------------------------------------
// Name: FontSortCallback()
// Desc: Callback function for sorting fonts by their names
//-----------------------------------------------------------------------------
int FontSortCallback( const VOID* a, const VOID* b )
{
    return stricmp( ((LOGFONT*)a)->lfFaceName, ((LOGFONT*)b)->lfFaceName );
}




//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Initialize the dialog and it's controls
//-----------------------------------------------------------------------------
BOOL CFontMakerMFCDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Link in the custom control used to show the texture
    m_TextureBitmap.SubclassDlgItem(IDC_TEXTURE, this);

    // Get access to the dialog's controls
    m_pFontNameComboBox  = (CComboBox*)GetDlgItem(IDC_FONTNAME);
    m_pFontStyleComboBox = (CComboBox*)GetDlgItem(IDC_FONTSTYLE);
    m_pFontSizeComboBox  = (CComboBox*)GetDlgItem(IDC_FONTSIZE);

    // Populate the list of available fonts
    EnumFonts( GetDC()->m_hDC, NULL, EnumFontsCallback, NULL );
    qsort( g_LogFont, g_dwNumFonts, sizeof(LOGFONT), FontSortCallback );
    
    for( DWORD i=0; i<g_dwNumFonts; i++ )
    {
        m_pFontNameComboBox->AddString( g_LogFont[i].lfFaceName );
    }

    // Initially, no font is selected
    m_pFontNameComboBox->SetWindowText( _T("<Choose font>") );
    m_iCurrentFont = -1;
    m_pFont        = NULL;

	// Set the initial texture size
	CString strWidth;
    CString strHeight;
	strWidth.Format( "%ld", g_dwTexWidth );
	strHeight.Format( "%ld", g_dwTexHeight );
    GetDlgItem( IDC_BITMAP_WIDTH )->SetWindowText( strWidth );
    GetDlgItem( IDC_BITMAP_HEIGHT )->SetWindowText( strHeight );

    // Deactivate controls, which are only valid after a font is picked
    ((CButton*)GetDlgItem( IDC_FIXED_RANGE_RADIO ))->SetCheck(TRUE);

    GetDlgItem( IDOK )->EnableWindow( FALSE );
    GetDlgItem( IDC_FIXED_RANGE_RADIO )->EnableWindow( FALSE );
    GetDlgItem( IDC_GLYPHRANGE_FROM )->EnableWindow( FALSE );
    GetDlgItem( IDC_GLYPHRANGE_TO )->EnableWindow( FALSE );
    GetDlgItem( IDC_EXTRACT_FROM_FILE )->EnableWindow( FALSE );
    GetDlgItem( IDC_GLYPHFILENAME )->EnableWindow( FALSE );
    GetDlgItem( IDC_FILESELECTOR )->EnableWindow( FALSE );
    GetDlgItem( IDC_BITMAP_WIDTH )->EnableWindow( FALSE );
    GetDlgItem( IDC_BITMAP_HEIGHT )->EnableWindow( FALSE );
    
    // Initialize the report columns in the list box
    m_pGlyphList = (CListCtrl*)GetDlgItem( IDC_GLYPH_LIST );
    m_pGlyphList->InsertColumn( 0, "Glpyh",   LVCFMT_CENTER, 55 );
    m_pGlyphList->InsertColumn( 1, "x",       LVCFMT_CENTER, 30 );
    m_pGlyphList->InsertColumn( 2, "y",       LVCFMT_CENTER, 30 );
    m_pGlyphList->InsertColumn( 3, "w",       LVCFMT_CENTER, 30 );
    m_pGlyphList->InsertColumn( 4, "h",       LVCFMT_CENTER, 30 );
    m_pGlyphList->InsertColumn( 5, "A",       LVCFMT_CENTER, 30 );
    m_pGlyphList->InsertColumn( 6, "B",       LVCFMT_CENTER, 30 );
    m_pGlyphList->InsertColumn( 7, "C",       LVCFMT_CENTER, 30 );

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // Set a default range of glyphs to use
    GetDlgItem( IDC_GLYPHRANGE_FROM )->SetWindowText( "32" );
    GetDlgItem( IDC_GLYPHRANGE_TO )->SetWindowText( "127" );
    ::ExtractValidGlyphsFromRange( 32, 127 );

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFontMakerMFCDlg::OnPaint() 
{
    if( IsIconic() )
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

HCURSOR CFontMakerMFCDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}




//-----------------------------------------------------------------------------
// Name: OnSelchangeFontname() 
// Desc: Called when the user selects a new font
//-----------------------------------------------------------------------------
void CFontMakerMFCDlg::OnSelchangeFontname() 
{
    m_iCurrentFont = m_pFontNameComboBox->GetCurSel();
    if( m_iCurrentFont < 0 )
        return;

    if( FALSE == GetDlgItem( IDC_FIXED_RANGE_RADIO )->IsWindowEnabled() &&
        FALSE == GetDlgItem( IDC_EXTRACT_FROM_FILE )->IsWindowEnabled() )
    {
        GetDlgItem( IDC_FIXED_RANGE_RADIO )->EnableWindow( TRUE );
        GetDlgItem( IDC_GLYPHRANGE_FROM )->EnableWindow( TRUE );
        GetDlgItem( IDC_GLYPHRANGE_TO )->EnableWindow( TRUE );
        GetDlgItem( IDC_EXTRACT_FROM_FILE )->EnableWindow( TRUE );
    }

    GetDlgItem( IDOK )->EnableWindow( TRUE );
	GetDlgItem( IDC_BITMAP_WIDTH )->EnableWindow( TRUE );
	GetDlgItem( IDC_BITMAP_HEIGHT )->EnableWindow( TRUE );

    // On the fist selection populate the style and size comboboxes
    static BOOL bStyleAndSizeListsArePopulated = FALSE;
    if( FALSE == bStyleAndSizeListsArePopulated )
    {
        // Populate the font styles
        m_pFontStyleComboBox->EnableWindow( TRUE );
        m_pFontStyleComboBox->AddString( "Regular" );
        m_pFontStyleComboBox->AddString( "Bold" );
        m_pFontStyleComboBox->AddString( "Italic" );
        m_pFontStyleComboBox->AddString( "Bold italic" );
        m_pFontStyleComboBox->SetCurSel( 1 );

        // Populate the font sizes
        m_pFontSizeComboBox->EnableWindow( TRUE );
        m_pFontSizeComboBox->AddString( "8" );
        m_pFontSizeComboBox->AddString( "9" );
        m_pFontSizeComboBox->AddString( "10" );
        m_pFontSizeComboBox->AddString( "11" );
        m_pFontSizeComboBox->AddString( "12" );
        m_pFontSizeComboBox->AddString( "14" );
        m_pFontSizeComboBox->AddString( "16" );
        m_pFontSizeComboBox->AddString( "18" );
        m_pFontSizeComboBox->AddString( "20" );
        m_pFontSizeComboBox->AddString( "22" );
        m_pFontSizeComboBox->AddString( "24" );
        m_pFontSizeComboBox->AddString( "26" );
        m_pFontSizeComboBox->AddString( "28" );
        m_pFontSizeComboBox->AddString( "36" );
        m_pFontSizeComboBox->AddString( "48" );
        m_pFontSizeComboBox->AddString( "72" );
        m_pFontSizeComboBox->SetCurSel( 6 );

        bStyleAndSizeListsArePopulated = TRUE;
    }

    g_LogFont[m_iCurrentFont].lfHeight = -MulDiv(16, GetDC()->GetDeviceCaps(LOGPIXELSY), 72);

    CreateFont();
}



//-----------------------------------------------------------------------------
// Name: CreateFont() 
// Desc: Uses the user-selected params to create a font
//-----------------------------------------------------------------------------
void CFontMakerMFCDlg::CreateFont() 
{
    // Create the newly selected font
    if( m_pFont != NULL ) 
        delete m_pFont;
    m_pFont = new CFont();

    CHAR strFontStyle[80];
    m_pFontStyleComboBox->GetWindowText( strFontStyle, 80 );

    switch( m_pFontStyleComboBox->GetCurSel() )
    {
        default:
        case 0: // Regular
            g_LogFont[m_iCurrentFont].lfWeight = 400;
            g_LogFont[m_iCurrentFont].lfItalic = FALSE;
            break;
        case 1: // Bold
            g_LogFont[m_iCurrentFont].lfWeight = 700;
            g_LogFont[m_iCurrentFont].lfItalic = FALSE;
            break;
        case 2: // Italic
            g_LogFont[m_iCurrentFont].lfWeight = 400;
            g_LogFont[m_iCurrentFont].lfItalic = TRUE;
            break;
        case 3: // Bold italic
            g_LogFont[m_iCurrentFont].lfWeight = 700;
            g_LogFont[m_iCurrentFont].lfItalic = TRUE;
            break;
    }

    m_pFont->CreateFontIndirect( &g_LogFont[m_iCurrentFont] );

    // Draw the glyphs and update the list of glyphs
    DrawGlyphs();
    UpdateGlyphList();
}




//-----------------------------------------------------------------------------
// Name: DrawGlyphs() 
// Desc: Draws the list of glyphs in the custom control
//-----------------------------------------------------------------------------
VOID CFontMakerMFCDlg::DrawGlyphs( CDC* pDC )
{
    if( m_pFont == NULL )
        return;

    if( NULL == pDC )
        pDC = GetDlgItem(IDC_TEXTURE)->GetDC();

    // Setup the DC for the font
    pDC->FillSolidRect( 0, 0, 512, 512, RGB(128,128,128));
    pDC->FillSolidRect( 0, 0, g_dwTexWidth, g_dwTexHeight, RGB(0,0,255));
    pDC->SetTextColor( RGB(255,255,255) );
    pDC->SelectObject( m_pFont );
    pDC->SetTextAlign( TA_LEFT|TA_TOP|TA_UPDATECP );
    pDC->SetMapMode( MM_TEXT );
	pDC->SetBkMode( TRANSPARENT );
	pDC->SetBkColor( RGB(0,0,255) );

	CRgn rgn;
    rgn.CreateRectRgn( 0, 0, g_dwTexWidth, g_dwTexHeight );
    pDC->SelectClipRgn( &rgn );
    
    CPen pen( PS_SOLID, 1, RGB(255,0,0) );
    pDC->SelectObject( &pen );

    // Get the effective font height
    WCHAR str[2] = L"A";
    SIZE  size;
    GetTextExtentPoint32W( pDC->m_hDC, str, 1, &size );

    DWORD g_dwStartGlyph = 32;
    DWORD g_dwEndGlyph   = 127;
    DWORD g_dwLeftOrigin = 1;
    DWORD g_dwTopOrigin  = 1;

    if( g_pGlyphs )
        delete g_pGlyphs;
    g_pGlyphs = new GLYPH_ATTR[g_dwNumGlyphs];
    
    GLYPH_ATTR* pGlyphs = g_pGlyphs;
    
    // Loop through all printable character and output them to the bitmap..
    // Meanwhile, keep track of the corresponding tex coords for each character.
    DWORD index = 0;
    DWORD x     = g_dwLeftOrigin;
    DWORD y     = g_dwTopOrigin;

    for( DWORD g=0; g<g_dwNumGlyphs; g++ )
    {
        WCHAR c = g_strGlyphString[g];
        
        // Set an unprintable character
        if( c==0 )
            c = 0xffff;

        str[0] = c;

        GetTextExtentPoint32W( pDC->m_hDC, str, 1, &size );

        // Get char width a different way
        int charwidth;
        GetCharWidth32( pDC->m_hDC, str[0], str[0], &charwidth );

        // Get the ABC widths for the letter
        ABC abc;
        if( FALSE == GetCharABCWidthsW( pDC->m_hDC, str[0], str[0], &abc ) )
        {
            abc.abcA = 0;
            abc.abcB = size.cx;
            abc.abcC = 0;
        }

		int w = abc.abcB;
        int h = size.cy;

		// Determine padding for outline and shadow effects
		int left_padding   = ( m_bBorder ? 1 : 0 );
		int right_padding  = ( m_bBorder ? ( m_bShadow ? 2 : 1 ) : ( m_bShadow ? 2 : 0 ) );
		int top_padding    = ( m_bBorder ? 1 : 0 );
		int bottom_padding = ( m_bBorder ? ( m_bShadow ? 2 : 1 ) : ( m_bShadow ? 2 : 0 ) );
        
		// Advance to the next line, if necessary
        if( x + h + left_padding + right_padding >= (int)g_dwTexWidth )
        {
            x  = g_dwLeftOrigin;
            y += h + top_padding + bottom_padding + 1;
        }

        if( g_ValidGlyphs[c] == 255 )
        {
            // Handle special characters

			// Draw a square box for a placeholder for custom glyph graphics
			w = h + left_padding + right_padding;
			h = h + top_padding + bottom_padding;

            abc.abcA = 0;
            abc.abcB = w;
            abc.abcC = 0;

            pDC->FillSolidRect( x, y, w, h, RGB(0,0,0) );
        }
        else
        {
            int sx = x;
            int sy = y;

            // Adjust ccordinates to account for the leading edge
            if( abc.abcA >= 0 )
                x += abc.abcA;
            else
                sx -= abc.abcA;

            // Hack to adjust for Kanji
            if( str[0] > 0x1000 )
            {
                sx += abc.abcA;
            }

			// Add padding to the width and height
			w += left_padding + right_padding;
			h += top_padding + bottom_padding;
			abc.abcA -= left_padding;
			abc.abcB += left_padding + right_padding;
			abc.abcC -= right_padding;

			if( m_bBorder )
			{
			    pDC->SetTextColor( RGB(0,0,0) );
				pDC->MoveTo( sx+0, sy+0 ); ExtTextOutW( pDC->m_hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
				pDC->MoveTo( sx+1, sy+0 ); ExtTextOutW( pDC->m_hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
				pDC->MoveTo( sx+2, sy+0 ); ExtTextOutW( pDC->m_hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
				pDC->MoveTo( sx+0, sy+1 ); ExtTextOutW( pDC->m_hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
				pDC->MoveTo( sx+2, sy+1 ); ExtTextOutW( pDC->m_hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
				pDC->MoveTo( sx+0, sy+2 ); ExtTextOutW( pDC->m_hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
				pDC->MoveTo( sx+1, sy+2 ); ExtTextOutW( pDC->m_hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
				pDC->MoveTo( sx+2, sy+2 ); ExtTextOutW( pDC->m_hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );

				if( m_bShadow )
				{
				    pDC->MoveTo( sx+3, sy+3 ); ExtTextOutW( pDC->m_hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
				}
				
				pDC->SetTextColor( RGB(255,255,255) );

				// Output the letter
				pDC->MoveTo( sx+1, sy+1 ); ExtTextOutW( pDC->m_hDC, sx, sy, ETO_OPAQUE, NULL, str, 1, NULL );
			}
			else
			{
				if( m_bShadow )
				{
					pDC->SetTextColor( RGB(0,0,0) );
					pDC->MoveTo( sx+2, sy+2 ); ExtTextOutW( pDC->m_hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
					pDC->SetTextColor( RGB(255,255,255) );
				}

				// Output the letter
				pDC->MoveTo( sx, sy );
				ExtTextOutW( pDC->m_hDC, sx, sy, ETO_OPAQUE, NULL, str, 1, NULL );
			}

            // Hack for extended characters (like Kanji) that don't seem to report
            // correct ABC widths. In this case, use the width calculated from
            // drawing the glyph.
            if( str[0] > 0x1000 )
            {
                CPoint pos = pDC->GetCurrentPosition();
                abc.abcB = pos.x - sx;

                if( abc.abcC < 0 )
                    abc.abcB -= abc.abcC;

                w = abc.abcB;
            }
        }

        // Store the glyph attributes
        pGlyphs[index].x        = x;
        pGlyphs[index].y        = y;
        pGlyphs[index].w        = w;
        pGlyphs[index].h        = h;
        pGlyphs[index].a        = abc.abcA;
        pGlyphs[index].b        = abc.abcB;
        pGlyphs[index].c        = abc.abcC;
        pGlyphs[index].fLeft   = ((FLOAT)(x+0)) / g_dwTexWidth;
        pGlyphs[index].fTop    = ((FLOAT)(y+0)) / g_dwTexHeight;
        pGlyphs[index].fRight  = ((FLOAT)(x+w)) / g_dwTexWidth;
        pGlyphs[index].fBottom = ((FLOAT)(y+h)) / g_dwTexHeight;
        pGlyphs[index].wOffset  = (short)(abc.abcA);
        pGlyphs[index].wWidth   = (short)(abc.abcB);
        pGlyphs[index].wAdvance = (short)(abc.abcB + abc.abcC);
        index++;

        // Advance the cursor to the next position
        x += w + 1;
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateGlyphList() 
// Desc: Updates the list of glyphs in the list control
//-----------------------------------------------------------------------------
VOID CFontMakerMFCDlg::UpdateGlyphList()
{
    if( m_pFont == NULL )
        return;

    CListCtrl* m_pGlyphList;
    m_pGlyphList = (CListCtrl*)GetDlgItem( IDC_GLYPH_LIST );
    m_pGlyphList->DeleteAllItems();

    GLYPH_ATTR* pGlyphs = g_pGlyphs;
    
    for( DWORD i=0; i<g_dwNumGlyphs; i++ )
    {
        WCHAR c = g_strGlyphString[i];

        TCHAR strBuffer[80];
        _stprintf( strBuffer, _T("%04lx (%c)"), c, c );
        int _i = m_pGlyphList->InsertItem( i, strBuffer );

        _stprintf( strBuffer, _T("%d"), pGlyphs[i].x );
        m_pGlyphList->SetItemText( _i, 1, strBuffer );
        
        _stprintf( strBuffer, _T("%d"), pGlyphs[i].y );
        m_pGlyphList->SetItemText( _i, 2, strBuffer );
        
        _stprintf( strBuffer, _T("%d"), pGlyphs[i].w );
        m_pGlyphList->SetItemText( _i, 3, strBuffer );
        
        _stprintf( strBuffer, _T("%d"), pGlyphs[i].h );
        m_pGlyphList->SetItemText( _i, 4, strBuffer );
        
        _stprintf( strBuffer, _T("%d"), pGlyphs[i].a );
        m_pGlyphList->SetItemText( _i, 5, strBuffer );
        
        _stprintf( strBuffer, _T("%d"), pGlyphs[i].b );
        m_pGlyphList->SetItemText( _i, 6, strBuffer );
        
        _stprintf( strBuffer, _T("%d"), pGlyphs[i].c );
        m_pGlyphList->SetItemText( _i, 7, strBuffer );
    }
}




//-----------------------------------------------------------------------------
// Functions for handling user input for font parameters
//-----------------------------------------------------------------------------
void CFontMakerMFCDlg::OnEditupdateFontname() 
{
    // Prevent user from editting the font text
    m_pFontNameComboBox->SetCurSel( m_iCurrentFont );
}

void CFontMakerMFCDlg::OnSelchangeFontstyle() 
{
    CreateFont();
}

void CFontMakerMFCDlg::OnSelchangeFontsize() 
{
    int iHeight = 16;

    switch( m_pFontSizeComboBox->GetCurSel() )
    {
        case 0: iHeight =  8; break;
        case 1: iHeight =  9; break;
        case 2: iHeight = 10; break;
        case 3: iHeight = 11; break;
        case 4: iHeight = 12; break;
        case 5: iHeight = 14; break;
        case 6: iHeight = 16; break;
        case 7: iHeight = 18; break;
        case 8: iHeight = 20; break;
        case 9: iHeight = 22; break;
        case 10: iHeight = 24; break;
        case 11: iHeight = 26; break;
        case 12: iHeight = 28; break;
        case 13: iHeight = 36; break;
        case 14: iHeight = 48; break;
        case 15: iHeight = 72; break;
    }

    g_LogFont[m_iCurrentFont].lfHeight = -MulDiv(iHeight, GetDC()->GetDeviceCaps(LOGPIXELSY), 72);

    CreateFont();
}

void CFontMakerMFCDlg::OnKillfocusFontsize() 
{
    CHAR strFontSize[80];
    m_pFontSizeComboBox->GetWindowText( strFontSize, 80 );
    int iHeight = max( 0, min( 256, atoi( strFontSize ) ) );
    g_LogFont[m_iCurrentFont].lfHeight = -MulDiv(iHeight, GetDC()->GetDeviceCaps(LOGPIXELSY), 72);

    CreateFont();
}




//-----------------------------------------------------------------------------
// Functions for handling user input for the glyph list control
//-----------------------------------------------------------------------------
void CFontMakerMFCDlg::OnItemclickGlyphList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
    // TODO: Add your control notification handler code here
    
    *pResult = 0;
}

void CFontMakerMFCDlg::OnClickGlyphList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    // TODO: Add your control notification handler code here
    *pResult = 0;
}

int g_iOldSelectedGlyph = -1;

void CFontMakerMFCDlg::OnItemchangedGlyphList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
    // TODO: Add your control notification handler code here
    
    CDC* pDC = GetDlgItem(IDC_TEXTURE)->GetDC();

    // Setup the DC for the font
    pDC->SetTextColor( RGB(255,255,255) );
    pDC->SetBkColor( 0xffff00ff );
    pDC->SetBkMode( TRANSPARENT );


    POSITION pos = m_pGlyphList->GetFirstSelectedItemPosition();
    int item = m_pGlyphList->GetNextSelectedItem( pos );
    

    if( g_iOldSelectedGlyph >= 0 )
    {
        int x1 = g_pGlyphs[g_iOldSelectedGlyph].x - 1;
        int y1 = g_pGlyphs[g_iOldSelectedGlyph].y - 1;
        int x2 = g_pGlyphs[g_iOldSelectedGlyph].x + g_pGlyphs[g_iOldSelectedGlyph].w;
        int y2 = g_pGlyphs[g_iOldSelectedGlyph].y + g_pGlyphs[g_iOldSelectedGlyph].h;

        CPen pen( PS_SOLID, 1, RGB(0,0,255) );
        pDC->SelectObject( &pen );

        pDC->MoveTo( x1, y1 );
        pDC->LineTo( x2, y1 );
        pDC->LineTo( x2, y2 );
        pDC->LineTo( x1, y2 );
        pDC->LineTo( x1, y1 );
    }

    int x1 = g_pGlyphs[item].x - 1;
    int y1 = g_pGlyphs[item].y - 1;
    int x2 = g_pGlyphs[item].x + g_pGlyphs[item].w;
    int y2 = g_pGlyphs[item].y + g_pGlyphs[item].h;

    CPen pen( PS_SOLID, 1, RGB(255,0,0) );
    pDC->SelectObject( &pen );

    pDC->MoveTo( x1, y1 );
    pDC->LineTo( x2, y1 );
    pDC->LineTo( x2, y2 );
    pDC->LineTo( x1, y2 );
    pDC->LineTo( x1, y1 );

    g_iOldSelectedGlyph = item;

    *pResult = 0;
}




//-----------------------------------------------------------------------------
// Name: WriteTargaFile()
// Desc: Writes 32-bit RGBA data to a .tga file
//-----------------------------------------------------------------------------
HRESULT WriteTargaFile( TCHAR* strFileName, DWORD dwWidth, DWORD dwHeight,
                        DWORD* pRGBAData )
{
    struct TargaHeader
    {
        BYTE IDLength;
        BYTE ColormapType;
        BYTE ImageType;
        BYTE ColormapSpecification[5];
        WORD XOrigin;
        WORD YOrigin;
        WORD ImageWidth;
        WORD ImageHeight;
        BYTE PixelDepth;
        BYTE ImageDescriptor;
    } tga;

    // Create the file
    FILE* file = fopen( strFileName, "wb" );
    if( NULL == file )
        return E_FAIL;

    // Write the TGA header
    ZeroMemory( &tga, sizeof(tga) );
    tga.IDLength        = 0;
    tga.ImageType       = 2;
    tga.ImageWidth      = (WORD)dwWidth;
    tga.ImageHeight     = (WORD)dwHeight;
    tga.PixelDepth      = 32;
    tga.ImageDescriptor = 0x28;
    fwrite( &tga, sizeof(TargaHeader), 1, file );

    // Write the pixels
    fwrite( pRGBAData, sizeof(DWORD), dwHeight*dwWidth, file );

    // Close the file and return okay
    fclose( file );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: WriteFontInfoFile()
// Desc: Writes the font's glyph info to a file
//-----------------------------------------------------------------------------
HRESULT WriteFontInfoFile( CHAR* strFileName )
{
    // Create the info file
    FILE* file = fopen( strFileName, "wb" );

    // Write out the file version
    fwrite( &g_dwVersion,    sizeof(DWORD), 1, file ); 

    // Write out the font height
    g_dwFontEffectiveHeight = g_pGlyphs[0].h;
    fwrite( &g_dwFontEffectiveHeight, sizeof(DWORD), 1, file ); 

    // Write out the texture values
    fwrite( &g_dwTexWidth,   sizeof(DWORD), 1, file ); 
    fwrite( &g_dwTexHeight,  sizeof(DWORD), 1, file ); 
    fwrite( &g_dwTexBPP,     sizeof(DWORD), 1, file ); 

    // Write the translator table
    fwrite( &g_cMaxGlyph, sizeof(WCHAR), 1, file ); 
    fwrite( g_TranslatorTable, sizeof(WORD), g_cMaxGlyph+1, file ); 

    // Write the glyph attributes to the file
    fwrite( &g_dwNumGlyphs, sizeof(DWORD), 1, file ); 
    for( DWORD i=0; i<g_dwNumGlyphs; i++ )
    {
        fwrite( &g_pGlyphs[i], sizeof(FILE_GLYPH_ATTR), 1, file ); 
    }

    // Close the file and return okay
    fclose( file );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OnOK()
// Desc: Handles when the user hits OK, this writes out a binary .abc file
//       which contains the spacing info for all the glyphs
//-----------------------------------------------------------------------------
void CFontMakerMFCDlg::OnOK() 
{
	if(0)
	{
		FILE* f = fopen( "c:\\dump.txt", "w" );
		for( DWORD i=32;i<1000;i++ )
		{
			WCHAR str[2] = L"A";
			str[0] = (WCHAR)i;
			fprintf( f, "%S", str );
		}
		fclose( f );
	}

    LONG lHeight = -g_LogFont[m_iCurrentFont].lfHeight * 72 / GetDeviceCaps(GetDlgItem(IDC_TEXTURE)->GetDC()->m_hDC, LOGPIXELSY);

    CHAR strFilename[80];

	// Save the font info file (.abc)
    sprintf( strFilename, "%s_%d.abc", g_LogFont[m_iCurrentFont].lfFaceName, lHeight );
	WriteFontInfoFile( strFilename );


    CPaintDC dc(this); // device context for painting
    CDC memDC;
    CBitmap bitmap;
    memDC.CreateCompatibleDC(&dc);
    bitmap.CreateCompatibleBitmap( &dc, g_dwTexWidth, g_dwTexHeight );
    CBitmap *pOldmemDCBitmap = (CBitmap*)memDC.SelectObject(&bitmap);
    
    DrawGlyphs( &memDC );

	DWORD* pRGBAData = new DWORD[g_dwTexWidth*g_dwTexHeight];
	bitmap.GetBitmapBits( g_dwTexWidth*g_dwTexHeight*sizeof(DWORD), pRGBAData );

	// Convert the RGBA data
	for( DWORD i=0; i<g_dwTexWidth*g_dwTexHeight; i++ )
	{
		DWORD red = ( 0x00ff0000 & pRGBAData[i] ) >> 16L;
		DWORD grn = ( 0x0000ff00 & pRGBAData[i] ) >>  8L;
		DWORD blu = ( 0x000000ff & pRGBAData[i] ) >>  0L;

		if( red == blu && grn == blu )
		{
			// Must be an opaque pixel )
			pRGBAData[i] = 0xff000000 | (red<<16L) | (red<<8L) | (red<<0L);
		}
		else
		{
			// Write out transparent pixel
			pRGBAData[i] = ((256-blu)<<24L) | 0x00000000;
		}
	}

	// Save the font image file (.tga)
    sprintf( strFilename, "%s_%d.tga", g_LogFont[m_iCurrentFont].lfFaceName, lHeight );
	WriteTargaFile( strFilename, g_dwTexWidth, g_dwTexHeight, pRGBAData );
	delete[] pRGBAData;

    memDC.SelectObject(pOldmemDCBitmap);

	// Don't actually exit
    // CDialog::OnOK();
}




//-----------------------------------------------------------------------------
// Functions for handling user input for the glyph range
//-----------------------------------------------------------------------------
void CFontMakerMFCDlg::ExtractValidGlyphsFromRange() 
{
    CString strFrom;
    CString strTo;
    GetDlgItem( IDC_GLYPHRANGE_FROM )->GetWindowText( strFrom );
    GetDlgItem( IDC_GLYPHRANGE_TO )->GetWindowText( strTo );

    WORD wFrom = (WORD)max( 0, atoi( strFrom ) );
    WORD wTo   = (WORD)min( 65535, atoi( strTo ) );
    ::ExtractValidGlyphsFromRange( wFrom, wTo );
    CreateFont();
}

void CFontMakerMFCDlg::OnFixedRangeRadio() 
{
    GetDlgItem( IDC_GLYPHRANGE_FROM )->EnableWindow( TRUE );
    GetDlgItem( IDC_GLYPHRANGE_TO )->EnableWindow( TRUE );
    GetDlgItem( IDC_GLYPHFILENAME )->EnableWindow( FALSE );
    GetDlgItem( IDC_FILESELECTOR )->EnableWindow( FALSE );

    ExtractValidGlyphsFromRange();
}

void CFontMakerMFCDlg::OnKillfocusGlyphrangeFrom() 
{
    ExtractValidGlyphsFromRange();
}

void CFontMakerMFCDlg::OnKillfocusGlyphrangeTo() 
{
    ExtractValidGlyphsFromRange();
}




//-----------------------------------------------------------------------------
// Functions for handling user input for the glyph range
//-----------------------------------------------------------------------------
void CFontMakerMFCDlg::ExtractValidGlyphsFromFile() 
{
    CString strFileName;
    GetDlgItem( IDC_GLYPHFILENAME )->GetWindowText( strFileName );

    if( strFileName.IsEmpty() )
        return;

    ::ExtractValidGlyphsFromFile( (LPCTSTR)strFileName );
    CreateFont();
}

void CFontMakerMFCDlg::OnExtractFromFile() 
{
    GetDlgItem( IDC_GLYPHRANGE_FROM )->EnableWindow( FALSE );
    GetDlgItem( IDC_GLYPHRANGE_TO )->EnableWindow( FALSE );
    GetDlgItem( IDC_GLYPHFILENAME )->EnableWindow( TRUE );
    GetDlgItem( IDC_FILESELECTOR )->EnableWindow( TRUE );

    ExtractValidGlyphsFromFile();
}

void CFontMakerMFCDlg::OnFileselector() 
{
    static TCHAR strFileName[MAX_PATH] = _T("");
    static TCHAR strFileName2[MAX_PATH]    = _T("");
    static TCHAR strInitialDir[MAX_PATH]  = _T("c:\\");

    // Display the OpenFileName dialog. Then, try to load the specified file
    OPENFILENAME ofn = { sizeof(OPENFILENAME), NULL, NULL,
                         _T("Text files (.txt)\0*.txt\0\0"), 
                         NULL, 0, 1, strFileName, MAX_PATH, strFileName2, MAX_PATH, 
                         strInitialDir, _T("Open Text File"), 
                         OFN_FILEMUSTEXIST, 0, 1, NULL, 0, NULL, NULL };

    if( TRUE == GetOpenFileName( &ofn ) )
    {
        GetDlgItem(IDC_GLYPHFILENAME)->SetWindowText( ofn.lpstrFile);
        ExtractValidGlyphsFromFile();
    }
}

void CFontMakerMFCDlg::OnKillfocusGlyphfilename() 
{
    ExtractValidGlyphsFromFile();
}




//-----------------------------------------------------------------------------
// Name: OnKillfocusBitmapSize()
// Desc: For handling user input for bitmap size
//-----------------------------------------------------------------------------
void CFontMakerMFCDlg::OnKillfocusBitmapSize() 
{
	CString strWidth;
    CString strHeight;
    GetDlgItem( IDC_BITMAP_WIDTH )->GetWindowText( strWidth );
    GetDlgItem( IDC_BITMAP_HEIGHT )->GetWindowText( strHeight );

    g_dwTexWidth  = (DWORD)max( 64, min( 1024, atoi( strWidth ) ) );
    g_dwTexHeight = (DWORD)max( 64, min( 1024, atoi( strHeight ) ) );

	strWidth.Format( "%ld", g_dwTexWidth );
	strHeight.Format( "%ld", g_dwTexHeight );
    GetDlgItem( IDC_BITMAP_WIDTH )->SetWindowText( strWidth );
    GetDlgItem( IDC_BITMAP_HEIGHT )->SetWindowText( strHeight );

    DrawGlyphs();
}




//-----------------------------------------------------------------------------
// Name: class CTextureBitmap
// Desc: A custom control for displaying the glyphs
//-----------------------------------------------------------------------------
CTextureBitmap::CTextureBitmap()
{
    CTextureBitmap::RegisterWndClass(AfxGetInstanceHandle());

    m_dwWidth  = 256;
    m_dwHeight = 256;
}

CTextureBitmap::~CTextureBitmap()
{
}


BEGIN_MESSAGE_MAP(CTextureBitmap, CWnd)
    //{{AFX_MSG_MAP(CTextureBitmap)
    ON_WM_PAINT()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CTextureBitmap::RegisterWndClass(HINSTANCE hInstance)
{
    WNDCLASS wc;
    wc.lpszClassName = "TEXTURE_BITMAP"; // matches class name in client
    wc.hInstance = hInstance;
    wc.lpfnWndProc = ::DefWindowProc;
    wc.hCursor = ::LoadCursor(NULL, IDC_CROSS);
    wc.hIcon = 0;
    wc.lpszMenuName = NULL;
    wc.hbrBackground = (HBRUSH) ::GetStockObject(NULL_BRUSH);
    wc.style = CS_GLOBALCLASS; // To be modified
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;

    return (::RegisterClass(&wc) != 0);
}

void CTextureBitmap::OnPaint() 
{
    CPaintDC dc(this); // device context for painting
    CRect clientRect;
    GetClientRect(&clientRect);
    
    CBitmap *pOldMemDCBitmap = NULL;    
    CDC memDC;
    CBitmap bitmap;
    memDC.CreateCompatibleDC(&dc);
    bitmap.CreateCompatibleBitmap( &dc, clientRect.Width(), clientRect.Height() );
    CBitmap *pOldmemDCBitmap = (CBitmap*)memDC.SelectObject(&bitmap);
    
    ((CFontMakerMFCDlg*)GetParent())->DrawGlyphs( &memDC );

    dc.BitBlt( 0, 0, 1024, 1024, &memDC, 0, 0, SRCCOPY );
    memDC.SelectObject(pOldmemDCBitmap);
}


void CFontMakerMFCDlg::OnEffects() 
{
	m_bShadow = m_bBorder = FALSE;

	if( ((CButton*)GetDlgItem(IDC_EFFECTS_SHADOW))->GetCheck() )
		m_bShadow = TRUE;

	if( ((CButton*)GetDlgItem(IDC_EFFECTS_BORDER))->GetCheck() )
		m_bBorder = TRUE;

	DrawGlyphs();
    UpdateGlyphList();
}
