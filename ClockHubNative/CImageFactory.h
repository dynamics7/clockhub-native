// (C) ultrashot 2011-2013
// All rights reserved
#ifndef IMAGES_H
#define IMAGES_H

#include "RendererCache.h"

#include < imaging.h > 

#include "ImageLibrary.h"
#include "Imaging.h"

void DrawBitmap(HDC hdcDest, HDC hdcSrc, int x, int y, int width, int height, int useAlphaBlend);

HBITMAP LoadTransparent(wchar_t *name );
HRESULT Imaging_SaveToFile(HBITMAP handle, LPTSTR filename, LPCTSTR format);

class CImageFactoryImpl
{
public:
    CImageFactoryImpl();
    ~CImageFactoryImpl();
    HRESULT Initialize();
    HRESULT SaveToFile(HBITMAP handle, LPTSTR filename, LPCTSTR format);
    HBITMAP LoadTransparent(wchar_t *name );
    BOOL LoadTransparent(wchar_t *name, RENDERER_IMAGE *rendererImage);
private:
    IImagingFactory *_factory;
    ImageCodecInfo *_imageCodecInfo;
    UINT _imageCodecCount;
};

extern CImageFactoryImpl ImageFactory;

#endif
