#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
using namespace std;

enum class SnakeTheme  { GREEN, COLORFUL, BLUE, ORANGE };
enum class FruitTheme  { APPLE, ORANGE_FRUIT, BLUEBERRY, GOLDEN };

class Snake {
public:
    Snake() {}
    ~Snake() {}

    void reset(sf::RenderWindow &w, int cols, int rows,
               bool wallWrap, SnakeTheme st, FruitTheme ft, int barH);
    void add_food(sf::RenderWindow &w);
    void move(int direction);
    bool is_dead();
    bool is_on_food();
    void draw(sf::RenderWindow &w);

private:
    int cellW, cellH, cols, rows, barH;
    vector<sf::Vector2i> body;
    sf::Vector2i food;
    bool wallWrap;
    SnakeTheme snakeTheme;
    FruitTheme fruitTheme;

    void       drawCell(sf::RenderWindow &w, int gx, int gy, sf::Color c, int shrink);
    sf::Color  getBodyColor(int index, int total);
    void       drawFruit(sf::RenderWindow &w);
};