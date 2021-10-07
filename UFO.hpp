#pragma once

#include "Base.h"
#include "ScreenBuffer.h"
#include "defines.h"
#include "ScoreBoard.hpp"
#include "Fort.hpp"
#include <vector>

using namespace Waterpipe;
using namespace std;

class Bomb : public Base
{
protected:
    const int speed = 5;
    int x;
    int y;
    int counter {speed};
public:
    Bomb(int _x, int _y)
    {
        x = _x;
        y = _y;
    }
BEGIN_WHEN_DEF
    always()
    {
        counter--;
    }

    when(counter == 0)
    {
        counter = speed;
        screenBuffer->Print(x, y, ' ');
        y++;
        screenBuffer->Print(x, y, '0', 7);
        EmitMessage(null_for_broadcast, 
                    BOMB_POSITION,
                    x << 16 | y,
                    nullptr);
    }

    when(y == screenHeight)
    {
        //Destroy myself
        screenBuffer->Print(x, y, ' ');
        RemoveFromWaterpipe();
    }
END_WHEN_DEF
};

class Attacker;
class UFO : public Base
{
protected:
    int x;
    int y;
    int direction;
    int speed;
    int counter {1};
    int animateCounter {20};
    bool shot { false };
    shared_ptr<Attacker> attacker;
public:
    UFO(shared_ptr<Attacker> _attacker) : Base()
    {
        direction = rand() > RAND_MAX / 2 ? 1 : -1;
        if (direction == 1)
            x = 1;
        else
            x = screenWidth - 2;
        y = static_cast<int>(static_cast<float>(rand()) / RAND_MAX * 6.f + 2);
        speed = rand() > RAND_MAX / 2 ? 3 : 4;

        attacker = _attacker;
    }

    bool IsShot(int xBullet, int yBullet)
    {
        if (xBullet >= x - 1 && xBullet <= x + 1 && yBullet >= y - 1 && yBullet <= y + 1)
        {
            shot = true;
        }
        return shot;
    }

    void DrawExplosion()
    {
        switch (counter)
        {
        case -10:
            screenBuffer->Print(x - 1, y - 1, " * ", 5);
            screenBuffer->Print(x - 1, y,     "***", 5);
            screenBuffer->Print(x - 1, y + 1, " * ", 5);
            break;
        case -20:
            screenBuffer->Print(x - 1, y - 1, "* *", 1);
            screenBuffer->Print(x - 1, y,     "* *", 1);
            screenBuffer->Print(x - 1, y + 1, "* *", 1);
            break;
        case -30:
            screenBuffer->Print(x - 1, y - 1, "   ", 1);
            screenBuffer->Print(x - 1, y,     " * ", 1);
            screenBuffer->Print(x - 1, y + 1, "   ", 1);
            break;
        case -35:
            screenBuffer->Print(x - 1, y,     "   ");
            break;
        }
    }

    void Show()
    {
        if (x > 1 && x < 79)
        {
            screenBuffer->Print(x - 1, y - 1, " _ ", 2);
            screenBuffer->Print(x - 1, y,     "|_|", 6);
            if (animateCounter > 10)
                screenBuffer->Print(x - 1, y + 1, "/ \\", 3);
            else if (animateCounter > 0)
                screenBuffer->Print(x - 1, y + 1, "\\ /", 3);
        }
    }

    void Hide()
    {
        if (x < 1)
            x = 1;
        if (x > screenWidth - 2)
            x = screenWidth - 2;
        screenBuffer->Print(x - 1, y - 1, "   ");
        screenBuffer->Print(x - 1, y,     "   ");
        screenBuffer->Print(x - 1, y + 1, "   ");
    }
BEGIN_WHEN_DEF
    always()
    {
        counter--;
        animateCounter--;
    }

    when(animateCounter == 0)
    {
        animateCounter = 20;
    }

    when(shot)
    {
        DrawExplosion();
        if (counter == -40)
        {
            Hide();
            EmitMessage(dynamic_pointer_cast<Base>(attacker), DESTROY_OBJECT);
            RemoveFromWaterpipe();
            EmitMessage(typeid(ScoreBoard).name(), HIT);
        }
    }

    when(counter == 0 && !shot)
    {
        counter = speed;
        Hide();
        x += direction;
        if (x > screenWidth - 2 || x < 1)
        {
            EmitMessage(dynamic_pointer_cast<Base>(attacker), DESTROY_OBJECT);
            RemoveFromWaterpipe();
            EmitMessage(typeid(ScoreBoard).name(), LOST);
        }
        else
        {
            Show();
            if (rand() < RAND_MAX / 100)
                Base::CreateToWaterpipe<Bomb>(x, y + 1);
        }
    }

END_WHEN_DEF

BEGIN_MESSAGE_DEF
    onmsg(BULLET_POSITION, msg)
    {
        if (!shot && IsShot(msg.param >> 16, msg.param & 0xffff) && msg.receipt != nullptr)
            msg.receipt(msg);
    }
END_MESSAGE_DEF

};

class Attacker : public Base
{
protected:
    int ufoCount {0};
    bool gameOver {false};
public:
    Attacker() : Base()
    {}

BEGIN_WHEN_DEF
    when(ufoCount < 6 && !gameOver)
    {
        shared_ptr<UFO> newUFO = Base::CreateToWaterpipe<UFO>(dynamic_pointer_cast<Attacker>(shared_from_this()));
        ufoCount++;
    }

END_WHEN_DEF

BEGIN_MESSAGE_DEF
    onmsg(DESTROY_OBJECT, msg)
    {
        ufoCount--;
    }

    onmsg(GAME_START, msg)
    {
        gameOver = false;
    }

    onmsg(GAME_OVER, msg)
    {
        gameOver = true;
    }
END_MESSAGE_DEF
};