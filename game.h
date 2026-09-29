#ifndef GAME_H
#define GAME_H

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

static void GameUpdateAndRender(game_offscreen_buffer *Buffer, int xOffset, int yOffset, game_sound_output_buffer *SoundBuffer);


#endif
