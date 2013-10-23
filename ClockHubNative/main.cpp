// (C) ultrashot 2011-2013
// All rights reserved
// LiveTokens.cpp : Defines the entry point for the console application.
//

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
#include "TileUpdater.h"

//#define TEST_BUILD

static DWORD MainThreadId = 0;

ITokenManager *tokenManager = NULL;

VOID TimerProc(
               _In_  HWND hwnd,
               _In_  UINT uMsg,
               _In_  UINT_PTR idEvent,
               _In_  DWORD dwTime
               );

bool UpdateTokenShownState(bool noConditionSet = false);

class MyTimer : public CTimer
{
public:

    void SyncAndRun()
    {
        SYSTEMTIME localTime;
        GetLocalTime(&localTime);
        if (localTime.wSecond == 0)
            Enable(TimerProc, 60 * 1000);
        else
            Enable(TimerProc, (60 - localTime.wSecond) * 1000);
    }

    void ApplyConditions()
    {
#ifdef DEBUG_TO_FILE
        SYSTEMTIME time;
        GetLocalTime(&time);
        DEBUGLOG_INIT()
            fprintf(fLog, "[%02d:%02d.%02d] OnTop: %d IsUnlocked: %d IsPinned: %d\r\n", time.wHour, time.wMinute, time.wSecond, 
            MainConditionalRender.GetConditionValue(RenderCondition::IsOnTop),
            MainConditionalRender.GetConditionValue(RenderCondition::IsUnlocked),
            MainConditionalRender.GetConditionValue(RenderCondition::IsPinned));
        DEBUGLOG_DEINIT()

#endif

            if (MainConditionalRender.AllConditionsTrue())
            {
                SyncAndRun();
            }
            else
            {
                Kill();
            }
    }

    void Kill()
    {
        Disable();
    }
};

MyTimer timer;

VOID TimerProc(
               _In_  HWND hwnd,
               _In_  UINT uMsg,
               _In_  UINT_PTR idEvent,
               _In_  DWORD dwTime
               )
{
    if (MainConditionalRender.AllConditionsTrue())
    {
        timer.SyncAndRun();

        /* timer runs in main thread context, so we can safely call UpdateTime without invoking */
        UpdateTime(TIME_CHANGE);
    }
    else
    {
        timer.Kill();
    }
}

LONG oldPercentageValue = -1;

class MyNotificationListener : public PMNotificationListener
{
public:

    MyNotificationListener()
    {
        m_cRef = 0;
        m_IsRegistered = 0;
    }

    virtual ~MyNotificationListener() 
    {
    }

    HRESULT QueryInterface (REFIID riid, LPVOID * ppvObj)
    {
        return E_NOTIMPL;
    }
    ULONG AddRef()
    {
        InterlockedIncrement(&m_cRef);
        return m_cRef;
    }
    ULONG Release()
    {
        ULONG ulRefCount = InterlockedDecrement(&m_cRef);
        if (m_cRef == 0)
        {
            delete this;
        }
        return ulRefCount;
    }

    virtual void HandleTokenNotification(PACMANCLIENT_TOKEN_NOTIFICATION* pToken)
    {
        if (pToken->header.notificationType == TOKEN_NOTIFICATION) /* extra check */
        {
            int len = wcslen(CLOCKHUB_TOKEN);
            if (pToken->tokenIdLength == len && wcsnicmp(&pToken->tokenId, CLOCKHUB_TOKEN, len) == 0)
            {
                GUID guid;
                if (CLSIDFromString(L"{f64d312e-52ce-4718-885e-997b00927fc8}", &guid) == S_OK)
                {
                    if (memcmp(&pToken->header.productId, &guid, sizeof(GUID)) == 0)
                    {
                        if (pToken->notificationType == TOKEN_PINNED || 
                            pToken->notificationType == TOKEN_CREATED)
                        {
                            UpdateFromXml(pToken->header.appId);
                            MainConditionalRender.SetCondition(1, RenderConditionPair(RenderCondition::IsPinned, UpdateTokenShownState()));
                            timer.ApplyConditions();
                        }
                        else if (pToken->notificationType == TOKEN_UNPINNED ||
                            pToken->notificationType == TOKEN_DELETED)
                        {
                            MainConditionalRender.SetCondition(1, RenderConditionPair(RenderCondition::IsPinned, false));
                        }
                    }
                }
            }
        }
    }

