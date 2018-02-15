/////////////////////////////////////////////////////////////////////////////
// Name:        AppUtility.h
// Purpose:     wxGLCanvas demo program
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _APP_GLCONTEXT_
#define _APP_GLCONTEXT_

#include "wx/glcanvas.h"
//#include "ASSVS.h"

// the rendering context used by all GL canvases
class AppGLContext : public wxGLContext
{
public:
    AppGLContext(wxGLCanvas *canvas);

    // render the cube showing it at given angles
    void DrawRotatedCube(float xangle, float yangle);

private:
    // textures for the cube faces
    GLuint m_textures[6];
};

wxString glGetwxString(GLenum name);

#endif // _APP_GLCONTEXT_
