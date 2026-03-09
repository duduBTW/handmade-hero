#if !defined(HANDMADE_H)

#include <stdint.h>

#if HANDMADE_SLOW
#define Assert(Expression) \
  if (!(Expression))       \
  {                        \
    *(int *)0 = 0;         \
  }
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)

#define internal static
#define local_persist static
#define global_variable static
#define Pi32 3.14159265359

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef float float32;
typedef double float64;

typedef int32 bool32;

/** Services that the game provides to the platform layer. */
struct game_offscreen_buffer
{
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

struct game_button_state
{
  int HalfTransitionCount;
  bool32 EndedDown;
};

struct game_controller_input
{
  bool32 IsAnalog;

  float32 StartX;
  float32 StartY;

  float32 MinX;
  float32 MinY;

  float32 MaxX;
  float32 MaxY;

  float32 EndX;
  float32 EndY;

  game_button_state Up;
  game_button_state Down;
  game_button_state Left;
  game_button_state Right;
  game_button_state LeftShoulder;
  game_button_state RightShoulder;
};

struct game_input
{
  game_controller_input Controllers[4];
};

struct game_memory
{
  bool32 IsInitialized;
  uint64 PermanentStorageSize;
  void *PermanentStorage;

  uint64 TransientStorageSize;
  void *TransientStorage;
};

void GameUpdateHandler(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer);

struct game_state
{
  int BlueOffset;
  int GreenOffset;
};

#if HANDMADE_INTERNAL
struct debug_read_file_result
{
  void *Contents;
  int32 ContentSize;
};
debug_read_file_result DEBUGPlatformReadEntireFile(char *FileName);
void DEBUGPlatformFreeFileMemory(void *Memory);

bool32 DEBUGPlatformWriteEntireFile(char *FileName, uint32 MemorySize, void *Memory);
#endif

// Helpers

inline uint32 SafeTruncateUint64(uint64 Value)
{
  Assert(Value <= 0xFFFFFFFF);
  return (uint32)Value;
}

#define HANDMADE_H
#endif