#include "Aim.h"
#include <math.h>
#include <cstdlib>

CAim::CAim()
{
    pTrack = NULL;
    pShear = NULL;

    m_pCrd = NULL;
}

CAim::~CAim()
{
    ClearLists();
    Release();

    if (m_pCrd != NULL)
        delete[] m_pCrd;
}

void CAim::Init()
{
    unsigned short *Ukazka1, *Ukazka2;

    memset( pTrack,                     0, width4 * height4 * sizeof(pTrack[0]));
    memset( pTrack,                     1, width4 *           sizeof(pTrack[0]));
    memset( pTrack +(height4-1)*width4, 1, width4 *           sizeof(pTrack[0]));
    
    Ukazka1 = pTrack;
    Ukazka2 = pTrack + width4 - 1;
    
    for(int i = 0; i < height4; i++, Ukazka1 += width4, Ukazka2 += width4 )
    {
        *Ukazka1 = 1;
        *Ukazka2 = 1;
    }
}

void CAim::InitPar(unsigned char *pImg, int nImgHeight, int nImgWidth, int CountOfAim)
{
    Release();

    height  = nImgHeight; //pImage->Height();
    width   = nImgWidth;  //pImage->Width();
    height4 = height+2;
    width4  = width+2;

    pShear = pImg;

    mCountOfAim    = 0;
    mCountOfPoints = 0;
    mCountOfRegion = 1;

    a1=0.15;
    a2=0.45;
    a3=0.75;

    mCountOfMetok = CountOfAim;

    m_pCrd = NULL;
}

void CAim::Release()
{
    m_vWhite.clear();
    m_vBlack.clear();
    m_vStorage.clear();
    m_vFinal.clear();
}

void CAim::ClearLists()
{
    m_vWhite.clear();
    m_vBlack.clear();
    m_vStorage.clear();
}

void CAim::Processing()
{
    pTrack = new unsigned short[height4*width4];
    if(!pTrack)
        return;

    progress = 0;
    FindAll();

    delete []pTrack;
}

void CAim::FindAll()
{
	unsigned char  *Zptr, *ZZptr;
    unsigned short *Qptr, *QQptr;
    unsigned char threshod = 140;
    for ( unsigned char h = 200; h >= threshod; h -= 20 )
    {
    	ZZptr = Zptr = pShear + width4 + 1;
    	QQptr = Qptr = pTrack + width4 + 1;
       	mCountOfRegion = 1;

        Init();

		double fp = double( 250 - h )/( 250 - threshod );
		fp -= 0.5f;
		progress = 0.5f * sin( 3.1415f * fp ) + 0.5;
        for ( int y = 1; y < height + 1; y++, ZZptr += width4, QQptr += width4 )
        {
             Zptr = ZZptr;
             Qptr = QQptr;
             for( int x = 1; x < width + 1; x++, Zptr++, Qptr++ )
             {
                 if( !*Qptr )
                 {
                     int shift = Select(x,y,h);

                     x    += shift;
                     Zptr += shift;
                     Qptr += shift;

                     stPoint temp;
                     if(( rcCur.top != rcCur.bottom ) && ( rcCur.left != rcCur.right ))
                     {
                         temp.x = x-shift;
                         temp.y = y;
                         if (*Zptr>=h)
                         {
                             m_vWhite.push_back( CCode( &rcCur, &temp, mCountOfRegion ));
                         }
                         else
                         {
                             m_vBlack.push_back( CCode( &rcCur, &temp, mCountOfRegion ));
                         }
                     }
                 }
             }
         }

        Find();
        Inzhener();
        ClearLists();   
	}

    if ( m_pCrd != NULL )
        delete [] m_pCrd;

    int nu, nCnt = 0;
	m_pCrd = new target[mCountOfAim];

    for ( int i = 0, iEnd = (int)m_vFinal.size(); i < iEnd; i++ )
    {
        ellipse = (m_vFinal[i]).ellipse;
        nu = Decoded();

        if( nu != 0 )
        {
            m_pCrd[ nCnt ].x   = (m_vFinal[i]).ellipse.x0-1;
            m_pCrd[ nCnt ].y   = (m_vFinal[i]).ellipse.y0-1;
            m_pCrd[ nCnt ].num = nu;
            nCnt++;
        }
        else
        {
            mCountOfAim--;
        }

    }
}

