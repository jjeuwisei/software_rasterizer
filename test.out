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
  Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) == (ArrayCount(Input->Controllers[0].Buttons)));
  Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

  game_state *GameState = (game_state *)Memory->PermanentStorage;
  if(!Memory->IsInitialized)
  {
    char *File_Name = __FILE__;
    debug_read_file_result File_Handle = DEBUGPlatformReadFile(File_Name);
    if(File_Handle.Contents)
    {
      DEBUGPlatformWriteFile("test.out", File_Handle.ContentsSize, File_Handle.Contents);
      DEBUGPlatformFreeFileMemory(File_Handle.Contents);
    }
    else
    {
      //fail!!
    }
    GameState->ToneHz = 256;
    Memory->IsInitialized = true;
  }
  
  for(int ControllerIndex = 0;
      ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
  {
    game_controller_input *Controller = GetController(Input, ControllerIndex);
    if(Controller->IsAnalog)
    {
      GameState->ToneHz = 256 + (int)(128.0f*(Controller->StickAverageY));
      GameState->BlueOffset += (int)(4.0f * (Controller->StickAverageX));
    }
    else
    {
      //Digital
      if(Controller->MoveLeft.EndedDown)
      {
        GameState->GreenOffset -= 1;
      }
      if(Controller->MoveRight.EndedDown)
      {
        GameState->GreenOffset += 1;
      }
      if(Controller->MoveUp.EndedDown)
      {
        GameState->BlueOffset -= 1;
      }
      if(Controller->MoveDown.EndedDown)
      {
        GameState->BlueOffset += 1;
      }
    }
    if(Controller->ActionDown.EndedDown)
    {
      GameState->BlueOffset += 1;
    }
  }

  GameOutputSound(SoundBuffer, GameState->ToneHz);
  RenderGradient(Buffer, GameState->GreenOffset, GameState->BlueOffset);
}
