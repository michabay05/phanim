#include "phanim.h"

// TODO: when its create, handle what the target value should be
//       ideally, it should start as a point and create itself

static Arena obj_arena = {0};

int main(void) {
    InitWindow(800, 600, "Physics Animations");

    Line *l1 = arena_alloc(&obj_arena, sizeof(Line));
    l1->start = (Vector2) {100, 100};
    l1->end = (Vector2) {500, 500};
    l1->stroke_width = 3.0f;
    l1->color = RED;

    Line *l2 = arena_alloc(&obj_arena, sizeof(Line));
    l2->start = (Vector2) {100, 100};
    l2->end = (Vector2) {500, 100};
    l2->stroke_width = 5.0f;
    l2->color = SKYBLUE;

    Anim anim1 = {
        .start = l1,
        .end = l1,
        .anim_obj = AO_LINE,
        .anim_kind = AK_CREATE,
        .anim_time = 0.0f,
        .duration = 2.0f,
    };
    size_t ind1 = phanim_add(anim1);

    Anim anim2 = {
        .start = l1,
        .end = l2,
        .anim_obj = AO_LINE,
        .anim_kind = AK_POSITION_TRANSFORM,
        .anim_time = 3.0f,
        .duration = 2.0f,
    };
    size_t ind2 = phanim_add(anim2);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        BeginDrawing();
        ClearBackground(GetColor(0x0));
        {
            phanim_animate(ind1, dt);
            phanim_animate(ind2, dt);
            // Line current = *anim_s.start;
            // Line end = *anim_s.end;
            // Vector2 temp_start = Vector2Lerp(current.start, end.start, t);
            // Vector2 temp_end = Vector2Lerp(current.end, end.end, t);
            // float stroke_w = Lerp(current.stroke_width, end.stroke_width, t);
            // Color clr = ColorLerp(current.color, end.color, t);
            // DrawLineEx(temp_start, temp_end, stroke_w, clr);
        }
        EndDrawing();
    }

    arena_free(&obj_arena);
    CloseWindow();
    return 0;
}
