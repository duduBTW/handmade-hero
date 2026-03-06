#include "handmade.h"

void RenderWeirdGradient(game_offscreen_buffer *Buffer, int xOffset, int yOffset)
{
  uint8 *Row = (uint8 *)Buffer->Memory;
  for (int Y = 0; Y < Buffer->Height; ++Y)
  {
    uint32 *Pixel = (uint32 *)Row;
    for (int X = 0; X < Buffer->Width; ++X)
    {
      uint8 Blue = X + xOffset;
      uint8 Green = Y + yOffset;
      uint8 Red = 255;
      *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
    }

    Row += Buffer->Pitch;
  }
}

void GameUpdateHandler(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
{
  Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

  game_state *GameState = (game_state *)Memory->PermanentStorage;
  if (!Memory->IsInitialized)
  {
    Memory->IsInitialized = true;
  }

  RenderWeirdGradient(Buffer, GameState->BlueOffset, GameState->GreenOffset);

  GameState->BlueOffset++;
  GameState->GreenOffset++;
}