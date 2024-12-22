#include "raylib.h"
#include "raymath.h"
#include <assert.h>

// TODOs
//   [ ] Add a mechanism to group animations
//   [ ] Improve smooth interpolations
//   [ ] Added video rendering feature
//   [ ] Implement mouse position to screen unit (for debugging)
//       - Crosshair-style
//   [ ] Determine how to render objects
//       - Right now, all objects are rendered always. Rendering is determined by
//         the opacity of the object
//       - POTENTIAL SOLUTION: When objects are initially created, they aren't renderable.
//          However, on the first frame that an anim with its object id is updated, it
//          becomes renderable
//   [ ] Setup a better pausing, i.e. waiting for X seconds, system
//   [ ] Establish a coordinate systems based around relative units
//   [ ] Remove obj and anim id because it's redundant (index and id are the same thing)

#define ARENA_IMPLEMENTATION
#include "phanim.h"

#define DEFAULT_INIT_CAP 10
#define DEFAULT_LINE_THICKNESS 3.0f

// TODO: add a group id to group anims that should happen at the same time
typedef struct {
    size_t id;
    void *ptr;
    void *start;
    void *target;
    AnimValType val_type;
    float anim_time;
    float duration;
    InterpFunc func;
} Anim;

typedef struct {
    // Miscellaneous
    Arena obj_arena, anim_arena, temp_arena;
    float time;
    Color background;
    // Anims
    Anim *anims;
    size_t anim_count, anim_capacity;
    size_t anim_current;
    bool completed;
    // Objects
    Object *objs;
    size_t obj_count, obj_capacity;
} Phanim;

static Phanim CORE = {0};

static float rate_func(InterpFunc func, float anim_time, float duration);
static size_t phanim_add_anim(Anim anim);
static size_t phanim_add_obj(Object obj);
static float *phanim_dfloat(float val);
static Vector2 *phanim_dvec2(Vector2 val);
static Color *phanim_dcolor(Color val);
static void anim_print(Anim *a);
static void assert_id(size_t id, bool is_anim);

void PhanimInit(void)
{
    CORE.anims = NULL;
    CORE.anim_count = 0;
    CORE.anim_capacity = DEFAULT_INIT_CAP;
    CORE.anim_current = 0;
    CORE.completed = false;
    CORE.obj_arena = (Arena) {0};
    CORE.anim_arena = (Arena) {0};
    CORE.temp_arena = (Arena) {0};
    CORE.time = 0.0f;
}

void PhanimDeinit(void)
{
    arena_free(&CORE.obj_arena);
    arena_free(&CORE.anim_arena);
    arena_free(&CORE.temp_arena);
}

float PhanimGetTime(void)
{
    return CORE.time;
}

size_t PhanimCurrentAnimId(void)
{
    return !CORE.completed ? CORE.anim_current : PhanimAnimCount();
}

size_t PhanimAnimCount(void)
{
    return CORE.anim_count;
}

Color PhanimGetBackground(void)
{
    return CORE.background;
}

void PhanimSetBackground(Color color)
{
    CORE.background = color;
}

float PhanimTotalAnimTime(void)
{
    float total = 0.0f;
    for (size_t i = 0; i < CORE.anim_count; i++) {
        Anim *a = &CORE.anims[i];
        total += a->duration;
    }
    return total;
}

size_t PhanimMakeAnim(void *ptr, void *start, void *target, AnimValType val_type, float duration)
{
    Anim a = {
        .id = CORE.anim_count,
        .ptr = ptr,
        .start = start,
        .target = target,
        .val_type = val_type,
        .anim_time = 0.0,
        .duration = duration,
        .func = RF_CUBIC_SMOOTH_STEP
    };

    return phanim_add_anim(a);
}

void PhanimChangeInterpFunc(size_t id, InterpFunc func)
{
    assert_id(id, true);
    CORE.anims[id].func = func;
}

void PhanimPause(float duration)
{
    PhanimMakeAnim(NULL, NULL, NULL, AVT_FLOAT, duration);
}

size_t PhanimLine(Vector2 start, Vector2 end, Color color)
{
    LineData l = {
        .start = start,
        .size = Vector2Subtract(end, start),
        .thickness = DEFAULT_LINE_THICKNESS,
        .color = color,
    };

    Object obj = {
        .id = CORE.obj_count,
        .kind = OK_LINE,
        .should_render = true,
        .line = l,
    };

    return phanim_add_obj(obj);
}

size_t PhanimCircle(Vector2 center, float radius, Color color)
{
    CircleData c = {
        .center = center,
        .radius = radius,
        .color = color,
        .stroke_width = 0.0f,
        .stroke_color = BLANK,
    };

    Object obj = {
        .id = CORE.obj_count,
        .kind = OK_CIRCLE,
        .should_render = true,
        .circle = c,
    };

    return phanim_add_obj(obj);
}

void PhanimTransformPos(size_t id, Vector2 start, Vector2 target, float duration)
{
    assert_id(id, false);
    Object *obj = &CORE.objs[id];
    Vector2 *ptr = NULL;
    switch (obj->kind) {
        case OK_LINE: {
            ptr = &obj->line.start;
        } break;

        case OK_RECT: {
            PHANIM_TODO("Implement POS_TRANSFORM for OK_RECT");
        } break;

        case OK_CIRCLE: {
            ptr = &obj->circle.center;
        } break;

        default: {
            PHANIM_UNREACHABLE("Unknown object kind!");
        } break;
    }

    PhanimMakeAnim(ptr, phanim_dvec2(start), phanim_dvec2(target), AVT_VEC2, duration);
}

