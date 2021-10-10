#include <time.h>    // clock_gettime
#include <unistd.h>  // usleep
#include <string.h>  // memset

#include <stdint.h>  // uint64_t etc
#include <stdlib.h>  // malloc
#include <stdio.h>   // printf
#include <string.h>  // memset

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include <sys/ipc.h>
#include <sys/shm.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef float  f32;
typedef double f64;

enum ui_command_type {
    DRAW_RECT,
    DRAW_LINE,
    DRAW_TEXT,
};

enum ui_direction {
    DIRECTION_NONE = 0x00,
    DIRECTION_HORIZONTAL = 0x01,
    DIRECTION_VERTICAL = 0x02,
};

struct ui_rectange {
    int x;
    int y;
    int width;
    int height;
};

struct ui_command {
    enum ui_command_type type;
    struct ui_rectange rect;
};

struct keyboard_input {
    int symbol;
    int shift;
    int ctrl;
};