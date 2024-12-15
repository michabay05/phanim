#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "raylib.h"
#include "raymath.h"
#include "arena.h"

// Taken from 'https://github.com/tsoding/nob.h/blob/main/nob.h'
#define PHANIM_UNUSED(value) (void)(value)
#define PHANIM_TODO(message) do { TraceLog(LOG_WARNING, "%s:%d: TODO: %s\n", __FILE__, __LINE__, message); abort(); } while(0)
#define PHANIM_UNREACHABLE(message) do { TraceLog(LOG_WARNING, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while(0)

#define vec2(cx, cy) CLITERAL(Vector2){cx, cy}
#define color(r, g, b, a) CLITERAL(Color){ r, g, b, a }

typedef unsigned char u8;
typedef enum {
    RF_LINEAR,
    RF_SINE,
    RF_CUBIC_SMOOTH_STEP,
    RF_QUINTIC_SMOOTH_STEP,
    RF_3B1B_SMOOTH_STEP,
} RateFunc;

typedef enum {
    OK_LINE,
    OK_RECT,
    OK_CIRCLE,
} ObjKind;

typedef enum {
    AK_CREATE,
    AK_POSITION_TRANSFORM,
    AK_COLOR_FADE,
    AK_SCALE,
    AK_PAUSE,
} AnimKind;

typedef enum {
    AVT_U8,
    AVT_FLOAT,
    AVT_VEC2,
    AVT_COLOR,
} AnimValType;

typedef struct {
    Vector2 start;
    Vector2 end;
    float stroke_width;
    Color color;
} LineData;

typedef struct {
    Vector2 center;
    float radius;
    Color color;
    float stroke_width;
    Color stroke_color;
} Circle;

typedef struct {
    ObjKind kind;
    bool should_render;
    union {
        Circle circle;
    };
} Object;

void phanim_init(void);
void phanim_deinit(void);
float phanim_get_time(void);
size_t phanim_anim_count(void);
float phanim_total_anim_time(void);
void phanim_make_anim(void *ptr, void *start, void *target, AnimValType val_type, float duration);
size_t phanim_circle(Vector2 center, float radius, Color color);
void phanim_move(size_t id, Vector2 target, float duration);
void phanim_fade(size_t id, u8 target, float duration);
void phanim_pause(float duration);
bool phanim_update(float dt);
void phanim_render(void);
