// (C) ultrashot 2011-2013
// All rights reserved
#include "stdafx.h"
#include "regext.h"
#include "..\..\common\Theme.hpp"

DWORD AccentColor = 0xFFFFFF;

void UpdateThemeColors()
{
    AccentColor = Shell::Theme::GetAccentColor();
}
