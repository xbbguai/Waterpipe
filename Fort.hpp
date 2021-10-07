#pragma once

#include "Base.h"
#include "ScreenBuffer.h"
#include <iostream>
#include "defines.h"
#include "UFO.hpp"

using namespace Waterpipe;
using namespace std;

class Bullet : public Base
{
protected:
    short x {0};
    short y {0};
    int counter {0};
public:
    Bullet(short xOrg) : Base()
    {
        x = xOrg;
        y = screenHeight - 3;
    }
    ~Bullet()
    {
    }
 
BEGIN_WHEN_DEF
    always()
    {
        counter++;
    }

    when(counter == 3 && y > 0)
    {
        screenBuffer->Print(x, y, ' ');
        y--;
        screenBuffer->Print(x, y, 'o', 2);
        counter = 0;
        //Broadcast my position. If I hit any enemy, I should vanish myself and get destroied.
        EmitMessage(typeid(UFO).name(), //Broadcast to UFOs. Every object in the Base will get this message.
                    BULLET_POSITION,    //message = BULLET_POSITION 
                    x << 16 | y,        //Position. 
                    nullptr,            //No data attached.
                    [&](Message msg) -> void { //Receipt function/lambda expression to be called when this bullet hits a UFO.
                        screenBuffer->Print(x, y, ' '); 
                        y = -1; 
                        RemoveFromWaterpipe();
                    });
    }

    when(y == 0)
    {
        screenBuffer->Print(x, y, ' ');
        y = -1; //Avoid re-entry of when(y == 0)
        //Remove myself
        RemoveFromWaterpipe();
    }
END_WHEN_DEF

};

class Fort : public Base
{
protected:
    int counter {0};
    int x {screenWidth / 2 - 2};
    int cx {0};
    bool shot {false};
    bool gameOver {false};
public:
    Fort() : Base() {}

BEGIN_WHEN_DEF
    always()
    {
        counter++;
    }

    when(counter == 3 && !shot)
    {
        counter = 0;
        screenBuffer->Print(x, screenHeight - 1, "   ");
        screenBuffer->Print(x + 1, screenHeight - 2, " ");
        if (!gameOver)
        {
            x += cx;
            if (x < 0)
                x = 0;
            if (x > screenWidth - 4)
                x = screenWidth - 4;
            screenBuffer->Print(x, screenHeight - 1, "/M\\", 3);
            screenBuffer->Print(x + 1, screenHeight - 2, "A", 3);
        }
    }

    when(shot && !gameOver)
    {
        //Animate an explosion
        int step = counter / 5;
        switch (step)
        {
        case 0:
            screenBuffer->Print(x + 1, screenHeight - 2, "_", 5);
            screenBuffer->Print(x, screenHeight - 1, "/*\\", 5);
            break;
        case 1:
            screenBuffer->Print(x + 1, screenHeight - 2, "*", 5);
            screenBuffer->Print(x, screenHeight - 1, "***", 5);
            break;
        case 2:
            screenBuffer->Print(x, screenHeight - 2, "***", 5);
            screenBuffer->Print(x, screenHeight - 1, " * ", 5);
            break;
        case 3:
            screenBuffer->Print(x, screenHeight - 2, "* *", 5);
            screenBuffer->Print(x, screenHeight - 1, "   ", 5);
            break;
        case 4:
            screenBuffer->Print(x, screenHeight - 2, "   ", 5);
            screenBuffer->Print(x, screenHeight - 1, "   ", 3);
            gameOver = true;
            break;
        }
    }

    when(gameOver)
    {
        screenBuffer->Print(screenWidth / 2 - 4, screenHeight / 2, "GAME OVER", 7);
    }
END_WHEN_DEF

BEGIN_MESSAGE_DEF
    onmsg(KEYBOARD, msg)
    {
        if (!gameOver && !shot)
        {
            if (msg.param == 'a')
                cx = -1;
            else if (msg.param == 'd')
                cx = 1;
            else if (msg.param == 's')
                cx = 0;
            else if (msg.param == ' ')
            {
                //Fire. Create a new bullet.
                Base::CreateToWaterpipe<Bullet>(x + 1);
            }
        }
        else if (gameOver && msg.param == ' ')
        {
            gameOver = false;
            counter = 0;
            x = screenWidth / 2 - 2;
            cx = 0;
            shot = false;
            screenBuffer->Print(screenWidth / 2 - 4, screenHeight / 2, "         ", 7);

            EmitMessage(null_for_broadcast, GAME_START);
        }
    }

    onmsg(BOMB_POSITION, msg)
    {
        int xBomb = msg.param >> 16;
        int yBomb = msg.param & 0xffff;
        if (xBomb >= x && xBomb <= x + 2 && yBomb >= screenHeight - 1)
        {
            //I'm hit. Game over.
            shot = true;
            EmitMessage(null_for_broadcast, GAME_OVER);
        } 
    }
END_MESSAGE_DEF
};