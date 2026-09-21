#include <windows.h>
#include <xinput.h>
#include <stdint.h>
#include <dsound.h>
#include <math.h>
using namespace std;

typedef float float32_t;

#define Pi 3.1415927f

static bool Running;
static BITMAPINFO BitmapInfo;
static int BitmapWidth;
static int BitmapHeight;
static LPDIRECTSOUNDBUFFER SecondaryBuffer;


#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
  return (ERROR_DEVICE_NOT_CONNECTED);
}
static x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_


#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
  return 0;
}

static x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_


#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create); 

static void Win32LoadXInput(void)
{
  HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
  if(!XInputLibrary)
  {
    XInputLibrary = LoadLibraryA("xinput1_3.dll");
  }
  if(XInputLibrary)
  {
    XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
    XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
  }
}

static void Win32InitDSound(HWND Window, int32_t SamplesPerSecond, int32_t BufferSize)
{
  HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

  if(DSoundLibrary)
  {
    
    direct_sound_create *DirectSoundCreate = 
      (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
    LPDIRECTSOUND DirectSound;

    if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
    {      
      WAVEFORMATEX WaveFormat = {};
      WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
      WaveFormat.nChannels = 2;
      WaveFormat.nSamplesPerSec = SamplesPerSecond;
      WaveFormat.wBitsPerSample = 16;
      WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8;
      WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
      WaveFormat.cbSize = 0;

      if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
      {
        //primary buffer
        //
        DSBUFFERDESC BufferDescription = {};
        BufferDescription.dwSize = sizeof(BufferDescription);
        BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
        LPDIRECTSOUNDBUFFER PrimaryBuffer;
        
        if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
        {
          HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
          if(SUCCEEDED(Error))
            {
              OutputDebugStringA("primary buffer format set");
              // finally set the primary buffer
            }
          else
            {

              //
            }
        }
        else
        {

          //
        }
      
      }
      else
      {
        //Setting cooperative level failed
      }

      //Secondary Buffer
      DSBUFFERDESC BufferDescription = {};
      BufferDescription.dwSize = sizeof(BufferDescription);
      BufferDescription.dwFlags = 0;
      BufferDescription.dwBufferBytes = BufferSize;
      BufferDescription.lpwfxFormat = &WaveFormat;

      HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &SecondaryBuffer, 0);
      if(SUCCEEDED(Error))
      {
        OutputDebugStringA("secondary buffer created");
      }
      else
      {

      }
    
    }
    else
    {

    }
  }
  else
  {

  }
}
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