void CAim::Find()
{	
    CCode lcWhiteF, lcWhiteS;
    CCode lcBlackF, lcBlackS;

    int nW = m_vWhite.size();
    int nB = m_vBlack.size();
	for ( int i = 0; i < nW; i++ )
	{
        lcWhiteF = m_vWhite[ i ];
        if (IsItCircle(&lcWhiteF))
		{
			for( int j = 0; j < nB; j++ )
			{
                lcBlackF = m_vBlack[ j ];
                if (IsCircleInRing(&lcWhiteF,&lcBlackF))
				{
					for ( int k = 0; k < nW; k++ )
					{
                        lcWhiteS = m_vWhite[ k ];
						if (IsRingInRing(&lcWhiteF,&lcBlackF,&lcWhiteS))
						{
							for( int m = 0; m < nB; m++ )
							{
                                lcBlackS = m_vBlack[ m ];
								if (IsRingInBlank(&lcWhiteS,&lcBlackS) && IsRatioCorrect(&lcWhiteF,&lcBlackF,&lcWhiteS) )
								{
                                        m_vStorage.push_back( CCode( &lcWhiteS.rc, &lcWhiteS.pnt, lcWhiteS.num ));
										goto once_more;
								}
							}
						}
					}
				}
			}
		}
        once_more:;
	}
}

int CAim::aline(double num)
{
    double x,ost;
    ost=modf(num,&x);

    if(ost>=0.5) 
        x++;
    else if(ost<=-0.5)
        x--;

    return int(x);
}

