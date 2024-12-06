#include "phanim.h"

// TODO: when it's a CREATE animation, handle what the target value should be
//       ideally, it should start as a point and create itself

static Arena obj_arena = {0};

int main(void) {
    InitWindow(800, 600, "Physics Animations");
    SetTargetFPS(60);

    Line *l1 = arena_alloc(&obj_arena, sizeof(Line));
    l1->start = (Vector2) {100, 100};
    l1->end = (Vector2) {500, 500};
    l1->stroke_width = 3.0f;
    l1->color = RED;

    float *start = arena_alloc(&obj_arena, sizeof(float));
    *start = 3.0f;

    float *end = arena_alloc(&obj_arena, sizeof(float));
    *end = 20.0f;

    Anim anim1 = {
        .ptr = &l1->stroke_width,
        .start = start,
        .target = end,
        .obj_kind = AOK_LINE,
        .anim_kind = AK_SCALE,
        .anim_time = 0.0f,
        .duration = 1.5f,
    };
    size_t ind1 = phanim_add(anim1);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        phanim_animate(ind1, dt);

        BeginDrawing();
        ClearBackground(GetColor(0x0));
        DrawLineEx(l1->start, l1->end, l1->stroke_width, l1->color);
        DrawFPS(10, 10);
        EndDrawing();
    }

    arena_free(&obj_arena);
    CloseWindow();
    return 0;
}
