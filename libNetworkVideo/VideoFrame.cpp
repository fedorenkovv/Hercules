#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <math.h>
#include <cassert>

#include <new>

#include "VideoFrame.h"



// ========================================================================== //
//
//  Function Name: Constructor
//
//  Abstract: Constructor
//
//  Parameters: NONE
//
//  Return Values:
//        None 
//
// ========================================================================== //

CVideoFrame::CVideoFrame() 
{ 
    m_pData        = NULL;
    m_iWidth       = 0;
    m_iHeight      = 0;
    m_iStrideWidth = 0;
	m_bRGB         = false;
	m_bRGBY = false;
	m_iJpegFrameSize = 0;
}


CVideoFrame::CVideoFrame( const CVideoFrame& Image )
{
    m_pData        = NULL;
    m_iWidth       = 0;
    m_iHeight      = 0;
    m_iStrideWidth = 0;
	m_bRGB         = false;
	m_bRGBY = false;
	
	*this = Image;
}

// ========================================================================== //
//
//  Function Name: Constructor
//
//  Abstract: Constructor
//
//  Parameters:
//        int  iWidth  - image to create pixel width
//        int  iHeight - image to create pixel height 
//        bool bstride - if true create stride width image
//
//  Return Values:
//        None 
//
// ========================================================================== //

CVideoFrame::CVideoFrame( int iWidth, int iHeight, bool bStride /*= true*/ )
{
    m_pData     = NULL;
    m_iWidth    = 0;
    m_iHeight   = 0;

	m_bRGB		= false;
	m_bRGBY = false;

    Create( iWidth, iHeight, bStride );
}

// ========================================================================== //
//
//  Function Name: Constructor
//
//  Abstract: Constructor
//
//  Parameters:
//        unsigned char * pData - pointer to raster array that contains
//                                an 8bpp halftone image
//        int  iWidth           - image to create pixel width
//        int  iHeight          - image to create pixel height 
//        bool bstride          - if true create stride width image
//
//  Return Values:
//        None 
//
// ========================================================================== //

CVideoFrame::CVideoFrame( unsigned char* pData, int iWidth, int iHeight, int iStrideWidth )
{
    Attach( pData, iWidth, iHeight, iStrideWidth );
}


// ========================================================================== //
//
//  Function Name: Destructor
//
//  Abstract: Destructor
//
//  Parameters: None
//
//  Return Values: None 
//
// ========================================================================== //

CVideoFrame::~CVideoFrame( void )
{
    Release();
}


// TODO
int CVideoFrame::PushDataLine( int nLineNum, void* pData, int DataLen )
{
    if( nLineNum < 0 || nLineNum > m_iHeight - 2 )
        return -1;

    if( DataLen < (m_iWidth * 2) + m_iWidth )  // Data length | YCbCr for two lines
        return -1;

    if( nLineNum % 2 )
        return -1;


    unsigned char* pRawData  = (unsigned char*)pData;

    for( int n = 0; n < DataLen; n += 6 )
    {
        unsigned char* pLineData = &m_pData[ m_iStrideWidth * nLineNum + (n/3) ];

        pLineData[ 0                  ] = pRawData[ n + 0 ];
        pLineData[ m_iStrideWidth     ] = pRawData[ n + 1 ];

        pLineData[ 1                  ] = pRawData[ n + 3 ];
        pLineData[ 1 + m_iStrideWidth ] = pRawData[ n + 4 ];
    }

//    memcpy( &m_pData[ m_iStrideWidth * nLineNum ], pData, DataLen );

    return 0;
}


// TODO
int CVideoFrame::PopDataLine( int nLineNum, void* pData, int DataLen )
{
    if( nLineNum < 0 || nLineNum > m_iHeight - 2 )
        return -1;

    if( DataLen < m_iWidth )
        return -1;

    if( nLineNum % 2 )
        return -1;

    if( DataLen < (m_iWidth * 2) + m_iWidth )  // Data length | YCbCr for two lines
        return -1;


    unsigned char* pRawData  = (unsigned char*)pData;

    for( int n = 0; n < DataLen; n += 6 )
    {
        unsigned char* pLineData = &m_pData[ m_iStrideWidth * nLineNum + (n / 3) ];

        pRawData[ n + 0 ] = pLineData[ 0                  ];
        pRawData[ n + 1 ] = pLineData[ m_iStrideWidth     ];
        pRawData[ n + 2 ] = 127;
        pRawData[ n + 3 ] = pLineData[ 1                  ];
        pRawData[ n + 4 ] = pLineData[ 1 + m_iStrideWidth ];
        pRawData[ n + 5 ] = 127;
    }

//        memcpy( pData, &m_pData[ m_iStrideWidth * nLineNum ], DataLen );

    return 0;
}


// ========================================================================== //
//
//  Function Name: Create
//
//  Abstract: Create current instance of image
//
//  Parameters:
//        int  iWidth  - image to create pixel width
//        int  iHeight - image to create pixel height 
//        bool bstride - if true create stride width image
//  Return Values:
//        Always 0 
//
// ========================================================================== //

int CVideoFrame::Create( int iWidth, int iHeight, bool bStride /* = true  */ )
{
    assert( iWidth > 0 && iHeight > 0 );

    if( iWidth <= 0 || iHeight <= 0 )
        return -7;

    Release();

    if( bStride )
        m_iStrideWidth = (iWidth + 3) & ~3;
    else
        m_iStrideWidth = iWidth;

	if ((m_pData = new(std::nothrow) unsigned char[m_iStrideWidth * iHeight]) == NULL)
        return -4;

    m_iWidth  = iWidth;
    m_iHeight = iHeight;

	m_bRGB = false;
	m_bRGBY = false;

    return 0;
}

