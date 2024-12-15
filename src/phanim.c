#include "raylib.h"
#include <assert.h>

// TODOs
//   - Add a mechanism to group animations
//   - Improve smooth interpolations
//   - Added video rendering feature
//   - Implement mouse position to screen unit (for debugging)
//   - Determine how to render objects
//       - Right now, all objects are rendered always. Rendering is determined by
//         the opacity of the object

#define ARENA_IMPLEMENTATION
#include "phanim.h"

#define DEFAULT_INIT_CAP 10

// TODO: add a group id to group anims that should happen at the same time
typedef struct {
    void *ptr;
    void *start;
    void *target;
    AnimValType val_type;
    float anim_time;
    float duration;
} Anim;

typedef struct {
    // Miscellaneous
    Arena obj_arena, anim_arena, temp_arena;
    float time;
    // Anims
    Anim *anims;
    size_t anim_count, anim_capacity;
    size_t anim_current;
    // Objects
    Object *objs;
    size_t obj_count, obj_capacity;
} Phanim;

static Phanim CORE = {0};

static float rate_func(RateFunc func, float anim_time, float duration);
static float cubic_smoothstep(float x, float min, float max);
static float quintic_smoothstep(float x, float min, float max);
static size_t phanim_add_anim(Anim anim);
static size_t phanim_add_obj(Object obj);
static Vector2 *phanim_vec2(Vector2 val);
static u8 *phanim_u8(u8 val);
static void anim_print(Anim *a);

void phanim_init(void)
{
    CORE.anims = NULL;
    CORE.anim_count = 0;
    CORE.anim_capacity = DEFAULT_INIT_CAP;
    CORE.anim_current = 0;
    CORE.obj_arena = (Arena) {0};
    CORE.anim_arena = (Arena) {0};
    CORE.temp_arena = (Arena) {0};
    CORE.time = 0.0f;
}

void phanim_deinit(void)
{
    arena_free(&CORE.obj_arena);
    arena_free(&CORE.anim_arena);
    arena_free(&CORE.temp_arena);
}

float phanim_get_time(void)
{
    return CORE.time;
}

size_t phanim_anim_count(void)
{
    return CORE.anim_count;
}

float phanim_total_anim_time(void)
{
    float total = 0.0f;
    for (int i = 0; i < CORE.anim_count; i++) {
        Anim *a = &CORE.anims[i];
        total += a->duration;
    }
    return total;
}

void phanim_make_anim(void *ptr, void *start, void *target, AnimValType val_type, float duration)
{
    Anim a = {
        .ptr = ptr,
        .start = start,
        .target = target,
        .val_type = val_type,
        .anim_time = 0.0,
        .duration = duration
    };

    phanim_add_anim(a);
}

void phanim_pause(float duration)
{
    phanim_make_anim(NULL, NULL, NULL, AVT_FLOAT, duration);
}

// Returns 'true' if animation is complete
bool phanim_update(float dt)
{
    // TraceLog(LOG_INFO, "anim_current = %d", CORE.anim_current);
    // TraceLog(LOG_INFO, "anim_count = %d", CORE.anim_count);
    if (CORE.time >= phanim_total_anim_time()) return true;
    CORE.time += dt;

    Anim *a = &CORE.anims[CORE.anim_current];
    if (a->anim_time >= a->duration) {
        CORE.anim_current += 1;
        if (CORE.anim_current < CORE.anim_count) {
            a = &CORE.anims[CORE.anim_current];
        } else {
            // Here, index will be out of bounds
            return true;
        }
    }
    if (a->anim_time < a->duration) {
        a->anim_time += dt;
    }
    // float t = rate_func(RF_LINEAR, a->anim_time, a->duration);
    float t = cubic_smoothstep(a->anim_time, 0.0f, a->duration);
    // float t = quintic_smoothstep(a->anim_time, 0.0f, a->duration);

    switch (a->val_type) {
        case AVT_VEC2: {
            Vector2 start = *(Vector2*)a->start;
            Vector2 *ptr = (Vector2*)a->ptr;
            Vector2 target = *(Vector2*)a->target;
            *ptr = Vector2Lerp(start, target, t);
        } break;

        case AVT_FLOAT: {
            if (a->ptr == NULL && a->target == NULL) {
                // Probably, a pause scene
            } else {
                float start = *(float*)a->start;
                float *ptr = (float*)a->ptr;
                float target = *(float*)a->target;
                *ptr = Lerp(start, target, t);
            }
        } break;

        case AVT_U8: {
            u8 start = *(u8*)a->start;
            u8 *ptr = (u8*)a->ptr;
            u8 target = *(u8*)a->target;
            u8 result = start + t*(target - start);
            *ptr = result;
        } break;

        case AVT_COLOR: {
            Color start = *(Color*)a->start;
            Color *ptr = (Color*)a->ptr;
            Color target = *(Color*)a->target;
            *ptr = ColorLerp(start, target, t);
        } break;

        default: {
            PHANIM_UNREACHABLE("Unknown anim value type!");
        } break;
    }

    return false;
}

