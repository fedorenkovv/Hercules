////
////  HerculesView.m
////  HerculesRemoteControl
////
////  Created by Vladimir Knyaz on 23.02.16.
////  Copyright © 2016 Vladimir Knyaz. All rights reserved.
////
//
//#include "HerculesView.h"
//#include <OpenGL/gl.h>
//
//#include "IMImageFile.h"
//#include "IMLibTypes.h"
//#include "IMTIFFImageFileFormat.h"
//#include "IMVideoClient.h"
//
//IMImageFile *m_image;
//
//GLuint m_textureName = 0;
//
//GLfloat m_vertices[4][2];
//GLfloat m_textureCoords[4][2];
//
//@implementation HerculesView
//
//- (void)awakeFromNib
//{
//    IMRegion m_region;
//    
//    m_region.size.width = 1.0;
//    m_region.size.height = 1.0;
//    m_region.origin.x = 0;
//    m_region.origin.y = 0;
//    
//    m_vertices[0][0] = m_region.origin.x;
//    m_vertices[0][1] = m_region.origin.y;
//    
//    m_vertices[1][0] = m_region.origin.x;
//    m_vertices[1][1] = m_region.origin.y + m_region.size.height;
//    
//    m_vertices[2][0] = m_region.origin.x + m_region.size.width;
//    m_vertices[2][1] = m_region.origin.y + m_region.size.height;
//    
//    m_vertices[3][0] = m_region.origin.x + m_region.size.width;
//    m_vertices[3][1] = m_region.origin.y;
//}
//
//- (void)setImage:(unsigned char*) image
//{
//    if (m_textureName == 0)
//    {
//        glEnable(GL_TEXTURE_2D);
//        glGenTextures(1, &m_textureName);
//        glBindTexture(GL_TEXTURE_2D, m_textureName);
//        
//        glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
//        glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
//        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
//        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
//        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
//        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//        
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    }
//    
//    IMImageFile *m_image = reinterpret_cast<IMImageFile*>(image);
//    unsigned char *pixelData = m_image->image();
//    glEnable(GL_TEXTURE_2D);
//    glBindTexture(GL_TEXTURE_2D, m_textureName);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image->pixelsWide(), m_image->pixelsHigh(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixelData);
//}
//
//- (void)drawRect:(NSRect)dirtyRect {
//    [super drawRect:dirtyRect];
//    
//    // Drawing code here.
//    glClearColor(0, 0, 0, 0);
//    
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    
//    glViewport(dirtyRect.origin.x, dirtyRect.origin.y, dirtyRect.size.width, dirtyRect.size.height);
//    
//    glEnable(GL_TEXTURE_2D);
//    
//    glBindTexture(GL_TEXTURE_2D, m_textureName);
//    
//    glVertexPointer(2, GL_FLOAT, 0, m_vertices);
//    glTexCoordPointer(2, GL_FLOAT, 0, m_textureCoords);
//    
//    glEnableClientState(GL_VERTEX_ARRAY);
//    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//    
//    glDrawArrays(GL_POLYGON, 0, 4);
//    
//    glDisableClientState(GL_VERTEX_ARRAY);
//    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//    
//    glFlush();
//}
//
//@end
