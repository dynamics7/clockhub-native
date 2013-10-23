// (C) ultrashot 2011-2013
// All rights reserved
#pragma once

class CTimer
{
public:
    CTimer();

    ~CTimer();

    void Enable(TIMERPROC proc, DWORD timeout);
    void Disable();
private:
    UINT timerEvent;
    CRITICAL_SECTION csTimer;
};
