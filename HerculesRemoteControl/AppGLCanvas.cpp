///////////////////////////////////////////////////////////////////////////////
// Name:        AppGLCanvas.cpp
// Purpose:     wxGLCanvas demo program
// Author:      Julian Smart
// Modified by: Vadim Zeitlin to use new wxGLCanvas API (2007-04-09)
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"
#include <wx/stdpaths.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_GLCANVAS
    #error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

#include "AppGLCanvas.h"
#include "IMLib.h"
#include "IMImageView.h"
#include <iomanip>

#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "../../sample.xpm"
#endif

// для возможности использовать wxGetApp
DECLARE_APP(MyApp)

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// control ids
enum
{
    DrawTimer = wxID_HIGHEST + 1
};

// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

static void CheckGLError()
{
    GLenum errLast = GL_NO_ERROR;

    for ( ;; )
    {
        GLenum err = glGetError();
        if ( err == GL_NO_ERROR )
            return;

        // normally the error is reset by the call to glGetError() but if
        // glGetError() itself returns an error, we risk looping forever here
        // so check that we get a different error than the last time
        if ( err == errLast )
        {
            wxLogError(wxT("OpenGL error state couldn't be reset."));
            return;
        }

        errLast = err;

        wxLogError(wxT("OpenGL error %d"), err);
    }
}

// ----------------------------------------------------------------------------
// AppGLCanvas
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(AppGLCanvas, wxGLCanvas)
EVT_PAINT(AppGLCanvas::OnPaint)
EVT_KEY_DOWN(AppGLCanvas::OnKeyDown)
EVT_LEFT_DOWN(AppGLCanvas::OnMouseLeftDown)
EVT_MIDDLE_DOWN(AppGLCanvas::OnMouseMidDown)
EVT_RIGHT_DOWN(AppGLCanvas::OnMouseRightDown)
EVT_TIMER(DrawTimer, AppGLCanvas::OnDrawTimer)
wxEND_EVENT_TABLE()

AppGLCanvas::AppGLCanvas(wxWindow *parent, int *attribList)
// With perspective OpenGL graphics, the wxFULL_REPAINT_ON_RESIZE style
// flag should always be set, because even making the canvas smaller should
// be followed by a paint event that updates the entire canvas with new
// viewport settings.
: wxGLCanvas(parent, wxID_ANY, attribList,
wxDefaultPosition, wxDefaultSize,
wxFULL_REPAINT_ON_RESIZE),
m_xangle(30.0),
m_yangle(30.0),
m_drawTimer(this, DrawTimer),
m_useStereo(false),
m_isSVSInit(false),
m_drawMode(AS_EVS_DRAW_MODE),
m_textureName(-1),
m_needsToDrawRunway(false),
m_needsToDrawPFD(false),
m_stereoWarningAlreadyDisplayed(false),
m_isFullscreen(true),
m_imageWidth(0),
m_imageHeight(0),
m_viewportX(0.0),
m_viewportY(0.0),
m_viewportWidth(1.0),
m_viewportHeight(1.0)
{
	if (attribList)
	{
		int i = 0;
		while (attribList[i] != 0)
		{
			if (attribList[i] == WX_GL_STEREO)
				m_useStereo = true;
			++i;
		}
	}
}

// рассчёт координат в курсора мыши в подобласти
void AppGLCanvas::mouseScreenToView(ArgusMousePosition& state, bool global)
{
	wxPoint framePnt, pnt;

	pnt.x = state.x;
	pnt.y = state.y;

	if (global)
	{
		framePnt = GetScreenPosition();
		pnt = pnt - framePnt;
	}
	
	state.viewX = pnt.x - m_viewportX;
	state.viewY = pnt.y - m_viewportY;


	// переходим в систему координат viewport
	state.viewX /= m_viewportWidth;
	state.viewY /= m_viewportHeight;

	int curRow, curCol;
	curCol = floor(state.viewX / m_viewWidth);
	curRow = floor(state.viewY / m_viewHeight);

	state.activeView = curCol + curRow * m_cols;

	// переходим в систему координат вида
	state.viewX -= m_viewWidth * curCol;
	state.viewX *= m_cols;

	state.viewY -= m_viewHeight * curRow;
	state.viewY *= m_rows;

	if (state.activeView < 0 || state.activeView > m_visibleSubviewsIndices.size() - 1)
	{
		state.activeView = -1;
		state.viewX = -1.0;
		state.viewY = -1.0;
	}
}

