#pragma once



class CJPG_codec
{
public:
	CJPG_codec();
	~CJPG_codec();

	int DecodeJPG(unsigned char* pDataJPG, int iJPG_Size, unsigned char** pDataBMP, int* iWidth, int* iHeight, int* iPixelSize, int* iBMP_Size);
	int DecodeJPG_preallocation(unsigned char* pDataJPG, int iJPG_Size, unsigned char* pDataBMP, int iWidth, int iHeight, int iPixelSize, int iBMP_Size);

	int EncodeJPG(unsigned char* pDataBMP, int iWidth, int iHeight, int iPixelSize, int iBMP_Size, unsigned char** pDataJPG, int* iJPG_Size, int iQuality);


private:

	void Reset();

	



	unsigned char* m_pBMP_data;
	int m_iBMP_Size;

	unsigned char* m_pJPG_data;
	unsigned long m_iJPG_Size;

	int m_iWidth;
	int m_iHeight;
	int m_iPixelSize;

};

