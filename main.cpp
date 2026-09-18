#include <windows.h>
#include <stdint.h>
using namespace std;


static bool Running;
static BITMAPINFO BitmapInfo;
static int BitmapWidth;
static int BitmapHeight;

struct RGBBuffer{
  BITMAPINFO Info;
  void *Memory;
  int Width;
  int Height;
  int Stride;
};
static RGBBuffer GlobalBackBuffer;

struct WindowDimensions {
  int Width;
  int Height;
};

static WindowDimensions Win32GetWindowDimensions(HWND WindowHandle) 
{
    WindowDimensions result;
    RECT ClientRect;
    GetClientRect(WindowHandle, &ClientRect);
    result.Width = ClientRect.right - ClientRect.left;
    result.Height = ClientRect.bottom - ClientRect.top;
    return result;
}

static void Win32ShowGradient(RGBBuffer Buffer, int xOffset, int yOffset)
{
  uint8_t *Row = (uint8_t *)Buffer.Memory;
    for(int y = 0; y < Buffer.Height; ++y) 
    {
      uint32_t *Pixel = (uint32_t *)Row;
      for(int x = 0; x < Buffer.Width; ++x)
      {
        uint8_t green = y + yOffset;
        uint8_t red = x + xOffset;

        *Pixel++ = ((red << 16) | (green << 8));
      }
      Row += Buffer.Stride;
    }
}

static void Win32ResizeDIBSection(RGBBuffer *Buffer, int WindowWidth, int WindowHeight)
{
  if(Buffer->Memory)
  {
    VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
  }
  Buffer->Width = WindowWidth;
  Buffer->Height = WindowHeight;
  Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
  Buffer->Info.bmiHeader.biWidth = Buffer->Width;
  Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
  Buffer->Info.bmiHeader.biPlanes = 1;
  Buffer->Info.bmiHeader.biBitCount = 32;
  Buffer->Info.bmiHeader.biCompression = BI_RGB;
  int BytesPerPixel = 4;

  int BitmapMemorySize = Buffer->Width * Buffer->Height * BytesPerPixel;
  Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
  Buffer->Stride = Buffer->Width * BytesPerPixel;
} 


static void Win32DisplayBuffer(HDC BitmapDeviceContext, RGBBuffer Buffer, int WindowWidth, int WindowHeight)
{
  StretchDIBits(BitmapDeviceContext, 0, 0, WindowWidth, 
                WindowHeight, 0, 0, Buffer.Width, Buffer.Height, 
                Buffer.Memory, &Buffer.Info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND WindowHandle, UINT msg, WPARAM wparam, LPARAM lparam) {
  LRESULT Result = 0;
  switch(msg) 
  {
    case WM_CLOSE: 
      // if (hBitmap != nullptr) {
      //   DeleteObject(hBitmap);
      //   hBitmap = nullptr;
      // }
      // CleanupOffScreenDC();
      Running = false;
      break;
    case WM_DESTROY:
      Running = false;
      break;
    case WM_LBUTTONDOWN:
      // is_drawing = true;
      break;
    case WM_LBUTTONUP:
      // is_drawing = false;
      break;
    case WM_MOUSEMOVE:
      // int xpos = GET_X_LPARAM(lparam);
      // int ypos = GET_Y_LPARAM(lparam);
      // if(is_drawing) {
      //   SetPixelColor(pBits, win_width, xpos, ypos, 255, 0, 0);
      //   InvalidateRect(WindowHandle, NULL, NULL);
      // }
      break;
    case WM_ERASEBKGND:
      return 1;
    case WM_SIZE:
      {
      }
      break;
    case WM_PAINT:
      {
        PAINTSTRUCT Paint;
        HDC BitmapDeviceContext = BeginPaint(WindowHandle, &Paint);
        WindowDimensions Dimensions = Win32GetWindowDimensions(WindowHandle);
        Win32DisplayBuffer(BitmapDeviceContext, GlobalBackBuffer, Dimensions.Width, Dimensions.Height);
        EndPaint(WindowHandle, &Paint);
      }
      break;
    default:
      Result = DefWindowProc(WindowHandle, msg, wparam, lparam);
  }
  return Result;
}

void CreateAndRegisterWindow(HINSTANCE Instance) {
  // wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  // wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
  // wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  // wc.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);

}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) 
{  
  WNDCLASS wc = {0};
  
  Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);
  
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = Win32MainWindowCallback;
  wc.hInstance = Instance;
  wc.lpszClassName = "TEST";

  if(RegisterClass(&wc)) 
  {
      HWND WindowHandle = CreateWindowEx(
      0, 
      wc.lpszClassName, 
      "window!!", 
      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
      nullptr, nullptr, Instance, nullptr);
      if (WindowHandle) 
      {
        Running = true;
        int xOffset = 0;
        int yOffset = 0;
        while(Running) 
        {
          MSG Message;
          while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE) > 0) 
          {
            if(Message.message == WM_QUIT) 
            {
              Running = false;  
            }

            TranslateMessage(&Message);
            DispatchMessage(&Message);
          } 
          Win32ShowGradient(GlobalBackBuffer, xOffset, yOffset);
          HDC DeviceContext = GetDC(WindowHandle);
          WindowDimensions Dimensions = Win32GetWindowDimensions(WindowHandle);
          Win32DisplayBuffer(DeviceContext, GlobalBackBuffer, Dimensions.Width, Dimensions.Height);
          ReleaseDC(WindowHandle, DeviceContext);
          ++xOffset;
          ++yOffset;
          }
        }
      }
  return 0;
} 




// auto CreateBitmapFromRGB(char *pdata, int width, int height) -> pair<HBITMAP, void*>{
//   BITMAPINFO bmi = {0};
//   bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
//   bmi.bmiHeader.biWidth = width;
//   bmi.bmiHeader.biHeight = -height;
//   bmi.bmiHeader.biPlanes = 1;
//   bmi.bmiHeader.biBitCount = 24;
//   bmi.bmiHeader.biCompression = BI_RGB;
//
//   HDC hdc = GetDC(nullptr);
//   void *pbits;
//   HBITMAP hbm = CreateDibSection(hdc, &bmi, DIB_RGB_COLORS, &pbits, nullptr, 0);
//   if (hbm != nullptr) {
//     memcpy(pbits, pdata, width * height * 3);
//   }
//   ReleaseDC(nullptr, hdc);
//   return {hbm, pBits};
// }
//
//