int CVideoFrame::CreateRGB(int iWidth, int iHeight)
{
	assert(iWidth > 0 && iHeight > 0);

	if (iWidth <= 0 || iHeight <= 0)
		return -7;

	Release();

	m_iStrideWidth = iWidth * 3;

	if ((m_pData = new(std::nothrow) unsigned char[m_iStrideWidth * iHeight]) == NULL)
		return -4;

	m_iWidth = iWidth;
	m_iHeight = iHeight;

	m_bRGB = true;
	m_bRGBY = false;

	return 0;
}

int CVideoFrame::CreateRGBY(int iWidth, int iHeight)
{
	assert(iWidth > 0 && iHeight > 0);

	if (iWidth <= 0 || iHeight <= 0)
		return -7;

	Release();

	m_iStrideWidth = iWidth * 3;

	if ((m_pData = new(std::nothrow) unsigned char[m_iStrideWidth * iHeight + (iWidth * iHeight)]) == NULL)
		return -4;

	m_iWidth = iWidth;
	m_iHeight = iHeight;

	m_bRGB = true;
	m_bRGBY = true;

	return 0;
}


// ========================================================================== //
//
//  Function Name: Release
//
//  Abstract: Release current instance of image
//
//  Parameters: NONE
//
//  Return Values:
//        Always 0 
//
// ========================================================================== //

int CVideoFrame::Release( void )
{
    if( m_pData )
    {
		delete []m_pData;
	}

    m_pData        = NULL;
    m_iWidth       = 0;
    m_iHeight      = 0;
    m_iStrideWidth = 0;
	m_bRGB         = false;
	m_bRGBY = false;

    return 0;
}

// ========================================================================== //
//
//  Function Name: Attach
//
//  Abstract: Attach an existing image to currnet instance
//
//  Parameters:
//        CVideoFrame - Pointer to image for attaching
//
//  Return Values:
//        Always 0 
//
// ========================================================================== //

int CVideoFrame::Attach( unsigned char* pData,
                       int iWidth, 
                       int iHeight, 
                       int iStrideWidth )
{
    m_pData   = pData;
    m_iWidth  = iWidth;
    m_iHeight = iHeight;
    m_iStrideWidth = iStrideWidth;

    return 0;
}

// ========================================================================== //
//
//  Function Name: GetItem
//
//  Abstract: Return value of image pixel
//
//  Parameters:
//        int x - X coordinate of pixel
//        int y - Y coordinate of pixel
//
//  Return Values:
//        - int - Value in point
//
// ========================================================================== //

#ifdef _DEBUG // Otherwise placed by inline function CVideoFrame::GetItem
unsigned char CVideoFrame::GetItem( int x, int y )
{
    assert( x >= 0 && x < m_iWidth );
    assert( y >= 0 && y < m_iHeight );

    return m_pData[ x + y * m_iStrideWidth ];
}
#endif // _DEBUG


// ========================================================================== //
//
//  Function Name: SetItem
//
//  Abstract: Set pixel value
//
//  Parameters:
//        int x               - X coordinate of pixel
//        int y               - Y coordinate of pixel
//        unsigned char btVal - Value for set
//
//  Return Values:
//        None
//
// ========================================================================== //

#ifdef _DEBUG // Otherwise placed by inline function CVideoFrame::SetItem() 
void CVideoFrame::SetItem( int x, int y, unsigned char btVal )
{
    assert( x >= 0 && x < m_iWidth );
    assert( y >= 0 && y < m_iHeight );

    m_pData[ x + y * m_iStrideWidth ] = btVal;
}
#endif // _DEBUG


// ========================================================================== //
//
//  Function Name: Fill
//
//  Abstract: Fill image pixels with given value
//
//  Parameters: 
//    unsigned char btValue - Pixel value to fill image
//
//  Return Values: NONE
//
// ========================================================================== //

void CVideoFrame::Fill( unsigned char btVal )
{
    unsigned long ulSize = m_iHeight * m_iStrideWidth;

    for( unsigned long ul = 0; ul < ulSize; ul++ )
    {
        m_pData[ ul ] = btVal;
    }

    //for( int y = 0; y < m_iHeight; ++y )
    //{
    //    for( int x = 0; x < m_iWidth; ++x )
    //    {
    //        SetItem( x, y, btVal );
    //    }
    //}
}


//
bool CVideoFrame::IsEmpty()
{
    if( m_pData == NULL || m_iWidth == 0 || m_iHeight == 0 || m_iStrideWidth == 0 )
        return true;
    else
        return false;
}


// ========================================================================== //
//
//  Function Name: IsIncluded
//
//  Abstract: Determines whether the specified coordinates 
//            is included into the image
//
//  Parameters: tetragon coordinates 
//
//  Return Values: 
//            true  - if included,
//            false - otherwise.
//
// ========================================================================== //

bool CVideoFrame::IsIncluded( int x1 /*= 0*/, int y1 /*= 0*/,
                            int x2 /*= 0*/, int y2 /*= 0*/,
                            int x3 /*= 0*/, int y3 /*= 0*/,
                            int x4 /*= 0*/, int y4 /*= 0*/ )
{
    if( x1 < 0 || x1 >= m_iWidth ||
        x2 < 0 || x2 >= m_iWidth ||
        x3 < 0 || x3 >= m_iWidth ||
        x4 < 0 || x4 >= m_iWidth )
    {
        return false;
    }

    if( y1 < 0 || y1 >= m_iHeight ||
        y2 < 0 || y2 >= m_iHeight ||
        y3 < 0 || y3 >= m_iHeight ||
        y4 < 0 || y4 >= m_iHeight )
    {
        return false;
    }

    return true;
}


