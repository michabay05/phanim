#include "phanim.h"

#define vec2(cx, cy) CLITERAL(Vector2){cx, cy}

static Arena obj_arena = {0};

typedef struct {
    Vector2 center;
    float radius;
    Color color;
    float stroke_width;
    Color stroke_color;
} Circle;

float *phanim_float(float val)
{
    float *f = arena_alloc(&obj_arena, sizeof(float));
    *f = val;
    return f;
}

Vector2 *phanim_vec2(float x, float y)
{
    Vector2 *v = arena_alloc(&obj_arena, sizeof(Vector2));
    *v = vec2(x, y);
    return v;
}

Circle *phanim_circle(Vector2 center, float radius, Color color)
{
    Circle *c = arena_alloc(&obj_arena, sizeof(Circle));
    *c = (Circle) {
        .center = center,
        .radius = radius,
        .color = color,
        .stroke_width = 0.0f,
        .stroke_color = BLANK,
    };
    return c;
}

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 600, "Physics Animations");
    SetTargetFPS(60);

    Circle *c = phanim_circle(vec2(160, 100), 30.0f, RED);
    Circle *c1 = phanim_circle(vec2(640, 500), 40.0f, BLUE);

    Vector2 *target = phanim_vec2(640, 500);
    phanim_make_anim(&c->center, target, AVT_VEC2, 2.0f);
    Vector2 *target1 = phanim_vec2(160, 100);
    phanim_make_anim(&c1->center, target1, AVT_VEC2, 1.5f);

    phanim_pause(2.5f);

    Vector2 *target2 = phanim_vec2(400, 300);
    phanim_make_anim(&c->center, target2, AVT_VEC2, 1.5f);
    phanim_make_anim(&c1->center, target2, AVT_VEC2, 1.5f);
    float *target3 = phanim_float(50);
    phanim_make_anim(&c1->radius, target3, AVT_VEC2, 1.5f);

    bool pause = true;
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            pause = !pause;
        }

        BeginDrawing();
        ClearBackground(GetColor(0x0));

            float dt = GetFrameTime();
            if (!pause) phanim_update(dt);
            DrawCircleV(c1->center, c1->radius, c1->color);
            DrawCircleV(c->center, c->radius, c->color);

        DrawFPS(10, 10);
        EndDrawing();
    }

    phanim_deinit();
    arena_free(&obj_arena);
    CloseWindow();
    return 0;
}
