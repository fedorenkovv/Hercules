#ifndef EXR2FLO_H
#define EXR2FLO_H


#include <string>

#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"


#include "..\flow-code\flowIO.h"
#include "..\flow-code\imageLib\imageLib.h"
#include "..\flow-code\colorcode.h"

// используем пространстав имён cv и std
using namespace cv;
using namespace std;

void MotionToColor(CFloatImage motim, CByteImage &colim, float maxmotion);

class Converter_
{
public:
	Converter_(){};
	void exr2flo(const char* exrFile, const char* outFlo);
	void flo2png(const char* floFile, const char* outPng);
	float m_maxMotion;
};

#endif // !EXR2FLO_H