// ========================================================================== //
//
//  Function Name: FillRect
//
//  Abstract: Fill rectangle area with given value
//
//  Parameters:
//        int iLeft   - Left coordinate of rectangle
//        int iTop    - Top coordinate of rectangle
//        int iRight  - Right coordinate of rectangle
//        int iBottom - Bottom coordinate of rectangle
//        unsigned char btVal - Value for fill
//
//  Return Values:
//        - int - always 0
//
// ========================================================================== //

int CVideoFrame::FillRect( int iLeft,  int iTop,
                         int iRight, int iBottom, 
                         unsigned char btVal )
{
    assert( iLeft < iRight );
    assert( iTop < iBottom );

    for( int y = iTop; y < iBottom; y++ )
    {
        for( int x = iLeft; x < iRight; x++ )
        {
            SetItem( x, y, btVal );
        }
    }

    return 0;
}


// ========================================================================== //
//
//  Function Name: PutRect
//
//  Abstract: Put rectangular subimage into the image
//
//  Parameters:
//        int iPosX   - 
//        int iPosY   - 
//
//  Return Values:
//        - int - always 0
//
// ========================================================================== //

int CVideoFrame::PutRect( int iPosX, int iPosY,
                        CVideoFrame& Source, 
                        int iLeft, int iTop, int iRight, int iBottom )
{
    assert( iLeft < iRight );
    assert( iTop < iBottom );

    for( int y = iTop, y1 = iPosY; y < iBottom; y++, y1++ )
    {
        for( int x = iLeft, x1 = iPosX; x < iRight; x++, x1++ )
        {
            SetItem( x1, y1, Source.GetItem( x, y ) );
        }
    }

    return 0;
}


// ========================================================================== //
//
//  Function Name: CutRect
//
//  Abstract: Cut rectangular subimage from the image
//
//  Parameters:
//        CVideoFrame* pDstImage
//        int iLeft    - 
//        int iTop     -
//        int iRight   -
//        int iBottom  -
//
//  Return Values:
//        - int - always 0
//
// ========================================================================== //

int CVideoFrame::CutRect( CVideoFrame* pDstImage, int iLeft, int iTop, int iRight, int iBottom )
{
//    assert( iLeft >= 0 && iTop >= 0 && iRight < m_iWidth && iBottom < m_iHeight );

    if( iLeft < 0 ) iLeft = 0;
    if( iTop  < 0 ) iTop = 0;

    if( iRight  >= m_iWidth  ) iRight = m_iWidth - 1;
    if( iBottom >= m_iHeight ) iBottom = m_iHeight - 1;

    int nResult = pDstImage->Create( iRight - iLeft, iBottom - iTop );
    if( nResult < 0 )
        return nResult;


    for( int y = iTop; y < iBottom; y++ )
    {
        for( int x = iLeft; x < iRight; x++ )
        {
            pDstImage->SetItem( x - iLeft, y - iTop, GetItem( x, y ));
        }
    }

    return 0;
}

// ========================================================================== //
//
//  Function Name: Inverse
//
//  Abstract: Inverse image using bitwise inversing
//
//  Parameters: None
//
//  Return Values: None
//
// ========================================================================== //

void CVideoFrame::Inverse()
{
    unsigned long ulImgSize = m_iHeight * m_iStrideWidth;

    for( unsigned long ul = 0; ul < ulImgSize; ul++ )
        m_pData[ ul ] = ~m_pData[ ul ];
}


// ========================================================================== //
//
//  Function Name: Rotate90CW
//
//  Abstract: Rotate image on 90 degrees clockwise
//
//  Parameters: None
//
//  Return Values:
//         - int - 0 (0) if function was successfully executed
//         - (-4) if function was failure
//
// ========================================================================== //

int CVideoFrame::Rotate90CW()
{
    int iWidth  = m_iHeight;
    int iHeight = m_iWidth;

    CVideoFrame NewImage;
    if ( NewImage.Create( iWidth, iHeight, true ) != 0 )
        return -4;

    for( int y = 0; y < iHeight; y++ )
    {
        for( int x = 0; x < iWidth; x++ )
        {
            NewImage.SetItem( x, y, GetItem( y, m_iHeight - x - 1 ) );
        }
    }

    *this = NewImage;

    return 0;
}


// ========================================================================== //
//
//  Function Name: Rotate180CW
//
//  Abstract: Rotate image on 180 degrees clockwise
//
//  Parameters:
//        None
//
//  Return Values:
//         - int - 0 (0) if function was successfully executed
//         - (-4) if function was failure
//
// ========================================================================== //

int CVideoFrame::Rotate180CW()
{
    CVideoFrame NewImage;
    if ( NewImage.Create( m_iWidth, m_iHeight, true ) != 0 )
        return -4;

    for( int y = 0; y < m_iHeight; y++ )
    {
        for( int x = 0; x < m_iWidth; x++ )
        {
            NewImage.SetItem( x, y, GetItem( m_iWidth - x - 1, m_iHeight - y - 1 ) );
        }
    }

    *this = NewImage;

    return 0;
}

// ========================================================================== //
//
//  Function Name: Rotate270CW
//
//  Abstract: Rotate image on 270 degrees clockwise
//
//  Parameters: None
//
//  Return Values:
//         - int - 0 (0) if function was successfully executed
//         - (-4) if function was failure
//
// ========================================================================== //

int CVideoFrame::Rotate270CW()
{
    int iWidth  = m_iHeight;
    int iHeight = m_iWidth;

    CVideoFrame NewImage;
    if ( NewImage.Create( iWidth, iHeight, true ) != 0 )
        return -4;

    for( int y = 0; y < iHeight; y++ )
    {
        for( int x = 0; x < iWidth; x++ )
        {
            NewImage.SetItem( x, y, GetItem( m_iWidth - y - 1, x ) );
        }
    }

    *this = NewImage;

    return 0;
}

