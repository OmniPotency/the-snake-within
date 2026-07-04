#include "Snake.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
using namespace std;

void Snake::reset(sf::RenderWindow &w, int c, int r,
                  bool wrap, SnakeTheme st, FruitTheme ft, int bh)
{
    cols = c; rows = r; barH = bh;
    wallWrap = wrap; snakeTheme = st; fruitTheme = ft;
    cellW = w.getSize().x / cols;
    cellH = (w.getSize().y - barH) / rows;
    body.clear();
    body.push_back({cols/2,   rows/2});
    body.push_back({cols/2-1, rows/2});
    body.push_back({cols/2-2, rows/2});
    add_food(w);
}

void Snake::add_food(sf::RenderWindow &w) {
    while (true) {
        food = {rand() % cols, rand() % rows};
        bool hit = false;
        for (auto &b : body) if (b == food) { hit = true; break; }
        if (!hit) break;
    }
}

void Snake::move(int dir) {
    sf::Vector2i head = body.front();
    if      (dir == 1) head.x--;
    else if (dir == 2) head.x++;
    else if (dir == 3) head.y--;
    else if (dir == 4) head.y++;

    if (wallWrap) {
        if (head.x < 0)     head.x = cols - 1;
        if (head.x >= cols) head.x = 0;
        if (head.y < 0)     head.y = rows - 1;
        if (head.y >= rows) head.y = 0;
    }

    body.insert(body.begin(), head);
    if (!is_on_food()) body.pop_back();
}

bool Snake::is_on_food() { return body.front() == food; }

bool Snake::is_dead() {
    sf::Vector2i h = body.front();
    if (!wallWrap)
        if (h.x < 0 || h.x >= cols || h.y < 0 || h.y >= rows) return true;
    for (int i = 1; i < (int)body.size(); i++)
        if (body[i] == h) return true;
    return false;
}

sf::Color Snake::getBodyColor(int idx, int total) {
    float t = (total <= 1) ? 0.f : (float)idx / (float)(total - 1);
    switch (snakeTheme) {
    case SnakeTheme::GREEN:
        // dark forest green head -> slightly lighter tail
        return sf::Color((uint8_t)(15 + t*30), (uint8_t)(90 + t*50), (uint8_t)(15 + t*20));
    case SnakeTheme::COLORFUL: {
        float hue = t * 300.f;
        float h6  = hue / 60.f;
        int   i6  = (int)h6 % 6;
        float f   = h6 - (int)h6, q = 1.f - f;
        float tb[6][3] = {{1,f,0},{q,1,0},{0,1,f},{0,q,1},{f,0,1},{1,0,q}};
        // darker: multiply by 150 instead of 180
        return sf::Color((uint8_t)(tb[i6][0]*150+20),
                         (uint8_t)(tb[i6][1]*150+20),
                         (uint8_t)(tb[i6][2]*150+20));
    }
    case SnakeTheme::BLUE:
        return sf::Color((uint8_t)(10+t*30),(uint8_t)(40+t*60),(uint8_t)(120+t*70));
    case SnakeTheme::ORANGE:
        return sf::Color((uint8_t)(150+t*40),(uint8_t)(55+t*40),(uint8_t)(5+t*10));
    default:
        return sf::Color::Green;
    }
}

void Snake::drawFruit(sf::RenderWindow &w) {
    sf::Color fc;
    switch (fruitTheme) {
    case FruitTheme::APPLE:        fc = sf::Color(170, 20,  20);  break; // dark red
    case FruitTheme::ORANGE_FRUIT: fc = sf::Color(200,110,  10);  break; // dark orange
    case FruitTheme::BLUEBERRY:    fc = sf::Color( 30, 30, 170);  break; // dark blue
    case FruitTheme::GOLDEN:       fc = sf::Color(180,145,   0);  break; // dark gold
    }
    float rad = (float)(cellW/2 - 3);
    sf::CircleShape cs(rad);
    cs.setFillColor(fc);
    cs.setPosition({(float)(food.x*cellW + 3), (float)(food.y*cellH + 3 + barH)});
    w.draw(cs);
    // shine dot
    sf::CircleShape shine(2.5f);
    shine.setFillColor(sf::Color(255,255,255,130));
    shine.setPosition({(float)(food.x*cellW + cellW/2 - 6),
                       (float)(food.y*cellH + barH + 4)});
    w.draw(shine);
}

void Snake::drawCell(sf::RenderWindow &w, int gx, int gy, sf::Color c, int s) {
    sf::RectangleShape r;
    r.setSize({(float)(cellW - s*2), (float)(cellH - s*2)});
    r.setPosition({(float)(gx*cellW + s), (float)(gy*cellH + s + barH)});
    r.setFillColor(c);
    w.draw(r);
}

void Snake::draw(sf::RenderWindow &w) {
    for (int x = 0; x < cols; x++)
        for (int y = 0; y < rows; y++) {
            sf::Color bg = (x+y)%2==0 ? sf::Color(170,215,81) : sf::Color(162,209,73);
            drawCell(w, x, y, bg, 0);
        }
    drawFruit(w);
    int sz = (int)body.size();
    for (int i = sz-1; i >= 0; i--) {
        drawCell(w, body[i].x, body[i].y, getBodyColor(i, sz), 2);
    }
    // eyes
    sf::CircleShape eye(2.5f);
    eye.setFillColor(sf::Color::White);
    eye.setPosition({(float)(body[0].x*cellW + cellW/2 - 7),
                     (float)(body[0].y*cellH + barH + cellH/2 - 5)});
    w.draw(eye);
    eye.setPosition({(float)(body[0].x*cellW + cellW/2 + 2),
                     (float)(body[0].y*cellH + barH + cellH/2 - 5)});
    w.draw(eye);
}