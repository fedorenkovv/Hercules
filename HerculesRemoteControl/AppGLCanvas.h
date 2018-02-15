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
// ������ ���������
//------------------------------------
// ����� ����������� �������
static const int AS_EVS_DRAW_MODE = 0;
// ����� ���������������� �������
static const int AS_SVS_DRAW_MODE = 1;
// ����� ���������������� �������
static const int AS_CVS_DRAW_MODE = 2;
// ����� ���
static const int AS_HUD_DRAW_MODE = 3;

class IMImageFile;

//------------------------------------
// ����� AppGLCanvas - ��������� ������ ������ SVS, EVS, CVS, ������ ������������� ���
//------------------------------------
class AppGLCanvas : public wxGLCanvas
{
public:
	AppGLCanvas(wxWindow *parent, int *attribList = NULL);
	~AppGLCanvas()
	{
		//glDeleteTextures(1, &m_textureName);
	}

	// ������������� ������� SVS
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

	// ��������� �����������
	void setImages(unsigned char* image0, unsigned char* image1, unsigned char* image2, unsigned char* image3, unsigned char* image4, int width, int height);

	// ��������� �����������
	void setImage(int imageN, unsigned char* image0, int width, int height, int samplesPerPixel);

	// ������� ��������� � ������� ���� � ����������
	void mouseState(ArgusMouseState& state);

	// �������� �� �������� ��������� � ���������� ����������
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

	// ������� ��������� �����
	IMView m_cameraView;

	// ������ ����������� ��������� �����
	vector<IMImageView*> m_cameraViews;

	// ������ ����������� � �����
	vector<IMImageFile*> m_cameraImages;

	// ����� ������� �����������
	vector<int> m_visibleSubviewsIndices;

	IMImageFile *m_image;

	// ��������� ���
	void drawRunway();

	// ������������� �������
	void initTextures();

	// ���������������� �� SVS
	bool m_isSVSInit;

	// ����� ���������
	int m_drawMode;

	// ������ �����������
	int m_imageWidth;
	// ������ �����������
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


	// ������������ ����� ������������� �������
	bool m_isFullscreen;

	int m_fullScreenIndex;
	IMRegion m_fullScreenRegion;

	// ���������� ����� �� �������� ������������ ������
	bool m_needsToDrawRunway;

	// ���������� ����� �� �������� �����
	bool m_needsToDrawPFD;

	// �������������� �������� ��� ��������� �����������
	GLuint m_textureName;

	// �������������� �������� ��� ��������� ����������� ���������� ����������
	GLuint m_cockpitViewTextureName;

	// �������������� �������� ��� ��������� ����������� ���������� ����������
	GLuint m_HUDGlassTextureName;

	// angles of rotation around x- and y- axis
	float m_xangle,
		m_yangle;
	
	wxTimer m_drawTimer;
	bool m_useStereo,
		m_stereoWarningAlreadyDisplayed;

	// ����������� - ������ ���
	wxImage *m_HUDGlassImageLoader;

	unsigned char *m_HUDGlassImage;

	boost::mutex *m_mutex;

	wxDECLARE_EVENT_TABLE();
};

wxString glGetwxString(GLenum name);

#endif // _APP_GLCAVAS_
