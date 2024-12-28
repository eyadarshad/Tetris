#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream>

// Include embedded Arial font
#include "arial.h"

using namespace std;

const int GRID_WIDTH = 10;
const int GRID_HEIGHT = 20;
const int TILE_SIZE = 30;
const sf::Color COLORS[] = {sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Yellow, sf::Color::Cyan};

// Tetromino shapes
const vector<vector<vector<int>>> TETROMINOES = {
    {{1, 1, 1, 1}},                  // I
    {{1, 1}, {1, 1}},                // O
    {{0, 1, 0}, {1, 1, 1}},          // T
    {{1, 1, 0}, {0, 1, 1}},          // S
    {{0, 1, 1}, {1, 1, 0}},          // Z
    {{1, 1, 1}, {1, 0, 0}},          // L
    {{1, 1, 1}, {0, 0, 1}}           // J
};

struct Tetromino {
    vector<vector<int>> shape;
    sf::Color color;
    int x = 0, y = 0;

    void rotate() {
        vector<vector<int>> rotated(shape[0].size(), vector<int>(shape.size()));
        for (size_t i = 0; i < shape.size(); ++i) {
            for (size_t j = 0; j < shape[i].size(); ++j) {
                rotated[j][shape.size() - 1 - i] = shape[i][j];
            }
        }
        shape = rotated;
    }
};

bool checkCollision(const array<array<int, GRID_WIDTH>, GRID_HEIGHT>& grid, const Tetromino& tetromino, int offsetX, int offsetY) {
    for (size_t i = 0; i < tetromino.shape.size(); ++i) {
        for (size_t j = 0; j < tetromino.shape[i].size(); ++j) {
            if (tetromino.shape[i][j] && (tetromino.x + j + offsetX < 0 || tetromino.x + j + offsetX >= GRID_WIDTH || tetromino.y + i + offsetY >= GRID_HEIGHT ||
                                         grid[tetromino.y + i + offsetY][tetromino.x + j + offsetX])) {
                return true;
            }
        }
    }
    return false;
}

void mergeTetromino(array<array<int, GRID_WIDTH>, GRID_HEIGHT>& grid, const Tetromino& tetromino) {
    for (size_t i = 0; i < tetromino.shape.size(); ++i) {
        for (size_t j = 0; j < tetromino.shape[i].size(); ++j) {
            if (tetromino.shape[i][j]) {
                grid[tetromino.y + i][tetromino.x + j] = 1;
            }
        }
    }
}

void clearLines(array<array<int, GRID_WIDTH>, GRID_HEIGHT>& grid, int& score) {
    for (int i = GRID_HEIGHT - 1; i >= 0; --i) {
        bool fullLine = true;
        for (int j = 0; j < GRID_WIDTH; ++j) {
            if (!grid[i][j]) {
                fullLine = false;
                break;
            }
        }
        if (fullLine) {
            for (int k = i; k > 0; --k) {
                grid[k] = grid[k - 1];
            }
            grid[0].fill(0);
            score += 100;
            i++;
        }
    }
}

