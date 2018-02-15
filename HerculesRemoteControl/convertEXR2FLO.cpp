#include "convertEXR2FLO.h"


void Converter_::exr2flo(const char* exrFile_c, const char* outFlo_c)
{
	string exrFile = exrFile_c;
	string outFlo = outFlo_c;

	// загружаем изображения exr
	Mat image = imread(exrFile.c_str(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);

	int nBands = 2;

	CShape sh(image.cols, image.rows, nBands);
	CFloatImage img;
	img.ReAllocate(sh);
	// заполняем пиксели
	for (int i = 0; i < image.rows; i++)
	for (int j = 0; j < image.cols; j++)
	{
		Vec3f val = image.at<Vec3f>(i, j);
		img.Pixel(j, i, 0) = val.val[0];
		img.Pixel(j, i, 1) = val.val[1];
	}
	WriteFlowFile(img, outFlo.c_str());

}

void Converter_::flo2png(const char* floFile_c, const char* outPng_c)
{
		string floFile = floFile_c;
		string outPng = outPng_c;

		CFloatImage im;
		ReadFlowFile(im, floFile.c_str());
		CByteImage outim;
		CShape sh = im.Shape();
		sh.nBands = 3;
		outim.ReAllocate(sh);
		outim.ClearPixels();
		MotionToColor(im, outim, m_maxMotion);
		WriteImageVerb(outim, /*outPng.c_str()*/outPng_c, 1);

}

void MotionToColor(CFloatImage motim, CByteImage &colim, float maxmotion)
{
	CShape sh = motim.Shape();
	int width = sh.width, height = sh.height;
	colim.ReAllocate(CShape(width, height, 3));
	int x, y;
	// determine motion range:
	float maxx = -999, maxy = -999;
	float minx = 999, miny = 999;
	float maxrad = -1;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			float fx = motim.Pixel(x, y, 0);
			float fy = motim.Pixel(x, y, 1);
			if (unknown_flow(fx, fy))
				continue;
			maxx = __max(maxx, fx);
			maxy = __max(maxy, fy);
			minx = __min(minx, fx);
			miny = __min(miny, fy);
			float rad = sqrt(fx * fx + fy * fy);
			maxrad = __max(maxrad, rad);
		}
	}
	//printf("max motion: %.4f  motion range: u = %.3f .. %.3f;  v = %.3f .. %.3f\n",
	//	maxrad, minx, maxx, miny, maxy);


	if (maxmotion > 0) // i.e., specified on commandline
		maxrad = maxmotion;

	if (maxrad == 0) // if flow == 0 everywhere
		maxrad = 1;

	if (0)
		fprintf(stderr, "normalizing by %g\n", maxrad);

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			float fx = motim.Pixel(x, y, 0);
			float fy = motim.Pixel(x, y, 1);
			uchar *pix = &colim.Pixel(x, y, 0);
			if (unknown_flow(fx, fy)) {
				pix[0] = pix[1] = pix[2] = 0;
			}
			else {
				computeColor(fx / maxrad, fy / maxrad, pix);
			}
		}
	}
}
