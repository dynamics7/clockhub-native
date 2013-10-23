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

void DrawMediumHtcClock(HDC hDC, int width, int height, COMMON_TILE_DATA *data)
{
	RECT rect = {0, 0, width, height};
	FillRect(hDC, &rect, data->cacheData.hAccentBrush);

	if (data->isTime24 || (data->hour / 10) != 0)
	{
		System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[L_DIGIT_0 + (data->hour / 10)].hDC, 11+4,   42,  62, 90, false);
	}

	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[L_DIGIT_0 + (data->hour % 10)].hDC, 11+74, 42,  62, 90, false);
	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[L_COLON].hDC,               11+137,  42,  41, 90, false);
	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[L_DIGIT_0 + (data->minute / 10)].hDC, 11+178,  42,  62, 90, false);
	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[L_DIGIT_0 + (data->minute % 10)].hDC, 11+248,  42,  62, 90, false);

	if (data->isAlarmInQueue)
	{
		System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[ALARM_ICON].hDC, width - 62 - 4/*width - 62 - 13*//*13 - 9*/, height -2 - 62/*- 48 - 4 - 62 - 48*/ - 7, 62, 62, false);
	}
	System::Drawing::DrawBitmap(hDC, data->cacheData.hBitmaps[BATTERY_ICON].hDC, 18 - 7 + 1, height - 48 - 8 - 4 - 42 - 34 - 6 - 3 - 2, 34, 34, false);

	COLORREF oldColor = SetTextColor(hDC, 0xFFFFFF);
	int oldBkMode = SetBkMode(hDC, TRANSPARENT);
	HFONT hOldFont = (HFONT)SelectObject(hDC, data->cacheData.hMediumTileAmPmFont);
	if (!data->isTime24)
	{
		System::Drawing::DrawString(hDC, 11+252+12, 142, width, height, data->isPm ? L"PM" : L"AM", DT_LEFT);
	}
	SelectObject(hDC, data->cacheData.hMediumTileBatteryPercentFont);
	System::Drawing::DrawString(hDC, 18 + 34 - 8, 12, width - 18, height - 48 - 8 - 4 - 42, data->wsBatteryPercent, DT_BOTTOM | DT_LEFT);
	SelectObject(hDC, data->cacheData.hMediumTileWeekdayFont);
	System::Drawing::DrawString(hDC, 18, 12, width - 18, height - 48 - 8 - 4, data->wsWeekday, DT_BOTTOM | DT_LEFT);
	SelectObject(hDC, data->cacheData.hMediumTileDateFont);
	System::Drawing::DrawString(hDC, 18, 12, width - 18, height /*- 48 - 48*/ - 18, data->wsDate, DT_BOTTOM | DT_LEFT);
	SelectObject(hDC, hOldFont);
	SetBkMode(hDC, oldBkMode);
	SetTextColor(hDC, oldColor);
}

void SaveMediumTileToFile(wchar_t *fileName, COMMON_TILE_DATA *data)
{
	BLANKTILE *tile = CreateBlankTile(336, 336);
	if (tile)
	{
		DrawMediumHtcClock(tile->hDC, 336, 336, data);
		if (data->isCracked)
			RenderCrack(tile->hDC);
		ImageFactory.SaveToFile(tile->hBitmap, fileName, L"png");
		ReleaseBlankTile(tile);
	}
}