int CAim::Select(int x, int y,unsigned char h)
{
    std::vector<stStack> vStack;

    int top;//cache "rcCur.top" in local variable for speed increase
	int bottom;
	int right;
	int left;
	int ret;
	int i;
	unsigned short n;
	unsigned char * pshear;
	unsigned short * ptrack;
	pshear=pShear+y*width4;
	ptrack=pTrack+y*width4;
	mCountOfRegion++;
	n=mCountOfRegion;
	int xL,xR,xLb,xRb;
	unsigned char bri;
	unsigned short tra;


	bri=pshear[x];
	bool color = (bri>=h);


	if (color)
	{
		xL=x-1;
		xR=x+1;
		top=bottom=y;
		left=right=x;
		ptrack[x]=n;
		bri=pshear[xL];
		tra=ptrack[xL];

		while ((!tra) && (bri>=h))
		{
			ptrack[xL]=n;
			xL--;
			bri=pshear[xL];
			tra=ptrack[xL];

		}

		bri=pshear[xR];
		tra=ptrack[xR];

		while ((!tra) && (bri>=h))
		{
			ptrack[xR]=n;
			xR++;
			bri=pshear[xR];
			tra=ptrack[xR];

		}

		if (left>xL+1) left=xL+1;
		if (right<xR-1) right=xR-1;
		ret=xR-x-1;


		// IIIANOEEE A NOAE xR   xL  y
        m_Stack.xL = xL;
        m_Stack.xR = xR;
        m_Stack.y  = y;
        vStack.push_back( m_Stack );

		while ( !vStack.empty() )
		{
			//ecaeaeaai eii?aeiaou ec noaea
			//y  ea?aeiaay eii?aeiaoa io?acea
			//xLb
			//xRb
            xLb = vStack.back().xL;
            xRb = vStack.back().xR;
            y   = vStack.back().y;
            vStack.pop_back();

			for (i=-1;i<=2;i+=3)
			{
				y+=i;
				x=xLb;
				pshear=pShear+y*width4;
				ptrack=pTrack+y*width4;
				bri=pshear[x];
				tra=ptrack[x];


				if ((!tra) && (bri>=h))
				{
					xL=x-1;
					xR=x+1;
					ptrack[x]=n;
					bri=pshear[xL];
					tra=ptrack[xL];

					while ((!tra) && (bri>=h))
					{
						ptrack[xL]=n;
						xL--;
						tra=ptrack[xL];
						bri=pshear[xL];
					}

					bri=pshear[xR];
					tra=ptrack[xR];

					while ((!tra) && (bri>=h))
					{
						ptrack[xR]=n;
						xR++;
						bri=pshear[xR];
						tra=ptrack[xR];
					}

					//iiiauaai a noae xL xR y

                    m_Stack.xL = xL;
                    m_Stack.xR = xR;
                    m_Stack.y  = y;
                    vStack.push_back( m_Stack );

                    if (left>xL+1) left=xL+1;
					if (right<xR-1) right=xR-1;
					if (top>y) top=y;
					if (bottom<y) bottom=y;

					x=xR-1;

				}

				x++;

				bri=pshear[x];
				tra=ptrack[x];

				while (true)
				{
					while (tra || (bri<h))
					{
						if( x>=xRb )
							goto finish;
						x++;
						bri=pshear[x];
						tra=ptrack[x];
					}

					ptrack[x]=n;
					xL=x-1;
					xR=x+1;
					bri=pshear[xR];
					tra=ptrack[xR];

					while ((!tra) && (bri>=h))
					{
						ptrack[xR]=n;
						xR++;
						bri=pshear[xR];
						tra=ptrack[xR];
					}
					//iiiauaai a noae xR xL e y

                    m_Stack.xL = xL;
                    m_Stack.xR = xR;
                    m_Stack.y  = y;
                    vStack.push_back( m_Stack );

					if (left>xL+1) left=xL+1;
					if (right<xR-1) right=xR-1;
					if (top>y) top=y;
					if (bottom<y) bottom=y;

				}
		finish:;

			}


		}

	}
	else                         //≈—À» ›“Œ «¿À»¬ ¿ ◊®–ÕŒ… Œ¡À¿“»
	{
		xL=x;
		xR=x;
		top=bottom=y;
		left=right=x;
		ptrack[x]=n;
		bri=pshear[xL-1];
		tra=ptrack[xL-1];

		while ((!tra) && (bri<h))
		{
			xL--;
			ptrack[xL]=n;
			bri=pshear[xL-1];
			tra=ptrack[xL-1];

		}

		bri=pshear[xR+1];
		tra=ptrack[xR+1];

		while ((!tra) && (bri<h))
		{
			xR++;
			ptrack[xR]=n;
			bri=pshear[xR+1];
			tra=ptrack[xR+1];

		}

		if (left>xL) left=xL;
		if (right<xR) right=xR;
		ret=xR-x;


		// IIIANOEEE A NOAE xR   xL  y

        m_Stack.xL = xL;
        m_Stack.xR = xR;
        m_Stack.y  = y;
        vStack.push_back( m_Stack );

		while ( !vStack.empty() )
		{
 			//ecaeaeaai eii?aeiaou ec noaea
			//y  ea?aeiaay eii?aeiaoa io?acea
			//xLb
			//xRb

            xLb = vStack.back().xL;
            xRb = vStack.back().xR;
            y   = vStack.back().y;
            vStack.pop_back();

			for (int i=-1;i<=2;i+=3)
			{
				y+=i;
				x=xLb;
				pshear=pShear+y*width4;
				ptrack=pTrack+y*width4;
				bri=pshear[x];
				tra=ptrack[x];


				if ((!tra) && (bri<h))
				{
					xL=x;
					xR=x;
					ptrack[x]=n;
					bri=pshear[xL-1];
					tra=ptrack[xL-1];

					while ((!tra) && (bri<h))
					{
						xL--;
						ptrack[xL]=n;
						tra=ptrack[xL-1];
						bri=pshear[xL-1];
					}

					bri=pshear[xR+1];
					tra=ptrack[xR+1];

					while ((!tra) && (bri<h))
					{
						xR++;
						ptrack[xR]=n;
						bri=pshear[xR+1];
						tra=ptrack[xR+1];
					}

					//iiiauaai a noae xL xR y

                    m_Stack.xL = xL;
                    m_Stack.xR = xR;
                    m_Stack.y  = y;
                    vStack.push_back( m_Stack );


					if (left>xL) left=xL;
					if (right<xR) right=xR;
					if (top>y) top=y;
					if (bottom<y) bottom=y;

					x=xR;

				}

				x++;

				bri=pshear[x];
				tra=ptrack[x];

				while (true)
				{
					while (tra || (bri>=h))
					{
						if( x>=xRb )
							goto finish1;
						x++;
						bri=pshear[x];
						tra=ptrack[x];
					}

					ptrack[x]=n;
					xL=x;
					xR=x;
					bri=pshear[xR+1];
					tra=ptrack[xR+1];

					while ((!tra) && (bri<h))
					{
						xR++;
						ptrack[xR]=n;
						bri=pshear[xR+1];
						tra=ptrack[xR+1];
					}
					//iiiauaai a noae xR xL e y

                    m_Stack.xL = xL;
                    m_Stack.xR = xR;
                    m_Stack.y  = y;
                    vStack.push_back( m_Stack );

					if (left>xL) left=xL;
					if (right<xR) right=xR;
					if (top>y) top=y;
					if (bottom<y) bottom=y;

				}
		finish1:;
			}
		}
	}

	rcCur.left   = left;
	rcCur.right  = right;
	rcCur.bottom = bottom;
	rcCur.top    = top;

	return ret;
}

