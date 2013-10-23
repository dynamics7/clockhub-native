// (C) ultrashot 2011-2013
// All rights reserved
#include "stdafx.h"
#include "CTimer.h"

CTimer::CTimer()
{
    timerEvent = 0;
    InitializeCriticalSection(&csTimer);
}

CTimer::~CTimer()
{
    timerEvent = 0;
    DeleteCriticalSection(&csTimer);
}

void CTimer::Enable(TIMERPROC proc, DWORD timeout)
{
    EnterCriticalSection(&csTimer);
#ifdef DEBUG_TO_FILE
    SYSTEMTIME time;
    GetLocalTime(&time);
    DEBUGLOG_INIT()
        fprintf(fLog, "[%02d:%02d.%02d] CTimer::Enable(%X, %d ms)\r\n", 
        time.wHour, time.wMinute, time.wSecond, 
        proc, timeout);
    DEBUGLOG_DEINIT()

#endif
        if (timerEvent || proc == NULL || timeout == 0)
            KillTimer(NULL, timerEvent);
    if (proc && timeout)
        timerEvent = SetTimer(NULL, 0, timeout, proc);
    LeaveCriticalSection(&csTimer);
}

void CTimer::Disable()
{
    Enable(NULL, 0);
}
