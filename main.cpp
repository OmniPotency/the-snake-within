#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <tuple>
#include "Snake.h"
using namespace std;

const int COLS=20, ROWS=20, CELL=30, BAR_H=40;
const int W=COLS*CELL, H=ROWS*CELL+BAR_H;

sf::Text makeText(const sf::Font &f, const string &s, int sz, sf::Color c){
    sf::Text t(f,s,sz); t.setFillColor(c); return t;
}
void center(sf::Text &t, float x, float y){
    auto b=t.getLocalBounds();
    t.setOrigin({b.position.x+b.size.x/2.f, b.position.y+b.size.y/2.f});
    t.setPosition({x,y});
}

// opts: {key, label, box_color}
void drawSelScreen(sf::RenderWindow &w, const sf::Font &f,
                   const string &title,
                   const vector<tuple<string,string,sf::Color>> &opts,
                   sf::Color headerColor)
{
    w.clear(sf::Color(28,28,35));

    sf::RectangleShape bar({(float)W,70.f});
    bar.setFillColor(headerColor);
    bar.setPosition({0,0});
    w.draw(bar);

    auto ttxt = makeText(f, title, 32, sf::Color::White);
    ttxt.setStyle(sf::Text::Bold);
    center(ttxt, W/2.f, 35.f);
    w.draw(ttxt);

    float y = 120.f;
    for (auto &[key, label, boxColor] : opts) {
        sf::RectangleShape badge({46.f, 42.f});
        badge.setFillColor(boxColor);
        badge.setOutlineColor(sf::Color(200,200,200,120));
        badge.setOutlineThickness(1.f);
        badge.setPosition({W/2.f-200.f, y-21.f});
        w.draw(badge);

        auto ktxt = makeText(f, key, 22, sf::Color::White);
        ktxt.setStyle(sf::Text::Bold);
        center(ktxt, W/2.f-177.f, y);
        w.draw(ktxt);

        auto ltxt = makeText(f, label, 22, sf::Color(215,215,215));
        auto lb = ltxt.getLocalBounds();
        ltxt.setOrigin({lb.position.x, lb.position.y+lb.size.y/2.f});
        ltxt.setPosition({W/2.f-138.f, y});
        w.draw(ltxt);

        y += 62.f;
    }
    w.display();
}

void drawReadyScreen(sf::RenderWindow &w, const sf::Font &f,
                     bool wallWrap, SnakeTheme st, FruitTheme ft, bool blink)
{
    w.clear(sf::Color(28,28,35));

    auto title = makeText(f,"READY?", 70, sf::Color(80,200,80));
    title.setStyle(sf::Text::Bold);
    center(title, W/2.f, 90.f);
    w.draw(title);

    // summary box
    sf::RectangleShape box({400.f,180.f});
    box.setFillColor(sf::Color(40,40,55));
    box.setOutlineColor(sf::Color(80,80,100));
    box.setOutlineThickness(1.f);
    box.setPosition({W/2.f-200.f, 170.f});
    w.draw(box);

    string wallStr  = wallWrap ? "Wrap Around  (Nokia)" : "Die on Wall  (Classic)";
    string snakeStr = st==SnakeTheme::GREEN?"Classic Green":
                      st==SnakeTheme::COLORFUL?"Colorful Rainbow":
                      st==SnakeTheme::BLUE?"Ocean Blue":"Sunset Orange";
    string fruitStr = ft==FruitTheme::APPLE?"Apple":
                      ft==FruitTheme::ORANGE_FRUIT?"Orange":
                      ft==FruitTheme::BLUEBERRY?"Blueberry":"Golden";

    float sy = 195.f;
    for (auto &[label, val] : vector<pair<string,string>>{
            {"Wall Mode",  wallStr},
            {"Snake",      snakeStr},
            {"Fruit",      fruitStr}})
    {
        auto lbl = makeText(f, label+":", 20, sf::Color(150,150,170));
        lbl.setPosition({W/2.f-185.f, sy});
        w.draw(lbl);
        auto val2 = makeText(f, val, 20, sf::Color::White);
        val2.setPosition({W/2.f-60.f, sy});
        w.draw(val2);
        sy += 52.f;
    }

    if (blink) {
        auto prompt = makeText(f,"Press any key to start!", 26, sf::Color(200,200,200));
        center(prompt, W/2.f, 400.f);
        w.draw(prompt);
    }

    w.display();
}