// рассчёт координат в курсора мыши в подобласти
void AppGLCanvas::mouseState(ArgusMouseState& state)
{
	mouseScreenToView(state.active);

	if (m_clickPosition.x != -1)
	{
		state.click = m_clickPosition;
		mouseScreenToView(state.click, false);
	}

	m_clickPosition.x = -1;
	m_clickPosition.y = -1;
}

// установка изображений
void AppGLCanvas::setImages(unsigned char* image0, unsigned char* image1, unsigned char* image2, unsigned char* image3, unsigned char* image4, int width, int height)
{
	m_visibleSubviewsIndices.clear();

	IMImageView *pView = 0;

	const double third = 1.0 / 3.0;
	double llx, lly;
	int curRow, curCol;

	if ((m_imageWidth != width || m_imageHeight != height) && !m_cameraImages.empty())
	{
		int j;
		for (j = 0; j < 5; j++)
		{
			delete m_cameraImages[j];
			delete m_cameraViews[j];
		}
		m_cameraImages.clear();
		m_cameraViews.clear();
        m_cameraView.clearSubviews();
	}	

    if (m_cameraImages.empty() && m_cameraViews.size() == 0)
	{
		int j;
		for (j = 0; j < 5; j++)
		{
			IMImageFile *pImage = new IMImageFile();
			m_cameraImages.push_back(pImage);

			pView = new IMImageView();
			pView->setImage(pImage);
			m_cameraViews.push_back(pView);
			m_cameraView.addSubview(pView);
		}
		
		m_imageWidth = width;
		m_imageHeight = height;
	}

	if (image0 != 0)
	{
		m_cameraImages[0]->setPixelsWide(width);
		m_cameraImages[0]->setPixelsHigh(height);
		m_cameraImages[0]->setImage(image0);
		m_cameraImages[0]->setSamplesPerPixel(3);
		m_cameraViews[0]->setImage(m_cameraImages[0]);
		m_visibleSubviewsIndices.push_back(0);
	}
	if (image1 != 0)
	{
		m_cameraImages[1]->setPixelsWide(width);
		m_cameraImages[1]->setPixelsHigh(height);
		m_cameraImages[1]->setImage(image1);
		m_cameraViews[1]->setImage(m_cameraImages[1]);
		m_visibleSubviewsIndices.push_back(1);
	}
	if (image2 != 0)
	{
		m_cameraImages[2]->setPixelsWide(width);
		m_cameraImages[2]->setPixelsHigh(height);
		m_cameraImages[2]->setImage(image2);
		m_cameraViews[2]->setImage(m_cameraImages[2]);
		m_visibleSubviewsIndices.push_back(2);
	}
	if (image3 != 0)
	{
		m_cameraImages[3]->setPixelsWide(width);
		m_cameraImages[3]->setPixelsHigh(height);
		m_cameraImages[3]->setImage(image3);
		m_cameraViews[3]->setImage(m_cameraImages[3]);
		m_visibleSubviewsIndices.push_back(3);
	}
	if (image4 != 0)
	{
		m_cameraImages[4]->setPixelsWide(width);
		m_cameraImages[4]->setPixelsHigh(height);
		m_cameraImages[4]->setImage(image4);
		m_cameraViews[4]->setImage(m_cameraImages[4]);
		m_visibleSubviewsIndices.push_back(4);
	}

	// определяем размеры описывающего прямоугольника для всех подобластей
	switch (m_visibleSubviewsIndices.size())
	{
	case 1:
		m_rows = 1;
		m_cols = 1;
		break;
	case 2:
		m_rows = 1;
		m_cols = 2;
		break;
	case 3:
		m_rows = 2;
		m_cols = 2;
		break;
	case 4:
		m_rows = 2;
		m_cols = 2;
		break;
	case 5:
		m_rows = 2;
		m_cols = 3;
		break;
	default:
		m_rows = 1;
		m_cols = 1;
		break;
	}

	curRow = 0;
	curCol = 0;
	m_viewWidth = 1.0;
	m_viewWidth /= m_cols;
	m_viewHeight = 1.0;
	m_viewHeight /= m_rows;
	llx = 0;
	lly = 1.0 - m_viewHeight;

	auto iter = m_cameraViews.begin();

	for (iter = m_cameraViews.begin(); iter != m_cameraViews.end(); iter++)
	{
		(*iter)->setIsVisible(false);
	}

	auto iter2 = m_visibleSubviewsIndices.begin();

	for (iter2 = m_visibleSubviewsIndices.begin(); iter2 != m_visibleSubviewsIndices.end(); iter2++)
	{
		int j = *iter2;

		m_cameraViews[j]->setRegion(IMRegion(llx, lly, m_viewWidth, m_viewHeight));
		m_cameraViews[j]->setIsVisible(true);

		curCol++;
		llx += m_viewWidth;
		if (curCol == m_cols)
		{
			llx = 0;
			curCol = 0;
			curRow++;
			lly -= m_viewHeight;
		}
	}

	Refresh();
}

