#include "phanim.h"

void CircleScenes(void)
{
    // Red ball
    Color red_blank = { 230, 41, 55, 0 };
    Color red = { 230, 41, 55, 255 };
    size_t red_ball = PhanimCircle(vec2(200, 150), 20, red_blank);

    PhanimFadeColor(red_ball, red_blank, red, 1.0f);
    PhanimTransformPos(red_ball, vec2(200, 150), vec2(700, 500), 1.0f);

    // Green ball
    Color green_blank = { 0, 228, 48, 0 };
    Color green = { 0, 228, 48, 255 };
    size_t green_ball = PhanimCircle(vec2(600, 150), 20, green_blank);

    PhanimFadeColor(green_ball, green_blank, green, 1.0f);
    PhanimTransformPos(green_ball, vec2(600, 150), vec2(100, 500), 1.0f);

    PhanimTransformPos(red_ball, vec2(700, 500), vec2(400, 300), 2.0f);
    PhanimScaleSizeFloat(red_ball, 20, 40, 1.0f);
    PhanimTransformPos(green_ball, vec2(100, 500), vec2(400, 300), 2.0f);
    int anim_id = PhanimScaleSizeFloat(green_ball, 20, 30, .6f);
    PhanimChangeInterpFunc(anim_id, RF_SINE_PULSE);
}

void SceneMain(void)
{
    PhanimSetBackground(color(20, 20, 20, 255));

    CircleScenes();
}
