/////////////////////////////////////////////////////////////////////////////
// Name:        AppGLCanvas.h
// Purpose:     wxGLCanvas demo program
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _APP_GLCAVAS_
#define _APP_GLCAVAS_

#include "IMLib.h"

#include "wx/glcanvas.h"
#include "wx/image.h"
#include "wx/timer.h"
#include "AppGLContext.h"
#include "IMImageView.h"
#include "IMImageFile.h"
#include "ArgusDialog.h"
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

class MyApp : public wxApp
{
public:
	// the GL context we use for all our mono rendering windows
	AppGLContext *m_glContext;

	MyApp()
	{
		m_glContext = NULL;
	};
	virtual bool OnInit();
	virtual int OnExit();
	// Returns the shared context used by all frames and sets it as current for
	// the given canvas.
	AppGLContext& GetContext(wxGLCanvas *canvas, bool useStereo)
	{
		AppGLContext *glContext;
		{
			if (!m_glContext)
			{
				// Create the OpenGL context for the first mono window which needs it:
				// subsequently created windows will all share the same context.
				m_glContext = new AppGLContext(canvas);
			}
			glContext = m_glContext;
		}

		glContext->SetCurrent(*canvas);

		return *glContext;
	}
};


//------------------------------------
// режимы отрисовки
//------------------------------------
// режим улучшенного видения
static const int AS_EVS_DRAW_MODE = 0;
// режим синтезированного видения
static const int AS_SVS_DRAW_MODE = 1;
// режим комбинированного видения
static const int AS_CVS_DRAW_MODE = 2;
// режим ИЛС
static const int AS_HUD_DRAW_MODE = 3;

class IMImageFile;

//------------------------------------
// класс AppGLCanvas - отрисовка кадров систем SVS, EVS, CVS, режима моделирования ИЛС
//------------------------------------
class AppGLCanvas : public wxGLCanvas
{
public:
	AppGLCanvas(wxWindow *parent, int *attribList = NULL);
	~AppGLCanvas()
	{
		//glDeleteTextures(1, &m_textureName);
	}

	// устанавливает систему SVS
	/*void setSVS(ASSVS *svs)
	{
		m_svs = svs;
	}*/

	void setDrawMode(int drawMode)
	{
		m_drawMode = drawMode;
	}

	void setNeedsToDrawRunway(bool needsToDrawRunway)
	{
		m_needsToDrawRunway = needsToDrawRunway;
	}

	void setNeedsToDrawPFD(bool needsToDrawPFD)
	{
		m_needsToDrawPFD = needsToDrawPFD;
	}

	/*void setNavData(NAV_PARSER_GPS_DATA *navData)
	{
		m_navData = navData;
	}

	void setFlightData(PIL_RAW_DATA *flightData)
	{
		m_flightData = flightData;
	}*/

	void setMutex(boost::mutex *mutex)
	{
		m_mutex = mutex;
	}

	int drawMode() const
	{
		return m_drawMode;
	}

	void setVisibleSubviews(vector<int> visibleSubviewsIndices)
	{
		m_visibleSubviewsIndices = visibleSubviewsIndices;
	}

	// установка изображений
	void setImages(unsigned char* image0, unsigned char* image1, unsigned char* image2, unsigned char* image3, unsigned char* image4, int width, int height);

	// установка изображения
	void setImage(int imageN, unsigned char* image0, int width, int height, int samplesPerPixel);

	// рассчёт координат в курсора мыши в подобласти
	void mouseState(ArgusMouseState& state);

	// пересчёт из экранных координат в координаты подобласти
	void mouseScreenToView(ArgusMousePosition& state, bool global = true);

private:
	void OnPaint(wxPaintEvent& event);
	void Spin(float xSpin, float ySpin);
	void OnKeyDown(wxKeyEvent& event);
	void OnMouseLeftDown(wxMouseEvent& event);
	void OnMouseRightDown(wxMouseEvent& event);
	void OnMouseMidDown(wxMouseEvent& event);
	void OnDrawTimer(wxTimerEvent& WXUNUSED(event));

	void updateViewsLayout();
	int  choiceObjectClass();

	// область просмотра камер
	IMView m_cameraView;

	// массив подобластей просмотра камер
	vector<IMImageView*> m_cameraViews;

	// массив изображений с камер
	vector<IMImageFile*> m_cameraImages;

	// номер видимых подобластей
	vector<int> m_visibleSubviewsIndices;

	IMImageFile *m_image;

	// отрисовка ВПП
	void drawRunway();

	// инициализация текстур
	void initTextures();

	// инициализирована ли SVS
	bool m_isSVSInit;

	// режим отрисовки
	int m_drawMode;

	// ширина изображения
	int m_imageWidth;
	// высота изображения
	int m_imageHeight;

	double m_viewportX;
	double m_viewportY;
	double m_viewportWidth;
	double m_viewportHeight;

	int m_rows;
	int m_cols;
	double m_viewWidth;
	double m_viewHeight;

	ArgusMousePosition m_clickPosition;


	// переключение между полноэкранным режимом
	bool m_isFullscreen;

	int m_fullScreenIndex;
	IMRegion m_fullScreenRegion;

	// определяет нужно ли рисовать распознанные полосы
	bool m_needsToDrawRunway;

	// определяет нужно ли рисовать шкалы
	bool m_needsToDrawPFD;

	// индентификатор текстуры для отрисовки изображения
	GLuint m_textureName;

	// индентификатор текстуры для отрисовки изображения закабинной обсатновки
	GLuint m_cockpitViewTextureName;

	// индентификатор текстуры для отрисовки изображения закабинной обсатновки
	GLuint m_HUDGlassTextureName;

	// angles of rotation around x- and y- axis
	float m_xangle,
		m_yangle;
	
	wxTimer m_drawTimer;
	bool m_useStereo,
		m_stereoWarningAlreadyDisplayed;

	// изображение - стекло ИЛС
	wxImage *m_HUDGlassImageLoader;

	unsigned char *m_HUDGlassImage;

	boost::mutex *m_mutex;

	wxDECLARE_EVENT_TABLE();
};

wxString glGetwxString(GLenum name);

#endif // _APP_GLCAVAS_
