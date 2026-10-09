#ifndef GAME_H
#define GAME_H

#if SLOW_BUILD
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define ArrayCount(Array) sizeof(Array) / sizeof((Array)[0])

#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)

#if INTERNAL_BUILD
static void *DEBUGPlatformReadFile(char *file_name);
static void DEBUGPlatformFreeFileMemory(void *Memory);
static bool DEBUGPlatformWriteEntireFile(void *Memory);
#endif

inline uint32_t SafeTruncateUInt64(uint64_t Value)
{
  Assert(Value <= 0xFFFFFF);
  uint32_t Result = (uint32_t) Value;
  return Result;
}

struct game_offscreen_buffer
{
  void *Memory;
  int Width;
  int Height;
  int Stride;
};

struct game_sound_output_buffer
{
  int SampleCount;
  int SamplesPerSecond;
  int16_t *Samples;
};

struct game_button_state
{
  int HalfTransitionCount;
  bool EndedDown;
};

struct game_controller_input
{
  bool IsAnalog;
  float32_t StartX;
  float32_t StartY;

  float32_t MinX;
  float32_t MinY;

  float32_t MaxX;
  float32_t MaxY;

  float32_t EndX;
  float32_t EndY;
  union
  {
    game_button_state Buttons[6];
    struct
    {
      game_button_state Up;
      game_button_state Down;
      game_button_state Left;
      game_button_state Right;
      game_button_state LeftShoulder;
      game_button_state RightShoulder;
    };
  };
};

struct game_input
{
  game_controller_input Controllers[4];
};

struct game_memory
{
  bool IsInitialized;
  uint64_t PermanentStorageSize;
  void *PermanentStorage;
  uint64_t TransientStorageSize;
  void *TransientStorage;
};

struct game_state
{
  int ToneHz;
  int GreenOffset;
  int BlueOffset;
};

static void GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer, game_sound_output_buffer *SoundBuffer);

static void GameOutputSound(game_sound_output_buffer *SoundBuffer);

static void RenderGradient(game_offscreen_buffer *Buffer, int Blueoffset, int GreenOffset);

#endif
