#include "game.h"

static void GameOutputSound(game_sound_output_buffer *SoundBuffer, int ToneHz)
{
  static float32_t tSine;
  int16_t ToneVolume = 3000;
  int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz; 

  int16_t *SampleOut = SoundBuffer->Samples;
  for(int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; 
      ++SampleIndex)
  {
    float32_t SineValue = sinf(tSine); 
    int16_t SampleValue = (int16_t)(SineValue * ToneVolume);
    *SampleOut++ =  SampleValue;
    *SampleOut++ =  SampleValue;

    tSine += 2.0 * Pi * 1.0f / WavePeriod;
  }
}

static void RenderGradient(game_offscreen_buffer *Buffer, int xOffset, int yOffset)
{
  uint8_t *Row = (uint8_t *)Buffer->Memory;
  for(int y = 0; y < Buffer->Height; y++)
    {
      uint32_t *Pixel = (uint32_t*)Row;
      for(int x = 0; x < Buffer->Width; x++)
      {
        uint8_t Green = x + xOffset;
        uint8_t Red = y + yOffset;

        *Pixel++ = (Red << 16) | (Green << 8 );
      }
      Row += Buffer->Stride;
    }
}

static void GameUpdateAndRender(game_offscreen_buffer *Buffer, game_sound_output_buffer *SoundBuffer, int xOffset, int yOffset, int ToneHz)
{
  GameOutputSound(SoundBuffer, ToneHz);
  RenderGradient(Buffer, xOffset, yOffset);
}
