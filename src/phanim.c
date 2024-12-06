#include <assert.h>

#define ARENA_IMPLEMENTATION
#include "phanim.h"

#define DEFAULT_INIT_CAP 10

typedef struct {
    Anim *items;
    size_t count, capacity;
    Arena arena;
    float time;
} Phanim;

static Phanim CORE = {0};

static float rate_func(RateFunc func, float anim_time, float duration);
static void phanim_direct_render(void *ptr, AnimObj obj);
static void phanim_anim_line(Anim *a, float t);
static void anim_print(Anim *a);

bool phanim_init() {
    CORE.items = NULL;
    CORE.count = 0;
    CORE.capacity = DEFAULT_INIT_CAP;
    CORE.arena = (Arena) {0};
    CORE.time = 0.0f;
}

size_t phanim_add(Anim anim) {
    if (CORE.count >= CORE.capacity) {
        size_t new_capacity = CORE.capacity == 0 ? DEFAULT_INIT_CAP : CORE.capacity*2;
        CORE.items = arena_realloc(&CORE.arena, CORE.items, CORE.capacity * sizeof(*CORE.items), new_capacity * sizeof(*CORE.items));
        CORE.capacity = new_capacity;
    }
    size_t ind = CORE.count;
    CORE.items[ind] = anim;
    anim_print(&CORE.items[ind]);
    CORE.count++;
    return ind;
}

void phanim_animate(size_t ind, float dt) {
    CORE.time += dt;
    Anim *a = &CORE.items[ind];
    if (a->anim_time <= CORE.time) {
        if (CORE.time < a->anim_time + a->duration) {
            a->anim_time += dt;
        } else {
            phanim_direct_render(a->end, a->anim_obj);
            return;
        }
    } else {
        return;
    }
    float t = rate_func(RF_CUBIC_SMOOTH_STEP, a->anim_time, a->anim_time + a->duration);
    // Clamp values between 0 and 1
    t = Clamp(t, 0.0f, 1.0f);

    switch (a->anim_obj) {
        case AO_LINE: phanim_anim_line(a, t); break;
        default:
            PHANIM_TODO("AOK_RECT && AOK_CIRCLE");
            break;
    }
}

static void phanim_direct_render(void *ptr, AnimObj obj) {
    switch (obj) {
        case AO_LINE:
            Line *l = ptr;
            DrawLineEx(l->start, l->end, l->stroke_width, l->color);
            break;
        default:
            PHANIM_TODO("AOK_RECT && AOK_CIRCLE");
            break;
    }
}

static void phanim_anim_line(Anim *a, float t) {
    Line *ls = a->start;
    Line *le = a->end;
    switch (a->anim_kind) {
        case AK_CREATE: {
            Vector2 current = Vector2Lerp(ls->start, ls->end, t);
            DrawLineEx(ls->start, current, ls->stroke_width, ls->color);
        } break;
        case AK_POSITION_TRANSFORM: {
            Vector2 start = Vector2Lerp(ls->start, le->start, t);
            Vector2 end = Vector2Lerp(ls->end, le->end, t);
            DrawLineEx(start, end, ls->stroke_width, ls->color);
        } break;
        default:
            PHANIM_TODO("Look at other anim kinds that haven't been implemented!");
            break;
    }
}

static float rate_func(RateFunc func, float anim_time, float duration) {
    // Cubic and Quintic smooth step sources
    //   - Source: https://en.wikipedia.org/wiki/Smoothstep
    //   - Source: https://thebookofshaders.com/glossary/?search=smoothstep
    float x = anim_time / duration;
    switch (func) {
        case RF_LINEAR:
            return x;
        case RF_SINE:
            // sin((PI / 2.0f) * x)
            return sinf((PI / 2.0f) * x);
        case RF_CUBIC_SMOOTH_STEP:
            // AMD cubic smooth step
            // 3x^2 - 2x^3
            return (3*x*x) - (2*x*x*x);
        case RF_QUINTIC_SMOOTH_STEP:
            // AMD quintic smooth step
            // 6x^5 - 15x^4 + 10x^3
            return (6*x*x*x*x*x) - (15*x*x*x*x) + (10*x*x*x);
        case RF_3B1B_SMOOTH_STEP:
            // s = 1 - anim_time
            // (x^3) * (10s^2 + 5sx + x^2)
            float s = 1 - x;
            return (x*x*x) * ((10*s*s) + (5*s*x) + (x*x));
    }
}

static void anim_print(Anim *a) {
    TraceLog(LOG_INFO, "Anim {");
    TraceLog(LOG_INFO, "    obj: %d", a->anim_obj);
    TraceLog(LOG_INFO, "    kind: %d", a->anim_kind);
    TraceLog(LOG_INFO, "    time: %.2f", a->anim_time);
    TraceLog(LOG_INFO, "    duration: %.2f", a->duration);
    TraceLog(LOG_INFO, "}");
}