void phanim_render(void)
{
    for (int i = 0; i < CORE.obj_count; i++) {
        Object *o = &CORE.objs[i];
        if (!o->should_render) continue;
        switch (o->kind) {
            case OK_CIRCLE: {
                Circle *c = &o->circle;
                DrawCircleV(c->center, c->radius, c->color);
            } break;

            default: {
                PHANIM_TODO("Implement render for OK_RECT && OK_LINE");
            } break;
        }
    }
}

size_t phanim_circle(Vector2 center, float radius, Color color)
{
    Circle c = {
        .center = center,
        .radius = radius,
        .color = color,
        .stroke_width = 0.0f,
        .stroke_color = BLANK,
    };

    Object obj = {
        .kind = OK_CIRCLE,
        .should_render = true,
        .circle = c,
    };

    return phanim_add_obj(obj);
}

void phanim_move(size_t id, Vector2 target, float duration)
{
    Object *obj = &CORE.objs[id];
    Vector2 start = {0};
    Vector2 *ptr = NULL;
    switch (obj->kind) {
        case OK_LINE: {
            PHANIM_TODO("Implement AK_MOVE for OK_LINE");
        } break;

        case OK_RECT: {
            PHANIM_TODO("Implement AK_MOVE for OK_RECT");
        } break;

        case OK_CIRCLE: {
            ptr = &obj->circle.center;
            start = *ptr;
        } break;

        default: {
            PHANIM_UNREACHABLE("Unknown object kind!");
        } break;
    }

    phanim_make_anim(ptr, phanim_vec2(start), phanim_vec2(target), AVT_VEC2, duration);
}

static Vector2 *phanim_vec2(Vector2 val)
{
    Vector2 *v = arena_alloc(&CORE.temp_arena, sizeof(Vector2));
    *v = val;
    return v;
}

static u8 *phanim_u8(u8 val)
{
    u8 *u = arena_alloc(&CORE.temp_arena, sizeof(u8));
    *u = val;
    return u;
}

static size_t phanim_add_anim(Anim anim)
{
    if (CORE.anim_count >= CORE.anim_capacity) {
        size_t new_cap = CORE.anim_capacity == 0 ? DEFAULT_INIT_CAP : CORE.anim_capacity*2;
        CORE.anims = arena_realloc(&CORE.anim_arena, CORE.anims, CORE.anim_capacity * sizeof(*CORE.anims), new_cap * sizeof(*CORE.anims));
        CORE.anim_capacity = new_cap;
    }

    size_t ind = CORE.anim_count;
    CORE.anims[ind] = anim;
    CORE.anim_count++;
    return ind;
}

static size_t phanim_add_obj(Object obj)
{
    if (CORE.obj_count >= CORE.obj_capacity) {
        size_t new_cap = CORE.obj_capacity == 0 ? DEFAULT_INIT_CAP : CORE.obj_capacity*2;
        CORE.objs = arena_realloc(&CORE.obj_arena, CORE.objs, CORE.obj_capacity * sizeof(*CORE.objs), new_cap * sizeof(*CORE.objs));
        CORE.obj_capacity = new_cap;
    }

    size_t ind = CORE.obj_count;
    CORE.objs[ind] = obj;
    CORE.obj_count++;
    return ind;
}

static float rate_func(RateFunc func, float anim_time, float duration)
{
    // Cubic and Quintic smooth step sources
    //   - Source: https://en.wikipedia.org/wiki/Smoothstep
    //   - Source: https://thebookofshaders.com/glossary/?search=smoothstep
    float x = Clamp(anim_time / duration, 0.0f, 1.0f);
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
    return val;
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
        case AVT_U8: {
            TraceLog(LOG_INFO, "    obj: %d", *(u8*)a->ptr);
            TraceLog(LOG_INFO, "    target: %d", *(u8*)a->target);
            TraceLog(LOG_INFO, "    value type: u8 (unsigned char)");
        } break;

        case AVT_FLOAT: {
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

        default: {
            PHANIM_UNREACHABLE("Unknown anim value type");
        } break;
    }
    TraceLog(LOG_INFO, "    time: %.2f", a->anim_time);
    TraceLog(LOG_INFO, "    duration: %.2f", a->duration);
    TraceLog(LOG_INFO, "}");
}
