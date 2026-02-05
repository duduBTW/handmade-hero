#include <windows.h>

WNDPROC Wndproc;

#define internal static
#define local_persist static
#define global_variable static

global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void Win32ResizeDIBSection(int Width, int Height)
{
    if (BitmapHandle)
    {
        DeleteObject(BitmapHandle);
    }

    if (!BitmapDeviceContext)
    {
        BitmapDeviceContext = CreateCompatibleDC(0);
    }

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = Width;
    BitmapInfo.bmiHeader.biHeight = Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    BitmapHandle = CreateDIBSection(BitmapDeviceContext, &BitmapInfo, DIB_RGB_COLORS, &BitmapMemory, 0, 0);
}

internal void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
    StretchDIBits(DeviceContext,
                  X, Y, Width, Height,
                  X, Y, Width, Height,
                  0, 0, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT MainWindowCallBack(
    HWND Window,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam)
{
    LRESULT result = 0;
    switch (Message)
    {
    case WM_SIZE:
    {
        RECT ClientRect;
        GetClientRect(Window, &ClientRect);
        int Width = ClientRect.right - ClientRect.left;
        int Height = ClientRect.bottom - ClientRect.top;

        Win32ResizeDIBSection(Width, Height);
        OutputDebugStringA("Resize");
    }
    break;
    case WM_DESTROY:
    {
        Running = false;
    }
    break;
    case WM_CLOSE:
    {
        Running = false;
    }
    break;
    case WM_ACTIVATEAPP:
    {
        OutputDebugStringA("Activate");
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT paint;

        HDC DeviceContext = BeginPaint(Window, &paint);
        int X = paint.rcPaint.left;
        int Y = paint.rcPaint.top;
        int Width = paint.rcPaint.right - paint.rcPaint.left;
        int Height = paint.rcPaint.bottom - paint.rcPaint.top;

        Win32UpdateWindow(DeviceContext, X, Y, Width, Height);

        EndPaint(Window, &paint);
        break;
    }

    default:
    {
        result = DefWindowProc(Window, Message, WParam, LParam);
        break;
    }
    }

    return result;
}

int CALLBACK WinMain(
    HINSTANCE instance,
    HINSTANCE prevInstance,
    LPSTR cmdLine,
    int nShowCmd)
{
    WNDCLASSA windowClass = {};
    windowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    windowClass.lpfnWndProc = MainWindowCallBack;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (!RegisterClassA(&windowClass))
    {
        // TODO: log error
    }

    HWND WindowHandle = CreateWindowExA(
        0,
        windowClass.lpszClassName,
        "Handmade Hero",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        instance,
        0);

    if (!WindowHandle)
    {
        // TODO: log error
    }

    Running = true;
    MSG Message;
    while (Running)
    {
        BOOL MessageResult = GetMessageA(&Message, 0, 0, 0);
        if (MessageResult <= 0)
        {
            break;
        }

        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return (0);
}