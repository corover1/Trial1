#include <iostream>
#include "raylib.h"
#include <deque>
#include "raymath.h"

// Custom colors using Color struct
Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};

// Number and size of cells in pixels
int cellSize = 30;
int cellCount = 25;
// Offset size in pixels, from origin
int offset = 75;

// Var to limit how fast snake moves
double lastUpdateTime = 0;

// Searches for a element within a deque and returns true if inside, false otherwise
bool ElementInDeque(Vector2 element, std::deque<Vector2> deque) {

    for(unsigned int i = 0; i < deque.size(); i++) {
        if(Vector2Equals(deque[i], element)) {
            return true;
        }
    }
    return false;
}

// If enough time has passed since last update, then trigger event
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

    // Body segments stored in a deque for easy removal and insertion at both ends
    std::deque<Vector2> body = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
    // Direction is a cardinal direction the snake is facing at a given instance
    Vector2 direction = {1,0};
    // Bool for if snake eats food at a given moment
    bool addSegment = false;

    // Get coords for body segments, and draws them
    void Draw() {
        for(int i = 0; i < body.size(); i++) {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{(float)offset + x * cellSize, (float)offset + y *cellSize, float(cellSize), float(cellSize)};
            DrawRectangleRounded(segment, 0.75, 6, darkGreen);
        }
    }

    // Update segment coords
    void Update() {
        if(addSegment) {
            addSegment = false;
        }else {
            body.pop_back();
        }
        body.push_front(Vector2Add(body[0], direction));
    }

    // Resets to initial game state
    void Reset() {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1,0};
    }
};

// Food items
class Food {

public:
    // Vector2 holds an x and y value on a grid
    Vector2 position;
    // Texture holds pixel data for an image
    Texture2D texture;

    // Constructor
    Food(std::deque<Vector2> snakeBody) {

        // Creates image object that loads pixel data from an image file
        Image image = LoadImage("../Graphics/apple1.png");
        // Creates a texture from the image we just created and loads it
        texture = LoadTextureFromImage(image);
        // Frees memory and unloads image
        UnloadImage(image);
        // Food generates a random pos on map
        position = GenerateRandomPos(snakeBody);
    }

    // Destructor
    ~Food() {
        // Frees memory and unloads texture
        UnloadTexture(texture);
    }

    // Draws the food image
    void Draw() {
        // Needs a texture, x-coord for topleft corner,y-coord for topleft corner, and a tint color
        DrawTexture(texture, offset + position.x *cellSize, offset + position.y *cellSize, WHITE);
    }

    // Generates random cell on grid (coord)
    Vector2 GenerateRandomCell() {
        float x = GetRandomValue(0, cellCount-1);
        float y = GetRandomValue(0, cellCount-1);
        return Vector2{x,y};
    }

    // Generate random position using GenerateRandomCell, and makes sure it doesn't share a coord with snake body
    Vector2 GenerateRandomPos(std::deque<Vector2> snakeBody) {
        Vector2 position = GenerateRandomCell();
        // If shares a coord with snake, generates again until it doesn't
        while (ElementInDeque(position, snakeBody)) {
            position = GenerateRandomCell();
        }
        return position;
    }
};

class Game {
public:
    // Game has a snake, food, running bool, score and sounds for actions
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;
    Sound eatSound;
    Sound wallSound;
    bool gameWon = false;

    // Initializes sounds when Game is created
    Game() {
        InitAudioDevice();
        eatSound = LoadSound("../Sounds/SnakeEat.mp3");
        wallSound = LoadSound("../Sounds/GameStart.mp3");
    }

    // Unloads sounds and frees memory
    ~Game() {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    // When game calls to draw, draws the food and snake
    void Draw() {
        food.Draw();
        snake.Draw();
    }

    //
    void Update() {
        if(running) {
            snake.Update();
            if(snake.body.size() == cellCount * cellCount) {
                gameWon = true;
            } else {
                CheckCollisionWithFood();
                CheckCollisionWithEdges();
                CheckCollisionWithTail();
            }
        }
    }

    // If snake touches food, new food pos is generated, snake elongates, score incremented and sound is played
    void CheckCollisionWithFood() {
        if(Vector2Equals(snake.body[0], food.position)) {
            food.position = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);
        }
    }

    // If snake hits border game resets
    void CheckCollisionWithEdges() {
        if(snake.body[0].x == (float)cellCount || snake.body[0].x == -1) {
            PlaySound(wallSound);
            GameOver();
        }
        if(snake.body[0].y == (float)cellCount || snake.body[0].y == -1) {
            PlaySound(wallSound);
            GameOver();
        }
    }

    // If snake hits itself game resets
    void CheckCollisionWithTail() {
        std::deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if(ElementInDeque(snake.body[0], headlessBody)) {
            GameOver();
        }
    }

    // Resets snake position and size, food gets regenerated, and score resets
    void GameOver() {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        score = 0;
    }

    // Calls GameOver and draws text onto screen over win condition
    void GameWin() {
        int winScore = score;
        GameOver();
        score = winScore;
        DrawText("You Win!", offset+ (cellCount * cellSize / 2), (offset + cellCount * cellSize / 2), 80, darkGreen);
    }

    // Helper function, resets win and score
    void GameSet() {
        if(gameWon && score != 0) {
            gameWon = false;
            score = 0;
        }
    }
};

int main() {
    std::cout << "Starting the game..." << std::endl;
    InitWindow(2 * offset + cellSize*cellCount, 2 * offset +cellSize*cellCount, "Retro Snake Game");
    SetTargetFPS(60);

    Game game = Game();

    while(!WindowShouldClose()) {
        BeginDrawing();

        // Here checks for keyboard strokes to move the snake
        if(eventTriggered(0.2)) {
            game.Update();
        }
        if(IsKeyPressed(KEY_UP) && game.snake.direction.y != 1) {
            game.snake.direction = {0, -1};
            game.running = true;
            game.GameSet();
        }
        if(IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1) {
            game.snake.direction = {0, 1};
            game.running = true;
            game.GameSet();
        }
        if(IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1) {
            game.snake.direction = {-1, 0};
            game.running = true;
            game.GameSet();
        }
        if(IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1) {
            game.snake.direction = {1, 0};
            game.running = true;
            game.GameSet();
        }

        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset -5, (float)cellCount*cellSize+10, (float)cellCount*cellSize+10}, 5, darkGreen);
        DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
        if(game.gameWon) {
            game.GameWin();
        }
        DrawText(TextFormat("%i", game.score), offset - 5, offset + cellCount*cellSize + 10, 40, darkGreen);
        DrawText("Controls: WASD", offset+ cellCount * cellSize - 310, offset + cellCount * cellSize+ 10, 40, darkGreen);
        game.Draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

