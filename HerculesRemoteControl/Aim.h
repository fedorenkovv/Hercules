#ifndef __AIM__
#define __AIM__

#include "code.h"
#include <vector>

struct xellipse
{
	double x0, y0, xR, yR, Q;
};

struct target
{
	double         x;   // Координат центра метки по оси абсцисс
	double         y;   // Координат центра метки по оси ординат
	unsigned short num; // Идентификатор метки
};

class concentric
{
public:
	concentric( stRect   *r,
                xellipse *e,
                double    q
              )
	{
		rc      = *r;
		ellipse = *e;
		err     =  q;
	}

	stRect   rc;
	xellipse ellipse;
	double   err;

};

class CAim
{
public:
    CAim();
    virtual ~CAim();

    void InitPar( unsigned char *pImg,
                  int            nImgHeight,
                  int            nImgWidth,
                  int            CountOfAim
                );

    int            GetCountOfAim()    { return mCountOfAim; }
    unsigned short GetCountOfRegion() { return (unsigned short)( mCountOfRegion - 1 ); }

    target &operator[]( int i );

    // new
    double GetProgress() { return progress; }

    void Processing(); //  [5-2-2015 Skink]

    int mCountOfAim;

    target  *m_pCrd;

public:

    void Init();
    void Release(); //  [9-2-2015 Skink]
    void ClearLists();
    void Find();
    void FindAll();
    void Scanf();
    void Granica(stPoint);
    void Inzhener();

    bool IsRatioCorrect ( CCode  *, CCode *, CCode * );
    bool IsCircleInRing ( CCode  *, CCode *          );
    bool IsRingInRing   ( CCode  *, CCode *, CCode * );
    bool IsRingInBlank  ( CCode  *, CCode *          );
    bool IsItCircle     ( CCode  *                   );
    bool toCanonicForm  ( double *                   );
    bool IsPointOnBorder( stPoint                    );

    int aline(double num);
    int Select(int,int,unsigned char);
    int Decoded();

    double CalcError( stPoint *);
    double * matrix ( stPoint *);

    stPoint * ExtractBorder(CCode *);

//     CPtrList rcWhite;   //список белых областей
//     CPtrList rcBlack;   //список чёрных областей
//     CPtrList rcStorage; //темповый список использующийся на каждом срезе 
//     CPtrList trgFinal;  //финальный список областей сосовляющих кодированную метку
    
    std::vector<CCode>      m_vWhite;   //список белых областей
    std::vector<CCode>      m_vBlack;   //список чёрных областей
    std::vector<CCode>      m_vStorage; //временнный список использующийся на каждом срезе 
    std::vector<concentric> m_vFinal;   //итоговый список областей, сосовляющих кодированную метку
    
    struct stStack
    {
        int  xL;
        int  xR;
        int  y;
    } m_Stack;         //просто стек

    stRect   rcCur;    //темповая переменная
    xellipse ellipse;  //темповая переменная

    unsigned short *pTrack;
    unsigned char  *pShear;
    
    unsigned char  *m_pImg;             //  [2/5/2015 Skink]
    int             m_nImgHeight;       //  [2/5/2015 Skink]
    int             m_nImgWidth;        //  [2/5/2015 Skink]
    int             m_nImgStrideWidth;  //  [2/5/2015 Skink]


    CCode   *rect;

    double progress;

    double a1; // a1-радиус белой метки
    double a2; // a2-радиус до начала кодосодержащего кольца
    double a3; // a3-радиус до внутренней стороны внешнего белого кольца

    int             mCountOfPoints;
    int             mCountOfMetok;
    int             height,width,height4,width4;
    unsigned short  mCountOfRegion;
    unsigned short  fnt_clr,rgn_clr;
    unsigned char   Hist[360];
};

#endif // __AIM__

int aline(double num);
