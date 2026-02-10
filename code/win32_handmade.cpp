#include <windows.h>
#include <stdint.h>
#include <Xinput.h>

WNDPROC Wndproc;

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;

struct win32_window_dimensions
{
    int Width;
    int Height;
};

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

// win InputGetState dynamic definition
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(DyXInputGetStateStub)
{
    return 0;
}
global_variable x_input_get_state *DyXInputGetState = DyXInputGetStateStub;

// win InputSetState dynamic definition
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(DyXInputSetStateStud)
{
    return 0;
}
global_variable x_input_set_state *DyXInputSetState = DyXInputSetStateStud;

global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBlackBuffer;

internal void
Win32LoadXInput(void)
{
    HMODULE XinputLib = LoadLibraryA("xinput1_3.dll");
    if (!XinputLib)
    {
        return;
    }

    DyXInputGetState = (x_input_get_state *)GetProcAddress(XinputLib, "XInputGetState");
    DyXInputSetState = (x_input_set_state *)GetProcAddress(XinputLib, "XInputSetState");
}

win32_window_dimensions Win32GetWindowDimensions(HWND Window)
{
    win32_window_dimensions Result;
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    return (Result);
}

internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer, int xOffset, int yOffset)
{
    uint8 *Row = (uint8 *)Buffer.Memory;
    for (int Y = 0; Y < Buffer.Height; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for (int X = 0; X < Buffer.Width; ++X)
        {
            uint8 Blue = X + xOffset;
            uint8 Green = Y + yOffset;
            uint8 Red = 255;
            *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
        }

        Row += Buffer.Pitch;
    }
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = Buffer->BytesPerPixel * (Buffer->Width * Buffer->Height);
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    Buffer->Pitch = Width * Buffer->BytesPerPixel;
}

internal void Win32DisplayBufferToWindow(win32_offscreen_buffer Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight, int X, int Y, int Width, int Height)
{
    StretchDIBits(DeviceContext,
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, Buffer.Width, Buffer.Height,
                  Buffer.Memory, &Buffer.Info, DIB_RGB_COLORS, SRCCOPY);
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
        break;
    }
    case WM_DESTROY:
    {
        Running = false;
        break;
    }
    case WM_CLOSE:
    {
        Running = false;
        break;
    }
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

        win32_window_dimensions Dimensions = Win32GetWindowDimensions(Window);
        Win32DisplayBufferToWindow(GlobalBlackBuffer, DeviceContext,
                                   Dimensions.Width, Dimensions.Height,
                                   X, Y, Width, Height);

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
    Win32LoadXInput();

    WNDCLASSA windowClass = {};
    windowClass.style = CS_OWNDC | CS_VREDRAW;
    windowClass.lpfnWndProc = MainWindowCallBack;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "HandmadeHeroWindowClass";

    Win32ResizeDIBSection(&GlobalBlackBuffer, 1280, 720);

    if (!RegisterClassA(&windowClass))
    {
        // TODO: log error
    }

    HWND Window = CreateWindowExA(
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

    if (!Window)
    {
        // TODO: log error
    }

    Running = true;
    int xOffset = 0;
    int yOffset = 0;
    while (Running)
    {
        MSG Message;
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
        {
            if (Message.message == WM_QUIT)
            {
                Running = false;
            }

            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }

        for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ControllerIndex++)
        {
            XINPUT_STATE ControllerState;
            if (DyXInputGetState(ControllerIndex, &ControllerState) != ERROR_SUCCESS)
            {
                // Controller not plugged in.
                break;
            }

            XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

            bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

            bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
            bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);

            bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);

            bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
            bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
            bool CButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
            bool DButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

            int16 StickX = Pad->sThumbLX;
            int16 StickY = Pad->sThumbLY;
        }

        RenderWeirdGradient(GlobalBlackBuffer, xOffset, yOffset);

        HDC DeviceContext = GetDC(Window);
        win32_window_dimensions Dimensions = Win32GetWindowDimensions(Window);
        Win32DisplayBufferToWindow(GlobalBlackBuffer, DeviceContext,
                                   Dimensions.Width, Dimensions.Height,
                                   0, 0,
                                   Dimensions.Width, Dimensions.Height);
        ReleaseDC(Window, 0);

        ++xOffset;
        ++yOffset;
    }

    return (0);
}