static void Win32ShowGradient(RGBBuffer *Buffer, int xOffset, int yOffset)
{
  uint8_t *Row = (uint8_t *)Buffer->Memory;
    for(int y = 0; y < Buffer->Height; ++y) 
    {
      uint32_t *Pixel = (uint32_t *)Row;
      for(int x = 0; x < Buffer->Width; ++x)
      {
        uint8_t green = y + yOffset;
        uint8_t red = x + xOffset;

        *Pixel++ = ((red << 16) | (green << 8));
      }
      Row += Buffer->Stride;
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


static void Win32DisplayBuffer(HDC BitmapDeviceContext, RGBBuffer *Buffer, int WindowWidth, int WindowHeight)
{
  StretchDIBits(BitmapDeviceContext, 0, 0, WindowWidth, 
                WindowHeight, 0, 0, Buffer->Width, Buffer->Height, 
                Buffer->Memory, &Buffer->Info, DIB_RGB_COLORS, SRCCOPY);
}

struct win32_sound_output
{

  int SamplesPerSecond;
  int RunningSampleIndex;
  int ToneVolume;
  int ToneHz;
  int SquareWavePeriod;
  int WavePeriod;
  int BytesPerSample;
  int SecondaryBufferSize;
};

void Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD BytesToLock, DWORD BytesToWrite)
{

  VOID *Region1;
  DWORD Region1Size;
  VOID *Region2;
  DWORD Region2Size;
  if(SUCCEEDED(SecondaryBuffer->Lock(BytesToLock, BytesToWrite, 
      &Region1, &Region1Size, 
      &Region2, &Region2Size, 0)))
    // DO an assert that region1 and region 2 is valid
  {

    DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
    int16_t *SampleOut = (int16_t *)Region1;
    for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; 
        ++SampleIndex)
    {
      float32_t t = 2.0 * Pi * (float32_t)SoundOutput->RunningSampleIndex 
        / (float32_t)SoundOutput->WavePeriod;
      float32_t SineValue = sinf(t);
      int16_t SampleValue = (int16_t)(SineValue * SoundOutput->ToneVolume);
      *SampleOut++ =  SampleValue;
      *SampleOut++ =  SampleValue;
      SoundOutput->RunningSampleIndex++;
    }

    DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
    SampleOut = (int16_t *)Region2;
    for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; 
        ++SampleIndex)
    {
      float32_t t = 2.0 * Pi * (float32_t)SoundOutput->RunningSampleIndex 
        / (float32_t)SoundOutput->WavePeriod;
      float32_t SineValue = sinf(t);
      int16_t SampleValue = (int16_t)(SineValue * SoundOutput->ToneVolume);
      *SampleOut++ = SampleValue;
      *SampleOut++ = SampleValue;
      SoundOutput->RunningSampleIndex++;
    }
  }
  SecondaryBuffer->Unlock(Region1, Region1Size, 
      Region2, Region2Size);
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
    case WM_KEYUP:
    {
      uint32_t VKCode = wparam;
      bool WasDown = ((lparam  & (1 << 30)) != 0);
      bool IsDown = ((lparam & (1 << 31)) == 0);
      if(WasDown != IsDown)
      {
        if(VKCode == 'W')
        {
           
        }
        else if (VKCode == 'A')
        {

        }
        else if (VKCode == 'S')
        {

        }
        else if (VKCode == 'D')
        {

        }
        else if (VKCode == 'Q')
        {

        }
        else if (VKCode == 'E')
        {

        }
        else if (VKCode == 'A')
        {

        }
        else if (VKCode == 'A')
        {

        }
      }
    }break;
    case WM_KEYDOWN:
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
        Win32DisplayBuffer(BitmapDeviceContext, &GlobalBackBuffer, Dimensions.Width, Dimensions.Height);
        EndPaint(WindowHandle, &Paint);
      }
      break;
    default:
      Result = DefWindowProc(WindowHandle, msg, wparam, lparam);
  }
  return Result;
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
      Win32LoadXInput();    
      HWND WindowHandle = CreateWindowEx(
      0, 
      wc.lpszClassName, 
      "window!!", 
      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
      nullptr, nullptr, Instance, nullptr);
      if (WindowHandle) 
      {
        HDC DeviceContext = GetDC(WindowHandle);
        Running = true;
        bool SoundIsPlaying = false;
        
        win32_sound_output SoundOutput = {};
        SoundOutput.SamplesPerSecond = 48000 ;
        SoundOutput.RunningSampleIndex = 0;
        SoundOutput.ToneVolume = 16000;
        SoundOutput.ToneHz = 256;
        SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond/SoundOutput.ToneHz;
        SoundOutput.BytesPerSample = sizeof(int16_t) * 2;
        SoundOutput.SecondaryBufferSize = 
          SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
        
        Win32InitDSound(WindowHandle, SoundOutput.SamplesPerSecond, 
            SoundOutput.SecondaryBufferSize);
        Win32FillSoundBuffer(&SoundOutput, 0, SoundOutput.SecondaryBufferSize);
        SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

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
          
          for(DWORD ControllerIndex = 0;
              ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex)
          {
            XINPUT_STATE ControllerState;
            if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
            {
                XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                bool up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                bool down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                bool left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                bool right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                bool start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                bool back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                bool leftshoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                bool rightshoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                bool Abt= (Pad->wButtons & XINPUT_GAMEPAD_A);
                bool Bbt = (Pad->wButtons & XINPUT_GAMEPAD_B);
                bool Xbt= (Pad->wButtons & XINPUT_GAMEPAD_X);
                bool Ybt = (Pad->wButtons & XINPUT_GAMEPAD_Y);
                
                int16_t StickX = Pad->sThumbLX;
                int16_t StickY = Pad->sThumbLY;

                xOffset += StickX >> 12;
                yOffset += StickY >> 12;
                
                XINPUT_VIBRATION Vibration;
                Vibration.wLeftMotorSpeed = 65535;
                Vibration.wRightMotorSpeed = 65535;
                XInputSetState(0, &Vibration);
            } 
          }

          Win32ShowGradient(&GlobalBackBuffer, xOffset, yOffset);
          //Direct Sound Output
          
          DWORD PlayCursor;
          DWORD WriteCursor;
          if(SUCCEEDED(SecondaryBuffer->GetCurrentPosition(&PlayCursor, 
              &WriteCursor)))
          { 
            DWORD BytesToLock = (SoundOutput.RunningSampleIndex 
              * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
            DWORD BytesToWrite;
            if(BytesToLock == PlayCursor)
            {
                BytesToWrite = 0;
            }
            else if(BytesToLock > PlayCursor)
            {
              BytesToWrite = SoundOutput.SecondaryBufferSize - BytesToLock;
              BytesToWrite += PlayCursor;
            }
            else 
            {
              BytesToWrite = PlayCursor - BytesToLock ;
            }
          Win32FillSoundBuffer(&SoundOutput, BytesToLock, BytesToWrite);
        }
        
          WindowDimensions Dimensions = Win32GetWindowDimensions(WindowHandle);
          Win32DisplayBuffer(DeviceContext, &GlobalBackBuffer, 
              Dimensions.Width, Dimensions.Height);
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
