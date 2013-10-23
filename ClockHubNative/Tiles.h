// (C) ultrashot 2011-2013
// All rights reserved
#pragma once

#include "ImageLibrary.h"
#include "Imaging.h"
#include "RendererCache.h"

typedef struct
{
	// basic parameters
	SYSTEMTIME localTime;
	BOOL isAlarmInQueue;
	DWORD batteryPercent;
	BOOL isTime24;

	// render-time parameters
	int hour;
	int minute;
	bool isPm;
	wchar_t wsWeekday[50];
	wchar_t wsDate[50];
	wchar_t wsBatteryPercent[50];

	bool isCracked;

	// cache data
	RENDERER_CACHE_DATA cacheData;
}COMMON_TILE_DATA;


void SaveLargeTileToFile(wchar_t *fileName, COMMON_TILE_DATA *data);
void SaveMediumTileToFile(wchar_t *fileName, COMMON_TILE_DATA *data);
void SaveSmallTileToFile(wchar_t *fileName, COMMON_TILE_DATA *data);

void PrepareCommonTileData(COMMON_TILE_DATA *data, SYSTEMTIME localTime, BOOL isAlarmInQueue, int batteryPercent, BOOL isTime24, BOOL isCracked = FALSE);
void RemoveCommonTileData(COMMON_TILE_DATA *data);
void RenderCrack(HDC hDC);