//-------------------------i?iaa?ee ia no?oeoo?iinou -------------------------

bool CAim::IsRatioCorrect(CCode * a,CCode * b,CCode * c)
{
	int ax,ay,bx,by,cx,cy;
	double Sa,Sb,S;
	double sqa,sqb;
	ax=a->rc.right-a->rc.left+1;
	ay=a->rc.bottom-a->rc.top+1;
	bx=b->rc.right-b->rc.left+1;
	by=b->rc.bottom-b->rc.top+1;
	cx=c->rc.right-c->rc.left+1;
	cy=c->rc.bottom-c->rc.top+1;
	
	sqa=a1*a1;
	sqb=a3*a3;
	S=cx*cy;
	Sa=ax*ay/S;
	Sb=bx*by/S;
	Sa=Sa/sqa;
	Sb=Sb/sqb;

	if (fabs(Sa-1)>0.45) return false; //    «ƒ≈—‹ «¿ƒ¿Õ€ Œÿ»¡ » œŒ —Œ“ÕŒÿ≈Õ»ﬂÃ –¿«Ã≈–Œ¬ œÀŒŸ¿ƒ≈… 
	if (fabs(Sb-1)>0.30) return false; //  Œ◊≈Õ‹ “ŒÕ Œ≈ Ã≈—“Œ ¬ œ–Œ√–¿ÃÃ≈

	return true;
}


bool CAim::IsItCircle(CCode *a)
{
	unsigned short look;
	stRect aRect = a->rc;
    stRect eRect;
    
    for( size_t iter = 0, iterEnd = m_vBlack.size(); iter < iterEnd; iter++ )
    {
        look = pTrack[ (m_vBlack[ iter ]).pnt.y * width4 + (m_vBlack[ iter ]).pnt.x - 1 ];
        if ( look == a->num )
        {
            eRect = (m_vBlack[ iter ]).rc;

            if( (aRect.top    < eRect.top    ) &&
                (aRect.bottom > eRect.bottom ) &&
                (aRect.left   < eRect.left   ) &&
                (aRect.right  > eRect.right  )
              )
                return false;
        }
    }

	return true;
}

bool CAim::IsCircleInRing(CCode * a,CCode * b)
{

	unsigned short bNum,look;

	bNum=b->num;
	look=pTrack[a->pnt.y*width4+a->pnt.x-1];

	if (look!=bNum)
		return false;
	else
	{
		stRect bRect,aRect;
		int bx,by;

		bRect=b->rc;
		aRect=a->rc;
	
		
		
		bx=(bRect.left+bRect.right)/2;
		by=(bRect.top+bRect.bottom)/2;

		if (
			(bx<=aRect.right) && (bx>=aRect.left) && 
			(by<=aRect.bottom) && (by>=aRect.top)
			)
		{
			return true;
		
		}

		return false;
	
	}
	
}