// ========================================================================== //
//
//  Function Name: Rotate90CCW
//
//  Abstract: Rotate image on 90 degrees counter-clockwise
//
//  Parameters: None
//
//  Return Values:
//         - int - 0 (0) if function was successfully executed
//         - (-4) if function was failure
//
// ========================================================================== //

int CVideoFrame::Rotate90CCW()
{
    return Rotate270CW();
}

// ========================================================================== //
//
//  Function Name: Rotate180CCW
//
//  Abstract: Rotate image on 180 degrees counter-clockwise
//
//  Parameters: None
//
//  Return Values:
//         - int - 0 (0) if function was successfully executed
//         - (-4) if function was failure
//
// ========================================================================== //

int CVideoFrame::Rotate180CCW()
{
    return Rotate180CW();
}

// ========================================================================== //
//
//  Function Name: Rotate270CCW
//
//  Abstract: Rotate image on 270 degrees counter-clockwise
//
// Parameters: None
//
//  Return Values:
//         - int - 0 (0) if function was successfully executed
//         - (-4) if function was failure
//
// ========================================================================== //

int CVideoFrame::Rotate270CCW()
{
    return Rotate90CW();
}

// ========================================================================== //
//
//  Function Name: operator=
//
//  Abstract: Overloading of the operator=, 
//            copy image into the CVideoFrame instance from another instance
//
//  Parameters: CVideoFrame& SrcImage - Source image
//
//  Return Values:
//         - CVideoFrame& - Reference to this image
//
// ========================================================================== //

CVideoFrame& CVideoFrame::operator= (const CVideoFrame& SrcImage )
{
    if( this == &SrcImage )
        return *this;
    
    if( m_pData )
        delete []m_pData;

    if( SrcImage.m_iWidth == 0 || SrcImage.m_iHeight == 0 || SrcImage.m_iStrideWidth == 0 )
    {
        Release();

        return *this;
    }

    m_pData = new unsigned char[ SrcImage.m_iHeight * SrcImage.m_iStrideWidth ];

    if( m_pData == NULL )
        return *this;

    memcpy( m_pData, SrcImage.m_pData, SrcImage.m_iHeight * SrcImage.m_iStrideWidth );

    m_iWidth       = SrcImage.m_iWidth;
    m_iHeight      = SrcImage.m_iHeight;
    m_iStrideWidth = SrcImage.m_iStrideWidth;

	m_bRGB = SrcImage.m_bRGB;
	m_bRGBY = SrcImage.m_bRGBY;

    nID            = SrcImage.nID;
    VegaInfo       = SrcImage.VegaInfo;       // TODO
    FlightData     = SrcImage.FlightData;     // TODO
    NavData        = SrcImage.NavData;        // TODO

    return *this;
}

// ========================================================================== //
//
// Function Name: FlipVertical
//
//  Abstract: Flip image vertical
//
//  Parameters:
//        None
//
//  Return Values:
//        0
//
// ========================================================================== //

int CVideoFrame::FlipVertical( void )
{
    unsigned char* pRowDataPtr = new unsigned char[ m_iStrideWidth ];

    for( int nRow = 0; nRow < m_iHeight / 2; nRow++ )
    {
        // Copying from row to buffer
        memcpy( pRowDataPtr, 
                &m_pData[ nRow * m_iStrideWidth ], 
                m_iStrideWidth );

        // Copying from row to row
        memcpy( &m_pData[ nRow * m_iStrideWidth ], 
                &m_pData[ ( m_iHeight - nRow - 1 ) * m_iStrideWidth ], 
                m_iStrideWidth );

        // Copying from buffer to row
        memcpy( &m_pData[ ( m_iHeight - nRow - 1 ) * m_iStrideWidth ], 
                pRowDataPtr, 
                m_iStrideWidth );
    }

    delete []pRowDataPtr;

    return 0;
}


// ========================================================================== //
//
// Function Name: FillTriangle
//
//  Abstract: Fill triangle with given value
//
//  Parameters:
//        int Ax, int Ay - Triangle vertex coordinates
//        int Bx, int By - Triangle vertex coordinates
//        int Cx, int Cy - Triangle vertex coordinates
//        unsigned char btFill - Fill value (default: 127)
//
//  Return Values:
//        - 0 (0) if function was successfully executed
//        -  value < 0 if error occurred (see Common.h)
//
// ========================================================================== //

