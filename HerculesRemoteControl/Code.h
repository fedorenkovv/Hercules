#ifndef __CODE__
#define __CODE__

struct stRect
{
    long    left;
    long    top;
    long    right;
    long    bottom;
};

struct stPoint
{
    long  x;
    long  y;
};

class CCode  
{
public:
    CCode();
    CCode( stRect         *rect,
           stPoint        *point,
           unsigned short  n
         );
    ~CCode();
    
    stRect         rc;
	stPoint        pnt;
	unsigned short num;
};

#endif // __CODE__