bool CAim::IsRingInRing(CCode * a, CCode * b,CCode * c)
{

	unsigned short cNum,look;
	cNum=c->num;

	look=pTrack[b->pnt.y*width4+b->pnt.x-1];

	if (look!=cNum)
		return false;
	else
	{
		stRect cRect,aRect;
		int cx,cy;
		
		cRect=c->rc;
		aRect=a->rc;
		cx=(cRect.left+cRect.right)/2;
		cy=(cRect.top+cRect.bottom)/2;

		if (
			(cx<=aRect.right) && (cx>=aRect.left) && 
			(cy<=aRect.bottom) && (cy>=aRect.top)
			)
		{
			return true;
		}
		return false;
	}
}

bool CAim::IsRingInBlank(CCode * c,CCode * d)
{
	unsigned short look,dNum;
	dNum=d->num;
	
	look=pTrack[c->pnt.y*width4+c->pnt.x-1];

	if (look==dNum)
	{
		stRect dRect,cRect;
		dRect=d->rc;
		cRect=c->rc;

		if (( dRect.top    < cRect.top    ) &&
            ( dRect.bottom > cRect.bottom ) &&
			( dRect.left   < cRect.left   ) &&
            ( dRect.right  > cRect.right  )
           )
           return true;
	}

	return false;
}

//----------------------------------------------------------------------//
stPoint * CAim::ExtractBorder(CCode * cod)
{
    mCountOfPoints=0;
	int k=0;
	rgn_clr=pTrack[cod->pnt.y*width4+cod->pnt.x];
	fnt_clr=pTrack[cod->pnt.y*width4+cod->pnt.x-1];
	Granica(cod->pnt);
	stPoint * a;
	a=new stPoint[mCountOfPoints];
		
	for(int i=cod->rc.top;i<=cod->rc.bottom;i++)
	{
		for (int j=cod->rc.left;j<=cod->rc.right;j++)
		{
			if (pTrack[i*width4+j]==0)
			{
				pTrack[i*width4+j]=rgn_clr;
				a[k].x = j;
                a[k].y = i;
				k++;
			}
		}
	}

	return a;
}

void CAim::Granica(stPoint pnt)
{
	pTrack[pnt.y*width4+pnt.x]=0;
	mCountOfPoints++;

	pnt.x--;
	pnt.y--;
	if (IsPointOnBorder(pnt)) 
		Granica(pnt);

	pnt.x++;
	if (IsPointOnBorder(pnt)) 
		Granica(pnt);

	pnt.x++;
	if (IsPointOnBorder(pnt)) 
		Granica(pnt);

	pnt.y++;
	if (IsPointOnBorder(pnt)) 
		Granica(pnt);

	pnt.y++;
	if (IsPointOnBorder(pnt)) 
		Granica(pnt);
	
	pnt.x--;
	if (IsPointOnBorder(pnt)) 
		Granica(pnt);

	pnt.x--;
	if (IsPointOnBorder(pnt)) 
		Granica(pnt);

	pnt.y--;
	if (IsPointOnBorder(pnt)) 
		Granica(pnt);


}

bool CAim::IsPointOnBorder(stPoint pnt)
{
	unsigned short clr;
	clr=pTrack[pnt.y*width4+pnt.x];
	if (clr!=rgn_clr) return false;

	pnt.x--;
	pnt.y--;

	clr=pTrack[pnt.y*width4+pnt.x];
	if(clr==fnt_clr) return true;

	pnt.x++;

	clr=pTrack[pnt.y*width4+pnt.x];
	if(clr==fnt_clr) return true;

	pnt.x++;

	clr=pTrack[pnt.y*width4+pnt.x];
	if(clr==fnt_clr) return true;

	pnt.y++;

	clr=pTrack[pnt.y*width4+pnt.x];
	if(clr==fnt_clr) return true;

	pnt.y++;

	clr=pTrack[pnt.y*width4+pnt.x];
	if(clr==fnt_clr) return true;

	pnt.x--;

	clr=pTrack[pnt.y*width4+pnt.x];
	if(clr==fnt_clr) return true;

	pnt.x--;

	clr=pTrack[pnt.y*width4+pnt.x];
	if(clr==fnt_clr) return true;

	pnt.y--;

	clr=pTrack[pnt.y*width4+pnt.x];
	if(clr==fnt_clr) return true;

	return false;

}

