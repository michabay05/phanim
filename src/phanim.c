#include <assert.h>

// TODOs
//   - Groups
//   - Smooth interpolations
//   - Rendering into a video
//   - Mouse position to screen unit

#define ARENA_IMPLEMENTATION
#include "phanim.h"

#define DEFAULT_INIT_CAP 10

typedef struct {
    Anim *items;
    size_t anim_count, anim_capacity;
    size_t anim_current;
    bool anim_completed;
    Arena arena;
    float time;
} Phanim;

static Phanim CORE = {0};

static float rate_func(RateFunc func, float anim_time, float duration);
static float cubic_smoothstep(float x, float min, float max);
static float quintic_smoothstep(float x, float min, float max);
static size_t phanim_add(Anim anim);
static void phanim_direct_render(void *ptr, AnimObjKind obj);
// static void phanim_anim_line(Anim *a, float t);
// static void phanim_anim_circle(Anim *a, float t);
static void anim_print(Anim *a);

void phanim_init(void)
{
    CORE.items = NULL;
    CORE.anim_count = 0;
    CORE.anim_capacity = DEFAULT_INIT_CAP;
    CORE.anim_current = 0;
    CORE.anim_completed = false;
    CORE.arena = (Arena) {0};
    CORE.time = 0.0f;
}

void phanim_deinit(void)
{
    arena_free(&CORE.arena);
}

void phanim_make_anim(void *obj, void *target, AnimValType val_type, float duration)
{
    Anim a = {
        .ptr = obj,
        .target = target,
        .val_type = val_type,
        .anim_time = 0.0,
        .duration = duration
    };

    phanim_add(a);
}

void phanim_pause(float duration)
{
    Anim a = {
        .ptr = NULL,
        .target = NULL,
        .val_type = AVT_SCALAR,
        .anim_time = 0.0,
        .duration = duration
    };

    phanim_add(a);
}

// Returns 'true' if animation is complete
bool phanim_update(float dt)
{
    // TraceLog(LOG_INFO, "anim_current = %d", CORE.anim_current);
    // TraceLog(LOG_INFO, "anim_count = %d", CORE.anim_count);
    if (CORE.anim_completed) return true;
    CORE.time += dt;

    Anim *a = &CORE.items[CORE.anim_current];
    if (a->anim_time >= a->duration) {
        CORE.anim_current += 1;
        if (CORE.anim_current < CORE.anim_count) {
            a = &CORE.items[CORE.anim_current];
        } else {
            CORE.anim_completed = true;
            return true;
        }
    }
    if (a->anim_time < a->duration) {
        a->anim_time += dt;
    }
    // float t = rate_func(RF_LINEAR, a->anim_time, a->duration);
    float t = quintic_smoothstep(a->anim_time, 0.0f, a->duration);

    switch (a->val_type) {
        case AVT_VEC2: {
            Vector2 *ptr = (Vector2*)a->ptr;
            Vector2 target = *(Vector2*)a->target;
            *ptr = Vector2Lerp(*ptr, target, t);
        } break;

        case AVT_SCALAR: {
            if (a->ptr == NULL && a->target == NULL) {
                // Probably, a pause scene
            }
        } break;

        default: {
            PHANIM_TODO("Implement phanim_update() for AVT_SCALAR && AVT_COLOR");
        } break;
    }

    return false;
}

void phanim_reset_anim(void)
{
    CORE.anim_completed = false;
    CORE.anim_current = 0;
    for (size_t i = 0; i < CORE.anim_count; i++) {
        Anim *a = &CORE.items[i];
        a->anim_time = 0.0f;
    }
}

static size_t phanim_add(Anim anim)
{
    if (CORE.anim_count >= CORE.anim_capacity) {
        size_t new_cap = CORE.anim_capacity == 0 ? DEFAULT_INIT_CAP : CORE.anim_capacity*2;
        CORE.items = arena_realloc(&CORE.arena, CORE.items, CORE.anim_capacity * sizeof(*CORE.items), new_cap * sizeof(*CORE.items));
        CORE.anim_capacity = new_cap;
    }

    size_t ind = CORE.anim_count;
    CORE.items[ind] = anim;
    CORE.anim_count++;
    return ind;
}

