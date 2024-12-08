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
    AK_PAUSE,
} AnimKind;

typedef enum {
    AVT_SCALAR,
    AVT_VEC2,
    AVT_COLOR,
} AnimValType;

typedef struct {
    Vector2 start;
    Vector2 end;
    float stroke_width;
    Color color;
} Line;

// TODO: add a group id to group anims that should happen at the same time
typedef struct {
    void *ptr;
    void *target;
    AnimValType val_type;
    float anim_time;
    float duration;
} Anim;

bool phanim_init();
void phanim_make_anim(void *obj, void *target, AnimValType val_type, float duration);
void phanim_pause(float duration);
void phanim_update(float dt);