void PhanimFadeColor(size_t id, Color start, Color target, float duration)
{
    assert_id(id, false);
    Object *obj = &CORE.objs[id];
    Color *ptr = NULL;
    switch (obj->kind) {
        case OK_LINE: {
            ptr = &obj->line.color;
        } break;

        case OK_RECT: {
            PHANIM_TODO("Implement AK_MOVE for OK_RECT");
        } break;

        case OK_CIRCLE: {
            ptr = &obj->circle.color;
        } break;

        default: {
            PHANIM_UNREACHABLE("Unknown object kind!");
        } break;
    }

    PhanimMakeAnim(ptr, phanim_dcolor(start), phanim_dcolor(target), AVT_COLOR, duration);
}

size_t PhanimScaleSizeFloat(size_t id, float start, float target, float duration)
{
    assert_id(id, false);
    Object *obj = &CORE.objs[id];
    float *ptr = NULL;
    switch (obj->kind) {
        case OK_RECT:
        case OK_LINE: {
            PHANIM_WARN("Lines and Rects shouldn't be scaled using PhanimScaleSizeFloat()");
            return PHANIM_NO_ANIM;
        } break;

        case OK_CIRCLE: {
            ptr = &obj->circle.radius;
        } break;

        default: {
            PHANIM_UNREACHABLE("Unknown object kind!");
        } break;
    }

    return PhanimMakeAnim(ptr, phanim_dfloat(start), phanim_dfloat(target), AVT_FLOAT, duration);
}

void PhanimScaleSizeVec2(size_t id, Vector2 start, Vector2 target, float duration)
{
    assert_id(id, false);
    Object *obj = &CORE.objs[id];
    Vector2 *ptr = NULL;
    switch (obj->kind) {
        case OK_LINE: {
            ptr = &obj->line.size;
        } break;

        case OK_RECT: {
            PHANIM_TODO("Implement AK_MOVE for OK_RECT");
        } break;

        case OK_CIRCLE: {
            PHANIM_WARN("Circle shouldn't be scaled using PhanimScaleSizeVec2()");
            return;
        } break;

        default: {
            PHANIM_UNREACHABLE("Unknown object kind!");
        } break;
    }

    PhanimMakeAnim(ptr, phanim_dvec2(start), phanim_dvec2(target), AVT_VEC2, duration);
}

void PhanimUpdate(float dt)
{
    if (CORE.time >= PhanimTotalAnimTime()) {
        CORE.completed = true;
        return;
    }

    Anim *a = &CORE.anims[CORE.anim_current];
    if (a->anim_time >= a->duration) {
        CORE.anim_current += 1;
        if (CORE.anim_current < CORE.anim_count) {
            a = &CORE.anims[CORE.anim_current];
        } else {
            // Here, index will be out of bounds
            CORE.completed = true;
            return;
        }
    }

    CORE.time += dt;
    if (a->anim_time < a->duration) {
        a->anim_time += dt;
    }
    float t = rate_func(a->func, a->anim_time, a->duration);
    // float t = cubic_smoothstep(a->anim_time, 0.0f, a->duration);
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
                // Probably, a pause anim
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
}

void PhanimRender(void)
{
    for (size_t i = 0; i < CORE.obj_count; i++) {
        Object *o = &CORE.objs[i];
        if (!o->should_render) continue;
        switch (o->kind) {
            case OK_CIRCLE: {
                CircleData *c = &o->circle;
                DrawCircleV(c->center, c->radius, c->color);
            } break;

            case OK_LINE: {
                LineData *l = &o->line;
                DrawLineEx(l->start, Vector2Add(l->start, l->size), l->thickness, l->color);
            } break;

            default: {
                PHANIM_TODO("Implement render for OK_RECT && OK_LINE");
            } break;
        }
    }
}

static float *phanim_dfloat(float val)
{
    return arena_memdup(&CORE.temp_arena, &val, sizeof(float));
}

static Vector2 *phanim_dvec2(Vector2 val)
{
    return arena_memdup(&CORE.temp_arena, &val, sizeof(Vector2));
}

static Color *phanim_dcolor(Color val)
{
    return arena_memdup(&CORE.temp_arena, &val, sizeof(Color));
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

static float rate_func(InterpFunc func, float anim_time, float duration)
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
            // (-0.5 * cos(PI * x)) + 0.5
            val = (-0.5f * cosf(PI * x)) + 0.5;
            break;
        case RF_SINE_PULSE:
            // sin(PI * x)
            val = sinf(PI * x);
            break;
        case RF_CUBIC_SMOOTH_STEP:
            val = x * x * (3.0f - 2.0f * x);
            break;
        case RF_QUINTIC_SMOOTH_STEP:
            val = x * x * x * (x * (6.0f * x - 15.0f) + 10.0f);
            break;
        default:
            PHANIM_UNREACHABLE("Unknown rate function.");
            break;
    }
    return val;
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

static void assert_id(size_t id, bool is_anim)
{
    size_t max;
    const char *type;
    if (is_anim) {
        max = CORE.anim_count;
        type = "ANIM";
    } else {
        max = CORE.obj_count;
        type = "OBJ";
    }

    // No lower bound checking because `size_t` is always >= 0
    if (id >= max)
        TraceLog(LOG_FATAL, "%s: ID(%zu) is out of bounds. (ID >= %d)", type, id, max);
}
