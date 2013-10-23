// (C) ultrashot 2011-2013
// All rights reserved
#pragma once

#include "ImageLibrary.h"

typedef struct
{
	HDC hDC;
	int width;
	int height;
	HBITMAP hBitmap;
}RENDERER_IMAGE;

typedef struct
{
	// fonts
	HBRUSH hAccentBrush;

	/* Large tile */
	HFONT hLargeTileDateAndBatteryPercentFont;
	HFONT hLargeTileWeekdayFont;
	HFONT hLargeTileAmPmFont;

	/* Medium tile */
	HFONT hMediumTileAmPmFont;
	HFONT hMediumTileBatteryPercentFont;
	HFONT hMediumTileDateFont;
	HFONT hMediumTileWeekdayFont;

	/* Small tile */
	HFONT hSmallTileAmPmFont;
	HFONT hSmallTileBatteryPercentFont;

	/* Digits with accented background */
	RENDERER_IMAGE hBitmaps[IMAGE_FILE_COUNT];
}RENDERER_CACHE_DATA;
