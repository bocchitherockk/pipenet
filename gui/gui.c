#include <stddef.h> // size_t

#include <raylib.h>
#include "./Process.h"

#define SCREEN_WIDTH  1200
#define SCREEN_HEIGHT 1000

#define TOPBAR_WIDTH SCREEN_WIDTH
#define TOPBAR_HEIGHT (SCREEN_HEIGHT / 5)


Font times_new_roman = {0};


void renderTopBar() {    
    DrawRectangle(0, 0, TOPBAR_WIDTH, TOPBAR_HEIGHT, BLUE); {
        int button_width  = TOPBAR_WIDTH/5;
        int button_height = TOPBAR_HEIGHT/2;

        Rectangle server_button = {
            .x = TOPBAR_WIDTH/2/2 - button_width/2,
            .y = TOPBAR_HEIGHT/2 - button_height/2,
            .width = button_width,
            .height = button_height,
        };
        Rectangle client_button = {
            .x = TOPBAR_WIDTH*3/2/2 - button_width/2,
            .y = TOPBAR_HEIGHT/2 - button_height/2,
            .width = button_width,
            .height = button_height,
        };

        DrawRectangleRec(server_button, GREEN);
        DrawTextEx(times_new_roman, "create server", (Vector2){server_button.x + 10, server_button.y + 10}, 30, 1, BLACK);
        
        DrawRectangleRec(client_button, GREEN);
        DrawTextEx(times_new_roman, "create client", (Vector2){client_button.x + 10, client_button.y + 10}, 30, 1, BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse_position = GetMousePosition();
            if (CheckCollisionPointRec(mouse_position, server_button)) {
                Process_create(PROCESS_TYPE_SERVER);
            } else if(CheckCollisionPointRec(mouse_position, client_button)) {
                Process_create(PROCESS_TYPE_CLIENT);
            }
        }
    }
}

static void _RenderProcess(int x, int y, Process *process) {
    int width = SCREEN_WIDTH / 2 * 0.8;
    int height = 80;

    Rectangle next_button          = { .x = x, .y = y + height*0, .width = width, .height = height };
    Rectangle line_content_display = { .x = x, .y = y + height*1, .width = width, .height = height };
    Rectangle question_display     = { .x = x, .y = y + height*2, .width = width, .height = height };
    Rectangle answer_display       = { .x = x, .y = y + height*3, .width = width, .height = height };

    int a = process->process_state == PROCESS_STATE_EXECUTING ? 255/2 : 255;
    DrawRectangleRec(next_button         , (Color){ .r = 180, .g = 255, .b = 255, .a = a });
    DrawRectangleRec(line_content_display, (Color){ .r = 255, .g = 218, .b = 185, .a = a });
    DrawRectangleRec(question_display    , (Color){ .r = 144, .g = 238, .b = 144, .a = a });
    DrawRectangleRec(answer_display      , (Color){ .r = 255, .g = 182, .b = 193, .a = a });

    DrawTextEx(times_new_roman, "next",                (Vector2){next_button.x + 10, next_button.y + next_button.height/2},                            20, 0.2, BLACK);
    DrawTextEx(times_new_roman, process->line_content, (Vector2){line_content_display.x + 10, line_content_display.y + line_content_display.height/2}, 20, 0.2, BLACK);
    DrawTextEx(times_new_roman, process->question,     (Vector2){question_display.x + 10, question_display.y + question_display.height/2},             20, 0.2, BLACK);
    DrawTextEx(times_new_roman, process->answer,       (Vector2){answer_display.x + 10, answer_display.y + answer_display.height/2},                   20, 0.2, BLACK);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse_position = GetMousePosition();
        if (CheckCollisionPointRec(mouse_position, next_button)) {
            Process_next(process);
        }
    }
}

void RenderServer() {
    if (server.is_set == false) return;
    int top_margin = 50;
    int width = SCREEN_WIDTH / 2 * 0.8;
    int x = SCREEN_WIDTH/4 - width/2;
    int y = TOPBAR_HEIGHT + top_margin;
    _RenderProcess(x, y, &server);
}

void RenderClients() {
    int top_margin = 50;
    int bottom_margin = 0; // 0 for the first iteration, and then we will set it up
    int width = SCREEN_WIDTH / 2 * 0.8;
    int height = 80;
    int x = SCREEN_WIDTH*3/4 - width/2;
    
    for (size_t i = 0; i < clients.count; i++) {
        if (i != 0) bottom_margin = 50;
        int y = height*i*4 + TOPBAR_HEIGHT + top_margin + bottom_margin;
        _RenderProcess(x, y, &clients.items[i]);
    }
}


int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "pipenet");
    SetTargetFPS(60); // we set the max fps to 60
    times_new_roman = LoadFont("./gui/times.ttf"); // we load a font from a ttf file

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(WHITE);
            renderTopBar();
            RenderServer();
            RenderClients();
        EndDrawing();
        if (IsKeyPressed(KEY_A)) CloseWindow(); // we add the ability to close the window when 'q' is pressed.
        // note: the keyboard layout encoded in raylib is qwerty and not azerty, that's why i set it to KEY_A, but in reality you have to press 'q'
    }

    return 0;
}