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
#define PHANIM_INFO(message) do { TraceLog(LOG_INFO, "%s:%d: %s\n", __FILE__, __LINE__, message); abort(); } while(0)
#define PHANIM_WARN(message) do { TraceLog(LOG_WARNING, "%s:%d: %s\n", __FILE__, __LINE__, message); abort(); } while(0)

#define PHANIM_NO_ANIM ((size_t) -1)

#define vec2(cx, cy) CLITERAL(Vector2){cx, cy}
#define color(r, g, b, a) CLITERAL(Color){ r, g, b, a }

typedef unsigned char u8;
typedef enum {
    RF_LINEAR,
    RF_SINE,
    RF_SINE_PULSE,
    RF_CUBIC_SMOOTH_STEP,
    RF_QUINTIC_SMOOTH_STEP,
    RF_3B1B_SMOOTH_STEP,
} InterpFunc;

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
    Vector2 size;
    float thickness;
    Color color;
} LineData;

typedef struct {
    Vector2 center;
    float radius;
    Color color;
    float stroke_width;
    Color stroke_color;
} CircleData;

typedef struct {
    size_t id;
    ObjKind kind;
    bool should_render;
    union {
        LineData line;
        CircleData circle;
    };
} Object;

void PhanimInit(void);
void PhanimDeinit(void);
float PhanimGetTime(void);
size_t PhanimCurrentAnimId(void);
size_t PhanimAnimCount(void);
float PhanimTotalAnimTime(void);
Color PhanimGetBackground(void);
void PhanimSetBackground(Color color);

size_t PhanimCircle(Vector2 center, float radius, Color color);
size_t PhanimLine(Vector2 start, Vector2 end, Color color);

size_t PhanimMakeAnim(void *ptr, void *start, void *target, AnimValType val_type, float duration);
void PhanimChangeInterpFunc(size_t id, InterpFunc func);

void PhanimTransformPos(size_t id, Vector2 start, Vector2 target, float duration);
void PhanimFadeColor(size_t id, Color start, Color target, float duration);
size_t PhanimScaleSizeFloat(size_t id, float start, float target, float duration);
void PhanimScaleSizeVec2(size_t id, Vector2 start, Vector2 target, float duration);
void PhanimPause(float duration);

void PhanimUpdate(float dt);
void PhanimRender(void);