int CVideoFrame::FillTriangle( int Ax, int Ay,
                             int Bx, int By,
                             int Cx, int Cy,
                             unsigned char btFill /* = 127  */ )
{
    // Checking for validity
    if( m_iWidth <= 0 || m_iHeight <= 0 || m_pData == NULL ) 
        return -1;

    assert( (Ax >= 0 || Ax < m_iWidth)  && (Bx >= 0 || Bx < m_iWidth)  && (Cx >= 0 || Cx < m_iWidth) );
    assert( (Ay >= 0 || Ay < m_iHeight) && (By >= 0 || By < m_iHeight) && (Cy >= 0 || Cx < m_iHeight) );

    // Sorting of triangle vertexes by Y-coordinate.
    // The filling algorithm is require determined order of vertexes.

    int x, y;
    if( Ay > By ) 
    {
        x  = Ax; y  = Ay;
        Ay = By; Ax = Bx;
        Bx = x ; By = y ;
    }

    if( Ay > Cy ) 
    {
        x  = Ax; y  = Ay;
        Ay = Cy; Ax = Cx;
        Cx = x ; Cy = y ;
    }

    if( By > Cy ) 
    {
        x  = Bx; y  = By;
        By = Cy; Bx = Cx;
        Cx = x ; Cy = y ;
    }

    // Offsets calсulation
    float dxAB = (float)( Bx - Ax ) / (float)( By - Ay );
    float dxAC = (float)( Cx - Ax ) / (float)( Cy - Ay );
    float dxBC = (float)( Cx - Bx ) / (float)( Cy - By );

    // Filling of high right-angled triangle

    for( int Y = Ay; Y < By; Y++ )
    {
        if( Y < 0 )
            continue;

        if( Y >= m_iHeight )
            break;

        int x1 = Ax + (int)( dxAB * ( Y - Ay ) );
        int x2 = Ax + (int)( dxAC * ( Y - Ay ) );

        if( x1 > x2 )
        {
            int t = x1;
            x1 = x2;
            x2 = t;
        }

        if( x1 < 0 ) x1 = 0;
        if( x1 >= m_iWidth ) x1 = m_iWidth - 1;

        if( x2 < 0 ) x2 = 0;
        if( x2 >= m_iWidth ) x2 = m_iWidth - 1;

        for( x = x1; x < x2; ++x )
            SetItem( x, Y, btFill );
    }

    // Filling of low right-angled triangle

    int xAC = Ax + (int)( dxAC * (By - Ay) );

    for( int Y = By; Y < Cy; ++Y )
    {
        if( Y < 0 )
            continue;

        if( Y >= m_iHeight )
            break;

        int x1 = Bx + (int)( dxBC * ( Y - By ) );
        int x2 = xAC + (int)( dxAC * ( Y - By ) );

        if( x1 > x2 )
        {
            int t = x1;
            x1 = x2;
            x2 = t;
        }

        if( x1 < 0 ) x1 = 0;
        if( x1 >= m_iWidth ) x1 = m_iWidth - 1;

        if( x2 < 0 ) x2 = 0;
        if( x2 >= m_iWidth ) x2 = m_iWidth - 1;

        for( x = x1; x < x2; ++x )
            SetItem( x, Y, btFill );
    }

    return 0;
}

// ========================================================================== //
//
//  Function Name: FillTrapezoid
//
//  Abstract: Fill trapezoid with given value
//
//  Parameters:
//        int x1, int y1 - Trapezoid vertex coordinates
//        int x2, int y2 - Trapezoid vertex coordinates
//        int x3, int y3 - Trapezoid vertex coordinates
//        int x4, int y4 - Trapezoid vertex coordinates
//        unsigned char btFill - Fill value (default: 0)
//
//  Return Values:
//        - 0 (0) if function was successfully executed
//        -  value < 0 if error occurred (see Common.h)
//
// ========================================================================== //

int CVideoFrame::FillTrapezoid( int x1, int y1,
                              int x2, int y2,
                              int x3, int y3,
                              int x4, int y4,
                              unsigned char btFill /*= 0*/ )
{
    if( m_iWidth <= 0 || m_iHeight <= 0 || m_pData == NULL ) 
        return -1;          // Checking for validity

    FillTriangle( x1, y1, x3, y3, x4, y4, btFill );
    FillTriangle( x1, y1, x2, y2, x3, y3, btFill );

    return 0;
}


// ========================================================================== //
//
//  Function Name: GaussianBlur
//
//  Abstract: Cutting this image into a given image or on-self 
//            (if pImgPtr == NULL)
//
//  Parameters:
//        CVideoFrame* pBlurImage - Pointer to valid CVideoFrame instance,
//                                it pointer has been used for result storing 
//        int iLeft             - Left coordinate of rectangle
//        int iTop              - Top coordinate of rectangle
//        int iRight            - Right coordinate of rectangle
//        int iBottom           - Bottom coordinate of rectangle
//
//  Return Values:
//        - 0 (0) if function was successfully executed
//        -  value < 0 if error occurred (see Common.h)
//
// ========================================================================== //

int CVideoFrame::GaussianBlur( CVideoFrame* pBlurImage,
                             int iLeft,  int iTop,
                             int iRight, int iBottom,
                             int iMaskSize, int* pMask )
{
    assert( iLeft < iRight );
    assert( iTop < iBottom );

    if( !(iMaskSize % 2) )
        return -7;

    int iHalfMaskSize = iMaskSize / 2;

    int iMaskNorm = 0;
    for( int n = 0; n < iMaskSize; n++ )
        iMaskNorm += pMask[ n ];


    if( iLeft   <   iMaskSize ) iLeft = iMaskSize;
    if( iRight  >= m_iWidth - iMaskSize ) iRight = m_iWidth - iMaskSize;
    if( iTop    <   iMaskSize ) iTop  = iHalfMaskSize * 2;
    if( iBottom >= m_iHeight - iMaskSize ) iBottom = m_iHeight - iMaskSize;

    int iRectWidth  = iRight  - iLeft;
    int iRectHeight = iBottom - iTop;


    CVideoFrame ImgGaussBuff;

    int iResult = ImgGaussBuff.Create( iRectWidth, iRectHeight + iHalfMaskSize * 2 );
    if( iResult < 0 )
        return iResult;

    for( int x = iLeft; x < iRight; x++ )
    {
        for( int y = iTop - iHalfMaskSize; y < iBottom + iHalfMaskSize; y++ )
        {
            int iValue = 0;
            int iIndex = 0;
            for( int iMask = -iHalfMaskSize; iMask <= iHalfMaskSize; iMask++ )
            {
                iValue += GetItem( x + iMask, y ) * pMask [ iIndex ++ ];
            }

            ImgGaussBuff.SetItem( x - iLeft, y - iTop + iHalfMaskSize, iValue / iMaskNorm );
        }
    }

    iResult = pBlurImage->Create( iRectWidth, iRectHeight );
	if( iResult < 0 )
		return iResult;

    for( int x = 0; x < iRectWidth; x++ )
    {
        for( int y = 0; y < iRectHeight; y++ )
        {
            int iValue = 0;
            int iIndex = 0;
            for( int iMask = -iHalfMaskSize; iMask <= iHalfMaskSize; iMask++ )
            {
                iValue += ImgGaussBuff.GetItem( x, y + iHalfMaskSize + iMask ) * pMask [ iIndex ++ ];
            }

            pBlurImage->SetItem( x, y, iValue / iMaskNorm );
        }
    }

    return 0;
}


