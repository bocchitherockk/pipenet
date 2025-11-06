#include <raylib.h>

int main(void) {
    InitWindow(800, 600, "pipenet"); // first thing we have to initialize the window
    SetTargetFPS(60); // we set the max fps to 60
    Font times_new_roman = LoadFont("./gui/times.ttf"); // we load a font from a ttf file
    // this is the event loop
    while (!WindowShouldClose()) { // WindowShouldClose is the function responsible to return true if you press KEY_ESCAPE or you close the window with the mouse by clicking on the x
        BeginDrawing(); // the drawing of the application goes between BeginDrawing and EndDrawing
            ClearBackground(WHITE); // draw the entire screen with a color
            DrawRectangle(0, 0, 200, 100, RED);
            DrawTextEx(times_new_roman, "this is text", (Vector2){200, 300}, 30, 1, BLACK);
        EndDrawing();
        if (IsKeyPressed(KEY_Q)) CloseWindow(); // we add the ability to close the window when 'q' is pressed.
        // note: the keyboard layout is qwerty and not azerty, so pressing 'q' on your azerty keyboard w'ont do anything, you have to press 'a'
    }
    return 0;
}