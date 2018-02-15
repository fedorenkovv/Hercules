// HERCULESDLL.h

#ifdef HERCULESDLL_EXPORTS
#define HERCULESDLL_API __declspec(dllexport) 
#else
#define HERCULESDLL_API __declspec(dllimport) 
#endif

#include <opencv2/imgproc/imgproc.hpp>

namespace namespace_hercules
{
	// класс,реализующий управление роботом
	class HerculesController
	{
	public:
		// выполнить один шаг управления
		// вход синхронная пара кадров tv, ir
		// выход вектор управлеия Point2f [x, y]
		// x = (Pl + Pr)/2	-- газ
		// y = Pl - Pr		-- поворот
		static __declspec(dllexport) cv::Point2f step(cv::Mat tv, cv::Mat ir);

	};
}



////////////////////////////////