double *CAim::matrix( stPoint *pnt )
{
	double *A = new double[ 5 * mCountOfPoints ];
	double  A_T_A[ 25 ];
	double *Aptr;
	double  A_T_f[ 5 ];

	static double x[ 5 ];
    int i, j, k;
	
    Aptr = A;

	for (i=0;i<mCountOfPoints;i++)
	{
		*Aptr=pnt[i].x*pnt[i].x;
		Aptr++;
		*Aptr=pnt[i].x*pnt[i].y;
		Aptr++;
		*Aptr=pnt[i].y*pnt[i].y;
		Aptr++;
		*Aptr=pnt[i].x;
		Aptr++;
		*Aptr=pnt[i].y;
		Aptr++;
	}


	for (i=0;i<5;i++)
		for(j=0;j<5;j++)
			for (k=0,A_T_A[i*5+j]=0.;k<mCountOfPoints;k++)
				A_T_A[i*5+j]+=A[k*5+i]*A[k*5+j];

	for (i=0;i<5;A_T_f[i]=-A_T_f[i],i++)
		for(j=0,A_T_f[i]=0.;j<mCountOfPoints;++j)
			A_T_f[i]+=A[j*5+i];

	int strmap[5],colmap[5];
	for(i=0;i<5;++i)
		strmap[i]=colmap[i]=i;
	int imax,jmax;
	double max;


	// move along the main diagonal
	for(int ndx=0;ndx<4;++ndx)
	{
		// find maximum element in (5-ndx)*(5-ndx) matrix
		max=fabs(A_T_A[strmap[ndx]*5+colmap[ndx]]);
		imax=jmax=ndx;
		for(i=ndx;i<5;i++)
		{
			for(j=ndx;j<5;j++)
			{
				if(fabs(A_T_A[strmap[i]*5+colmap[j]])>max)
				{
					max=fabs(A_T_A[strmap[i]*5+colmap[j]]);
					imax=i;
					jmax=j;
				}
			}
		}
		if(imax!=ndx)
		{
			//remap strings (equations)
			int temp=strmap[imax];
			strmap[imax]=strmap[ndx];
			strmap[ndx]=temp;
		}
		if(jmax!=ndx)
		{
			//remap columns (variables)
			int temp=colmap[imax];
			colmap[imax]=colmap[ndx];
			colmap[ndx]=temp;
		}
		// now max. element is @(ndx,ndx)
		// selection of main element is completed
		// lets perform step of usual Gauss' method

		A_T_f[strmap[ndx]]/=A_T_A[strmap[ndx]*5+colmap[ndx]];
		for(i=4;i>ndx;--i)
			A_T_A[strmap[ndx]*5+colmap[i]]/=A_T_A[strmap[ndx]*5+colmap[ndx]];
		A_T_A[strmap[ndx]*5+colmap[ndx]]=1.;

		for(i=ndx+1;i<5;i++)
		{
			A_T_f[strmap[i]]-=A_T_f[strmap[ndx]]*A_T_A[strmap[i]*5+colmap[ndx]];
			for(j=4;j>ndx;j--)
				A_T_A[strmap[i]*5+colmap[j]]-=A_T_A[strmap[ndx]*5+colmap[j]]*A_T_A[strmap[i]*5+colmap[ndx]];
			A_T_A[strmap[i]*5+colmap[ndx]]=0.;
		}

	}

	A_T_f[strmap[4]]/=A_T_A[strmap[4]*5+colmap[4]];
	A_T_A[strmap[4]*5+colmap[4]]=1.;
	x[colmap[4]]=A_T_f[strmap[4]];
	for(i=3;i>=0;--i)
	{
		x[colmap[i]]=A_T_f[strmap[i]];
		for(j=4;j>i;--j)
			x[colmap[i]]-=A_T_A[strmap[i]*5+colmap[j]]*x[colmap[j]];
	}

	x[1]/=2.;
	x[3]/=2.;
	x[4]/=2.;

	delete A;

	return x;
}