    virtual void HandleLifeCycleNotification(PACMANCLIENT_LIFECYCLE_NOTIFICATION*) 
    { 
    }

    virtual void HandleBnsNotification(PACMANCLIENT_BNS_NOTIFICATION* ) 
    {
    }

    void Register(BOOL mode)
    {
        if (mode && !m_IsRegistered)
        {
            PMRegisterForNotification(this, TOKEN_NOTIFICATION, 0);
        }
        else if (!mode && m_IsRegistered)
        {
            PMUnregisterFromNotification(this);
        }
    }

private:
    LONG m_cRef;
    LONG m_IsRegistered;
};

bool UpdateTokenShownState(bool noConditionSet)
{
    bool isPromoted = false;
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
                    ITokenInfo *tokenInfo = NULL;
                    GetTokenInfo(appId, L"ClockHubToken", &tokenInfo);
                    if (tokenInfo)
                    {
                        isPromoted = tokenInfo->get_IsPromoted() ? true : false;
                        if (!noConditionSet)
                            MainConditionalRender.SetCondition(1, RenderConditionPair(RenderCondition::IsPinned, isPromoted));
                        delete tokenInfo;
                    }
                }
                delete info;
            }
        }
    }
    return isPromoted;
}

HREGNOTIFY hLockChangedNotify = NULL;

#define SHL_UNLOCK              0
#define SHL_DEVICE_LOCK         0x01
#define SHL_KEY_LOCK            0x02
#define SHL_SIM_LOCK            0x04
#define SHL_BOOT_LOCK           0x08
#define SHL_LOCK_IN_PROGRESS    0x80000000

BOOL IsLocked(DWORD lockState)
{
    if ((lockState & SHL_DEVICE_LOCK) ||
        (lockState & SHL_KEY_LOCK))
        return TRUE;
    return FALSE;
}

void LockChangedCallback(HREGNOTIFY hNotify,
                         DWORD dwUserData,
                         const PBYTE pData,
                         const UINT cbData)
{
    if (cbData == sizeof(DWORD))
    {
        DWORD dwValue = *(DWORD*)pData;
        MainConditionalRender.SetCondition(1, RenderConditionPair(RenderCondition::IsUnlocked, (IsLocked(dwValue) == FALSE) ? true : false));
    }
}

HREGNOTIFY hBatteryChangedNotify = NULL;

void BatteryChangedCallback(HREGNOTIFY hNotify,
                            DWORD dwUserData,
                            const PBYTE pData,
                            const UINT cbData)
{
    if (cbData == sizeof(DWORD))
    {
        DWORD dwValue = *(DWORD*)pData;
        dwValue = dwValue >> 16;
        if (dwValue != oldPercentageValue)
        {
            PostThreadMessage(MainThreadId, WM_LIVETOKENS_INVOKE, WM_LIVETOKENS_INVOKE_UPDATE_BATTERY_PERCENTAGE, 0);
        }
    }
}


typedef struct _PAGEINFO
{
    DWORD taskId;
    DWORD sessionId;
    DWORD dwProcessId;
    DWORD hPageHandle;
    HWND hwnd;
    DWORD orientations; 
    WCHAR szTaskUri[1024];
    WCHAR szPageId[256];
} PAGEINFO;

extern "C" HRESULT SHGetForegroundPageInfo(PAGEINFO* pPageInfo);


// The interface the client process implements to receive events
class MyShellEventCallback : public Shell::IShellEventCallback
{
public:
    MyShellEventCallback() : previouslyFound(FALSE), savedProcessId(0)
    {
    }

