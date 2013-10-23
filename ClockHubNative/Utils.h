// (C) ultrashot 2011-2013
// All rights reserved
#ifndef UTILS_H
#define UTILS_H

void ParseTime(SYSTEMTIME local, bool isTime24, int *outHour, int *outMinute, bool *outIsPm);
BOOL IsAlarmInQueue();
BOOL IsTime24();

#endif
