#include "JPG_decoder.h"

#include <new>
#include "STDDEF.H"

#include <stdio.h>

#include <iostream>


#include "libjpeg\jpeglib.h"
#include <setjmp.h>



CJPG_codec::CJPG_codec()
{
	m_pBMP_data = NULL;
	m_pJPG_data = NULL;

	Reset();
}


CJPG_codec::~CJPG_codec()
{
	Reset();
}

void CJPG_codec::Reset()
{
	if (m_pBMP_data != NULL)
		delete[] m_pBMP_data;

	if (m_pJPG_data != NULL)
		delete[] m_pJPG_data;

	m_iWidth = 0;
	m_iHeight = 0;
	m_iPixelSize = 0;

	m_iBMP_Size = 0;
	m_iJPG_Size = 0;
}

struct jpegErrorManager {
	/* "public" fields */
	struct jpeg_error_mgr pub;
	/* for return to caller */
	jmp_buf setjmp_buffer;
};
char jpegLastErrorMsg[JMSG_LENGTH_MAX];
void jpegErrorExit(j_common_ptr cinfo)
{
	/* cinfo->err actually points to a jpegErrorManager struct */
	jpegErrorManager* myerr = (jpegErrorManager*)cinfo->err;
	/* note : *(cinfo->err) is now equivalent to myerr->pub */

	/* output_message is a method to print an error message */
	/*(* (cinfo->err->output_message) ) (cinfo);*/

	/* Create the message */
	(*(cinfo->err->format_message)) (cinfo, jpegLastErrorMsg);

	/* Jump to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);

}

int CJPG_codec::DecodeJPG(unsigned char* pDataJPG, int iJPG_Size, unsigned char** pDataBMP, int* iWidth, int* iHeight, int* iPixelSize, int* iBMP_Size)
{
	if ((pDataJPG == NULL) || (iJPG_Size <= 0) || (pDataBMP == NULL) || (iWidth == NULL) || (iHeight == NULL) || (iPixelSize == NULL) || (iBMP_Size == NULL))
	{
		Reset();
		return -1;
	}	
		
	struct jpeg_decompress_struct m_cinfo_decode;

	jpegErrorManager jerr;
	m_cinfo_decode.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = jpegErrorExit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error. */
		std::cout << jpegLastErrorMsg << std::endl;
		jpeg_destroy_decompress(&m_cinfo_decode);
		return -100;
	}

	jpeg_create_decompress(&m_cinfo_decode);

	jpeg_mem_src(&m_cinfo_decode, pDataJPG, iJPG_Size);

	if (jpeg_read_header(&m_cinfo_decode, FALSE) != 1)
	{
		Reset();
		return -2;
	}
		

	if (!jpeg_start_decompress(&m_cinfo_decode))
	{
		Reset();
		return -3;
	}

	m_iWidth = m_cinfo_decode.output_width;
	m_iHeight = m_cinfo_decode.output_height;
	m_iPixelSize = m_cinfo_decode.output_components;	

	if (m_iBMP_Size < m_iWidth * m_iHeight * m_iPixelSize)
	{
		if (m_pBMP_data != NULL)
			delete[] m_pBMP_data;
		m_iBMP_Size = m_iWidth * m_iHeight * m_iPixelSize;
		m_pBMP_data = new(std::nothrow) unsigned char[m_iBMP_Size];
		
	}
	if (m_pBMP_data == NULL)
	{
		Reset();
		return -4;
	}

	int row_stride = m_iWidth * m_iPixelSize;

	while (m_cinfo_decode.output_scanline < m_cinfo_decode.output_height)
	{
		unsigned char *buffer_array[1];
		buffer_array[0] = m_pBMP_data + (m_cinfo_decode.output_scanline) * row_stride;

		jpeg_read_scanlines(&m_cinfo_decode, buffer_array, 1);
	}

	jpeg_finish_decompress(&m_cinfo_decode);

	*iWidth = m_iWidth;
	*iHeight = m_iHeight;
	*iPixelSize = m_iPixelSize;
	*pDataBMP = m_pBMP_data;
	*iBMP_Size = m_iWidth * m_iHeight * m_iPixelSize;
	

	return 0;
}

