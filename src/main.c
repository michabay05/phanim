#include "phanim.h"
#include "raylib.h"

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 600, "Physics Animations");
    SetTargetFPS(30);

    {
        size_t id = phanim_circle(vec2(200, 150), 20, color(230, 41, 55, 255));
        phanim_move(id, vec2(700, 500), 1.0f);
        TraceLog(LOG_INFO, "Anim count: %d", phanim_anim_count());
    }

    bool pause = true;
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            pause = !pause;
        }

        BeginDrawing();
        ClearBackground(GetColor(0x0));

            float dt = GetFrameTime();
            if (!pause) phanim_update(dt);
            phanim_render();

            DrawFPS(10, 10);
            // Paused or Playing text
            const char *pause_text = pause ? "Paused" : "Playing";
            DrawText(pause_text, 10, 40, 20, WHITE);
            // (Current time / Total time) text
            const char *time_text = TextFormat("%.2f / %.2f", phanim_get_time(), phanim_total_anim_time());
            DrawText(time_text, 10, 70, 20, WHITE);

            // Timeline
            float tbw = 0.9f * (float)GetScreenWidth();
            float tbh = 5.0f;
            Rectangle time_bar = {
                .x = 0.5f * ((float)GetScreenWidth() - tbw),
                .y = 0.95f * (float)GetScreenHeight(),
                .width = tbw,
                .height = tbh,
            };
            DrawRectangleRec(time_bar, LIGHTGRAY);
            Vector2 center = {
                time_bar.x + tbw * (phanim_get_time() / phanim_total_anim_time()),
                time_bar.y + (time_bar.height / 2.0f)
            };
            DrawCircleV(center, tbh, RED);

        EndDrawing();
    }

    phanim_deinit();
    CloseWindow();
    return 0;
}
