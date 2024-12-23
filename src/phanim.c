#include "raylib.h"
#include "raymath.h"
#include "resvg.h"
#include <stdio.h>
#include <unistd.h>

// TODOs
//   [ ] Add a mechanism to group animations
//   [ ] Improve smooth interpolations
//   [ ] Add video rendering feature
//   [ ] Implement mouse position to screen unit (for debugging)
//       - Crosshair-style
//   [ ] Determine how to render objects
//       - Right now, all objects are rendered always. Rendering is determined by
//         the opacity of the object
//       - *POTENTIAL SOLUTION*: When objects are initially created, they aren't renderable.
//          However, on the first frame that an anim with its object id is updated, it
//          becomes renderable
//   [ ] Setup a better pausing, i.e. waiting for X seconds, system
//   [ ] Establish a coordinate systems based around relative units
//   [ ] Remove obj and anim id because it's redundant (index and id are the same thing)
//   [ ] Link an animation to an object via an object's id
//   [ ] When loading a texture, raylib spams the stdout with 'texture loaded!' logging.
//       Find a way to remove or stop it.
//   [ ] *POTENTIAL IMPROVMENT*: For tex objects, find a way to prepare all the svg's before
//       hand so that it isn't done on every frame.
//   [ ] If an object is added to the list and isn't renderable, should it be required that
//       it's added(PhanimAddObject) first or can it immediately be added and animated??

#define ARENA_IMPLEMENTATION
#include "phanim.h"

#define DEFAULT_INIT_CAP 10
#define DEFAULT_LINE_THICKNESS 3.0f
#define DEFAULT_FONT_SIZE 25.0f
#define LATEX_OUT_DIR "./build/"
#define LATEX_TEX_FILE LATEX_OUT_DIR"input.tex"
#define LATEX_DVI_FILE LATEX_OUT_DIR"input.dvi"
#define LATEX_SVG_FILE LATEX_OUT_DIR"output.svg"

