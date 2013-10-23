// (C) ultrashot 2011-2013
// All rights reserved
#include "stdafx.h"
#include "CImageFactory.h"
#include < initguid.h > 
#include < imgguids.h > 

CImageFactoryImpl ImageFactory;

CImageFactoryImpl::CImageFactoryImpl()
{
    _factory = NULL;
    _imageCodecInfo = NULL;
    _imageCodecCount = 0;

}
CImageFactoryImpl::~CImageFactoryImpl()
{
    if (_factory)
        CoUninitialize();
}

HRESULT CImageFactoryImpl:: Initialize()
{
    if (_factory)
        return S_OK;
    HRESULT res;
    res = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (res == S_OK) 
    {
        res = CoCreateInstance(CLSID_ImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IImagingFactory, (void**)&_factory);
        if (res == S_OK)
        {
            res = _factory->GetInstalledEncoders(&_imageCodecCount, &_imageCodecInfo);
        }
    }
    return res;
}

HRESULT CImageFactoryImpl::SaveToFile(HBITMAP handle, LPTSTR filename, LPCTSTR format)
{
    HRESULT res = S_FALSE;
    if (Initialize() == S_OK)
    {
        if (_imageCodecCount == 0)
            return S_FALSE;
        // Get the particular encoder to use
        LPTSTR formatString;
        if (wcscmp(format, L"png") == 0)
            formatString = _T("image/png");
        else if (wcscmp(format, L"jpg") == 0)
            formatString = _T("image/jpeg");
        else if (wcscmp(format, L"gif") == 0)
            formatString = _T("image/gif");
        else if (wcscmp(format, L"bmp") == 0)
            formatString = _T("image/bmp");
        else
            return S_FALSE;

        CLSID encoderClassId;
        for(UINT i = 0; i < _imageCodecCount; ++i)
        {
            if (wcscmp(_imageCodecInfo[i].MimeType, formatString) == 0) 
            {
                encoderClassId = _imageCodecInfo[i].Clsid;
                break;
            }
        }
        IImageEncoder *imageEncoder = NULL;
        if (_factory->CreateImageEncoderToFile(&encoderClassId, filename, &imageEncoder) == S_OK) 
        {
            IImageSink* imageSink = NULL;
            res = imageEncoder->GetEncodeSink(&imageSink);
            if (res != S_OK)
                return res;

            BITMAP bm;
            GetObject (handle, sizeof(BITMAP), &bm);
            PixelFormatID pixelFormat;
            switch (bm.bmBitsPixel)
            {
            case 1: 
                pixelFormat = PixelFormat1bppIndexed;
                break;
            case 4: 
                pixelFormat = PixelFormat4bppIndexed;
                break;
            case 8: 
                pixelFormat = PixelFormat8bppIndexed;
                break;
            case 24: 
                pixelFormat = PixelFormat24bppRGB;
                break;
            default: 
                pixelFormat = PixelFormat32bppARGB;
                break;
            }

            BitmapData* bmData = new BitmapData();
            bmData->Height = bm.bmHeight;
            bmData->Width = bm.bmWidth;
            bmData->Scan0 = bm.bmBits;
            bmData->PixelFormat = pixelFormat;

            UINT bitsPerLine = bm.bmWidth * bm.bmBitsPixel;
            UINT bitAlignment = sizeof(LONG) * 8;
            UINT bitStride = bitAlignment * (bitsPerLine / bitAlignment);   // The image buffer is always padded to LONG boundaries
            if ((bitsPerLine % bitAlignment) != 0) bitStride += bitAlignment; // Add a bit more for the leftover values
            bmData->Stride = (bitStride / 8);

            IBitmapImage* pBitmap = NULL;
            _factory->CreateBitmapFromBuffer(bmData, &pBitmap);
            if (pBitmap)
            {
                IImage* pImage = NULL;
                pBitmap->QueryInterface(IID_IImage, (void**)&pImage); 
                if (pImage)
                {
                    res = pImage->PushIntoSink(imageSink);
                    if (res != S_OK) {
                        return res;
                    }
                    pImage->Release();
                }

                pBitmap->Release();
            }

            imageSink->Release();
            imageEncoder->TerminateEncoder();
            imageEncoder->Release();
        }
    }
    return res;
}

HBITMAP CImageFactoryImpl::LoadTransparent(wchar_t *name )
{
    if (Initialize() == S_OK)
    {
        IImage*			pImage;
        IBitmapImage*	pBitmapImage = NULL;
        ImageInfo		imageInfo; 
        BitmapData		bitmapData;

        _factory->CreateImageFromFile(name, &pImage );
        _factory->CreateBitmapFromImage( pImage, 0, 0, PixelFormat32bppPARGB, InterpolationHintNearestNeighbor , &pBitmapImage );
        pImage->GetImageInfo( &imageInfo ); 
        RECT rect={ 0, 0, imageInfo.Width, imageInfo.Height }; 

        pBitmapImage->LockBits(&rect, ImageLockModeRead, PixelFormat32bppPARGB, &bitmapData );

        return CreateBitmap( imageInfo.Width, imageInfo.Height, 1, GetPixelFormatSize( imageInfo.PixelFormat ), bitmapData.Scan0 );
    }
    return NULL;
}


BOOL CImageFactoryImpl::LoadTransparent(wchar_t *name, RENDERER_IMAGE *rendererImage)
{
    if (Initialize() == S_OK)
    {
        IImage*			pImage;
        IBitmapImage*	pBitmapImage = NULL;
        ImageInfo		imageInfo; 
        BitmapData		bitmapData;

        _factory->CreateImageFromFile(name, &pImage );
        _factory->CreateBitmapFromImage( pImage, 0, 0, PixelFormat32bppPARGB, InterpolationHintNearestNeighbor , &pBitmapImage );
        pImage->GetImageInfo( &imageInfo ); 
        RECT rect={ 0, 0, imageInfo.Width, imageInfo.Height }; 

        pBitmapImage->LockBits(&rect, ImageLockModeRead, PixelFormat32bppPARGB, &bitmapData );

        rendererImage->width = imageInfo.Width;
        rendererImage->height = imageInfo.Height;
        rendererImage->hBitmap = CreateBitmap( imageInfo.Width, imageInfo.Height, 1, GetPixelFormatSize( imageInfo.PixelFormat ), bitmapData.Scan0 );
        return TRUE;
    }
    return FALSE;
}