// установка изображений
void AppGLCanvas::setImage(int imageN, unsigned char* image0, int width, int height, int samplesPerPixel)
{
	m_visibleSubviewsIndices.clear();

	IMImageView *pView = 0;

	const double third = 1.0 / 3.0;
	double llx, lly;
	int curRow, curCol;

	if ((m_imageWidth != width || m_imageHeight != height) && !m_cameraImages.empty())
	{
		int j;
		for (j = 0; j < 5; j++)
		{
			delete m_cameraImages[j];
			delete m_cameraViews[j];
		}
		m_cameraImages.clear();
		m_cameraViews.clear();
		m_cameraView.clearSubviews();
	}

	if (m_cameraImages.empty() && m_cameraViews.size() == 0)
	{
		int j;
		for (j = 0; j < 5; j++)
		{
			IMImageFile *pImage = new IMImageFile();
			m_cameraImages.push_back(pImage);

			pView = new IMImageView();
			pView->setImage(pImage);
			m_cameraViews.push_back(pView);
			m_cameraView.addSubview(pView);
		}

		m_imageWidth = width;
		m_imageHeight = height;
	}

	if (image0 != 0)
	{
		m_cameraImages[imageN]->setPixelsWide(width);
		m_cameraImages[imageN]->setPixelsHigh(height);
		m_cameraImages[imageN]->setImage(image0);
		m_cameraImages[imageN]->setSamplesPerPixel(samplesPerPixel);
		m_cameraViews[imageN]->setImage(m_cameraImages[imageN]);
		m_visibleSubviewsIndices.push_back(0);
	}

	// определяем размеры описывающего прямоугольника для всех подобластей
	switch (m_visibleSubviewsIndices.size())
	{
	case 1:
		m_rows = 1;
		m_cols = 1;
		break;
	case 2:
		m_rows = 1;
		m_cols = 2;
		break;
	case 3:
		m_rows = 2;
		m_cols = 2;
		break;
	case 4:
		m_rows = 2;
		m_cols = 2;
		break;
	case 5:
		m_rows = 2;
		m_cols = 3;
		break;
	default:
		m_rows = 1;
		m_cols = 1;
		break;
	}

	curRow = 0;
	curCol = 0;
	m_viewWidth = 1.0;
	m_viewWidth /= m_cols;
	m_viewHeight = 1.0;
	m_viewHeight /= m_rows;
	llx = 0;
	lly = 1.0 - m_viewHeight;

	auto iter = m_cameraViews.begin();

	for (iter = m_cameraViews.begin(); iter != m_cameraViews.end(); iter++)
	{
		(*iter)->setIsVisible(false);
	}

	auto iter2 = m_visibleSubviewsIndices.begin();

	for (iter2 = m_visibleSubviewsIndices.begin(); iter2 != m_visibleSubviewsIndices.end(); iter2++)
	{
		int j = *iter2;

		m_cameraViews[j]->setRegion(IMRegion(llx, lly, m_viewWidth, m_viewHeight));
		m_cameraViews[j]->setIsVisible(true);

		curCol++;
		llx += m_viewWidth;
		if (curCol == m_cols)
		{
			llx = 0;
			curCol = 0;
			curRow++;
			lly -= m_viewHeight;
		}
	}

	Refresh();
}



