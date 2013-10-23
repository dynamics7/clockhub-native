// (C) ultrashot 2011-2013
// All rights reserved
#include "stdafx.h"
#include "Tiles.h"
#include "Utils.h"
#include "ThemeColors.h"
#include "CImageFactory.h"
#include "ImageLibrary.h"
#include "..\..\common\Drawing.hpp"
#include "regext.h"

/* cache */
static RENDERER_CACHE_DATA cacheData;
static bool isCacheInitialized = false;
static DWORD _prevAccentColor = 0;
static void InvalidateCacheData(RENDERER_CACHE_DATA *cache);

void PrepareCommonTileData(COMMON_TILE_DATA *data, SYSTEMTIME localTime, BOOL isAlarmInQueue, int batteryPercent, BOOL isTime24, BOOL isCracked)
{
	memcpy(&data->localTime, &localTime, sizeof(SYSTEMTIME));
	data->isAlarmInQueue = isAlarmInQueue;
	data->batteryPercent = batteryPercent;
	data->isTime24 = isTime24;

	data->isCracked = isCracked;

	bool isPm = false;
	int hour = 0, minute = 0;
	ParseTime(data->localTime, data->isTime24 ? true : false, &data->hour, &data->minute, &data->isPm);

	GetLocaleInfo(LOCALE_USER_DEFAULT, (data->localTime.wDayOfWeek == 0) ? LOCALE_SDAYNAME7 : (LOCALE_SDAYNAME1 + data->localTime.wDayOfWeek - 1), data->wsWeekday, 50);
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &data->localTime, NULL, data->wsDate, 50);
	swprintf(data->wsBatteryPercent, L"%d%%", data->batteryPercent);

	InvalidateCacheData(&cacheData);
	memcpy(&data->cacheData, &cacheData, sizeof(RENDERER_CACHE_DATA));
}


HFONT CreateFont(int fontsize, wchar_t *fontstr, int weight)
{
	LOGFONT lf;
	ZeroMemory(&lf, sizeof(lf));
	lf.lfWeight = weight;
	lf.lfHeight = -fontsize;
	lf.lfOutPrecision = OUT_RASTER_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN;
	wcscpy(lf.lfFaceName, fontstr);

	HFONT hFont = CreateFontIndirect(&lf);
	return hFont;
}

BOOL MyCreateDC(int width, int height, HDC *outHDC, HBITMAP *outHBM)
{
	HDC hDC = CreateCompatibleDC(NULL);
	if (hDC)
	{
		// create bitmap info header 
		BITMAPINFOHEADER infoHeader; 
		infoHeader.biSize          = sizeof(infoHeader); 
		infoHeader.biWidth         = width; 
		infoHeader.biHeight        = -height; 
		infoHeader.biPlanes        = 1; 
		infoHeader.biBitCount      = 24;
		infoHeader.biCompression   = BI_RGB; 
		infoHeader.biSizeImage     = 0;
		infoHeader.biXPelsPerMeter = 0;
		infoHeader.biYPelsPerMeter = 0;
		infoHeader.biClrUsed       = 0;
		infoHeader.biClrImportant  = 0;

		// dibsection information 
		BITMAPINFO info; 
		info.bmiHeader = infoHeader; 
		BYTE* memory = 0; 
		HBITMAP hbm = CreateDIBSection(hDC, &info, DIB_RGB_COLORS, (void**)&memory, 0, 0); 
		HBITMAP hbmOld = (HBITMAP)SelectObject(hDC, hbm);
		*outHDC = hDC;
		*outHBM = hbm;
	}
	return FALSE;
}

static void InvalidateCacheData(RENDERER_CACHE_DATA *cache)
{
	if (!isCacheInitialized)
	{
		cache->hLargeTileAmPmFont = CreateFont(27, L"Segoe WP", 0);
		cache->hLargeTileWeekdayFont = CreateFont(36, L"Segoe WP Semilight", 0);
		cache->hLargeTileDateAndBatteryPercentFont = CreateFont(36, L"Segoe WP", 450);

		cache->hMediumTileAmPmFont = CreateFont(27, L"Segoe WP", 0);
		cache->hMediumTileBatteryPercentFont = CreateFont(48, L"Segoe WP SemiLight", 0);
		cache->hMediumTileWeekdayFont = CreateFont(36, L"Segoe WP Semilight", 0);
		cache->hMediumTileDateFont = CreateFont(36, L"Segoe WP", 0);

		cache->hSmallTileAmPmFont = CreateFont(28, L"Segoe WP", 0);
		cache->hSmallTileBatteryPercentFont = CreateFont(28, L"Segoe WP", 0);
	}
	if (!isCacheInitialized || _prevAccentColor != AccentColor)
	{
		if (isCacheInitialized && cache->hAccentBrush != NULL)
		{
			DeleteObject(cache->hAccentBrush);
		}
		HBRUSH hBrush = CreateSolidBrush(AccentColor);
		cache->hAccentBrush = hBrush;
		
		for (int i = 0; i < IMAGE_FILE_COUNT; ++i)
		{
			if (isCacheInitialized && cache->hBitmaps[i].hDC != NULL)
			{
				DeleteDC(cache->hBitmaps[i].hDC);
				DeleteObject(cache->hBitmaps[i].hBitmap);
			}
			HDC hDC = NULL;
			HBITMAP hBitmap = NULL;
			MyCreateDC(imageFileDimensions[i].width, imageFileDimensions[i].height,
						&hDC, &hBitmap);
			RECT rect = {0, 0, imageFileDimensions[i].width, imageFileDimensions[i].height};
			FillRect(hDC, &rect, hBrush);
			System::Drawing::DrawBitmap(hDC, imageFileBitmaps[i], 0, 0, rect.right, rect.bottom, true);
			cache->hBitmaps[i].hBitmap = hBitmap;
			cache->hBitmaps[i].hDC = hDC;
		} 
		_prevAccentColor = AccentColor;
	}
	isCacheInitialized = true;
}

void RemoveCommonTileData(COMMON_TILE_DATA *data)
{
}


void RenderCrack(HDC hDC)
{
	wchar_t str[50] = {0};
	RegistryGetString(HKEY_LOCAL_MACHINE, L"ControlPanel\\Themes", L"CurrentTheme", str, 50);
	DWORD val = wcstoul(str, NULL, 10);
	DWORD dwImage = CRACKED;
	if (val == 0)
		dwImage = CRACKED_WHITE;
	else if (val == 1)
		dwImage = CRACKED_BLACK;
	System::Drawing::DrawBitmap(hDC, imageFileBitmaps[dwImage], 0, 0, imageFileDimensions[dwImage].width, imageFileDimensions[dwImage].height, true);
}
