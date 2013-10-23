// (C) ultrashot 2011-2013
// All rights reserved
#pragma once

//#define DEBUG_TO_FILE

#define DEBUGLOG_INIT() FILE *fLog = fopen("\\ClockLog.txt", "at");
#define DEBUGLOG_DEINIT() fflush(fLog); fclose(fLog);