void AppGLCanvas::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	// This is required even though dc is not used otherwise.
	wxPaintDC dc(this);

	// Set the OpenGL viewport according to the client size of this canvas.
	// This is done here rather than in a wxSizeEvent handler because our
	// OpenGL rendering context (and thus viewport setting) is used with
	// multiple canvases: If we updated the viewport in the wxSizeEvent
	// handler, changing the size of one canvas causes a viewport setting that
	// is wrong when next another canvas is repainted.
	const wxSize ClientSize = GetClientSize();

	AppGLContext& canvas = wxGetApp().GetContext(this, m_useStereo);

	int totalWidth, totalHeight, imageWidth, imageHeight;
	float x, y, width, height, xHUD, yHUD, widthHUD, heightHUD, windowAspectRatio, totalAspectRatio;

    if (m_cameraImages.empty())
	{
		return;
	}

	// определяем размеры описывающего прямоугольника для всех подобластей
	switch (m_visibleSubviewsIndices.size())
	{
		case 1:
			totalWidth = m_imageWidth;
			totalHeight = m_imageHeight;
			break;
		case 2:
			totalWidth = m_imageWidth * 2;
			totalHeight = m_imageHeight;
			break;
		case 3:
			totalWidth = m_imageWidth * 2;
			totalHeight = m_imageHeight * 2;
			break;
		case 4:
			totalWidth = m_imageWidth * 2;
			totalHeight = m_imageHeight * 2;
			break;
		case 5:
			totalWidth = m_imageWidth * 3;
			totalHeight = m_imageHeight * 2;
			break;
		default:
			totalWidth = 0;
			totalHeight = 0;
			break;
	}

	windowAspectRatio = ClientSize.x;
	windowAspectRatio /= ClientSize.y;

	totalAspectRatio = totalWidth;
	if (totalAspectRatio > 0.0)
	{
		totalAspectRatio /= totalHeight;
	}
	else
	{
		totalAspectRatio = 1.0;
	}

	// определяем размеры и положение области отрисовки
	if (totalAspectRatio <= windowAspectRatio)
	{
		m_viewportHeight = ClientSize.y;
		m_viewportY = 0.0;
		m_viewportWidth = ClientSize.y * totalAspectRatio;
		m_viewportX = (ClientSize.x - m_viewportWidth) / 2.0;
	}
	else
	{
		m_viewportHeight = ClientSize.x / totalAspectRatio;
		m_viewportY = (ClientSize.y - m_viewportHeight) / 2.0;
		m_viewportWidth = ClientSize.x;
		m_viewportX = 0.0;
	}
	
	// настраиваем размер области отрисовки
	glViewport(m_viewportX, m_viewportY, m_viewportWidth, m_viewportHeight);

	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_COLOR_MATERIAL);

	// настраиваем матрицу проекции
	glMatrixMode(GL_PROJECTION);
	glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

	// настраиваем матрицу модели и положения камеры
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.0f, -1.0f, 0.0f);
	glScalef(2.0f, 2.0f, 2.0f);
	
	// отрисовываем картинки со всех камер
	m_cameraView.draw();

	// меняем буффер OpenGL
	SwapBuffers();
}

