/*
 * Copyright (C) 2002 Jens Thiele <karme@berlios.de>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * this file includes code from glfont.c, by Sam Lantinga from the sdl-ttf package
 */

/*!
  \file texture.cpp
  \brief Texture
  \author Jens Thiele
*/

//#include "typedefs.h"
#include "texture.h"
#include <SDL_image.h>

Texture::Texture(int size)
{
  width=height=size;
  texcoord[0]=texcoord[1]=0;
  texcoord[2]=texcoord[3]=1;
  
  glGenTextures(1,&textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);                    
    
  // we do not upload any texture => pixels is NULL
  // => we can specify any format
  // the internal format has double usage:
  // old usage: specify number of color components (since we do not upload a texture this is of no use to us)
  // second: it is a hint to the driver
  // see also: http://www.opengl.org/developers/documentation/Specs/glspec1.1/node79.html#SECTION00680100000000000000
  //  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5, size, size, 0, GL_RGB, GL_UNSIGNED_SHORT, NULL);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R3_G3_B2, size, size, 0, GL_RGB, GL_UNSIGNED_SHORT, NULL);
    
  // Set the texture quality
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

  GL_ERRORS();
}


inline 
int 
power_of_two(int input)
{
  int value = 1;

  while ( value < input ) {
    value <<= 1;
  }
  return value;
}


static
SDL_Surface*
fitToPow2(SDL_Surface* surface)
{
  JGACHINE_CHECK(surface);
  JGACHINE_CHECK(surface->format);

  /* Use the surface width and height expanded to powers of 2 */
  int w = power_of_two(surface->w);
  int h = power_of_two(surface->h);
  if ((w==surface->w)&&(h==surface->h)) return NULL;
  
  SDL_Surface* image = SDL_CreateRGBSurface(SDL_SWSURFACE,
					    w, h,
					    surface->format->BitsPerPixel,
					    surface->format->Rmask, 
					    surface->format->Gmask, 
					    surface->format->Bmask, 
					    surface->format->Amask
					    );
  JGACHINE_CHECK(image);
  
  /* Save the alpha blending attributes */
  Uint32 saved_flags = surface->flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
  Uint8 saved_alpha = surface->format->alpha;
  if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
    SDL_SetAlpha(surface, 0, 0);
  }
  
  /* Copy the surface into the GL texture image */
  SDL_Rect area;
  area.x = 0;
  area.y = 0;
  area.w = surface->w;
  area.h = surface->h;
  SDL_BlitSurface(surface, &area, image, &area);
  
  /* Restore the alpha blending attributes */
  if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
    SDL_SetAlpha(surface, saved_flags, saved_alpha);
  }

  return image;
}


Texture::Texture(unsigned dsize, const char* data, const char *extension, const char *mimeType)
{
  SDL_PixelFormat RGB_PixelFormat = {
    NULL,
    18,3,
    0,0,0,0,0,0,0,0,  // TODO: do not really matter?
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
    0x000000FF, 
    0x0000FF00, 
    0x00FF0000, 
    0x00000000,
#else
    0xFF000000,
    0x00FF0000, 
    0x0000FF00, 
    0x00000000,
#endif
    0,
    0xff
  };
    
  SDL_PixelFormat RGBA_PixelFormat = {
    NULL,
    32,4,
    0,0,0,0,0,0,0,0, // TODO: do not really matter?
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
    0x000000FF, 
    0x0000FF00, 
    0x00FF0000, 
    0xFF000000,
#else
    0xFF000000,
    0x00FF0000, 
    0x0000FF00, 
    0x000000FF,
#endif
    0,
    0xff
  };

  texcoord[0]=texcoord[1]=0;
  texcoord[2]=texcoord[3]=1;

  SDL_Surface* image=loadImage(dsize, data, extension, mimeType);
  GLint internalformat;
  GLint format;
  GLenum type=GL_UNSIGNED_BYTE;
  SDL_PixelFormat* pixelformat;
  if (image->format->Amask) {
    // We have got a alpha channel !
    //    JGACHINE_MSG("Info:","image has alpha channel");
    internalformat=GL_RGBA8;
    format=GL_RGBA;
    haveAlphaChannel=true;
    pixelformat=&RGBA_PixelFormat;
  }else{
    //    JGACHINE_MSG("Info:","image does not have an alpha channel");
    // as fallback convert to rgb
    internalformat=GL_RGB8;
    format=GL_RGB;
    haveAlphaChannel=false;
    pixelformat=&RGB_PixelFormat;
  }
  SDL_Surface* tmp=SDL_ConvertSurface(image,pixelformat,SDL_SWSURFACE);
  SDL_FreeSurface(image);
  image=tmp;
  tmp=NULL;

  SDL_Surface* fit=fitToPow2(image);
  if (fit) {
    texcoord[2] = (GLfloat)image->w / fit->w;	/* Max X */
    texcoord[3] = (GLfloat)image->h / fit->h;	/* Max Y */
    SDL_FreeSurface(image);
    image=fit;
    fit=NULL;
    //    JGACHINE_MSG("Info:","Image was not power of 2 => fit it to power of 2 sized image");
  }

  width=image->w;
  height=image->h;
  
  glGenTextures(1,&textureID);
  GL_ERRORS();

  //  JGACHINE_MSG("Info:", "Texture ID: "<<textureID);

  glBindTexture(GL_TEXTURE_2D,textureID);
  int q=(2>1) ? GL_LINEAR : GL_NEAREST;
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,q); 
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,q);
  // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image,
  // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
  glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format , type, image->pixels);
  GL_ERRORS();
  SDL_FreeSurface(image);
}

Texture::~Texture()
{
  glDeleteTextures(1,&textureID);
  JGACHINE_MSG("Info:","freed texture: "<<textureID);
}

SDL_Surface*
Texture::loadImage(unsigned dsize, const char* data, const char *extension, const char *mimeType)
{
  SDL_RWops* rw = SDL_RWFromMem((void *)data,dsize);
  SDL_Surface* s=IMG_Load_RW(rw,true);
  if (!s) JGACHINE_FATAL("Could not load image");
  //  JGACHINE_MSG("Info:","got image s->w:"<<s->w<<" s->h:"<<s->h);
  return s;
};

void 
Texture::copyFromScreen(int swidth, int sheight)
{
  glBindTexture(GL_TEXTURE_2D,textureID);
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, swidth, sheight);
  GL_ERRORS();
}


