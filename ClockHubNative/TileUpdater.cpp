// (C) ultrashot 2011-2013
// All rights reserved
#include "stdafx.h"
#include "regext.h"
#include "CImageFactory.h"
#include "ThemeColors.h"
#include "Winnls.h"

#include "Tiles.h"
#include "ImageLibrary.h"
#include "Utils.h"
#include "pm7.h"
#include "notify.h"

#include "CTimer.h"

#include "ClockHub.h"
#include "snapi.h"
#include "..\..\common\pacmanclient.h"
#include "..\..\common\ShellEvents.hpp"
#include "..\..\common\Tiles.hpp"
#include "..\..\common\Process.hpp"

#include "CConditionalRender.hpp"


extern ITokenManager *tokenManager;

void UpdateFromXml(APPID appId)
{
#ifdef DEBUG_TO_FILE
    SYSTEMTIME dbgtime;
    GetLocalTime(&dbgtime);
#endif
    if (tokenManager)
    {
        char *test = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
            "<PushNotification>\r\n"
            "<Token TokenID=\"ClockHubToken\" TaskName=\"_default\">\r\n"
            "<TemplateType6>\r\n"
            "<SmallImageURI IsRelative=\"false\" IsResource=\"false\">\\Temp\\lt_TileSmall.png</SmallImageURI>\r\n"
            "<BackgroundImageURI IsRelative=\"false\" IsResource=\"false\">\\Temp\\lt_TileMedium.png</BackgroundImageURI>\r\n"
            "<LargeBackgroundImageURI IsRelative=\"false\" IsResource=\"false\">\\Temp\\lt_TileLarge.png</LargeBackgroundImageURI>\r\n"
            "<Count>0</Count>\r\n"
            "<Title> </Title>\r\n"
            "<Animation />\r\n"
            "</TemplateType6>\r\n"
            "</Token>\r\n"
            "</PushNotification>\r\n";
        HRESULT hr = tokenManager->UpdateTokenFromXML(appId, (PBYTE)test, strlen(test) + 1);
        if (hr != S_OK)
        {
#ifdef DEBUG_TO_FILE
            {
                DEBUGLOG_INIT();
                fprintf(fLog, "[%02d:%02d.%02d] UpdateFromXml refused (reason = HR %X)\r\n", 
                    dbgtime.wHour, dbgtime.wMinute, dbgtime.wSecond, hr);
                DEBUGLOG_DEINIT();

            }
#endif
        }
    }
    else
    {
#ifdef DEBUG_TO_FILE
        {
            DEBUGLOG_INIT();
            fprintf(fLog, "[%02d:%02d.%02d] UpdateFromXml refused (reason = bad tokenManager)\r\n", 
                dbgtime.wHour, dbgtime.wMinute, dbgtime.wSecond);
            DEBUGLOG_DEINIT();

        }
#endif
    }
}

CLOCKHUB_STATE_IMAGE previousState = {FALSE, 0};

