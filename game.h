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
struct debug_read_file_result
{
  uint32_t ContentsSize;
  void* Contents;
};
static debug_read_file_result DEBUGPlatformReadFile(char *file_name);
static void DEBUGPlatformFreeFileMemory(void *Memory);
static bool DEBUGPlatformWriteFile(char * File_Name, uint32_t File_Size, void *Memory);
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
  bool IsConnected;
  float32_t StickAverageX;
  float32_t StickAverageY;

  union
  {
    game_button_state Buttons[10];
    struct
    {
      game_button_state ActionUp;
      game_button_state ActionDown;
      game_button_state ActionLeft;
      game_button_state ActionRight;

      game_button_state MoveUp;
      game_button_state MoveDown;
      game_button_state MoveLeft;
      game_button_state MoveRight;

      game_button_state LeftShoulder;
      game_button_state RightShoulder;

      game_button_state Terminator;
    };
  };
};

struct game_input
{
  game_controller_input Controllers[5];
};

inline game_controller_input *GetController(game_input *Input, int ControllerIndex)
{
  Assert(ControllerIndex < ArrayCount(Input->Controllers));

  game_controller_input *Result = &Input->Controllers[ControllerIndex];
  return Result;
}

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

static void GameOutputSound(game_sound_output_buffer *SoundBuffer, int ToneHz);

static void RenderGradient(game_offscreen_buffer *Buffer, int Blueoffset, int GreenOffset);

#endif
