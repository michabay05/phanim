#include "phanim.h"

static Arena obj_arena = {0};

typedef struct {
    Vector2 center;
    float radius;
    Color color;
    float stroke_width;
    Color stroke_color;
} Circle;

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 600, "Physics Animations");
    SetTargetFPS(30);

    Circle *c = arena_alloc(&obj_arena, sizeof(Circle));
    *c = (Circle) {
        .center = (Vector2) {160, 100},
        .radius = 40,
        .color = RED,
        .stroke_width = 0.0f,
        .stroke_color = RED,
    };

    Vector2 *target = arena_alloc(&obj_arena, sizeof(Vector2));
    *target = (Vector2) {640, 500};
    phanim_make_anim(&c->center, target, AVT_VEC2, 2.5f);

    phanim_pause(2.5f);

    Vector2 *target1 = arena_alloc(&obj_arena, sizeof(Vector2));
    *target1 = (Vector2) {400, 300};
    phanim_make_anim(&c->center, target1, AVT_VEC2, 2.5f);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        phanim_update(dt);

        BeginDrawing();
        ClearBackground(GetColor(0x0));

        DrawCircleV(c->center, c->radius, c->color);

        DrawFPS(10, 10);
        EndDrawing();
    }

    arena_free(&obj_arena);
    CloseWindow();
    return 0;
}
