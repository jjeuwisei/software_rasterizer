#include <windows.h>
using namespace std;

static bool Running;

LRESULT CALLBACK MainWindowCallback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
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
    case WM_PAINT:
      {
        PAINTSTRUCT Paint;
        HDC hdc = BeginPaint(hwnd, &Paint);
        int X = Paint.rcPaint.left;
        int Y = Paint.rcPaint.top;
        int height = Paint.rcPaint.bottom - Paint.rcPaint.top;
        int width = Paint.rcPaint.right - Paint.rcPaint.left;
        static DWORD Operation = WHITENESS;
        PatBlt(hdc, X, Y, width, height, Operation);
        if(Operation == WHITENESS)
        { 
          Operation = BLACKNESS;
        }
        else
        { 
          Operation = WHITENESS;
        }
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
  wc.lpfnWndProc = MainWindowCallback;
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
        while(Running) 
        {
          MSG Message;
          BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
          if(MessageResult > 0)
          {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
          }
          else 
          {
            break;
          }
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
