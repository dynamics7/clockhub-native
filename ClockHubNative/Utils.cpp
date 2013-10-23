// (C) ultrashot 2011-2013
// All rights reserved
#include "stdafx.h"
#include "..\..\common\Alarms.hpp"
#include "regext.h"

void ParseTime(SYSTEMTIME local, bool isTime24, int *outHour, int *outMinute, bool *outIsPm)
{
	bool pm = true;
	int hour = local.wHour;
	if (!isTime24)
	{
		if (hour < 12)
			pm = false;
		if (local.wHour == 0)
			hour = 12;
		else if (local.wHour >= 13)
			hour = local.wHour - 12;
	}
	*outIsPm = pm;
	*outHour = hour;
	*outMinute = local.wMinute;
}

BOOL IsAlarmInQueue()
{
	int index = -1;
	HKEY hKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Clock", 0, KEY_ALL_ACCESS, &hKey) == S_OK && hKey)
	{
		for (int i = 0; i < 32; ++i)
		{
			Shell::CAlarm alarm;
			alarm.ReadAlarm(hKey, i);
			ALARM_INFO info = alarm.GetAlarmInfo();
			if (info.wFlags)
			{
				RegCloseKey(hKey);
				return TRUE;
			}
		}
	}
	RegCloseKey(hKey);
	return FALSE;
}

BOOL IsTime24()
{
	DWORD dwTime24 = 0;
	wchar_t wsTime24[10] = L"1";
	if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ITIME, wsTime24, 10))
	{
		if (wcsicmp(wsTime24, L"0") == 0)
			dwTime24 = 0;
		else if (wcsicmp(wsTime24, L"1") == 0)
			dwTime24 = 1;
	}
	return dwTime24 ? TRUE : FALSE;
}