#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

// Declare custom colors, { R, G, B, A} 0-255
Color green = { 173, 204, 96, 255 };
Color darkGreen = { 43, 51, 24, 255 };


// Set the properties of the game grid
int cellSize = 30;  // Size of each individual cell
int cellCount = 25; // # of rows and columns (set to match 750x750 px window)
// Note: 30 * 25 = 750
int offset = 75;

// lastUpdateTime: Time that last update of snake occurred
double lastUpdateTime = 0;

// ElementInDeque: Searches deque (2nd param) for element (1st param), returns true if found
bool ElementInDeque(Vector2 element, std::deque<Vector2> deque) {
    for (unsigned int i = 0; i < deque.size(); i++) {   // For each element in deque:
        if (Vector2Equals(deque[i], element)) {  // If query matches current deque element:
            return true; // If found, return true
        } 
    }
    // If query is not found, return false:
    return false;
}

// Function 'eventTriggered': check if an interval (parameter) of real-time has passed
bool eventTriggered(double interval) {
    // currentTime: time since window initialization
    double currentTime = GetTime(); // GetTime(): gets elapsed time since window initialization
    if (currentTime - lastUpdateTime >= interval) { // If time passed since last update > interval (200ms),
        lastUpdateTime = currentTime;               // set last update to the current time
        return true;                                // and return true
    }
    return false;                                   // else, return false
}

// Class 'Snake': Represents snake body
class Snake {
public:
    // Declare snake 'body' deque at defined locations (04,09 to 06,09)
    std::deque<Vector2> body = { Vector2{6,9}, Vector2{5,9}, Vector2{4,9} };

    
    Vector2 direction = { 1, 0 };   // Set snake direction

    bool addSegment = false;    // addSegment: true if snake should grow in size

    // Method 'Draw': draws snake using coordinates in body deque
    void Draw() {
        // For each element in 'body' deque:
        for (unsigned int i = 0; i < body.size(); i++) {  // (use unsigned int bc deques.size returns unsigned int)
            float x = body[i].x;  // Get x/y coordinates
            float y = body[i].y;

            // Declare Rectangle object 'segment' using x/y coordinates
            Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize,(float)cellSize,(float)cellSize };

            // Draw rounded rectangle using Rectangle 'segment', to represent snake body
            DrawRectangleRounded(segment, 0.6, 6, darkGreen);

            // Draw square at the x/y coordinates given, taking up entire cell
            //DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, darkGreen);
        }
    }

    // Method 'Update': updates the Snake body
    void Update() {

        // At front of snake/deque, add coordinate of (head + direction)
        body.push_front(Vector2Add(body[0], direction));

        if (addSegment == true) {   // If 'addSegment' is true, turn it off
            addSegment = false; 
        } else {   
            body.pop_back();    // Removes last element in snake/deque
        }
    }

    // Method: resets snake position and direction
    void Reset() {
        body = { Vector2{6,9}, Vector2{5,9}, Vector2{4,9} }; // Reset snake body to original position
        direction = { 1, 0 }; // Reset direction to original direction (Right)
    }

};

// Class 'Food': Represents food that snake can eat
class Food {
public:
    
    Vector2 position;            // Declare Vector2 to hold food position
    Texture2D texture;           // Make new Texture2D object, to hold food image

    // Class constructor:
    Food(std::deque<Vector2> snakeBody) {

        // Image loading/unloading:
        Image image = LoadImage("Graphics/food.png");   // Import image at 'Graphics/food.png'
        texture = LoadTextureFromImage(image);          // put image inside Texture2D object
        UnloadImage(image);                             // Unload image to save memory

        // Generate random position NOT inside snakeBody deque
        position = GenerateRandomPos(snakeBody);  

    }

    // Destructor: calls when object is destroyed
    ~Food() {
        UnloadTexture(texture);     // When food is gone, unload Texture2D object
    }

    // Method 'Draw': Draw square for food
    void Draw() {
        // The below code is for drawing a placeholder rectangle:
        //DrawRectangle(position.x * cellSize, position.y * cellSize, cellSize, cellSize, darkGreen);

        // The below code uses 'food.png' for the food image
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    // Method: Get random x/y coordinate for food (range: 0 to (cellCount-1), because range is 0-24)
    Vector2 GenerateRandomCell() {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{ x, y };
    }


    // Method 'GenerateRandomPos': Generate random position in grid (NOT occupied by snake)
    Vector2 GenerateRandomPos(std::deque<Vector2> snakeBody) {

        // Generate new random coordinates, store in Vector2 'position'
        Vector2 position = GenerateRandomCell(); 

        // Keep generating new random coordinates until you get one not inside snakeBody deque
        while (ElementInDeque(position, snakeBody)) {  // While: coordinates are in snakeBody
            position = GenerateRandomCell();           // generate new coordinates
        } 

        return position;
    }
};

// Class 'Game': Contains code for snake, food, and Update
class Game {
public:
    // Declare objects:
    Snake snake = Snake();          // declare snake
    Food food = Food(snake.body);   // declare food object NOT inside snake's body
    
