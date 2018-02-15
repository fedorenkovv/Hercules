#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include <stdint.h>

struct WIND_DATA		//! ��������� �����
{
	float psi;
	float velocity;
};

struct NET_PROGNOZ_STRUCT 		//! ��������� ����������� ��������� � ��������� ��
{
	//! ��������
	float v;

	float h_zad;
	//! ���������� ���������
	float dist_left;
	//!���������� �����
	float time_left;
	//! ���������� ��������� ������
	float rash;
	//! ������� ������� �������
	float ostatok_now;

	//! ������� ������� �������
	float ostatok_future;

};

struct PROGNOZ_SEND			//! ����������� ����� ���������
{
	NET_PROGNOZ_STRUCT v1; //! ������ �� �������� �������� �������� ������� v1
	NET_PROGNOZ_STRUCT v2; //! ������ �� �������� ���������� �������� v2
	NET_PROGNOZ_STRUCT vr; //! ������ �� �������� �������� ������� �������� ������ vr
	NET_PROGNOZ_STRUCT polet; //! ������ �� �������� ������������ � ������ / ���������� ����������� ������
	NET_PROGNOZ_STRUCT landing	; //! �� ������������
};


struct PIL_RAW_DATA
{
	float teta;             //! ������
	float gamma;            //! ����

	float aplha;            //! ���� �����
	float betta;            //! ���� ����������
	float fpa;              //! ����������� ����
	float vel_ist;          //! �������� ��������
	float vel_prib;         //! ��������� ��������
	float mach;             //! ����� ����
	float alt_bar;          //! ��������������� ������
	float alt_abs;          //! ���������� ��������������� ������ ��� ����� ���������� ��������
	float Vy;               //! ������������ ��������

	float nx;               //! ���������� ����������
	float ny;               //! ������������ ����������
	float nz;               //! ������� ����������

	float psi_ist;          //! �������� ����
	float wY;               //! ������� �������� � ������ ������� ���������
	float Pst;              //! �������� ����������� �������� 0

	int reserved0;
	int reserver1;
	int reserver2;
	int reserved3;

	float radioheight;		// �����������

	WIND_DATA wind;
	PROGNOZ_SEND prognoz;

	int valid_flag;         //! ����� ����������
//	inline int IsValid( int mask ){ return (valid_flag & mask);}
//	inline int SetValid( int mask , int val ){ if ( val ) valid_flag |= mask; else valid_flag &= ~mask;}
};


struct NAV_PARSER_GPS_DATA
{
	uint64_t time;     //! ����� � �������� �� ������ ������
	double lat;             //! ������ � ���
	double lon;             //! ������� � ���
	float alt;              //! �������������� ������
	float psi_put;          //! ������� ����
	float V_put;            //! ������� ��������
	float Vy;               //! ������������ �������� �� GPS
	int32_t alt_regime;         //!0 - �������������� ������, 1 - ������������� ������.
	int32_t otkaz;              //! ����� �������/���������� ��������� ���
	int32_t reserved1;
	int32_t reserved2;
};


struct VEGA_INFO
{
	int32_t nFramesCount;

	int32_t nTVFrameWidth;
	int32_t nTVFrameHeight;
	int32_t nTVFrameStride;

	int32_t nIRFrameWidth;
	int32_t nIRFrameHeight;
	int32_t nIRFrameStride;
};

//!��� EnhVision, ������������� ����� �� ������� ��������� ������ �� ��������
enum EnhChannel{ EnhChannelTV, EnhChannelIR, EnhChannelBoth, EnhChannelIR2 };

#endif // __STRUCTS_H__
