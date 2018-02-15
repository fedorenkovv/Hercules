#include "Code.h"

CCode::CCode()
{
}

CCode::CCode( stRect         *rect,
              stPoint        *point,
              unsigned short  n
            )
{
	num = n;
	pnt = *point;
	rc  = *rect;
}

CCode::~CCode()
{
}