int CJPG_codec::DecodeJPG_preallocation(unsigned char* pDataJPG, int iJPG_Size, unsigned char* pDataBMP, int iWidth, int iHeight, int iPixelSize, int iBMP_Size)
{
	if ((pDataJPG == NULL) || (iJPG_Size <= 0) || (pDataBMP == NULL) || (iWidth == NULL) || (iHeight == NULL) || (iPixelSize == NULL) || (iBMP_Size == NULL) || (iWidth * iHeight * iPixelSize > iBMP_Size))
	{
		Reset();
		return -1;
	}

	struct jpeg_decompress_struct m_cinfo_decode;

	jpegErrorManager jerr;
	m_cinfo_decode.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = jpegErrorExit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error. */
		std::cout << jpegLastErrorMsg << std::endl;
		jpeg_destroy_decompress(&m_cinfo_decode);
		return -100;
	}

	jpeg_create_decompress(&m_cinfo_decode);

	jpeg_mem_src(&m_cinfo_decode, pDataJPG, iJPG_Size);

	if (jpeg_read_header(&m_cinfo_decode, FALSE) != 1)
	{
		Reset();
		return -2;
	}


	if (!jpeg_start_decompress(&m_cinfo_decode))
	{
		Reset();
		return -3;
	}

	if ((iWidth != m_cinfo_decode.output_width) ||
		(iHeight != m_cinfo_decode.output_height) ||
		(iPixelSize != m_cinfo_decode.output_components))
	{
		return -4;
	}

	m_iWidth = m_cinfo_decode.output_width;
	m_iHeight = m_cinfo_decode.output_height;
	m_iPixelSize = m_cinfo_decode.output_components;

	int row_stride = m_iWidth * m_iPixelSize;

	while (m_cinfo_decode.output_scanline < m_cinfo_decode.output_height)
	{
		unsigned char *buffer_array[1];
		buffer_array[0] = pDataBMP + (m_cinfo_decode.output_scanline) * row_stride;

		jpeg_read_scanlines(&m_cinfo_decode, buffer_array, 1);
	}

	jpeg_finish_decompress(&m_cinfo_decode);

	return 0;
}

int CJPG_codec::EncodeJPG(unsigned char* pDataBMP, int iWidth, int iHeight, int iPixelSize, int iBMP_Size, unsigned char** pDataJPG, int* iJPG_Size, int iQuality)
{
	if ((pDataBMP == NULL) || (iWidth <= 0) || (iHeight <= 0) || ((iPixelSize != 1) && (iPixelSize != 3)) || (iBMP_Size <= 0) || (pDataJPG <= 0) || (iJPG_Size == NULL))
	{
		Reset();
		return -1;
	}
	
	struct jpeg_compress_struct m_cinfo_encode;

	jpegErrorManager jerr;
	m_cinfo_encode.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = jpegErrorExit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error. */
		std::cout << jpegLastErrorMsg << std::endl;
		jpeg_destroy_compress(&m_cinfo_encode);
		return -100;
	}

	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&m_cinfo_encode);

	unsigned char* pJPG_data = NULL;
	unsigned long JPG_Size = 0;

	jpeg_mem_dest(&m_cinfo_encode, &pJPG_data, &JPG_Size);

	m_cinfo_encode.image_width = iWidth;      /* image width and height, in pixels */
	m_cinfo_encode.image_height = iHeight;
	m_cinfo_encode.input_components = iPixelSize;           /* # of color components per pixel */
	m_cinfo_encode.in_color_space = (J_COLOR_SPACE)((int)JCS_RGB * (int)(iPixelSize == 3) + (int)JCS_GRAYSCALE * (int)(iPixelSize == 1));       /* colorspace of input image */
	m_cinfo_encode.arith_code = FALSE;
	m_cinfo_encode.dct_method = JDCT_FLOAT;
	m_cinfo_encode.progressive_mode = FALSE;
	m_cinfo_encode.raw_data_in = TRUE;
	jpeg_set_defaults(&m_cinfo_encode);

	jpeg_set_quality(&m_cinfo_encode, iQuality, TRUE /* limit to baseline-JPEG values */);

	jpeg_start_compress(&m_cinfo_encode, TRUE);

	int row_stride = iWidth * iPixelSize; /* JSAMPLEs per row in image_buffer */

	while (m_cinfo_encode.next_scanline < m_cinfo_encode.image_height) {
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could pass
		* more than one scanline at a time if that's more convenient.
		*/
		unsigned char *buffer_array[1];
		buffer_array[0] = &pDataBMP[m_cinfo_encode.next_scanline * row_stride];
		(void)jpeg_write_scanlines(&m_cinfo_encode, buffer_array, 1);
	}

	jpeg_finish_compress(&m_cinfo_encode);

	if (m_pJPG_data != NULL)
		delete[] m_pJPG_data;

	m_pJPG_data = new unsigned char[JPG_Size];

	memcpy(m_pJPG_data, pJPG_data, JPG_Size);

	free(pJPG_data);
	
	jpeg_destroy_compress(&m_cinfo_encode);

	m_iJPG_Size = JPG_Size;

	*pDataJPG = m_pJPG_data;
	*iJPG_Size = m_iJPG_Size;

	return 0;
}