// ========================================================================== //
//
//  Function Name: RescaleBilinear
//
//  Abstract: Rescale current image by use of bilinear interpolation
//
//  Parameters:
//        CVideoFrame& newImage  - Reference to valid CVideoFrame instance 
//                               for result storing
//        double     dRatio    - coefficient of image enlargеment or reduction
//
//  Return Values:
//        - 0 (0) if function was successfully executed
//        -  value < 0 if error occurred (see Common.h)
//
// ========================================================================== //

int CVideoFrame::RescaleBilinear ( 
                                 CVideoFrame& newImage,
                                 double     dRatio
                               )
{
      int     nRes ; 
      int     nWidth , nHeight;
      int     i , j , x0 , y0 , x1 , y1 ;
      double  Ixy , Ix0y0 , Ix1y0 , Ix0y1 , Ix1y1 ;
      double  dXStep , dYStep ;
      double  x , y , dx , dy ;
      
      if ( dRatio <= 0.0 )
          return -7;

      nWidth = (int)(dRatio * (double) m_iWidth + 0.5) ;
      nHeight = (int)(dRatio * (double) m_iHeight + 0.5) ;

      if ( nWidth <= 4 || nHeight <= 4 || m_iWidth == 0 || m_iHeight == 0 )
          return -7;

      if( ( nRes = newImage.Create( nWidth, nHeight ) ) < 0 )
          return nRes ;

      dXStep = (double) m_iWidth / (double) nWidth ;
      dYStep = (double) m_iHeight / (double) nHeight ;

      Ixy = 128;
  
      for ( i = 0 , y = 0.0 ; i < nHeight ; i++ , y += dYStep )
      {
         for ( j = 0 , x = 0.0 ; j < nWidth ; j++ , x += dXStep )
         {

             x0 = (int) x;
             y0 = (int) y;

             x1 = x0 + 1;
             y1 = y0 + 1;

             if ( x0 < 0 || x1 >= m_iWidth || y0 < 0 || y1 >= m_iHeight )  
             {      
                 if ( Ixy > 255.0 )
                     newImage.SetItem( j, i, 255 );
                 else
                     newImage.SetItem( j, i, (unsigned char)( Ixy + 0.5 ) );

                 continue;
             }

             dx = x - (double)x0;
             dy = y - (double)y0;
             
             Ix0y0 = (double) GetItem( x0    , y0     );
             Ix1y0 = (double) GetItem( x0 + 1, y0     );
             Ix0y1 = (double) GetItem( x0    , y0 + 1 );
             Ix1y1 = (double) GetItem( x0 + 1, y0 + 1 );

             Ixy  =    (1.0 - dx) * (1.0 - dy) * Ix0y0
                     + (      dx) * (1.0 - dy) * Ix1y0
                     + (1.0 - dx) * (      dy) * Ix0y1
                     + (      dx) * (      dy) * Ix1y1;

             if ( Ixy > 255.0 )
                  newImage.SetItem( j, i, 255 );
             else
                  newImage.SetItem( j, i, (unsigned char)( Ixy + 0.5 ) );
        } 
    } 

    return 0;
}

/*****************************************************************************

Function Name: SafeRescaleBilinear

Abstract: Rescale current image using bilinear interpolation with safety
          borders processing

Parameters:
    CVideoFrame& newImage  - Link to valid CVideoFrame instance for result storing
    double  dRatio    - coefficient of image enlargеment or reduction
    bool    bSkipBrds - Do not process border's pixels. It will be 
                        filled by a forecast color
Return Values:
    - 0 (0) if function was successfully executed
    -  value < 0 if error occurred (see Common.h)

*****************************************************************************/
int CVideoFrame::SafeRescaleBilinear ( 
                                    CVideoFrame&    newImage,
                                    double        dRatio,
                                    unsigned char bgColor
                                    )
{
    int     nRes ; 
    int     nWidth , nHeight;
    int     i , j , x0 , y0 , x1 , y1 ;
    double  Ixy , Ix0y0 , Ix1y0 , Ix0y1 , Ix1y1 ;
    double  dXStep , dYStep ;
    double  x , y , dx , dy ;


    if ( dRatio <= 0.0 )
        return -7;


    nWidth = (int)(dRatio * (double) m_iWidth + 0.5) ;
    nHeight = (int)(dRatio * (double) m_iHeight + 0.5) ;

    if ( /*nWidth <= 4 || nHeight <= 4 ||*/ m_iWidth == 0 || m_iHeight == 0 )
        return -7;


    if( ( nRes = newImage.Create( nWidth, nHeight ) ) < 0 )
        return nRes ;

    dXStep = (double) m_iWidth / (double) nWidth ;
    dYStep = (double) m_iHeight / (double) nHeight ;

    Ixy = 128 ;

    for ( i = 0 , y = 0.0 ; i < nHeight ; i++ , y += dYStep )
    {
        for ( j = 0 , x = 0.0 ; j < nWidth ; j++ , x += dXStep )
        {
            x0 = (int) x ; x1 = x0 + 1 ;
            y0 = (int) y ; y1 = y0 + 1 ;

            dx = x - (double) x0 ;
            dy = y - (double) y0 ;

            if( x0     >= m_iWidth || y0     >= m_iHeight ) Ix0y0 = bgColor; else Ix0y0 = (double) GetItem( x0    , y0     );
            if( x0 + 1 >= m_iWidth || y0     >= m_iHeight ) Ix1y0 = bgColor; else Ix1y0 = (double) GetItem( x0 + 1, y0     );
            if( x0     >= m_iWidth || y0 + 1 >= m_iHeight ) Ix0y1 = bgColor; else Ix0y1 = (double) GetItem( x0    , y0 + 1 );
            if( x0 + 1 >= m_iWidth || y0 + 1 >= m_iHeight ) Ix1y1 = bgColor; else Ix1y1 = (double) GetItem( x0 + 1, y0 + 1 );

            Ixy  =    (1.0 - dx) * (1.0 - dy) * Ix0y0
                +        dx  * (1.0 - dy) * Ix1y0
                + (1.0 - dx) *        dy  * Ix0y1
                +        dx  *        dy  * Ix1y1 ;

            if ( Ixy > 255.0 )
                newImage.SetItem( j, i, 255 );
            else
                newImage.SetItem( j, i, (unsigned char)( Ixy + 0.5 ) );

        } 
    } 


    return 0;
}

