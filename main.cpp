#include <windows.h>
#include <xinput.h>
#include <stdint.h>
#include <dsound.h>
#include <math.h>
#include <malloc.h>

using namespace std;

typedef float float32_t;

#define Pi 3.1415927f

static bool Running;
static BITMAPINFO BitmapInfo;
static int BitmapWidth;
static int BitmapHeight;
static LPDIRECTSOUNDBUFFER SecondaryBuffer;

#include "game.cpp"
#include "win32_main.h"

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

static debug_read_file_result DEBUGPlatformReadFile(char *File_Name)
{
    debug_read_file_result Result = {};
    HANDLE File_Handle = CreateFileA(File_Name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(File_Handle != INVALID_HANDLE_VALUE)
    {
      LARGE_INTEGER File_Size;
      if(GetFileSizeEx(File_Handle, &File_Size))
      {            
        uint32_t File_Size32 = SafeTruncateUInt64(File_Size.QuadPart);
        Result.Contents = VirtualAlloc(0, File_Size32, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
        if(Result.Contents)
        {
          DWORD BytesRead;
          if(ReadFile(File_Handle, Result.Contents, File_Size32, &BytesRead, 0) && (File_Size32 == BytesRead))
          {
            Result.ContentsSize = BytesRead;
          }
          else
          {
            DEBUGPlatformFreeFileMemory(Result.Contents);
            Result.Contents = 0;
          }
        }
        else
        {
        }
      }
      else
      {
      }
      CloseHandle(File_Handle);
    }
    return Result;
}

static void DEBUGPlatformFreeFileMemory(void *Memory)
{
  if(Memory)
  {
    VirtualFree(Memory, 0, MEM_RELEASE);
  }
}

static bool DEBUGPlatformWriteFile(char *File_Name, uint32_t File_Size, void *Memory)
{
    bool Result = false;
    HANDLE File_Handle = CreateFileA(File_Name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(File_Handle != INVALID_HANDLE_VALUE)
    {
      DWORD BytesWritten;
      if(WriteFile(File_Handle, Memory, File_Size, &BytesWritten, 0))
      {
        Result = (BytesWritten == File_Size);
      }
      else
      {
      }
      CloseHandle(File_Handle);
    }
    else
    {
    }
    return Result;
}

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
        uint8_t green = y;
        uint8_t red = x;
        *Pixel++ = ((red << 16) | (green << 8));
      }
      Row += Buffer->Stride;
    }
}

static RGBBuffer GlobalBackBuffer;

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
  Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
  Buffer->Stride = Buffer->Width * BytesPerPixel;
} 

static void Win32DisplayBuffer(HDC BitmapDeviceContext, RGBBuffer *Buffer, int WindowWidth, int WindowHeight)
{
  StretchDIBits(BitmapDeviceContext, 0, 0, WindowWidth, 
                WindowHeight, 0, 0, Buffer->Width, Buffer->Height, 
                Buffer->Memory, &Buffer->Info, DIB_RGB_COLORS, SRCCOPY);
}

static void Win32ClearBuffer(win32_sound_output *SoundOutput)
{
  VOID *Region1;
  DWORD Region1Size;
  VOID *Region2;
  DWORD Region2Size;
  if(SUCCEEDED(SecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize, 
      &Region1, &Region1Size, 
      &Region2, &Region2Size, 0)))
    // DO an assert that region1 and region 2: is valid
  {
    uint8_t *DestSample = (uint8_t *)Region1;
   
    for(DWORD ByteIndex = 0; ByteIndex < Region1Size; 
        ++ByteIndex)
    {
      *DestSample++ = 0; 
    }
    DestSample = (uint8_t *)Region2;
   
    for(DWORD ByteIndex = 0; ByteIndex < Region2Size; 
        ++ByteIndex)
    {
      *DestSample++ = 0; 
    }
    SecondaryBuffer->Unlock(Region1, Region1Size, 
      Region2, Region2Size);
  }
}

void Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD BytesToLock, DWORD BytesToWrite, game_sound_output_buffer *SourceBuffer)
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
    int16_t *DestSample = (int16_t *)Region1;
    int16_t *SourceSample = SourceBuffer->Samples; 
    for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; 
        ++SampleIndex)
    {
      *DestSample++ =  *SourceSample++;
      *DestSample++ =  *SourceSample++;
      SoundOutput->RunningSampleIndex++;
    }
    DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
    DestSample = (int16_t *)Region2;
    for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; 
        ++SampleIndex)
    {
      *DestSample++ =  *SourceSample++;
      *DestSample++ =  *SourceSample++;
      SoundOutput->RunningSampleIndex++;
    }
    SecondaryBuffer->Unlock(Region1, Region1Size, 
      Region2, Region2Size);
  }
}


static void Win32ProcessKeyboardInput(game_button_state *NewState, bool IsDown)
{
  NewState->EndedDown = IsDown;
  ++NewState->HalfTransitionCount;
}

static void Win32ProcessXInput(DWORD XInputButtonState, game_button_state *OldState, DWORD ButtonBit, game_button_state *NewState)
{
  NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
  NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

LRESULT CALLBACK Win32MainWindowCallback(HWND WindowHandle, UINT msg, WPARAM wparam, LPARAM lparam)
{
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
  LARGE_INTEGER QueryPerformanceResult;
  QueryPerformanceFrequency(&QueryPerformanceResult);
  uint64_t QueryPerformance = QueryPerformanceResult.QuadPart;
  WNDCLASS wc = {};
  
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
        SoundOutput.BytesPerSample = sizeof(int16_t) * 2;
        SoundOutput.SecondaryBufferSize = 
        SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
        SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 60 ;        

        Win32InitDSound(WindowHandle, SoundOutput.SamplesPerSecond, 
            SoundOutput.SecondaryBufferSize);
        Win32ClearBuffer(&SoundOutput);
        SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

#if INTERNAL_BUILD
        LPVOID BaseAddress = (LPVOID)(Terabytes((uint64_t)2));
#else
        LPVOID BaseAddress = 0;
#endif
        int16_t *Samples = (int16_t *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

        game_memory GameMemory = {};
        GameMemory.PermanentStorageSize = Megabytes(64); 
        GameMemory.TransientStorageSize = Gigabytes((uint64_t)4); 
        uint64_t TotalStorageSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
        GameMemory.PermanentStorage =  VirtualAlloc(BaseAddress, TotalStorageSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
        GameMemory.TransientStorage = (uint8_t *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize;

        if(Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage)
        {
          game_input Input[2] = {};
          game_input *NewInput = &Input[0];
          game_input *OldInput = &Input[1];
          
          LARGE_INTEGER LastCounter;
          QueryPerformanceCounter(&LastCounter);
          uint64_t CycleCount = __rdtsc();
          while(Running) 
          {
            MSG Message;
            game_controller_input *KeyboardController = &NewInput->Controllers[0];
            game_controller_input TempController = {};
            *KeyboardController = TempController;

            while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE) > 0) 
            {
              if(Message.message == WM_QUIT) 
              {
                Running = false;  
              }
              switch(Message.message)
              {
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_KEYUP:
                case WM_KEYDOWN:
                {
                  uint32_t VKCode = (uint32_t)Message.wParam;
                  bool WasDown = ((Message.lParam  & (1 << 30)) != 0);
                  bool IsDown = ((Message.lParam & (1 << 31)) == 0);
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
                    else if (VKCode == 0x28)
                    {
                      Win32ProcessKeyboardInput(&KeyboardController->Down, IsDown);
                    }
                    else if (VKCode == 'VK_UP')
                    {

                    }
                  }
                  bool AltKeyWasDown = (Message.lParam & (1 << 29));
                  if((VKCode == VK_F4) && AltKeyWasDown)
                  {
                    Running = false;
                  }
                }break;
              default:
              {
              TranslateMessage(&Message);
              DispatchMessage(&Message);
              }
              }
            } 
            
            size_t MaxControllerCount = XUSER_MAX_COUNT;
            if(MaxControllerCount > ArrayCount(NewInput->Controllers))
            {
              MaxControllerCount = ArrayCount(NewInput->Controllers);
            }
            for(DWORD ControllerIndex = 0;
                ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex)
            {
              game_controller_input *OldController = &OldInput->Controllers[ControllerIndex];
              game_controller_input *NewController = &NewInput->Controllers[ControllerIndex];

              XINPUT_STATE ControllerState;
              if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
              {
                  XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                  bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                  bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                  bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                  bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                 
                  NewController->IsAnalog = true;
                  NewController->StartX = OldController->EndX;
                  NewController->StartY = OldController->EndY;

                  float32_t X;
                  if(Pad->sThumbLX < 0)
                  {
                    X = (float32_t) (Pad->sThumbLX / 32768.0f);
                  }
                  else
                  {
                    X = (float32_t) (Pad->sThumbLX / 32767.0f);
                  }
                  NewController->MinX = NewController->MaxY = NewController->EndX= X;

                  float32_t Y;
                  if(Pad->sThumbLY < 0)
                  {
                    Y = (float32_t) (Pad->sThumbLY / 32768.0f);
                  }
                  else
                  {
                    Y = (float32_t) (Pad->sThumbLY / 32767.0f);
                  }
                  NewController->MinY = NewController->MaxY = NewController->EndY = Y;

                  Win32ProcessXInput(Pad->wButtons, &OldController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER, &NewController->LeftShoulder);
                  Win32ProcessXInput(Pad->wButtons, &OldController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER, &NewController->RightShoulder);
                  Win32ProcessXInput(Pad->wButtons, &OldController->Down, XINPUT_GAMEPAD_A, &NewController->Down);
                  Win32ProcessXInput(Pad->wButtons, &OldController->Right, XINPUT_GAMEPAD_B, &NewController->Right);
                  Win32ProcessXInput(Pad->wButtons, &OldController->Left, XINPUT_GAMEPAD_X, &NewController->Left);
                  Win32ProcessXInput(Pad->wButtons, &OldController->Up, XINPUT_GAMEPAD_Y, &NewController->Up);

                  // bool right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                  // bool start = (Pad->wButtons & XINPUT_GAMEPAD_START);
                  
              } 
            }
            DWORD BytesToLock = 0;
            DWORD TargetCursor = 0;
            DWORD BytesToWrite = 0;
            DWORD PlayCursor = 0;
            DWORD WriteCursor = 0 ;
            bool SoundIsValid = false;
            
            if(SUCCEEDED(SecondaryBuffer->GetCurrentPosition(&PlayCursor, 
                &WriteCursor)))
            {
              BytesToLock = (SoundOutput.RunningSampleIndex 
                * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
              TargetCursor = (PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize;
              if(BytesToLock == TargetCursor)
              {
                  BytesToWrite = 0;
              }
              else if(BytesToLock > TargetCursor)
              {
                BytesToWrite = SoundOutput.SecondaryBufferSize - BytesToLock;
                BytesToWrite += TargetCursor;
              }
              else 
              {
                BytesToWrite = TargetCursor - BytesToLock ;
              }
              SoundIsValid = true;
            }
            game_sound_output_buffer SoundBuffer = {};
            SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
            SoundBuffer.Samples = Samples;
            SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
            game_offscreen_buffer Buffer = {};
            Buffer.Memory = GlobalBackBuffer.Memory;
            Buffer.Width = GlobalBackBuffer.Width;
            Buffer.Height = GlobalBackBuffer.Height;
            Buffer.Stride = GlobalBackBuffer.Stride;
            GameUpdateAndRender(&GameMemory, NewInput, &Buffer, &SoundBuffer);
            //Direct Sound Output
            if(SoundIsValid)
            { 
              Win32FillSoundBuffer(&SoundOutput, BytesToLock, BytesToWrite, &SoundBuffer);
            }

            WindowDimensions Dimensions = Win32GetWindowDimensions(WindowHandle);
            Win32DisplayBuffer(DeviceContext, &GlobalBackBuffer, 
                Dimensions.Width, Dimensions.Height);

            uint64_t CycleCount = __rdtsc();
            LARGE_INTEGER EndCounter;
            QueryPerformanceCounter(&EndCounter);

            int64_t CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
            float32_t MsPerFrame = (float32_t) (1000.0f * CounterElapsed) / (float32_t)QueryPerformance;
            float32_t FPS = (float32_t) QueryPerformance / (float32_t) CounterElapsed;

            LastCounter = EndCounter;

            game_input *Temp = NewInput;
            NewInput = OldInput;
            OldInput = Temp; 
          }
        }
        else
        {

        }
      }
    }
  return 0;
}

