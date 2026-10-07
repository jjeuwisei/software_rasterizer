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

static void RenderGradient(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
  uint8_t *Row = (uint8_t *)Buffer->Memory;
  for(int y = 0; y < Buffer->Height; y++)
    {
      uint32_t *Pixel = (uint32_t*)Row;
      for(int x = 0; x < Buffer->Width; x++)
      {
        uint8_t Blue = x + BlueOffset;
        uint8_t Green = y + GreenOffset;

        *Pixel++ = (Blue | (Green << 8 ));
      }
      Row += Buffer->Stride;
    }
}

static void GameUpdateAndRender(game_input *Input, game_offscreen_buffer *Buffer, game_sound_output_buffer *SoundBuffer)
{
  static int GreenOffset = 0;
  static int BlueOffset = 0;
  static int ToneHz = 256;

  game_controller_input *Input0 = &Input->Controllers[0];
  if(Input0->IsAnalog)
  {
    ToneHz = 256 + (int)(128.0f*(Input0->EndX));
    BlueOffset += (int)(4.0f * (Input0->EndY));
  }
  else
  {
    //Digital
  }

  if(Input0->Down.EndedDown)
  {
    GreenOffset += 1;
  }

  GameOutputSound(SoundBuffer, ToneHz);
  RenderGradient(Buffer, GreenOffset, BlueOffset);
}