    bool running = true;  // Is game running? Set to true (yes)
    int score = 0; // score: # foods eaten by snake

    // Draw method: Draws snake and food objects
    void Draw() {
        food.Draw();
        snake.Draw();
    }

    // Method: Runs update frame if game is running
    void Update() {
        if (running) {      // If game is running:
            snake.Update(); // Calls update for snake body
            CheckCollisionWithFood(); // Checks for food collision
            CheckCollisionWithEdges(); // Checks for edge collision
            CheckCollisionWithTail(); // Checks for snake colliding into itself
        }
    }

    // Method: checks if snake is touching food
    void CheckCollisionWithFood() { 
        if (Vector2Equals(snake.body[0], food.position)) {  // If snake head is touching food:
            food.position = food.GenerateRandomPos(snake.body); // reset food position,
            snake.addSegment = true; // turn on 'addSegment' switch
            score++; // increment score by 1
        }
    }

    // Method: ends game if snake touches edges
    void CheckCollisionWithEdges() {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1) {  // If snakeBody is touching X edges:
            GameOver();   // Ends game (you lose)
        } 
        // Else, if snake is touching Y edges:
        else if (snake.body[0].y == cellCount || snake.body[0].y == -1) {
            GameOver();   // Ends game (you lose)
        }
    }

    // Method: is triggered when the game is over
    void GameOver() { 
        snake.Reset();  // Resets snake position & direction
        food.position = food.GenerateRandomPos(snake.body); // Set food to new random position
        running = false; // Turn off game 'running' switch
        score = 0; // Reset score to 0
    }

    // Method: Checks if snake is colliding with itself
    void CheckCollisionWithTail() {
        std::deque<Vector2> headlessBody = snake.body; // Make copy of snake body
        headlessBody.pop_front(); // Removes snake's head from copy
        if (ElementInDeque(snake.body[0], headlessBody)) { // If snake's head is in headless copy:
            GameOver(); // Game Over if snake touches its own body
        } 
    }
};


// Main function:
int main() {

    std::cout << "Starting the game..." << std::endl; // Print starting message

    // Initialize window using cell size and cell count (# rows/columns), and accounting for offset
    InitWindow(2*offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Retro Snake");
    SetTargetFPS(60); // Set target FPS (max FPS)

    // Game object will contain snake, food, and update code:
    Game game = Game();  // Declare Game object

    while (WindowShouldClose() == false) {   // As long as window-close command is not true

// Draw window is a single Update frame: Begins with 'BeginDrawing', ends with 'EndDrawing'
        BeginDrawing();    // Begins drawing window

        if (eventTriggered(.2)) {  // If 200 ms of real-time has passed since last Snake update:
            game.Update();         // Updates the Snake object
        }

        // Set snake direction based on key input (forbid reversing movement)
        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1 ) {    // If key-up pressed, and snake not moving down:
            game.snake.direction = { 0, -1 }; // Snake object's direction set to Up
            game.running = true;              // Make game run, if not already on
        } else if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1) {    // If key-up pressed, and snake not moving up:
            game.snake.direction = { 0, 1 };  // Snake object's direction set to Down
            game.running = true;              // Make game run, if not already on
        } else if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1) {    // If key-up pressed, and snake not moving right:
            game.snake.direction = { -1, 0 }; // Snake object's direction set to Left
            game.running = true;              // Make game run, if not already on
        } else if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1) {    // If key-up pressed, and snake not moving left:
            game.snake.direction = { 1, 0 };  // Snake object's direction set to Right
            game.running = true;              // Make game run, if not already on
        }

        // Drawing
        ClearBackground(green); // Set background color to 'green'

        // Draw border separating game grid from offset area
        DrawRectangleLinesEx(Rectangle{(float)offset-5, (float)offset-5, (float)cellSize*cellCount+10, (float)cellSize*cellCount+10}, 5, darkGreen);

        // Draw game title at top offset
        DrawText("Jake the Snake", offset - 5, 20, 40, darkGreen);

        // Draws game score at bottom offset
        DrawText(TextFormat("%i",game.score), offset - 5, offset + cellSize * cellCount + 13, 40, darkGreen);



        game.Draw();        // Calls game draw method, drawing both snake and food


        EndDrawing();     // Ends drawing window
    }


    CloseWindow();   // close out window
    return 0;
}