// TODO: add a group id to group anims that should happen at the same time
typedef struct {
    size_t id, obj_id;
    void *ptr;
    void *start;
    void *target;
    AnimValType val_type;
    AnimKind kind;
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
    // SVG stuff
    resvg_render_tree *svg_tree;
    u8 *svg_img_data;
    size_t svg_width, svg_height;
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
static size_t make_anim(size_t id, void *ptr, void *start, void *target, AnimValType val_type, AnimKind kind, float duration);
static bool compile_latex(
    const char *tex_file, const char *out_dir,
    const char *dvi_file, const char *svg_file);
static void latex_to_svg(TexData *tex);
static void render_svg(Vector2 pos);

static bool compile_latex(
    const char *tex_file, const char *out_dir,
    const char *dvi_file, const char *svg_file)
{
    #define BUF_LEN 512
    char cmd[BUF_LEN] = {0};
    snprintf(
        cmd, BUF_LEN,
        "pdflatex -draftmode -interaction=nonstopmode -output-format=dvi"
        " -output-directory=%s %s", out_dir, tex_file
    );
    TraceLog(LOG_INFO, "Executed commmand: %s\n", cmd);
    int ret = system(cmd);
    if (ret == 0) return false;

    memset(cmd, 0, sizeof(cmd));
    snprintf(
        cmd, BUF_LEN,
        "dvisvgm -n -v 0 --output=%s %s", svg_file, dvi_file
    );
    TraceLog(LOG_INFO, "Executed commmand: %s\n", cmd);
    system(cmd);
    if (ret == 0) return false;

    return true;
}

const char *TEX_HEADER =
    "\\documentclass{article}\n"
    "\\usepackage{amsmath}\n"
    "\\usepackage{amssymb}\n"
    "\\usepackage{amsfonts}\n"
    "\\thispagestyle{empty}\n\n"
    "\\begin{document}\n";

static void latex_to_svg(TexData *tex)
{
    PhanimStr str;
    PhanimStrInit(&str, TEX_HEADER);
    PhanimStrAppend(&str, "\\begin{align*}\n");
    PhanimStrConcat(&str, &tex->text);
    PhanimStrAppend(&str, "\\end{align*}\n");

    // TODO: Save to file here!!

    // if (!compile_latex(LATEX_TEX_FILE, LATEX_OUT_DIR, LATEX_DVI_FILE, LATEX_SVG_FILE)) {
    //     PHANIM_WARN("Failed to compile latex");
    //     return;
    // }

    resvg_options *opt = resvg_options_create();
    resvg_options_load_system_fonts(opt);

    int err = resvg_parse_tree_from_file(LATEX_SVG_FILE, opt, &CORE.svg_tree);
    if (err != RESVG_OK) {
        PHANIM_WARN("SVG rendering doesn't work!");
    }

    resvg_size size = resvg_get_image_size(CORE.svg_tree);
    int factor = 1;
    int width = (int)size.width * factor;
    int height = (int)size.height * factor;

    CORE.svg_img_data = arena_alloc(&CORE.temp_arena, width * height * sizeof(int));
    CORE.svg_width = width;
    CORE.svg_height = height;

    resvg_transform transform = { 0 };
    transform.a = (float) factor;
    transform.d = (float) factor;

    resvg_render(CORE.svg_tree, transform, width, height, (char*)CORE.svg_img_data);
    resvg_tree_destroy(CORE.svg_tree);
}

static void render_svg(Vector2 pos)
{
    Image img = {
        .data = CORE.svg_img_data,
        .width = CORE.svg_width,
        .height = CORE.svg_height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    };
    Texture tex = LoadTextureFromImage(img);
    DrawTextureV(tex, pos, WHITE);
}

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

    // Initialize resvg logging library
    resvg_init_log();
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

void PhanimChangeInterpFunc(size_t id, InterpFunc func)
{
    assert_id(id, true);
    CORE.anims[id].func = func;
}

void PhanimPause(float duration)
{
    make_anim(PHANIM_NO_ANIM, NULL, NULL, NULL, AVT_FLOAT, AK_PAUSE, duration);
}

size_t PhanimLine(Vector2 start, Vector2 end, Color color)
{
    LineData l = {
        .pos = start,
        .size = Vector2Subtract(end, start),
        .thickness = DEFAULT_LINE_THICKNESS,
        .color = color,
    };

    Object obj = {
        .id = CORE.obj_count,
        .kind = OK_LINE,
        .should_render = false,
        .line = l,
    };

    return phanim_add_obj(obj);
}

size_t PhanimRect(Vector2 pos, Vector2 size, Color color)
{
    RectData r = {
        .pos = pos,
        .size = size,
        .color = color,
    };

    Object obj = {
        .id = CORE.obj_count,
        .kind = OK_RECT,
        .should_render = false,
        .rect = r,
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
        .should_render = false,
        .circle = c,
    };

    return phanim_add_obj(obj);
}

size_t PhanimTex(PhanimStr str, Vector2 pos)
{
    TexData tx = {
        .text = str,
        .position = pos,
        .font_size = DEFAULT_FONT_SIZE,
    };

    Object obj = {
        .id = CORE.obj_count,
        .kind = OK_TEX,
        .should_render = false,
        .tex = tx,
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
            ptr = &obj->line.pos;
        } break;

        case OK_RECT: {
            ptr = &obj->rect.pos;
        } break;

        case OK_CIRCLE: {
            ptr = &obj->circle.center;
        } break;

        default: {
            PHANIM_UNREACHABLE("Unknown object kind!");
        } break;
    }

    make_anim(id, ptr, phanim_dvec2(start), phanim_dvec2(target), AVT_VEC2, AK_POSITION_TRANSFORM, duration);
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
            ptr = &obj->rect.color;
        } break;

        case OK_CIRCLE: {
            ptr = &obj->circle.color;
        } break;

        default: {
            PHANIM_UNREACHABLE("Unknown object kind!");
        } break;
    }

    make_anim(id, ptr, phanim_dcolor(start), phanim_dcolor(target), AVT_COLOR, AK_COLOR_FADE, duration);
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

    return make_anim(id, ptr, phanim_dfloat(start), phanim_dfloat(target), AVT_FLOAT, AK_SCALE, duration);
}