    ~MyShellEventCallback()
    {
    }

    virtual void OnForegroundProcessChange(DWORD processid)
    {	
        if (savedProcessId == 0)
        {
            savedProcessId = Diagnostics::Process::GetProcessID(L"StartHost.exe");
        }
        if (savedProcessId == processid)
        {
            MainConditionalRender.SetCondition(1, RenderConditionPair(RenderCondition::IsOnTop, true));
        }
        else
        {
            MainConditionalRender.SetCondition(1, RenderConditionPair(RenderCondition::IsOnTop, false));
        }
    }

private:
    BOOL previouslyFound;
    DWORD savedProcessId;
};

IUnknown *listener = NULL;
MyShellEventCallback *callback = NULL;
MyNotificationListener *MyListener = NULL;

class CAutoActions
{
public:
    CAutoActions()
    {
    }

    ~CAutoActions()
    {
        if (tokenManager)
            delete tokenManager;
        if (listener)
        {
            listener->Release();
            delete listener;
        }
        if (callback)
            delete callback;
        if (hLockChangedNotify)
            RegistryCloseNotification(hLockChangedNotify);
        if (hBatteryChangedNotify)
            RegistryCloseNotification(hBatteryChangedNotify);
    }
};

CAutoActions autoActions;

void OnConditionChanged(CConditionalRender *render)
{
    DWORD dwLockState = 0;
    RegistryGetDWORD(HKEY_LOCAL_MACHINE, L"System\\State", L"Lock", &dwLockState);
    MyListener->Register(dwLockState ? FALSE : TRUE);
    PostThreadMessage(MainThreadId, WM_LIVETOKENS_INVOKE, WM_LIVETOKENS_INVOKE_SYNC, 0);
}