bool CAim::toCanonicForm(double* form)
{
	double big_delta,small_delta,S;
	S=form[0]+form[2];//a+c
	small_delta=form[0]*form[2]-form[1]*form[1];//ac-b^2
	big_delta=form[0]*form[2]-form[0]*form[4]*form[4]-form[1]*form[1]-form[3]*form[3]*form[2]+2.*form[1]*form[3]*form[4];
	if(!(small_delta>0. && big_delta*S<0.))
		return false;
	ellipse.x0=(form[1]*form[4]-form[2]*form[3])/small_delta;
	ellipse.y0=(form[1]*form[3]-form[0]*form[4])/small_delta;
	double D=sqrt((form[0]-form[2])*(form[0]-form[2])+4.*form[1]*form[1]);
	double a1=(form[0]+form[2]+D)/2.;
	double c1=(form[0]+form[2]-D)/2.;
	double r1=sqrt(-big_delta/(small_delta*a1));
	double r2=sqrt(-big_delta/(small_delta*c1));
	double Q=atan2(-2*form[1],(form[0]-form[2]))/2;
	ellipse.Q=Q;
	ellipse.xR=r1;
	ellipse.yR=r2;
	return true;
}

int CAim::Decoded()
{
	
	Scanf();
	int i,j;
	int key=0;
	int index=0;
	int razryad=0;
	double sum=0,temp=0;
	double angle;
	double max_font=0;
	double max_bright=0;
	angle=360./(2*mCountOfMetok+3);


	for (i=360;i<720;i++)
	{
		temp=0;
		for (j=-int(angle);j<=int(angle);j++)
		{
            temp+=Hist[(j+i)%360]*(1-(.2/int(angle))*std::abs(j));
		}


		if (temp>sum)
		{
			sum=temp;
			index=i;
		}
	
	}

	index-=360;
	temp=0;
	


	for (i=aline(index-0.425*angle);i<=aline(index+0.425*angle);i++)
	{
		max_bright+=Hist[i%360];
	}
    
	for (razryad=1;razryad<=(2*mCountOfMetok+1);razryad+=2)
	{
		sum=0;
		temp=(double)index+.5*angle+razryad*angle;
		for (i=aline(temp-0.425*angle);i<=aline(temp+0.425*angle);i++)
		{
			sum+=Hist[i%360];
		}
		(sum>max_font) ? max_font=sum:0;		
	
	}
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//
	max_font=max_font+(max_bright-max_font)*.3;
	//(ÓÚ 0 ‰Ó 1.0)

	for (razryad=0;razryad<mCountOfMetok;razryad++)
	{
		sum=0;
		temp=(double)index+2.5*angle+2*razryad*angle;
		for (i=aline(temp-0.425*angle);i<=aline(temp+0.425*angle);i++)
		{
			sum+=Hist[i%360];
		}

		if (sum>max_font) 
		{
				key|=(1<<razryad);	
		}
	
	}



	return key;
}

