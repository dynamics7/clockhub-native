// (C) ultrashot 2011-2013
// All rights reserved
#ifndef TILEFACTORY_H
#define TILEFACTORY_H


typedef struct
{
	HDC hDC;
	HBITMAP hBitmap;
	HBITMAP hbmOld;
}BLANKTILE;

BLANKTILE* CreateBlankTile(int width, int height);
void ReleaseBlankTile(BLANKTILE *tile);

#endif
