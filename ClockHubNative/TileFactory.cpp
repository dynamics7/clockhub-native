// (C) ultrashot 2011-2013
// All rights reserved
#include "stdafx.h"
#include "TileFactory.h"

BLANKTILE* CreateBlankTile(int width, int height)
{
	HDC hDC = CreateCompatibleDC(NULL);
	if (hDC)
	{
		// bitmap dimensions 
		int bitmap_dx = 691;
		int bitmap_dy = 336;

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

		BLANKTILE *tile = new BLANKTILE;
		if (tile)
		{
			tile->hDC = hDC;
			tile->hBitmap = hbm;
			tile->hbmOld = hbmOld;
			return tile;
		}
	}
	return NULL;
}

void ReleaseBlankTile(BLANKTILE *tile)
{
	if (tile)
	{
		if (tile->hbmOld && tile->hDC)
			SelectObject(tile->hDC, tile->hbmOld);
		if (tile->hBitmap)
			DeleteObject(tile->hBitmap);
		if (tile->hDC)
			DeleteDC(tile->hDC);
		delete tile;
	}
}