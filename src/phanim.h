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

typedef enum {
    RF_LINEAR,
    RF_SINE,
    RF_CUBIC_SMOOTH_STEP,
    RF_QUINTIC_SMOOTH_STEP,
    RF_3B1B_SMOOTH_STEP,
    RF_SHADER_SMOOTHSTEP,
} RateFunc;

typedef enum {
    AOK_LINE,
    AOK_RECT,
    AOK_CIRCLE,
} AnimObjKind;

typedef enum {
    AK_CREATE,
    AK_POSITION_TRANSFORM,
    AK_COLOR_FADE,
    AK_SCALE,
} AnimKind;

typedef struct {
    Vector2 start;
    Vector2 end;
    float stroke_width;
    Color color;
} Line;

typedef struct Anim {
    void *start;
    void *ptr;
    void *target;
    AnimObjKind obj_kind;
    AnimKind anim_kind;
    float anim_time;
    float duration;
} Anim;

bool phanim_init();
size_t phanim_add(Anim anim);
void phanim_animate(size_t id, float dt);