static float rate_func(RateFunc func, float anim_time, float duration)
{
    // Cubic and Quintic smooth step sources
    //   - Source: https://en.wikipedia.org/wiki/Smoothstep
    //   - Source: https://thebookofshaders.com/glossary/?search=smoothstep
    float x = anim_time / duration;
    float val;
    switch (func) {
        case RF_LINEAR:
            val = x;
            break;
        case RF_SINE:
            // (sin((PI * x) + (PI / 2)) + 1) * 0.5
            val = (sinf((PI * x) + (PI / 2.0f)) + 1.0f) * 0.5f;
            break;
        case RF_CUBIC_SMOOTH_STEP:
            // AMD cubic smooth step
            // 3x^2 - 2x^3
            val = (3*x*x) - (2*x*x*x);
            break;
        case RF_QUINTIC_SMOOTH_STEP:
            // AMD quintic smooth step
            // 6x^5 - 15x^4 + 10x^3
            val = (6*x*x*x*x*x) - (15*x*x*x*x) + (10*x*x*x);
            break;
        default:
            PHANIM_UNREACHABLE("Unknown rate function.");
            break;
    }
    return Clamp(val, 0.0f, 1.0f);
}

static float cubic_smoothstep(float x, float min, float max)
{
    // Scale, and clamp x to 0..1 range
    x = Clamp((x - min) / (max - min), 0.0, 1.0);

    return x * x * (3.0f - 2.0f * x);
}

static float quintic_smoothstep(float x, float min, float max)
{
    if (min >= max) TraceLog(LOG_FATAL, "quintic_smoothstep: min >= max");
    // Scale, and clamp x to 0..1 range
    x = Clamp((x - min) / (max - min), 0.0, 1.0);

    return x * x * x * (x * (6.0f * x - 15.0f) + 10.0f);
}

static void anim_print(Anim *a)
{
    TraceLog(LOG_INFO, "Anim {");
    switch (a->val_type) {
        case AVT_SCALAR: {
            TraceLog(LOG_INFO, "    obj: %.2f", *(float*)a->ptr);
            TraceLog(LOG_INFO, "    target: %.2f", *(float*)a->target);
            TraceLog(LOG_INFO, "    value type: float");
        } break;

        case AVT_VEC2: {
            Vector2 ptr = *(Vector2*)a->ptr;
            Vector2 target = *(Vector2*)a->target;
            TraceLog(LOG_INFO, "    obj: (%.2f, %.2f)", ptr.x, ptr.y);
            TraceLog(LOG_INFO, "    target: (%.2f, %.2f)", target.x, target.y);
            TraceLog(LOG_INFO, "    value type: Vector2");
        } break;

        case AVT_COLOR: {
            Color ptr = *(Color*)a->ptr;
            Color target = *(Color*)a->target;
            TraceLog(LOG_INFO, "    obj: (%d, %d, %d, %d)", ptr.r, ptr.g, ptr.b, ptr.a);
            TraceLog(LOG_INFO, "    target: (%d, %d, %d, %d)", target.r, target.g, target.b, target.a);
            TraceLog(LOG_INFO, "    value type: Color");
        } break;
    }
    TraceLog(LOG_INFO, "    time: %.2f", a->anim_time);
    TraceLog(LOG_INFO, "    duration: %.2f", a->duration);
    TraceLog(LOG_INFO, "}");
}

#if 0
static void phanim_anim_circle(Anim *a, float t)
{
    switch (a->anim_kind) {
        case AK_POSITION_TRANSFORM: {
            Vector2 start = *(Vector2*)a->start;
            Vector2 target = *(Vector2*)a->target;
            Vector2 *ptr = a->ptr;
            *ptr = Vector2Lerp(start, target, t);
        } break;
    }
}

static void phanim_anim_line(Anim *a, float t)
{
    switch (a->anim_kind) {
        case AK_CREATE:
        case AK_POSITION_TRANSFORM: {
            Vector2 start = *(Vector2*)a->start;
            Vector2 target = *(Vector2*)a->target;
            Vector2 *ptr = a->ptr;
            *ptr = Vector2Lerp(start, target, t);
        } break;

        case AK_COLOR_FADE: {
            Color start = *(Color*)a->start;
            Color target = *(Color*)a->target;
            Color *ptr = a->ptr;
            *ptr = ColorLerp(start, target, t);
        } break;

        case AK_SCALE: {
            float start = *(float*)a->start;
            float target = *(float*)a->target;
            float *ptr = a->ptr;
            *ptr = Lerp(start, target, t);
        } break;

        default:
            PHANIM_TODO("Look at other anim kinds that haven't been implemented!");
            break;
    }
}
#endif