// ========================================================================== //
//
//  Function Name: RescaleNearestNeighbour
//
//  Abstract: Rescale current image using nearest-neighbour pixel interpolation
//
//  Parameters:
//        CVideoFrame& newImage  - Reference to valid CVideoFrame instance  
//                               for result storing
//        double     dRatio    - coefficient of image enlargеment or reduction
//
//  Return Values:
//        - 0 (0) if function was successfully executed
//        -  value < 0 if error occurred (see Common.h)
//
// ========================================================================== //

int CVideoFrame::RescaleNearestNeighbour ( 
                                         CVideoFrame& newImage,
                                         double     dRatio
                                        )
{
    if ( m_iWidth <= 0 || m_iHeight <= 0 || dRatio <= 0.0 )
        return -1;

    // Вычитание единицы неодбодимо для корректного насчета шага.
    // При этом нумерация элементов в матрице изображения какбы начинается не с 0, а с 1.
    // iNewHeight и iNewWidth равняются не количеству элементов, а последним координатам.
    int iNewHeight = (int)( ( ( m_iHeight - 1 ) * dRatio ) + 0.5 );
    int iNewWidth = (int)( ( ( m_iWidth - 1 ) * dRatio ) + 0.5 );

    //см. первый комментарий
    int nRes = newImage.Create( iNewWidth + 1, iNewHeight + 1 );
    if ( nRes < 0 )
        return nRes;

    //см. первый комментарий
    double dStepX = (double)( m_iWidth - 1 ) / (double)iNewWidth;
    double dStepY = (double)( m_iHeight - 1 ) / (double)iNewHeight;

    double dYcoord = 0;
    for( int iYDest = 0; iYDest <= iNewHeight; iYDest++ )
    {        
        double dXCoord = 0;
        int iYSource = (int)( dYcoord + 0.5 );
        for( int iXDest = 0; iXDest <= iNewWidth; iXDest++ )
        {
            int iXSource = (int)( dXCoord + 0.5 );
            newImage.SetItem( iXDest, iYDest, GetItem( iXSource, iYSource ) );
            dXCoord += dStepX;
        }
        dYcoord += dStepY;    
    }

    return 0;
}

// ========================================================================== //
//
//  Function Name: Binarize
//
//  Abstract: Binarize the grayscale image contained in this class
//            instance by use of given threshold
//
//  Parameters:
//        unsigned char iThreshold  - threshold for image binarization  
//
//  Return Values:
//        - 0 (0) if function was successfully executed
//        -  value < 0 if error occurred (see Common.h)
//
// ========================================================================== //

int CVideoFrame::Binarize( unsigned char iThreshold )
{
      for (int y = 0; y < m_iHeight; y++)
      {
         for (int x = 0; x < m_iWidth; x++)
         {
             if( GetItem( x, y ) >= iThreshold )
                 SetItem( x, y, 255 );
             else
                 SetItem( x , y, 0 );
        } 
    } 

    return 0;
}


// ========================================================================== //
//
//  Function Name: MakeStride
//
//  Abstract: Make current image stridden
//
//  Parameters: None
//
//  Return Values: - 0 always
//
// ========================================================================== //

int CVideoFrame::MakeStride()
{
    int iStrideWidth = ( m_iWidth + 3) & ~3;

    if( iStrideWidth == m_iWidth )
        return 0;

    CVideoFrame NewImage;
    if( NewImage.Create( m_iWidth, m_iHeight, true ) != 0 )
        return -4;

    for( int y = 0; y < m_iHeight; y++ )
    {
        for( int x = 0; x < m_iWidth; x++ )
        {
            NewImage.SetItem( x, y, GetItem( x, y ) );
        }
    }
  
    *this = NewImage;

    return 0;
}


// ========================================================================== //
//
//  Function Name: MakeUnStride
//
//  Abstract: Make current image unstridden
//
//  Parameters: None
//
//  Return Values: - 0 always
//
// ========================================================================== //

int CVideoFrame::MakeUnStride()
{
    CVideoFrame NewImage;
    if( NewImage.Create( m_iWidth, m_iHeight, false ) != 0 )
        return -4;

    for( int y = 0; y < m_iHeight; y++ )
    {
        for( int x = 0; x < m_iWidth; x++ )
        {
            NewImage.SetItem( x, y, GetItem( x, y ) );           
        }
    } 

    *this = NewImage;

    return 0;
}




//////////////////////////////////////////////////////////////////////////////
//
//  Copy (with some changes) from wingdi.h
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _WINDOWS_ // Defined in <Windows.h>

typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef long           LONG;
typedef unsigned char  BYTE;

// Unsupported GCC compiler feature (for default target) under NetBSD
#pragma pack( push, 2 )

typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;

// Unsupported GCC compiler feature (for default target) under NetBSD
#pragma pack( pop )

#endif // _WINDOWS_

