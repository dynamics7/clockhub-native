// (C) ultrashot 2011-2013
// All rights reserved
#include "stdafx.h"
#include "ImageLibrary.h"
#include "CImageFactory.h"

wchar_t *imageFileNames[IMAGE_FILE_COUNT] = {L"\\Windows\\lt_l_digit_0.png", 
						L"\\Windows\\lt_l_digit_1.png",
						L"\\Windows\\lt_l_digit_2.png",
						L"\\Windows\\lt_l_digit_3.png",
						L"\\Windows\\lt_l_digit_4.png",

						L"\\Windows\\lt_l_digit_5.png",
						L"\\Windows\\lt_l_digit_6.png",
						L"\\Windows\\lt_l_digit_7.png",
						L"\\Windows\\lt_l_digit_8.png",
						L"\\Windows\\lt_l_digit_9.png",

						L"\\Windows\\lt_xl_digit_0.png", 
						L"\\Windows\\lt_xl_digit_1.png",
						L"\\Windows\\lt_xl_digit_2.png",
						L"\\Windows\\lt_xl_digit_3.png",
						L"\\Windows\\lt_xl_digit_4.png",

						L"\\Windows\\lt_xl_digit_5.png",
						L"\\Windows\\lt_xl_digit_6.png",
						L"\\Windows\\lt_xl_digit_7.png",
						L"\\Windows\\lt_xl_digit_8.png",
						L"\\Windows\\lt_xl_digit_9.png",

						L"\\Windows\\lt_l_digit_colon.png",
						L"\\Windows\\lt_xl_digit_colon.png",
						L"\\Windows\\lt_alarmicon.png",
						L"\\Windows\\lt_batteryicon.png",
						L"\\Windows\\lt_s_digit_0.png", 

						L"\\Windows\\lt_s_digit_1.png",
						L"\\Windows\\lt_s_digit_2.png",
						L"\\Windows\\lt_s_digit_3.png",
						L"\\Windows\\lt_s_digit_4.png",
						L"\\Windows\\lt_s_digit_5.png",

						L"\\Windows\\lt_s_digit_6.png",
						L"\\Windows\\lt_s_digit_7.png",
						L"\\Windows\\lt_s_digit_8.png",
						L"\\Windows\\lt_s_digit_9.png",
						L"\\Windows\\lt_s_digit_colon.png",

						L"\\Windows\\lt_s_alarmicon.png",
						L"\\Windows\\lt_cracked.png",
						L"\\Windows\\lt_cracked_black.png",
						L"\\Windows\\lt_cracked_white.png"};


HBITMAP imageFileBitmaps[IMAGE_FILE_COUNT] = {0};
DimensionPair imageFileDimensions[IMAGE_FILE_COUNT] = {{0,0}};

void LoadHBitmaps()
{
	for (int i = 0; i < IMAGE_FILE_COUNT; ++i)
	{
		RENDERER_IMAGE img;
		ImageFactory.LoadTransparent(imageFileNames[i], &img);
		imageFileBitmaps[i] = img.hBitmap;
		imageFileDimensions[i].width = img.width;
		imageFileDimensions[i].height = img.height;
	}
}

void FreeHBitmaps()
{
	for (int i = 0; i < IMAGE_FILE_COUNT; ++i)
	{
		if (imageFileBitmaps[i])
			DeleteObject(imageFileBitmaps[i]);
	}
}