int main() {
    srand(static_cast<unsigned>(time(0)));
    sf::RenderWindow window(sf::VideoMode(GRID_WIDTH * TILE_SIZE, GRID_HEIGHT * TILE_SIZE + 50), "Tetris");

    // Load font from memory
    sf::Font font;
    if (!font.loadFromMemory(arial_ttf, arial_ttf_size)) {
        cerr << "Failed to load font from memory!" << endl;
        return -1;
    }

    // Background setup
    sf::RectangleShape background(sf::Vector2f(GRID_WIDTH * TILE_SIZE, GRID_HEIGHT * TILE_SIZE));
    sf::Texture texture;
    if (texture.loadFromFile("coding/bg.jpg")) { // Replace "background.jpg" with your texture
        background.setTexture(&texture);
    } else {
        background.setFillColor(sf::Color(50, 50, 50)); // Fallback color
    }

    // Grid lines setup
    sf::RectangleShape gridLine(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    gridLine.setOutlineThickness(1);
    gridLine.setOutlineColor(sf::Color(60, 60, 60));

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(20);
    scoreText.setFillColor(sf::Color::Red);
    scoreText.setPosition(5, GRID_HEIGHT * TILE_SIZE + 5);

    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(30);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setString("Game Over!");
    gameOverText.setPosition(GRID_WIDTH * TILE_SIZE / 4, GRID_HEIGHT * TILE_SIZE / 2 - 50);

    array<array<int, GRID_WIDTH>, GRID_HEIGHT> grid = {0};
    Tetromino currentTetromino;
    int score = 0;
    bool gameOver = false;

    auto spawnTetromino = [&]() {
        int index = rand() % TETROMINOES.size();
        currentTetromino.shape = TETROMINOES[index];
        currentTetromino.color = COLORS[index % 5];
        currentTetromino.x = GRID_WIDTH / 2 - currentTetromino.shape[0].size() / 2;
        currentTetromino.y = 0;
        if (checkCollision(grid, currentTetromino, 0, 0)) {
            gameOver = true;
        }
    };

    spawnTetromino();

    sf::Clock clock;
    float timer = 0, delay = 0.5f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (!gameOver && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Left && !checkCollision(grid, currentTetromino, -1, 0)) {
                    currentTetromino.x--;
                }
                if (event.key.code == sf::Keyboard::Right && !checkCollision(grid, currentTetromino, 1, 0)) {
                    currentTetromino.x++;
                }
                if (event.key.code == sf::Keyboard::Down) {
                    delay = 0.05f;
                }
                if (event.key.code == sf::Keyboard::Space) {
                    Tetromino rotated = currentTetromino;
                    rotated.rotate();
                    if (!checkCollision(grid, rotated, 0, 0)) {
                        currentTetromino.rotate();
                    }
                }
            }
        }

        if (gameOver) {
            window.clear();
            sf::RectangleShape overlay(sf::Vector2f(GRID_WIDTH * TILE_SIZE, GRID_HEIGHT * TILE_SIZE));
            overlay.setFillColor(sf::Color(0, 0, 0, 150)); // Semi-transparent overlay
            window.draw(overlay);
            window.draw(gameOverText);
            window.display();
            continue;
        }

        timer += clock.restart().asSeconds();
        if (timer > delay) {
            if (!checkCollision(grid, currentTetromino, 0, 1)) {
                currentTetromino.y++;
            } else {
                mergeTetromino(grid, currentTetromino);
                clearLines(grid, score);
                spawnTetromino();
            }
            timer = 0;
        }

        delay = 0.5f;

        window.clear();

        // Draw background
        window.draw(background);

        // Draw grid
        for (int i = 0; i < GRID_HEIGHT; ++i) {
            for (int j = 0; j < GRID_WIDTH; ++j) {
                gridLine.setPosition(j * TILE_SIZE, i * TILE_SIZE);
                gridLine.setFillColor(grid[i][j] ? sf::Color(150, 150, 150) : sf::Color::Transparent);
                window.draw(gridLine);
            }
        }

        // Draw current tetromino
        for (size_t i = 0; i < currentTetromino.shape.size(); ++i) {
            for (size_t j = 0; j < currentTetromino.shape[i].size(); ++j) {
                if (currentTetromino.shape[i][j]) {
                    sf::RectangleShape tile(sf::Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                    tile.setFillColor(currentTetromino.color);
                    tile.setOutlineThickness(-1);
                    tile.setOutlineColor(sf::Color(30, 30, 30));
                    tile.setPosition((currentTetromino.x + j) * TILE_SIZE, (currentTetromino.y + i) * TILE_SIZE);
                    window.draw(tile);
                }
            }
        }

        // Draw score
        scoreText.setString("Score: " + to_string(score));
        window.draw(scoreText);

        window.display();
    }

    return 0;
}
