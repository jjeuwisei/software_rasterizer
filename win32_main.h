#ifndef WIN32_MAIN_H
#define WIN32_MAIN_H

struct RGBBuffer{
  BITMAPINFO Info;
  void *Memory;
  int Width;
  int Height;
  int Stride;
};

struct WindowDimensions {
int Width;
  int Height;
};

struct win32_sound_output
{

  int SamplesPerSecond;
  int RunningSampleIndex;
  int BytesPerSample;
  int SecondaryBufferSize;
  int LatencySampleCount;
  float32_t tSine;
};

#endif
