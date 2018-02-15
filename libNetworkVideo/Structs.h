#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include <stdint.h>

struct WIND_DATA		//! параметры ветра
{
	float psi;
	float velocity;
};

struct NET_PROGNOZ_STRUCT 		//! параметры прогнозного положения и состояния ЛА
{
	//! скорость
	float v;

	float h_zad;
	//! оставшаяся дистанция
	float dist_left;
	//!оставшееся время
	float time_left;
	//! мгновенный секундный расход
	float rash;
	//! текущий остаток топлива
	float ostatok_now;

	//! прогноз остатка топлива
	float ostatok_future;

};

struct PROGNOZ_SEND			//! комплексный набор прогнозов
{
	NET_PROGNOZ_STRUCT v1; //! данные по прогнозу скорости принятия решения v1
	NET_PROGNOZ_STRUCT v2; //! данные по прогнозу безопасной скорости v2
	NET_PROGNOZ_STRUCT vr; //! данные по прогнозу скорости подъема передней стойки vr
	NET_PROGNOZ_STRUCT polet; //! данные по прогнозу столкновения с землей / достижения необходимой высоты
	NET_PROGNOZ_STRUCT landing	; //! не используется
};


struct PIL_RAW_DATA
{
	float teta;             //! тангаж
	float gamma;            //! крен

	float aplha;            //! угол атаки
	float betta;            //! угол скольжения
	float fpa;              //! траекторный угол
	float vel_ist;          //! истинная скорость
	float vel_prib;         //! приборная скорость
	float mach;             //! число маха
	float alt_bar;          //! барометрическая высота
	float alt_abs;          //! абсолютная барометрическая высота без учета введенного давления
	float Vy;               //! вертикальная скорость

	float nx;               //! продольная перегрузка
	float ny;               //! вертикальная перегрузка
	float nz;               //! боковая перегрузка

	float psi_ist;          //! истинный курс
	float wY;               //! угловая скорость в земной системе координат
	float Pst;              //! заданное статическое давление 0

	int reserved0;
	int reserver1;
	int reserver2;
	int reserved3;

	float radioheight;		// радиовысота

	WIND_DATA wind;
	PROGNOZ_SEND prognoz;

	int valid_flag;         //! флаги валидности
//	inline int IsValid( int mask ){ return (valid_flag & mask);}
//	inline int SetValid( int mask , int val ){ if ( val ) valid_flag |= mask; else valid_flag &= ~mask;}
};


struct NAV_PARSER_GPS_DATA
{
	uint64_t time;     //! время в секундах от начала недели
	double lat;             //! широта в рад
	double lon;             //! долгота в рад
	float alt;              //! геометрическая высота
	float psi_put;          //! путевой угол
	float V_put;            //! путевая скорость
	float Vy;               //! вертикальная скорость по GPS
	int32_t alt_regime;         //!0 - геометрическая высота, 1 - относительная высота.
	int32_t otkaz;              //! флаги наличия/отсутствия коррекции ИНС
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

//!для EnhVision, сигнализирует какой из каналов требуется дальше по конвееру
enum EnhChannel{ EnhChannelTV, EnhChannelIR, EnhChannelBoth, EnhChannelIR2 };

#endif // __STRUCTS_H__
