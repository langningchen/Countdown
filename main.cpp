#include <time.h>
#include <iostream>
#include <windows.h>
#include <dwmapi.h>

#ifdef UNICODE
#define _tstring wstring
#define _tcscpy wcscpy
#define _ttostring to_wstring
#else
#define _tstring string
#define _tcscpy strcpy
#define _ttostring to_string
#endif

using namespace std;

struct AccentPolicy
{
    int AccentState;
    int AccentFlags;
    int GradientColor;
    int AnimationId;
};
struct WindowCompositionAttributeData
{
    int Attribute;
    PVOID Data;
    ULONG DataSize;
};
typedef BOOL(WINAPI *pSetWindowCompositionAttribute)(HWND, WindowCompositionAttributeData *);

_tstring ReplaceAll(_tstring Input, _tstring Find, _tstring Replace)
{
    size_t FoundPosition = 0;
    while ((FoundPosition = Input.find(Find, FoundPosition)) != _tstring::npos)
    {
        Input.replace(FoundPosition, Find.length(), Replace);
        FoundPosition += Replace.length();
    }
    return Input;
}

_tstring CountdownString;
int FontHeight;

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    PTSTR CurrentFilePathBuffer = new TCHAR[MAX_PATH];
    GetModuleFileName(NULL, CurrentFilePathBuffer, MAX_PATH);
    _tstring CurrentFilePath = CurrentFilePathBuffer;
    CurrentFilePath = CurrentFilePath.substr(0, CurrentFilePath.find_last_of('\\') + 1);
    _tstring INIPath = CurrentFilePath + TEXT("Settings.ini");
    TCHAR CountdownStringBuffer[MAX_PATH] = {0};
    GetPrivateProfileString(TEXT("Settings"), TEXT("CountdownString"), TEXT(""), CountdownStringBuffer, MAX_PATH, INIPath.c_str());
    CountdownString = CountdownStringBuffer;
    int TargetYear = GetPrivateProfileInt(TEXT("Settings"), TEXT("TargetYear"), -1, INIPath.c_str());
    int TargetMonth = GetPrivateProfileInt(TEXT("Settings"), TEXT("TargetMonth"), -1, INIPath.c_str());
    int TargetDay = GetPrivateProfileInt(TEXT("Settings"), TEXT("TargetDay"), -1, INIPath.c_str());
    FontHeight = GetPrivateProfileInt(TEXT("Settings"), TEXT("FontHeight"), -1, INIPath.c_str());

    if (CountdownString == TEXT("") || TargetYear == -1 || TargetMonth == -1 || TargetDay == -1 || FontHeight == -1)
    {
        CountdownString = TEXT("${AllDaysLeft} days till 01/01/2035");
        TargetYear = 2035;
        TargetMonth = 1;
        TargetDay = 1;
        FontHeight = 50;
        WritePrivateProfileString(TEXT("Settings"), TEXT("CountdownString"), CountdownString.c_str(), INIPath.c_str());
        WritePrivateProfileString(TEXT("Settings"), TEXT("TargetYear"), _ttostring(TargetYear).c_str(), INIPath.c_str());
        WritePrivateProfileString(TEXT("Settings"), TEXT("TargetMonth"), _ttostring(TargetMonth).c_str(), INIPath.c_str());
        WritePrivateProfileString(TEXT("Settings"), TEXT("TargetDay"), _ttostring(TargetDay).c_str(), INIPath.c_str());
        WritePrivateProfileString(TEXT("Settings"), TEXT("FontHeight"), _ttostring(FontHeight).c_str(), INIPath.c_str());
    }

    SYSTEMTIME TargetDate = {0};
    TargetDate.wYear = TargetYear;
    TargetDate.wMonth = TargetMonth;
    TargetDate.wDay = TargetDay;
    SYSTEMTIME CurrentDate = {0};
    GetSystemTime(&CurrentDate);

    FILETIME TargetFileDate;
    SystemTimeToFileTime(&TargetDate, &TargetFileDate);
    FILETIME CurrentFileTime;
    SystemTimeToFileTime(&CurrentDate, &CurrentFileTime);

    int AllDaysLeft = (int)((*(ULONGLONG *)&TargetFileDate - *(ULONGLONG *)&CurrentFileTime) / (10000000LL * 60 * 60 * 24));

    CountdownString = ReplaceAll(CountdownString, TEXT("${TargetYear}"), _ttostring(TargetYear));
    CountdownString = ReplaceAll(CountdownString, TEXT("${TargetMonth}"), _ttostring(TargetMonth));
    CountdownString = ReplaceAll(CountdownString, TEXT("${TargetDay}"), _ttostring(TargetDay));

    CountdownString = ReplaceAll(CountdownString, TEXT("${CurrentYear}"), _ttostring(CurrentDate.wYear));
    CountdownString = ReplaceAll(CountdownString, TEXT("${CurrentMonth}"), _ttostring(CurrentDate.wMonth));
    CountdownString = ReplaceAll(CountdownString, TEXT("${CurrentDay}"), _ttostring(CurrentDate.wDay));

    CountdownString = ReplaceAll(CountdownString, TEXT("${YearLeft}"), _ttostring(CurrentDate.wYear - TargetDate.wYear));
    CountdownString = ReplaceAll(CountdownString, TEXT("${MonthLeft}"), _ttostring(CurrentDate.wMonth - TargetDate.wMonth));
    CountdownString = ReplaceAll(CountdownString, TEXT("${DayLeft}"), _ttostring(CurrentDate.wDay - TargetDate.wDay));

    CountdownString = ReplaceAll(CountdownString, TEXT("${AllDaysLeft}"), _ttostring(AllDaysLeft));

    WNDCLASS WindowsClass = {0};
    WindowsClass.lpfnWndProc = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
    {
        switch (message)
        {
        case WM_CREATE:
        {
            HMODULE hModule = LoadLibrary(TEXT("user32.dll"));
            pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hModule, "SetWindowCompositionAttribute");
            AccentPolicy Policy = {4, 0, 0x00FFFFFF, 0};
            WindowCompositionAttributeData Data = {19, &Policy, sizeof(AccentPolicy)};
            SetWindowCompositionAttribute(hWnd, &Data);
            break;
        }
        case WM_PAINT:
        {
            SetWindowPos(hWnd, HWND_TOPMOST, (GetSystemMetrics(SM_CXSCREEN) - CountdownString.length() * FontHeight) / 2, 0, CountdownString.length() * FontHeight, FontHeight, SWP_SHOWWINDOW);
            PAINTSTRUCT Paint;
            HDC Hdc = BeginPaint(hWnd, &Paint);
            RECT Rect;
            GetClientRect(hWnd, &Rect);
            SetBkMode(Hdc, TRANSPARENT);
            SetTextColor(Hdc, RGB(255, 0, 0));
            LOGFONT Font = {0};
            Font.lfHeight = FontHeight;
            Font.lfWeight = FW_NORMAL;
            _tcscpy(Font.lfFaceName, TEXT("楷体"));
            SelectObject(Hdc, CreateFontIndirect(&Font));
            DrawText(Hdc, CountdownString.c_str(), CountdownString.length(), &Rect, DT_CENTER | DT_VCENTER);
            EndPaint(hWnd, &Paint);
            break;
        }
        case WM_RBUTTONUP:
        case WM_CLOSE:
        {
            PostQuitMessage(0);
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    };
    WindowsClass.hInstance = hInstance;
    WindowsClass.lpszClassName = TEXT("CountdownClass");
    RegisterClass(&WindowsClass);

    CreateWindowEx(WS_EX_TOOLWINDOW, TEXT("CountdownClass"), TEXT("Countdown"), WS_VISIBLE | WS_POPUP, 1, 1, 1, 1, NULL, NULL, hInstance, NULL);
    MSG Msg;
    while (GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return 0;
}
