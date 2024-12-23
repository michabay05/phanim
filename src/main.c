#define PHANIM_STR_IMPLEMENTATION
#include "phanim.h"
#include "raylib.h"
#include "scene.c"

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 600, "Physics Animations");
    SetTargetFPS(60);

    SceneMain();
    TraceLog(LOG_INFO, "Anim count: %d", PhanimAnimCount());

    bool pause = true;
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            pause = !pause;
        }

        BeginDrawing();
        ClearBackground(PhanimGetBackground());

            float dt = GetFrameTime();
            if (!pause) PhanimUpdate(dt);
            PhanimRender();

            DrawFPS(10, 10);
            // Paused or Playing text
            const char *pause_text = pause ? "Paused" : "Playing";
            DrawText(pause_text, 10, 40, 20, WHITE);
            // (Current time / Total time) text
            const char *time_text = TextFormat("%.2f / %.2f", PhanimGetTime(), PhanimTotalAnimTime());
            DrawText(time_text, 10, 70, 20, WHITE);
            // (Current anim id / Total anim id) text
            const char *anim_progress = TextFormat("%d / %d", PhanimCurrentAnimId(), PhanimAnimCount());
            DrawText(anim_progress, 10, 100, 20, WHITE);

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
                time_bar.x + tbw * (PhanimGetTime() / PhanimTotalAnimTime()),
                time_bar.y + (time_bar.height / 2.0f)
            };
            DrawCircleV(center, tbh, RED);

        EndDrawing();
    }

    PhanimDeinit();
    CloseWindow();
    return 0;
}