size_t PhanimScaleSizeVec2(size_t id, Vector2 start, Vector2 target, float duration)
{
    assert_id(id, false);
    Object *obj = &CORE.objs[id];
    Vector2 *ptr = NULL;
    switch (obj->kind) {
        case OK_LINE: {
            ptr = &obj->line.size;
        } break;

        case OK_RECT: {
            ptr = &obj->rect.size;
        } break;

        case OK_CIRCLE: {
            PHANIM_WARN("Circle shouldn't be scaled using PhanimScaleSizeVec2()");
            return PHANIM_NO_ANIM;
        } break;

        default: {
            PHANIM_UNREACHABLE("Unknown object kind!");
        } break;
    }

    return make_anim(id, ptr, phanim_dvec2(start), phanim_dvec2(target), AVT_VEC2, AK_SCALE, duration);
}

void PhanimAddObject(size_t id)
{
    // This is a temporary system. This will be changed!
    make_anim(id, NULL, NULL, NULL, AVT_VEC2, AK_IMMEDIATE, 0.0f);
}

void PhanimUpdate(float dt)
{
    // if (CORE.time >= PhanimTotalAnimTime()) {
    if (CORE.anim_current >= CORE.anim_count) {
        CORE.completed = true;
        return;
    }

    Anim *a = &CORE.anims[CORE.anim_current];
    Object *obj = &CORE.objs[a->obj_id];
    if (a->kind == AK_IMMEDIATE) {
        obj->should_render = true;
    }
    CORE.time += dt;
    if (a->anim_time >= a->duration) {
        CORE.anim_current += 1;
        if (CORE.anim_current < CORE.anim_count) {
            a = &CORE.anims[CORE.anim_current];
            obj = &CORE.objs[a->obj_id];
        } else {
            // Here, index will be out of bounds
            CORE.completed = true;
            return;
        }
    }

    if (!obj->should_render) {
        obj->should_render = true;
        return;
    }

    if (a->anim_time < a->duration) {
        a->anim_time += dt;
    }
    float t = rate_func(a->func, a->anim_time, a->duration);

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
        if (!o->should_render) {
            continue;
        }
        switch (o->kind) {
            case OK_CIRCLE: {
                CircleData *c = &o->circle;
                DrawCircleV(c->center, c->radius, c->color);
            } break;

            case OK_LINE: {
                LineData *l = &o->line;
                DrawLineEx(l->pos, Vector2Add(l->pos, l->size), l->thickness, l->color);
            } break;

            case OK_RECT: {
                RectData *r = &o->rect;
                Vector2 top_left = Vector2Subtract(r->pos, Vector2Scale(r->size, 0.5));
                DrawRectangleV(top_left, r->size, r->color);
            } break;

            case OK_TEX: {
                TexData *tex = &o->tex;
                latex_to_svg(tex);
                render_svg(tex->position);
            } break;

            default: {
                PHANIM_UNREACHABLE("Unknown object kind!");
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

static size_t make_anim(size_t id, void *ptr, void *start, void *target, AnimValType val_type, AnimKind kind, float duration)
{
    Anim a = {
        .id = CORE.anim_count,
        .obj_id = id,
        .ptr = ptr,
        .start = start,
        .target = target,
        .val_type = val_type,
        .kind = kind,
        .anim_time = 0.0,
        .duration = duration,
        .func = RF_CUBIC_SMOOTH_STEP
    };

    return phanim_add_anim(a);
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
