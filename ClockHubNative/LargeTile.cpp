// (C) ultrashot 2011-2013
// All rights reserved
#include "stdafx.h"
#include "TileFactory.h"
#include "Utils.h"
#include "imagelibrary.h"

#include "CImageFactory.h"
#include "ThemeColors.h"
#include "regext.h"
#include "Tiles.h"
#include "..\..\common\Drawing.hpp"

void DrawLargeHtcClock(HDC hDC, int width, int height, COMMON_TILE_DATA *data)
{
	RECT rect = {0, 0, width, height};
	FillRect(hDC, &rect, data->cacheData.hAccentBrush);
	
	// Images
	if (data->isTime24 || (data->hour / 10) != 0)
	{
		System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[XL_DIGIT_0 + (data->hour / 10)].hDC, 28+45,   10+36,  116, 166, false);
	}
	
	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[XL_DIGIT_0 + (data->hour % 10)].hDC, 28+168,  10+36,  116, 166, false);
	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[XL_COLON].hDC,       28+285,  10+36,  64,  166, false);
	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[XL_DIGIT_0 + (data->minute / 10)].hDC, 28+348,  10+36,  116, 166, false);
	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[XL_DIGIT_0 + (data->minute % 10)].hDC, 28+471,  10+36,  116, 166, false);

	if (data->isAlarmInQueue)
	{
		System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[ALARM_ICON].hDC, width - 62 - 13 - 5 + 14, height - 48 - 4 - 62, 62, 62, false);
	}

	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[BATTERY_ICON].hDC, width - 16 - 150 + 30 - 34 + 20 + 28 - 5, height - 4 - 34 - 7 - 12 + 8 - 2, 34, 34, false);

	// Strings
	COLORREF oldColor = SetTextColor(hDC, 0xFFFFFF);
	int oldBkMode = SetBkMode(hDC, TRANSPARENT);
	HFONT hOldFont = (HFONT)SelectObject(hDC, data->cacheData.hLargeTileDateAndBatteryPercentFont);
	System::Drawing::DrawString(hDC, 18, 12, width - 18, height - 12, data->wsDate, DT_BOTTOM);
	System::Drawing::DrawString(hDC, 18, 12, width - 18, height - 12, data->wsBatteryPercent, DT_BOTTOM | DT_RIGHT);
	
	if (!data->isTime24)
	{
		SelectObject(hDC, data->cacheData.hLargeTileAmPmFont);
		System::Drawing::DrawString(hDC, 28+50, 5+171, width, height, data->isPm ? L"PM" : L"AM", DT_LEFT);
	}
	
	SelectObject(hDC, data->cacheData.hLargeTileWeekdayFont);
	System::Drawing::DrawString(hDC, 18, 12, width - 18, height - 12 - 48 - 4, data->wsWeekday, DT_BOTTOM);

	SelectObject(hDC, hOldFont);
	SetBkMode(hDC, oldBkMode);
	SetTextColor(hDC, oldColor);
}

void SaveLargeTileToFile(wchar_t *fileName, COMMON_TILE_DATA *data)
{
	BLANKTILE *tile = CreateBlankTile(691, 336);
	if (tile)
	{
		DrawLargeHtcClock(tile->hDC, 691, 336, data);
		if (data->isCracked)
			RenderCrack(tile->hDC);
		ImageFactory.SaveToFile(tile->hBitmap, fileName, L"png");
		ReleaseBlankTile(tile);
	}
}