void ConditionCheck()
{
    DWORD dwLockState = 0;
    RegistryGetDWORD(HKEY_LOCAL_MACHINE, L"System\\State", L"Lock", &dwLockState);

    //FILE *f = _wfopen(L"\\ClockLogText.txt", L"at");
    bool isOnTop = false;
    PAGEINFO pageInfo;
    if (SHGetForegroundPageInfo(&pageInfo) == S_OK)
    {
#define STARTMENU L"app://5B04B775-356B-4AA0-AAF8-6491FFEA5602"
        if (wcsnicmp(pageInfo.szTaskUri, STARTMENU, wcslen(STARTMENU)) == 0)
        {
            isOnTop = true;
        }
    }

    bool isUnlocked = (IsLocked(dwLockState) == FALSE) ? true : false;
    bool isPinned = UpdateTokenShownState(true);
    MainConditionalRender.SetCondition(3, 
        RenderConditionPair(RenderCondition::IsUnlocked, isUnlocked),
        RenderConditionPair(RenderCondition::IsOnTop, isOnTop),
        RenderConditionPair(RenderCondition::IsPinned, isPinned)
        );
}
int _tmain(int argc, _TCHAR* argv[])
{
    /*
    // renderer test code
    LoadHBitmaps();
    UpdateThemeColors();
    SYSTEMTIME st;
    GetLocalTime(&st);

    COMMON_TILE_DATA data;
    PrepareCommonTileData(&data, st, TRUE, 50, TRUE);
    SaveLargeTileToFile(L"\\Temp\\Test_TileLarge_3.png", &data);
    SaveMediumTileToFile(L"\\Temp\\Test_TileMedium_3.png", &data);
    SaveSmallTileToFile(L"\\Temp\\Test_TileSmall_3.png", &data);
    RemoveCommonTileData(&data);
    return 0;
    */
    if (argc > 1 && wcsicmp(argv[1], L"-pin") == 0)
    {
        if (tokenManager == NULL)
            GetTokenManager(&tokenManager);
        Shell::Tiles::PinTokenAtPosition(tokenManager, L"{f64d312e-52ce-4718-885e-997b00927fc8}", L"ClockHubToken", 0xE1000000U);
        Sleep(200);
        keybd_event(VK_LWIN, 0, KEYEVENTF_SILENT, 0);
        Sleep(100);
        keybd_event(VK_LWIN, 0, KEYEVENTF_SILENT | KEYEVENTF_KEYUP, 0);
    }
    else if (argc > 1 && wcsicmp(argv[1], L"-unpin") == 0)
    {
        if (tokenManager == NULL)
            GetTokenManager(&tokenManager);
        //Shell::Tiles::UnpinToken(tokenManager, L"{f64d312e-52ce-4718-885e-997b00927fc8}", L"ClockHubToken");

        /* let's create cracked background */
        LoadHBitmaps();
        UpdateThemeColors();
        SYSTEMTIME st;
        GetLocalTime(&st);

        DWORD dwBatteryPercent = 1;
        SYSTEM_POWER_STATUS_EX2 battery;
        memset(&battery, 0, sizeof(SYSTEM_POWER_STATUS_EX2));
        GetSystemPowerStatusEx2(&battery, sizeof(SYSTEM_POWER_STATUS_EX2), FALSE);
        dwBatteryPercent = battery.BatteryLifePercent;
        COMMON_TILE_DATA data;
        PrepareCommonTileData(&data, st, IsAlarmInQueue(), dwBatteryPercent, IsTime24(), TRUE);
        SaveLargeTileToFile(L"\\Temp\\lt_TileLarge.png", &data);
        SaveMediumTileToFile(L"\\Temp\\lt_TileMedium.png", &data);
        SaveSmallTileToFile(L"\\Temp\\lt_TileSmall.png", &data);
        RemoveCommonTileData(&data);

        GUID guid;
        if (CLSIDFromString(L"{f64d312e-52ce-4718-885e-997b00927fc8}", &guid) == S_OK)
        {
            APPID appId;
            tokenManager->GetAppIDFromProductID(guid, &appId);
            UpdateFromXml(appId);
        }
        return 0;
    }
    MainThreadId = GetCurrentThreadId();
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
#ifdef TEST_BUILD
    MessageBox(NULL, L"Test", 0, 0);
#endif
    LoadHBitmaps();

#ifndef TEST_BUILD
    MyListener = new MyNotificationListener;
#endif
    UpdateTokenShownState();
    UpdateTime(TIME_CHANGE);
    timer.SyncAndRun();
#ifndef TEST_BUILD

    callback = new MyShellEventCallback;
    SHCreateShellEventListener(ForegroundProcessChange, TRUE, callback, &listener);

    RegistryNotifyCallback(HKEY_LOCAL_MACHINE, 
        L"System\\State", 
        L"Lock", 
        LockChangedCallback, 
        0, NULL, &hLockChangedNotify);


    RegistryNotifyCallback(SN_POWERBATTERYSTRENGTH_ROOT, 
        SN_POWERBATTERYSTRENGTH_PATH,
        SN_POWERBATTERYSTRENGTH_VALUE,
        BatteryChangedCallback, 
        0, NULL, &hBatteryChangedNotify);

    MainConditionalRender.SetCallback(OnConditionChanged);
    ConditionCheck();

    /* let's trigger update (if required) */
    PostThreadMessage(MainThreadId, WM_LIVETOKENS_INVOKE, WM_LIVETOKENS_INVOKE_SYNC, 0);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        if (msg.message == WM_LIVETOKENS_INVOKE)
        {
            if (msg.wParam == WM_LIVETOKENS_INVOKE_SYNC)
            {
                timer.ApplyConditions();
                if (MainConditionalRender.AllConditionsTrue())
                    UpdateTime(CONDITION_CHANGED);
            }
            else if (msg.wParam == WM_LIVETOKENS_INVOKE_UPDATE_BATTERY_PERCENTAGE)
            {
                if (MainConditionalRender.AllConditionsTrue())
                    UpdateTime(BATTERY_PERCENT_CHANGE);
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#endif

    return 0;
}

