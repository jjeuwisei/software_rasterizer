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

static void GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer, game_sound_output_buffer *SoundBuffer)
{
  Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

  game_state *GameState = (game_state *)Memory->PermanentStorage;
  if(!Memory->IsInitialized)
  {
    char *File_Name = __FILE__;
    debug_read_file_result File_Handle = DEBUGPlatformReadFile(File_Name);
    if(File_Handle.Contents)
    {
      DEBUGPlatformWriteFile("c:/repos/software_rasterizer/test.c", File_Handle.ContentsSize, File_Handle.Contents);
      DEBUGPlatformFreeFileMemory(File_Handle.Contents);
    }
    else
    {
      //fail!!
    }
    GameState->ToneHz = 256;
    Memory->IsInitialized = true;
  }

  game_controller_input *Input0 = &Input->Controllers[0];
  if(Input0->IsAnalog)
  {
    GameState->ToneHz = 256 + (int)(128.0f*(Input0->EndY));
    GameState->BlueOffset += (int)(4.0f * (Input0->EndX));
  }
  else
  {
    //Digital
  }

  if(Input0->Down.EndedDown)
  {
    GameState->GreenOffset += 1;
  }

  GameOutputSound(SoundBuffer, GameState->ToneHz);
  RenderGradient(Buffer, GameState->GreenOffset, GameState->BlueOffset);
}
