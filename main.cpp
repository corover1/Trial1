#include <iostream>
#include "raylib.h"
#include <deque>
#include "raymath.h"

// Custom colors using Color struct
Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};

// Number and size of cells
int cellSize = 30;
int cellCount = 25;

double lastUpdateTime = 0;

bool eventTriggered(double interval) {
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval) {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

// Snake
class Snake {
public:

    std::deque<Vector2> body = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
    Vector2 direction = {1,0};

    void Draw() {
        for(int i = 0; i < body.size(); i++) {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{x * cellSize, y *cellSize, float(cellSize), float(cellSize)};
            DrawRectangleRounded(segment, 0.75, 6, darkGreen);
        }
    }
    void Update() {
        body.pop_back();
        body.push_front(body[0] + direction);
    }
};

// Food items
class Food {

public:
    // Vector2 holds an x and y value on a grid
    Vector2 position;
    Texture2D texture;

    // Constructor
    Food() {

        // Creates image object that loads pixel data from an image file
        Image image = LoadImage("../Graphics/apple1.png");
        // Creates a texture from the image we just created and loads it
        texture = LoadTextureFromImage(image);
        // Frees memory and unloads image
        UnloadImage(image);
        position = GenerateRandomPos();
    }

    // Destructor
    ~Food() {
        // Frees memory and unloads texture
        UnloadTexture(texture);
    }

    // Draws the food image
    void Draw() {
        // Needs a texture, x-coord for topleft corner,y-coord for topleft corner, and a tint color
        DrawTexture(texture, position.x *cellSize, position.y *cellSize, WHITE);
    }

    // Generate random position using raylibs random value generator
    Vector2 GenerateRandomPos() {
        float x = GetRandomValue(0, cellCount-1);
        float y = GetRandomValue(0, cellCount-1);
        return Vector2{x, y};
    }
};

int main() {
    std::cout << "Starting the game..." << std::endl;
    InitWindow(cellSize*cellCount, cellSize*cellCount, "Retro Snake Game");
    SetTargetFPS(60);

    Food food = Food();
    Snake snake = Snake();

    while(!WindowShouldClose()) {
        BeginDrawing();

        if(eventTriggered(0.2)) {
            snake.Update();
        }
        if(IsKeyPressed(KEY_UP) && snake.direction.y != 1) {
            snake.direction = {0, -1};
        }
        if(IsKeyPressed(KEY_DOWN) && snake.direction.y != -1) {
            snake.direction = {0, 1};
        }
        if(IsKeyPressed(KEY_LEFT) && snake.direction.x != 1) {
            snake.direction = {-1, 0};
        }
        if(IsKeyPressed(KEY_RIGHT) && snake.direction.x != -1) {
            snake.direction = {1, 0};
        }

        ClearBackground(green);
        food.Draw();
        snake.Draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