void UpdateTime(CLOCKHUB_UPDATE_TIME_REASON reason)
{
#ifdef DEBUG_TO_FILE
    SYSTEMTIME time;
    GetLocalTime(&time);
    {

        DEBUGLOG_INIT();
        fprintf(fLog, "[%02d:%02d.%02d] UpdateTime (reason = %d; OnTop: %d IsUnlocked: %d IsPinned: %d)\r\n", time.wHour, time.wMinute, time.wSecond, 
            reason, 
            MainConditionalRender.GetConditionValue(RenderCondition::IsOnTop),
            MainConditionalRender.GetConditionValue(RenderCondition::IsUnlocked),
            MainConditionalRender.GetConditionValue(RenderCondition::IsPinned)
            );
        DEBUGLOG_DEINIT();

    }
#endif

    if (!MainConditionalRender.AllConditionsTrue())
        return;
    SYSTEMTIME localTime;
    GetLocalTime(&localTime);
    __try
    {

        BOOL isAlarmInQueue = IsAlarmInQueue();
        SYSTEM_POWER_STATUS_EX2 battery;
        memset(&battery, 0, sizeof(SYSTEM_POWER_STATUS_EX2));
        GetSystemPowerStatusEx2(&battery, sizeof(SYSTEM_POWER_STATUS_EX2), FALSE);

        UpdateThemeColors();
        BOOL isTime24 = IsTime24();
        // let's check if something changed (why to redraw if there is nothing new?)
        BOOL anyChange = TRUE;
        if (isAlarmInQueue == previousState.isAlarmInQueue &&
            battery.BatteryLifePercent == previousState.batteryPercent &&
            localTime.wDayOfWeek == previousState.localTime.wDayOfWeek &&
            localTime.wHour == previousState.localTime.wHour &&
            localTime.wMinute == previousState.localTime.wMinute &&
            localTime.wDay == previousState.localTime.wDay &&
            localTime.wMonth == previousState.localTime.wMonth &&	
            localTime.wYear == previousState.localTime.wYear &&
            isTime24 == previousState.isTime24 &&
            AccentColor == previousState.AccentColor)
            anyChange = FALSE;
        if (anyChange)
        {
            memcpy(&previousState.localTime, &localTime, sizeof(SYSTEMTIME));
            previousState.batteryPercent = battery.BatteryLifePercent;
            previousState.isAlarmInQueue = isAlarmInQueue;
            previousState.isTime24 = isTime24;
            previousState.AccentColor = AccentColor;

            UpdateThemeColors();
            COMMON_TILE_DATA data;
            PrepareCommonTileData(&data, localTime, isAlarmInQueue, battery.BatteryLifePercent, isTime24);
            SaveLargeTileToFile(L"\\Temp\\lt_TileLarge.png", &data);
            SaveMediumTileToFile(L"\\Temp\\lt_TileMedium.png", &data);
            SaveSmallTileToFile(L"\\Temp\\lt_TileSmall.png", &data);
            RemoveCommonTileData(&data);

            if (tokenManager == NULL)
                GetTokenManager(&tokenManager);

            if (tokenManager)
            {
                GUID guid;
                if (CLSIDFromString(L"{f64d312e-52ce-4718-885e-997b00927fc8}", &guid) == S_OK)
                {
                    IApplicationInfo *info = NULL;
                    if (GetApplicationInfoByProductID(guid, &info) == S_OK &&
                        info != NULL)
                    {
                        APPID appId = 0;
                        tokenManager->GetAppIDFromProductID(guid, &appId);

                        if (appId)
                        {
                            static bool xmlOnce = false;
                            if (xmlOnce == false)
                            {
                                UpdateFromXml(appId);
                                xmlOnce = true;
                            }
                            else
                            {
                                IToken *token = NULL;
                                tokenManager->GetToken(appId, CLOCKHUB_TOKEN, &token);
                                if (token)
                                {
                                    HRESULT hr = tokenManager->UpdateToken(token);
                                    if (hr != S_OK)
                                    {
#ifdef DEBUG_TO_FILE
                                        {
                                            DEBUGLOG_INIT();
                                            fprintf(fLog, "[%02d:%02d.%02d] UpdateTime refused (reason = HR %X)\r\n", 
                                                time.wHour, time.wMinute, time.wSecond, hr);
                                            DEBUGLOG_DEINIT();

                                        }
#endif
                                    }
                                }
                                else
                                {
#ifdef DEBUG_TO_FILE
                                    {
                                        DEBUGLOG_INIT();
                                        fprintf(fLog, "[%02d:%02d.%02d] UpdateTime refused (reason = bad token)\r\n", 
                                            time.wHour, time.wMinute, time.wSecond);
                                        DEBUGLOG_DEINIT();

                                    }
#endif
                                }
                            }
                        }
                        else
                        {
#ifdef DEBUG_TO_FILE
                            {
                                DEBUGLOG_INIT();
                                fprintf(fLog, "[%02d:%02d.%02d] UpdateTime refused (reason = bad AppId)\r\n", 
                                    time.wHour, time.wMinute, time.wSecond);
                                DEBUGLOG_DEINIT();

                            }
#endif
                        }
                        delete info;
                    }
                    else
                    {
#ifdef DEBUG_TO_FILE
                        {
                            DEBUGLOG_INIT();
                            fprintf(fLog, "[%02d:%02d.%02d] UpdateTime refused (reason = GetApplicationInfo failed)\r\n", 
                                time.wHour, time.wMinute, time.wSecond);
                            DEBUGLOG_DEINIT();

                        }
#endif
                    }
                }
                else
                {
#ifdef DEBUG_TO_FILE
                    {
                        DEBUGLOG_INIT();
                        fprintf(fLog, "[%02d:%02d.%02d] UpdateTime refused (reason = bad guid conversion)\r\n", 
                            time.wHour, time.wMinute, time.wSecond);
                        DEBUGLOG_DEINIT();

                    }
#endif
                }
            }
            else
            {
#ifdef DEBUG_TO_FILE
                {
                    DEBUGLOG_INIT();
                    fprintf(fLog, "[%02d:%02d.%02d] UpdateTime refused (reason = no token manager)\r\n", 
                        time.wHour, time.wMinute, time.wSecond);
                    DEBUGLOG_DEINIT();

                }
#endif
            }
        }
        else
        {
#ifdef DEBUG_TO_FILE
            {
                DEBUGLOG_INIT();
                fprintf(fLog, "[%02d:%02d.%02d] UpdateTime refused (reason = no change)\r\n", 
                    time.wHour, time.wMinute, time.wSecond);
                DEBUGLOG_DEINIT();

            }
#endif
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
#ifdef DEBUG_TO_FILE
        {
            DEBUGLOG_INIT();
            fprintf(fLog, "[%02d:%02d.%02d] UpdateTime refused (reason = exception)\r\n", 
                time.wHour, time.wMinute, time.wSecond);
            DEBUGLOG_DEINIT();

        }
#endif
    }
}