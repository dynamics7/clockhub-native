// (C) ultrashot 2011-2013
// All rights reserved
#include "stdafx.h"

#include "TileFactory.h"
#include "Utils.h"
#include "imagelibrary.h"

#include "CImageFactory.h"
#include "ThemeColors.h"
#include "regext.h"
#include "..\..\common\Drawing.hpp"
#include "Tiles.h"

void DrawSmallHtcClock(HDC hDC, int width, int height, COMMON_TILE_DATA *data)
{
	RECT rect = {0, 0, width, height};
	FillRect(hDC, &rect, data->cacheData.hAccentBrush);

	if (data->isTime24 || (data->hour / 10) != 0)
	{
		System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[S_DIGIT_0 + (data->hour / 10)].hDC, 4,   21,  31, 45, false);
	}

	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[S_DIGIT_0 + (data->hour % 10)].hDC,    37, 21,  31, 45, false);
	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[S_COLON].hDC,                    68,  21,  20, 44, false);
	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[S_DIGIT_0 + (data->minute / 10)].hDC,  89,  21,  31, 45, false);
	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[S_DIGIT_0 + (data->minute % 10)].hDC,  124,  21,  31, 45, false);

	if (data->isAlarmInQueue)
	{
		System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[S_ALARM_ICON].hDC, /*width - 62 - 13*/7, height - 5 - 32/*- 48 - 4 - 62 - 48*/, 28, 32, false);
	}

	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[BATTERY_ICON].hDC, width - 34 - 52 - 12 - 2, height - 4 - 34 + 2 - 2, 34, 34, false);

	COLORREF oldColor = SetTextColor(hDC, 0xFFFFFF);
	int oldBkMode = SetBkMode(hDC, TRANSPARENT);
	HFONT hOldFont = (HFONT)SelectObject(hDC, data->cacheData.hSmallTileAmPmFont);
	
	if (!data->isTime24)
	{
		System::Drawing::DrawString(hDC, 0, 71, width - 5, height, data->isPm ? L"PM" : L"AM", DT_RIGHT, 28, 0xFFFFFF, L"Segoe WP", 0);
	}
	SelectObject(hDC, data->cacheData.hSmallTileBatteryPercentFont);
	System::Drawing::DrawString(hDC, 11, 12, width - 4 - 2, height - 4, data->wsBatteryPercent, DT_BOTTOM | DT_RIGHT, 28, 0xFFFFFF, L"Segoe WP", 0);
	
	SelectObject(hDC, hOldFont);
	SetBkMode(hDC, oldBkMode);
	SetTextColor(hDC, oldColor);
}

void SaveSmallTileToFile(wchar_t *fileName, COMMON_TILE_DATA *data)
{
	BLANKTILE *tile = CreateBlankTile(159, 159);
	if (tile)
	{
		DrawSmallHtcClock(tile->hDC, 159, 159, data);
		if (data->isCracked)
			RenderCrack(tile->hDC);
		ImageFactory.SaveToFile(tile->hBitmap, fileName, L"png");
		ReleaseBlankTile(tile);
	}
}
