#include <windows.h>
#include <stdint.h>
using namespace std;


static bool Running;
static BITMAPINFO BitmapInfo;
static void *BitmapMemory;
static int BitmapWidth;
static int BitmapHeight;
static int BytesPerPixel = 4;
static void ShowGradient(int xOffset, int yOffset)
{
  int Width = BitmapWidth;
  int Height = BitmapHeight;
  int Stride = Width * BytesPerPixel; 
  uint8_t *Row = (uint8_t *)BitmapMemory;
    for(int y = 0; y < BitmapHeight; ++y) 
    {
      uint32_t *Pixel = (uint32_t *)Row;
      for(int x = 0; x < BitmapWidth; ++x)
      {
        uint8_t green = y + yOffset;
        uint8_t red = x + xOffset;

        *Pixel++ = ((red << 16) | (green << 8));
      }
      Row += Stride;
    }
}
static void Win32ResizeDIBSection(int Width, int Height)
{
  if(BitmapMemory)
  {
    VirtualFree(BitmapMemory, 0, MEM_RELEASE);
  }
  BitmapWidth = Width;
  BitmapHeight = Height;
  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = BitmapWidth;
  BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;
  
  int BitmapMemorySize = BitmapWidth * BitmapHeight * BytesPerPixel;
  BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
} 

static void Win32UpdateWindow(HDC BitmapDeviceContext, RECT *WindowRect)
{
  int WindowWidth = WindowRect->right - WindowRect->left;
  int WindowHeight = WindowRect->bottom - WindowRect->top; 
  StretchDIBits(BitmapDeviceContext, 0, 0, BitmapWidth, 
                BitmapHeight, 0, 0, WindowWidth, WindowHeight, 
                BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
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
      //   InvalidateRect(hwnd, NULL, NULL);
      // }
      break;
    case WM_ERASEBKGND:
      return 1;
    case WM_SIZE:
      {
      RECT ClientRect;
      GetClientRect(hwnd, &ClientRect);
      int X = ClientRect.left;
      int Y = ClientRect.top;
      int Width = ClientRect.right - ClientRect.left;
      int Height = ClientRect.bottom - ClientRect.top;
      Win32ResizeDIBSection(Width, Height);
      }
      break;
    case WM_PAINT:
      {
        PAINTSTRUCT Paint;
        HDC BitmapDeviceContext = BeginPaint(hwnd, &Paint);
        int X = Paint.rcPaint.left;
        int Y = Paint.rcPaint.top;
        int Width = Paint.rcPaint.right - Paint.rcPaint.left;
        int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
        RECT ClientRect;
        GetClientRect(hwnd, &ClientRect);
        Win32UpdateWindow(BitmapDeviceContext, &ClientRect);
        EndPaint(hwnd, &Paint);
      }
      break;
    default:
      Result = DefWindowProc(hwnd, msg, wparam, lparam);
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
  wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
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
            TranslateMessage(&Message);
            DispatchMessage(&Message);
  
            if(Message.message == WM_QUIT) 
            {
              Running = false;  
            }
          } 
          ShowGradient(xOffset, yOffset);
          HDC DeviceContext = GetDC(WindowHandle);
          RECT ClientRect;
          GetClientRect(WindowHandle, &ClientRect);
          int WindowWidth = ClientRect.right - ClientRect.left;
          int WindowHeight = ClientRect.bottom - ClientRect.top;
          Win32UpdateWindow(DeviceContext, &ClientRect);
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
