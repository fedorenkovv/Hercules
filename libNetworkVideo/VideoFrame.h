#ifndef __VIDEO_FRAME_H__
#define __VIDEO_FRAME_H__

#include <stdint.h>
#include "Structs.h"

#pragma pack(push)
#pragma pack(1)


struct VideoPacketHead
{
    int8_t           Reserved;    // Reserved; always (0)zero
    uint8_t			 FrameNum;    // Cycled frame number (0 - 255)
    uint16_t		 LineNum;     // Scanline number (1 - 240)
    uint8_t			 LinePackNum; // Packet number in scanline (1 - 2)
    uint8_t			 Flags;       // Flags; must be (0)zero
};


// __attribute__((packed));


//struct VKVR_HEAD
//{
//    char reserved;                  //! всегда 0
//    unsigned char cadr;             //! номер кадра, циклический счетчик от 0 до 255
//    unsigned short string_num;      //! номер строки от 1 до 240
//    unsigned char packet_num;       //! номер пакета в строке от 1 до 2
//    unsigned char flags;            //! флаги
//}__attribute__((packed));


struct VideoFrameDesc
{
    int32_t                 nID;            // TODO

    int32_t                 m_iWidth;       // Image width in pixels
    int32_t                 m_iHeight;      // Image height in pixels
    int32_t                 m_iStrideWidth; // Stridden image width

    VEGA_INFO           VegaInfo;       // TODO
    PIL_RAW_DATA        FlightData;     // TODO
    NAV_PARSER_GPS_DATA NavData;        // TODO

	//For jpeg support
	int32_t                 m_iJpegFrameSize;
};





class CVideoFrame : public VideoFrameDesc
{
public:
	// перенесено из VideoFrameDesc для устраения проблем совместимости с 32/64-битными указателями
	uint8_t*			m_pData;        // Pointer to image bits
	uint8_t				m_bRGB;
	uint8_t				m_bRGBY;
public:

    // Constructors
    CVideoFrame();

    CVideoFrame( const CVideoFrame& Frame );

    CVideoFrame( int iWidth, int iHeight, bool bStride = true );

    CVideoFrame( unsigned char* pData, int iWidth, int iHeight, int iStrideWidth );

    // Destructor
    ~CVideoFrame( void );



    // TODO
    int PushDataLine( int nLineNum, void* pData, int DataLen );

    // TODO
    int PopDataLine( int nLineNum, void* pData, int DataLen );



    // Create current instance of image
    int Create( int iWidth, int iHeight, bool bStride = true );

	// Create current instance of image (RGB)
	int CreateRGB(int iWidth, int iHeight);

	// Create current instance of image (RGB+Gray)
	int CreateRGBY(int iWidth, int iHeight);

    // Release current instance of image
    int Release( void );

    // Get pointer to image bits
    unsigned char* GetDataPtr( void ) { return m_pData; }

	unsigned char* GetDataPtr_Y(void)
	{
		if (!m_bRGBY)
			return NULL;

		return &m_pData[m_iWidth * m_iHeight * 3];
	}

    // Get image height
    int GetHeight( void ) const       { return m_iHeight; }

    // Get image width
    int GetWidth( void ) const        { return m_iWidth; }

    // Get image stride width
    int GetStrideWidth( void ) const  { return m_iStrideWidth; }

	int GetSize(void) const  { return m_iStrideWidth * m_iHeight; }

    //
	int GetBpp() const { return 8 * (int)!m_bRGB + 24 * (int)m_bRGB; }


    // Attach an existing image to the current instance
    int Attach( unsigned char* pData,
                int iWidth, 
                int iHeight, 
                int iStrideWidth );

    // Return value of image pixel
    unsigned char GetItem( int x, int y );

    // Set image pixel value
    void SetItem( int x, int y, unsigned char btVal );

    // Fill image pixels with given value
    void Fill( unsigned char btVal );

    //
    bool IsEmpty();

    // Determines whether the specified coordinates is included into the image
    bool IsIncluded( int x1 = 0, int y1 = 0,
                     int x2 = 0, int y2 = 0,
                     int x3 = 0, int y3 = 0,
                     int x4 = 0, int y4 = 0 );

    // Fill rectangle area with given value
    int FillRect( int iLeft,  int iTop,
                  int iRight, int iBottom, 
                  unsigned char btVal );

    // Put rectangular subimage into the image 
    int PutRect( int iPosX, int iPosY,
                 CVideoFrame& Source, 
                 int iLeft, int iTop, int iRight, int iBottom );
    
    // Cut rectangular subimage from the image
    int CutRect( CVideoFrame* pDstImage,
                 int iLeft,  int iTop,
                 int iRight, int iBottom );


    // Inverse image using a bitwise inversing
    void Inverse();

    // Rotate image on 90 degrees clockwise
    int Rotate90CW();

    // Rotate image on 180 degrees clockwise
    int Rotate180CW();

    // Rotate image on 270 degrees clockwise
    int Rotate270CW();

    // Rotate image on 90 degrees counter-clockwise
    int Rotate90CCW();

    // Rotate image on 180 degrees counter-clockwise
    int Rotate180CCW();

    // Rotate image on 270 degrees counter-clockwise
    int Rotate270CCW();


    // Overloading of the operator=
    // copy into this image from another
    CVideoFrame& operator= (const CVideoFrame& SrcImage );


    // Flip image vertical
    int FlipVertical( void );

    // Fill triangle with given value
    int FillTriangle( int Ax, int Ay,
                      int Bx, int By,
                      int Cx, int Cy,
                      unsigned char btFill = 127 );

    // Fill trapezoid with given value
    int FillTrapezoid( int x1, int y1,
                       int x2, int y2,
                       int x3, int y3,
                       int x4, int y4,
                       unsigned char btFill = 0 );

    // Cutting this image into a given image or itself (if pImgPtr == NULL)
    int GaussianBlur( CVideoFrame* pBlurImage,
                      int iLeft,  int iTop,
                      int iRight, int iBottom,
                      int iMaskSize, int* pMask );


    // Rescale current image by use of bilinear interpolation
    int RescaleBilinear ( CVideoFrame& newImage, double dRatio );
    
    // Rescale current image by use of bilinear interpolation with safety borders processing
    int SafeRescaleBilinear ( CVideoFrame& newImage, double dRatio, unsigned char bgColor );

    // Rescale current image using nearest-neighbour pixel interpolation
    int RescaleNearestNeighbour ( CVideoFrame& newImage, double dRatio);

    // Binarize current image
    int Binarize( unsigned char iThreshold );

    // Make current image stridden
    int MakeStride();

    // Make current image unstridden
    int MakeUnStride();

    // Save (8bpp) BMP grayscale image from the class instance
    int SaveBitMap( const char* szMaskName, ... );

    // Load (8bpp) BMP grayscale image in to the class instance
    int LoadBitMap( const char* szMaskName, ... );
};

#pragma pack(pop)

#ifndef _DEBUG

    inline unsigned char ::CVideoFrame::GetItem( int x, int y ) 
    { return m_pData[ x + y * m_iStrideWidth ]; }

    inline void ::CVideoFrame::SetItem( int x, int y, unsigned char btVal ) 
    { m_pData[ x + y * m_iStrideWidth ] = btVal; }

#endif // _DEBUG

#endif // __VIDEO_FRAME_H__