// ========================================================================== //
//
//  Function Name: LoadBitMap
//
//  Abstract: Load (8bpp) BMP grayscale image in to class instance
//
//  Parameters:
//        const char* szPathName - File name (and path) for loading of given file 
//                                into current CVideoFrame instance
//
//  Return Values:
//        - 0 (0) if function was successfully executed
//        -  value < 0 if error occurred (see Common.h)
//
// ========================================================================== //
int CVideoFrame::LoadBitMap( const char* szMaskName, ... )
{
    va_list params;
    va_start( params, szMaskName );

    char szPathName[ 1024 ];
    vsprintf( szPathName, szMaskName, params );

    // Open a file for binary reading
    FILE* hFile = fopen ( szPathName , "rb" ) ;
    if ( hFile == NULL )
        return -1;

    // Release previously allocated memory 
    if( m_pData )
        delete []m_pData;

    BITMAPFILEHEADER bmfh;
    // Read BMP file header
    if( fread( &bmfh, 1, sizeof( bmfh ), hFile ) != sizeof( BITMAPFILEHEADER ) )
        return -2;

    // Check file type 'BM' [0x4D42]
    if( bmfh.bfType != 0x4D42 )
        return -1;

    struct
    {
        BITMAPINFOHEADER  bmih;
        RGBQUAD           rgbqPalette[256];
    } bmInfo;

    // Read BMP information header
    if( fread( &bmInfo.bmih, sizeof( BITMAPINFOHEADER ), 1, hFile ) == 0 )
        return -2;

    // Support only 8bpp images
    if( bmInfo.bmih.biBitCount != 8 )
        return -1;

    m_iWidth = m_iStrideWidth = (int)bmInfo.bmih.biWidth;
    m_iHeight = (int)bmInfo.bmih.biHeight;

    // Calculate stridden width
    if( bmInfo.bmih.biWidth * bmInfo.bmih.biHeight < (long)( bmfh.bfSize - bmfh.bfOffBits ) )
    {
        for ( ; ( m_iStrideWidth & 0x3 ); )
            m_iStrideWidth++;
    }

    // Read a palette
    if( fread( &bmInfo.rgbqPalette, sizeof( RGBQUAD ), 256, hFile ) == 0 )
        return -2;

    // Allocate memory for bitmap
    if( !( m_pData = new unsigned char[ m_iStrideWidth * m_iHeight ] ) )
        return -4;

    // Read bitmap to allocated memory
    if( fread( m_pData, 1, m_iStrideWidth * m_iHeight, hFile ) == 0 )
        return -2;

    fclose ( hFile );

    // Making of halftone palette
    unsigned char btTones[ 256 ];
    for( int i = 0; i < 256; ++i )
    {
        btTones[ i ] = ( bmInfo.rgbqPalette[ i ].rgbRed   + 
                         bmInfo.rgbqPalette[ i ].rgbGreen + 
                         bmInfo.rgbqPalette[ i ].rgbBlue  ) / 3;
    }

    unsigned long ulPixelsCnt = m_iStrideWidth * m_iHeight;

    for( unsigned long ul = 0; ul < ulPixelsCnt; ++ul )
        m_pData[ ul ] = btTones[ m_pData[ ul ] ];

    // Flip image for creation of standard rows order
    FlipVertical();

    return 0;
}


// ========================================================================== //
//
//  Function Name: SaveBitMap
//
//  Abstract: Save (8bpp) BMP grayscale image from the class instance
//
//  Parameters:
//        const char* szPathName - File name (and path) for droping 
//                                 of current image into given file
//
//  Return Values:
//        - 0 (0) if function was successfully executed
//        -  value < 0 if error occurred (see Common.h)
//
// ========================================================================== //

int CVideoFrame::SaveBitMap( const char* szMaskName, ... )
{
    if( IsEmpty() )
        return -1;

    va_list  params;
    va_start ( params, szMaskName );

    char szPathName[ 1024 ];
    vsprintf( szPathName, szMaskName, params );

    // Open a file for binary writing
    FILE* hFile = fopen( szPathName , "wb" ) ;
    if( hFile == NULL )
        return -1;

    // Prepare BMP file header
    BITMAPFILEHEADER bmfh =
    {
        'B' | ( 'M' << 8 ),                   // bfType
        m_iStrideWidth * m_iHeight + 1078UL,  // bfSize
        0,                                    // bfReserved1
        0,                                    // bfReserved2
        1078UL                                // bfOffBits
    };

    // Write BMP file header
    if( fwrite( &bmfh, sizeof( bmfh ), 1, hFile ) == 0 )
        return -3;

    // Prepare BMP information file header
    BITMAPINFOHEADER bmih = 
    {
        40,        // biSize
        m_iWidth,  // biWidth
        m_iHeight, // biHeight
        1,         // biPlanes
        8,         // biBitCount
        0,         // biCompression
        0,         // biSizeImage
        0,         // biXPelsPerMeter
        0,         // biYPelsPerMeter
        0,         // biClrUsed
        0          // biClrImportant
    };

    // Write BMP information file header
    if( fwrite( &bmih, sizeof( BITMAPINFOHEADER ), 1, hFile ) == 0 )
        return -3;

    // Prepare halftone palette
    RGBQUAD Palette[ 256 ];
    for( int i = 0; i < 256; ++i )
    {
        Palette[ i ].rgbRed      = i;
        Palette[ i ].rgbGreen    = i;
        Palette[ i ].rgbBlue     = i;
        Palette[ i ].rgbReserved = 0;
    }

    // Write palette to BMP file
    if( fwrite ( Palette, sizeof( RGBQUAD ), 256, hFile ) == 0 )
        return -3;


    // Flip image for creation of reversed rows order
    FlipVertical();

    // Write bitmap data to BMP file
    if( fwrite( m_pData, m_iStrideWidth * m_iHeight, 1, hFile ) == 0 )
        return -3;

    // Restore standard rows order
    FlipVertical();

    fclose ( hFile ) ;

    return 0;
}
