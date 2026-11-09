#include <iostream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <vector>
#include <utility>
#include <Windows.h>
using namespace std;

const int nGridWidth = 60;
const int nGridHeight = 20;
unsigned char grid[nGridHeight][nGridWidth] = {};

int nScreenWidth = 120;
int nScreenHeight = 40;

bool IsSafe(int px, int py, vector<pair<int, int>> &snake) {
    if (grid[py][px] == 1) {
        return false;
    }

    // ## NOTE ##
    // We only loop before the tail
    // because the tail will be moved next and won't be
    // at that position anymore.
    // BUT THIS DOESN'T HANDLES THE CASE
    // WHEN THE SNAKE GROWS AND HITS THE TAIL
    // AT THE EXACT SAME FRAME.
    // **TO BE FIXED**
    // A better solution would be to first move the snake
    // first and eat the fruit first before checking for collisions.
    for (int i = 1; i < snake.size() - 1; i++) {
        if (snake[i].first == px && snake[i].second == py) {
            return false;
        }
    }

    return true;
}

int main()
{
    srand((unsigned) time(nullptr));

    // Initialize the grid boundaries
    // 0 -> empty space
    // 1 -> boundary
    for (int y = 0; y < nGridHeight; y++) {
        for (int x = 0; x < nGridWidth; x++) {
            if (x == 0 || x == nGridWidth - 1 || y == 0 || y == nGridHeight - 1) {
                grid[y][x] = 1;
            }
        }
    }

    // Snake
    // HEAD <- B1 <- B2 <- B3 ...
    // (x, y)
    vector<pair<int, int>> snake;
    snake.push_back({ nGridWidth / 2, nGridHeight / 2 });
    snake.push_back({ nGridWidth / 2 + 1, nGridHeight / 2 });

    // Fruit
    pair<int, int> fruit;
    fruit.first = 1 + rand() % (nGridWidth - 2);
    fruit.second = 1 + rand() % (nGridHeight - 2);

    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // Game State
    bool bGameOver = false;

    bool bKey[4];

    int nVelX = -1;
    int nVelY = 0;
    int nScore = 0;

    // Game Loop
    while (!bGameOver) {
        // GAME TIMING
        // Different delays for when the snake is moving in the up/down
        // direction, because the characters are almost twice as tall
        // than they are wide. So this makes it appear that the snake
        // is moving way faster in the up/down direction.
        this_thread::sleep_for(nVelY != 0 ? 100ms : 50ms);

        // INPUT
        for (int k = 0; k < 4; k++) {
            // Right Key: \x27
            // Left Key: \x25
            // Down Key: \x28
            // Up Key: \x26
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28\x26"[k]))) != 0;
        }

        // GAME LOGIC
        int nNextX = snake[0].first, nNextY = snake[0].second;
        // Ignoring the keystroke
        // which is in the opposite direction
        // of the current velocity. So that the snake
        // doesn't die like that.
        if (bKey[0] && nVelX != -1) { // Right
            nVelX = 1;
            nVelY = 0;
        }
        else if (bKey[1] && nVelX != 1) { // Left
            nVelX = -1;
            nVelY = 0;
        }
        else if (bKey[2] && nVelY != -1) { // Down
            nVelX = 0;
            nVelY = 1;
        }
        else if (bKey[3] && nVelY != 1) { // Up
            nVelX = 0;
            nVelY = -1;
        }

        // Move the snake's head
        if (IsSafe(snake[0].first + nVelX, snake[0].second + nVelY, snake)) {
            snake[0].first += nVelX;
            snake[0].second += nVelY;
        }
        else {
            bGameOver = true;
        }

        // Eat the fruit
        if (snake[0].first == fruit.first && snake[0].second == fruit.second) {
            nScore++;
            fruit.first = 1 + rand() % (nGridWidth - 2);
            fruit.second = 1 + rand() % (nGridHeight - 2);
            // The position of the tail being
            // appended doesn't matter as it will be updated very soon.
            // So this could have been { x, y }
            // where x, y are any arbitrary values.
            snake.push_back({ snake[nScore].first - nVelX, snake[nScore].second - nVelY });
        }

        // Shift the snake's body
        // Only shift if the snake's head has moved!
        if (nNextX != snake[0].first || nNextY != snake[0].second) {
            for (int i = 1; i < snake.size(); i++) {
                int nCurX = snake[i].first, nCurY = snake[i].second;
                snake[i].first = nNextX;
                snake[i].second = nNextY;
                nNextX = nCurX;
                nNextY = nCurY;
            }
        }

        // RENDER OUTPUT
        // Draw the grid
        for (int y = 0; y < nGridHeight; y++) {
            for (int x = 0; x < nGridWidth; x++) {
                screen[(y + 2) * nScreenWidth + (x + 2)] = L" #"[grid[y][x]];
            }
        }

        // Draw the fruit
        screen[(fruit.second + 2) * nScreenWidth + (fruit.first + 2)] = L'@';

        // Draw the snake
        bool snakeHead = true;
        for (auto& v : snake) {
            if (snakeHead) {
                screen[(v.second + 2) * nScreenWidth + (v.first + 2)] = L'O';
                snakeHead = false;
            }
            else {
                screen[(v.second + 2) * nScreenWidth + (v.first + 2)] = L'X';
            }
        }

        // Draw Score
        swprintf_s(&screen[2 * nScreenWidth + nGridWidth + 6], 16, L"SCORE: %8d", nScore);

        // Display Frame
        for (int y = 0; y < nScreenHeight; y++) {
            WriteConsoleOutputCharacter(hConsole, &screen[y * nScreenWidth], nScreenWidth, { 0, (SHORT)y }, &dwBytesWritten);
        }
    }

    // Game Over
    this_thread::sleep_for(1s);
    CloseHandle(hConsole);
    cout << "Game Over! Score: " << nScore << endl;
    system("pause");
    return 0;
}
