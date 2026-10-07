#ifndef GAME_H
#define GAME_H

#define ArrayCount(Array) sizeof(Array) / sizeof((Array)[0])
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

static void GameUpdateAndRender(game_input *Input, game_offscreen_buffer *Buffer, game_sound_output_buffer *SoundBuffer);

static void GameOutputSound(game_sound_output_buffer *SoundBuffer);

static void RenderGradient(game_offscreen_buffer *Buffer, int Blueoffset, int GreenOffset);

#endif
