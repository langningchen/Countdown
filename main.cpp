#include <time.h>
#include <iostream>
#include <windows.h>
#include <dwmapi.h>

using namespace std;

const int YEAR = 2024;
const int MON = 6;
const int DAY = 17;
const int FontHeight = 60;

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

wstring Output;
int DaysLeft;

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    tm EntranceExamDate = {0};
    EntranceExamDate.tm_year = YEAR - 1900;
    EntranceExamDate.tm_mon = MON - 1;
    EntranceExamDate.tm_mday = DAY + 1;
    DaysLeft = (mktime(&EntranceExamDate) - time(NULL)) / 60 / 60 / 24;
    Output = L"距离中考还有" + to_wstring(DaysLeft) + L"天";

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
            PAINTSTRUCT Paint;
            HDC Hdc = BeginPaint(hWnd, &Paint);
            RECT Rect;
            GetClientRect(hWnd, &Rect);
            SetBkMode(Hdc, TRANSPARENT);
            SetTextColor(Hdc, RGB(max(DaysLeft / 2, 255), min(255 - DaysLeft / 2, 0), 0));
            LOGFONT Font = {0};
            Font.lfHeight = FontHeight;
            Font.lfWeight = FW_NORMAL;
            wcscpy(Font.lfFaceName, L"楷体");
            SelectObject(Hdc, CreateFontIndirect(&Font));
            DrawText(Hdc, Output.c_str(), Output.length(), &Rect, DT_CENTER | DT_VCENTER);
            EndPaint(hWnd, &Paint);
            break;
        }
        case WM_CLOSE:
        {
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    };
    WindowsClass.hInstance = hInstance;
    WindowsClass.lpszClassName = L"CountdownClass";
    RegisterClass(&WindowsClass);

    HWND hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, L"CountdownClass", L"中考倒计时", WS_VISIBLE | WS_POPUP,
                               (GetSystemMetrics(SM_CXSCREEN) - Output.length() * FontHeight) / 2, 0,
                               Output.length() * FontHeight,
                               FontHeight, NULL, NULL, hInstance, NULL);
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    MSG Msg;
    while (GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return 0;
}
