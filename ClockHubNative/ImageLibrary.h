// (C) ultrashot 2011-2013
// All rights reserved
#pragma once

#define IMAGE_FILE_COUNT 39

enum ImageFiles {
	L_DIGIT_0 = 0,
	XL_DIGIT_0 = 10,
	L_COLON = 20,
	XL_COLON = 21,
	ALARM_ICON = 22,
	BATTERY_ICON = 23,
	S_DIGIT_0 = 24,
	S_COLON = 34,
	S_ALARM_ICON = 35,
	CRACKED = 36,
	CRACKED_BLACK = 37,
	CRACKED_WHITE = 38
};


extern HBITMAP imageFileBitmaps[IMAGE_FILE_COUNT];

void LoadHBitmaps();
void FreeHBitmaps();

typedef struct
{
	int width;
	int height;
}DimensionPair;

extern DimensionPair imageFileDimensions[IMAGE_FILE_COUNT];