void CAim::Scanf()
{
	double PI=3.14159265358979323846;
	double x0=ellipse.x0;
	double y0=ellipse.y0;
	double xR=ellipse.xR;
	double yR=ellipse.yR;
	double Q=ellipse.Q;
	double alfa;
	double xR2=xR*xR;
	double yR2=yR*yR;
	double sin_q=sin(Q);
	double cos_q=cos(Q);
	double cos_a,sin_a;
	double r,radius,maxR;
	double dx,dy,x,y;
	int maxR2,i;
	double u,v,br1,br2,br;
	double  data[360],temp;
	memset(data,0,360*sizeof(double));
	(xR>yR) ? maxR=xR:maxR=yR;
	maxR2=aline((a3-a2)*maxR);



	for (int l=0;l<720;l++)
	{
		alfa=atan2(yR*sin(l*PI/360),xR*cos(l*PI/360));		
		cos_a=cos(alfa);
		sin_a=sin(alfa);
		radius=sqrt(xR2*yR2/(yR2*cos_a*cos_a+xR2*sin_a*sin_a));
		
		for (i=1;i<=maxR2;i++)
		{
			r=radius*(a2+i/maxR);	
			dx=r*cos_a;
			dy=r*sin_a;
	
			x=x0+dx*cos_q+dy*sin_q;
			y=y0-dx*sin_q+dy*cos_q;
			u=modf(x,&temp);
			v=modf(y,&temp);
			br1=pShear[int(y)*width4+int(x)]*(1-u)+pShear[int(y)*width4+int(x+1)]*u;
			br2=pShear[int(y+1)*width4+int(x)]*(1-u)+pShear[int(y+1)*width4+int(x+1)]*u;
			br=br1*(1-v)+br2*v;
			data[int(l/2)]+=br;
		}

	}

	
	maxR2*=2;
	for(i=0;i<360;i++)
	{
		Hist[i]=(unsigned char)aline(data[i]/maxR2);	
	}
}

target& CAim::operator[](int i)
{
	return m_pCrd[i];
}

void CAim::Inzhener()
{
    std::vector<CCode>::iterator vStorage_iter;
    vStorage_iter = m_vStorage.begin();
	
    CCode   *cod;
	stPoint *zu;
	double  *form2;

    concentric *rama;
    std::vector<concentric>::iterator vFinal_iter;
    
	while( vStorage_iter != m_vStorage.end() )
	{
        cod = &*vStorage_iter;
        vStorage_iter++;

        zu = ExtractBorder( cod );
		form2 = matrix( zu );
		if(toCanonicForm( form2 ))
		{
            
			double err = CalcError( zu );
			if( err < 3.0 )                                  //œŒ√–≈ÿÕŒ—“‹ 3.0%
			{
                vFinal_iter = m_vFinal.begin();
                bool add = true;

				while( vFinal_iter != m_vFinal.end() )
				{
                    rama = &*vFinal_iter;
                    vFinal_iter++;

					if( ( rama->rc.top    >= cod->rc.top    ) &&
                        ( rama->rc.bottom <= cod->rc.bottom ) &&
					    ( rama->rc.left   >= cod->rc.left   ) &&
                        ( rama->rc.right  <= cod->rc.right  )
                      )
					{
						add = false;
						if(rama->err > err)
						{
							rama->err     = err;
							rama->rc      = cod->rc;
							rama->ellipse = ellipse;
						}
							goto step;
					}
				}

				if( add )
				{
                    m_vFinal.push_back( concentric( &cod->rc, &ellipse, err ));
					mCountOfAim++;
				}
			}
	step:;
		}
		delete zu;
	}

}

double CAim::CalcError(stPoint *pnt)
{

	double xR=ellipse.xR;
	double yR=ellipse.yR;
	double x0=ellipse.x0;
	double y0=ellipse.y0;
	double Q=ellipse.Q;
	double sin_q=sin(Q);
	double cos_q=cos(Q);
	double radius;
	double xR2=xR*xR;
	double yR2=yR*yR;
	double temp,err;
	double dx,dy;
	double alfa;
	double cos_a,sin_a;

	err=0;

	for (int i=0;i<mCountOfPoints;i++)
	{
		dx=pnt[i].x-x0;
		dy=pnt[i].y-y0;
		alfa=atan2(sin_q*dx+cos_q*dy,cos_q*dx-sin_q*dy);
		cos_a=cos(alfa);
		sin_a=sin(alfa);
		radius=sqrt(xR2*yR2/(yR2*cos_a*cos_a+xR2*sin_a*sin_a));
		temp=sqrt(dx*dx+dy*dy)/radius-1;
		err+=temp*temp;
	}

	err=100.*sqrt(err/mCountOfPoints);

	return err;


}