int main(){
    srand((unsigned)time(nullptr));

    sf::RenderWindow window(sf::VideoMode({(unsigned)W,(unsigned)H}),"The Snake Within");
    window.setFramerateLimit(60);

    sf::Font font;
    if(!font.openFromFile("arial.ttf")){cerr<<"Font missing\n";return 1;}

    enum State{MENU,SEL_WALL,SEL_SNAKE,SEL_FRUIT,READY,PLAYING,PAUSED,GAMEOVER};
    State state=MENU;

    Snake snake;
    int score=0, highScore=0, dir=2, nextDir=2;
    bool wallWrap=false;
    SnakeTheme  snakeTh=SnakeTheme::GREEN;
    FruitTheme  fruitTh=FruitTheme::APPLE;
    sf::Clock moveClock, blinkClock;
    bool blinkOn=true;

    // ── Static UI ─────────────────────────────────────────────────────────────
    auto titleTxt = makeText(font,"The Snake Within",52,sf::Color(80,200,80));
    titleTxt.setStyle(sf::Text::Bold);
    center(titleTxt, W/2.f, H/2.f-90.f);

    auto subTxt = makeText(font,"A Classic Reimagined",20,sf::Color(140,140,140));
    center(subTxt, W/2.f, H/2.f-30.f);

    auto enterTxt = makeText(font,"Press  ENTER  to Begin",26,sf::Color(200,200,200));
    center(enterTxt, W/2.f, H/2.f+40.f);

    auto scoreTxt  = makeText(font,"Score: 0",20,sf::Color::White);
    scoreTxt.setPosition({10.f,10.f});
    sf::Text bestTxt  = makeText(font,"Best: 0",20,sf::Color(255,215,0));
    sf::Text pauseHint= makeText(font,"[P] Pause",16,sf::Color(180,180,180));

    auto goTitle = makeText(font,"GAME  OVER",58,sf::Color(210,40,40));
    goTitle.setStyle(sf::Text::Bold);
    center(goTitle, W/2.f, H/2.f-80.f);

    auto goReplay= makeText(font,"Press ENTER to Play Again",24,sf::Color(200,200,200));
    center(goReplay, W/2.f, H/2.f+60.f);

    auto goMenu  = makeText(font,"Press M for Main Menu",20,sf::Color(150,150,180));
    center(goMenu, W/2.f, H/2.f+100.f);

    auto pauseTxt= makeText(font,"PAUSED",60,sf::Color::White);
    pauseTxt.setStyle(sf::Text::Bold);
    center(pauseTxt, W/2.f, H/2.f-20.f);

    auto pauseSub= makeText(font,"Press P to Resume",24,sf::Color(180,180,180));
    center(pauseSub, W/2.f, H/2.f+50.f);

    sf::RectangleShape overlay({(float)W,(float)H});
    overlay.setFillColor(sf::Color(0,0,0,160));

    sf::RectangleShape topBar({(float)W,(float)BAR_H});
    topBar.setFillColor(sf::Color(30,30,40));
    topBar.setPosition({0,0});

    sf::RectangleShape barLine({(float)W,2.f});
    barLine.setFillColor(sf::Color(60,60,80));
    barLine.setPosition({0,(float)(BAR_H-2)});

    // ── Main loop ─────────────────────────────────────────────────────────────
    while(window.isOpen())
    {
        if(blinkClock.getElapsedTime().asSeconds()>0.5f){
            blinkOn=!blinkOn; blinkClock.restart();
        }

        while(auto ev=window.pollEvent()){
            if(ev->is<sf::Event::Closed>()){window.close();return 0;}

            if(const auto* k=ev->getIf<sf::Event::KeyPressed>()){
                if(state==MENU){
                    if(k->code==sf::Keyboard::Key::Enter) state=SEL_WALL;
                }
                else if(state==SEL_WALL){
                    if(k->code==sf::Keyboard::Key::W){wallWrap=true; state=SEL_SNAKE;}
                    if(k->code==sf::Keyboard::Key::D){wallWrap=false;state=SEL_SNAKE;}
                }
                else if(state==SEL_SNAKE){
                    if(k->code==sf::Keyboard::Key::Num1){snakeTh=SnakeTheme::GREEN;    state=SEL_FRUIT;}
                    if(k->code==sf::Keyboard::Key::Num2){snakeTh=SnakeTheme::COLORFUL; state=SEL_FRUIT;}
                    if(k->code==sf::Keyboard::Key::Num3){snakeTh=SnakeTheme::BLUE;     state=SEL_FRUIT;}
                    if(k->code==sf::Keyboard::Key::Num4){snakeTh=SnakeTheme::ORANGE;   state=SEL_FRUIT;}
                }
                else if(state==SEL_FRUIT){
                    if(k->code==sf::Keyboard::Key::Num1){fruitTh=FruitTheme::APPLE;        state=READY;}
                    if(k->code==sf::Keyboard::Key::Num2){fruitTh=FruitTheme::ORANGE_FRUIT; state=READY;}
                    if(k->code==sf::Keyboard::Key::Num3){fruitTh=FruitTheme::BLUEBERRY;    state=READY;}
                    if(k->code==sf::Keyboard::Key::Num4){fruitTh=FruitTheme::GOLDEN;       state=READY;}
                }
                else if(state==READY){
                    // ANY key starts the game
                    snake.reset(window,COLS,ROWS,wallWrap,snakeTh,fruitTh,BAR_H);
                    score=0; dir=2; nextDir=2;
                    scoreTxt.setString("Score: 0");
                    state=PLAYING;
                    moveClock.restart();
                }
                else if(state==PLAYING){
                    if(k->code==sf::Keyboard::Key::P) state=PAUSED;
                    if(k->code==sf::Keyboard::Key::Left  && dir!=2) nextDir=1;
                    else if(k->code==sf::Keyboard::Key::Right && dir!=1) nextDir=2;
                    else if(k->code==sf::Keyboard::Key::Up    && dir!=4) nextDir=3;
                    else if(k->code==sf::Keyboard::Key::Down  && dir!=3) nextDir=4;
                }
                else if(state==PAUSED){
                    if(k->code==sf::Keyboard::Key::P){state=PLAYING;moveClock.restart();}
                }
                else if(state==GAMEOVER){
                    if(k->code==sf::Keyboard::Key::Enter) state=SEL_WALL;
                    if(k->code==sf::Keyboard::Key::M)     state=MENU;
                }
            }
        }

        // ── Update ────────────────────────────────────────────────────────────
        if(state==PLAYING){
            float step=max(0.07f, 0.20f - score*0.003f);
            if(moveClock.getElapsedTime().asSeconds()>=step){
                moveClock.restart();
                dir=nextDir;
                snake.move(dir);
                if(snake.is_dead()){
                    if(score>highScore) highScore=score;
                    state=GAMEOVER;
                } else if(snake.is_on_food()){
                    snake.add_food(window);
                    score++;
                    scoreTxt.setString("Score: "+to_string(score));
                }
            }
        }

        // ── Draw ──────────────────────────────────────────────────────────────
        window.clear(sf::Color(20,20,28));

        if(state==MENU){
            window.draw(titleTxt);
            window.draw(subTxt);
            if(blinkOn) window.draw(enterTxt);
        }
        else if(state==SEL_WALL){
            drawSelScreen(window,font,"WALL MODE",{
                {"W","Wrap Around  (Nokia Style)", sf::Color(40,80,180)},
                {"D","Die on Wall  (Classic)",     sf::Color(160,30,30)}
            }, sf::Color(50,70,140));
            continue;
        }
        else if(state==SEL_SNAKE){
            drawSelScreen(window,font,"CHOOSE SNAKE STYLE",{
                {"1","Classic Green",    sf::Color(20,110,20)},
                {"2","Colorful Rainbow", sf::Color(160,30,160)},
                {"3","Ocean Blue",       sf::Color(20,60,170)},
                {"4","Sunset Orange",    sf::Color(180,80,10)}
            }, sf::Color(40,90,40));
            continue;
        }
        else if(state==SEL_FRUIT){
            drawSelScreen(window,font,"CHOOSE FRUIT TYPE",{
                {"1","Apple  (Red)",   sf::Color(160,20,20)},
                {"2","Orange",         sf::Color(190,100,10)},
                {"3","Blueberry",      sf::Color(30,30,170)},
                {"4","Golden",         sf::Color(160,130,0)}
            }, sf::Color(140,90,20));
            continue;
        }
        else if(state==READY){
            drawReadyScreen(window,font,wallWrap,snakeTh,fruitTh,blinkOn);
            continue;
        }
        else if(state==PLAYING||state==PAUSED){
            window.draw(topBar);
            window.draw(barLine);

            scoreTxt.setString("Score: "+to_string(score));
            scoreTxt.setPosition({10.f,10.f});
            window.draw(scoreTxt);

            bestTxt.setString("Best: "+to_string(highScore));
            {auto b=bestTxt.getLocalBounds();
             bestTxt.setOrigin({b.position.x+b.size.x,b.position.y});
             bestTxt.setPosition({W-10.f,10.f});}
            window.draw(bestTxt);

            center(pauseHint, W/2.f, 20.f);
            window.draw(pauseHint);

            snake.draw(window);

            if(state==PAUSED){
                window.draw(overlay);
                window.draw(pauseTxt);
                window.draw(pauseSub);
            }
        }
        else if(state==GAMEOVER){
            window.draw(topBar);
            window.draw(barLine);
            scoreTxt.setPosition({10.f,10.f});
            window.draw(scoreTxt);
            bestTxt.setString("Best: "+to_string(highScore));
            {auto b=bestTxt.getLocalBounds();
             bestTxt.setOrigin({b.position.x+b.size.x,b.position.y});
             bestTxt.setPosition({W-10.f,10.f});}
            window.draw(bestTxt);
            snake.draw(window);
            window.draw(overlay);
            window.draw(goTitle);

            auto fsTxt=makeText(font,"Score: "+to_string(score),28,sf::Color::White);
            center(fsTxt, W/2.f, H/2.f-10.f);
            window.draw(fsTxt);

            auto hsTxt=makeText(font,
                score>=highScore?"NEW HIGH SCORE!":"Best: "+to_string(highScore),
                24, score>=highScore?sf::Color(255,215,0):sf::Color(180,180,180));
            center(hsTxt, W/2.f, H/2.f+25.f);
            window.draw(hsTxt);

            if(blinkOn) window.draw(goReplay);
            window.draw(goMenu);
        }

        window.display();
    }
    return 0;
}