void AppGLCanvas::Spin(float xSpin, float ySpin)
{
	m_xangle += xSpin;
	m_yangle += ySpin;

	Refresh(false);
}

int AppGLCanvas::choiceObjectClass()
{
	const wxString objectChoices[] =
	{
		wxT("0 - calibration"),
		wxT("1"),
		wxT("2"),
		wxT("3"),
		wxT("4"),
		wxT("5"),
	};

	int object = -1;

	while (object == -1)
		object = wxGetSingleChoiceIndex(wxT("Set object class"),
			wxT("Options"),
			WXSIZEOF(objectChoices),
			objectChoices,
			this);

	return object;
}

void AppGLCanvas::OnMouseLeftDown(wxMouseEvent& event)
{
	
	m_clickPosition.objectClass = choiceObjectClass();

	m_clickPosition.x = event.m_x;
	m_clickPosition.y = event.m_y;
	m_clickPosition.nButton = 0;
}

void AppGLCanvas::OnMouseRightDown(wxMouseEvent& event)
{
	m_clickPosition.x = event.m_x;
	m_clickPosition.y = event.m_y;
	m_clickPosition.nButton = 1;
}


void AppGLCanvas::OnMouseMidDown(wxMouseEvent& event)
{
	static bool firstClk = true;
	if (!firstClk){
		m_clickPosition.objectClass = choiceObjectClass();
		firstClk = !firstClk;
	}
	else firstClk = !firstClk;

	m_clickPosition.x = event.m_x;
	m_clickPosition.y = event.m_y;
	m_clickPosition.nButton = 2;

}

void AppGLCanvas::OnKeyDown(wxKeyEvent& event)
{
	float angle = 5.0;

	switch (event.GetKeyCode())
	{
	case WXK_RIGHT:
		Spin(0.0, -angle);
		break;

	case WXK_LEFT:
		Spin(0.0, angle);
		break;

	case WXK_DOWN:
		Spin(-angle, 0.0);
		break;

	case WXK_UP:
		Spin(angle, 0.0);
		break;

	case WXK_SPACE:
		if (m_drawTimer.IsRunning())
			m_drawTimer.Stop();
		else
			m_drawTimer.Start(25);
		break;

	default:
		event.Skip();
		return;
	}
}

void AppGLCanvas::OnDrawTimer(wxTimerEvent& WXUNUSED(event))
{
	//Refresh();
}

// инициализация текстур
void AppGLCanvas::initTextures()
{
	glGenTextures(1, &m_textureName);
	glBindTexture(GL_TEXTURE_2D, m_textureName);

	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// закабинная обстановка
	glGenTextures(1, &m_cockpitViewTextureName);
	glBindTexture(GL_TEXTURE_2D, m_cockpitViewTextureName);

	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// стекло ИЛС
	glGenTextures(1, &m_HUDGlassTextureName);
	glBindTexture(GL_TEXTURE_2D, m_HUDGlassTextureName);

	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (m_HUDGlassImageLoader->IsOk() && m_HUDGlassTextureName != -1)
	{
		unsigned char *pixelData = m_HUDGlassImage;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_HUDGlassImageLoader->GetWidth(), m_HUDGlassImageLoader->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
	}
}

wxString glGetwxString(GLenum name)
{
	const GLubyte *v = glGetString(name);
	if (v == 0)
	{
		// The error is not important. It is GL_INVALID_ENUM.
		// We just want to clear the error stack.
		glGetError();

		return wxString();
	}

	return wxString((const char*)v);
}