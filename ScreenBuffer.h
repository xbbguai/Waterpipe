#pragma once

#include "Base.h"
#include <iostream>

constexpr int screenWidth = 80;
constexpr int screenHeight = 24;

//colors:
// 0 = black
// 1 = red
// 2 = green
// 3 = yellow
// 4 = blue
// 5 = purple
// 6 = cayon
// 7 = white

class ScreenBuffer : public Waterpipe::Base
{
public:
    char screen[screenWidth][screenHeight];
    char color[screenWidth][screenHeight];
    void BlitToScreen();
    int counter {0};
    int starsCounter {0};
public:
    ScreenBuffer() 
    {
        //Disable cursor display
        std::cout << "\033[?25l";
        Clear();
    }
    ~ScreenBuffer()
    {
        //Enable cursor display
        std::cout << "\033[?25h";
    } 
    
    void Clear();
    void Print(int x, int y, char ch, char clr = 1)
    {
        screen[x][y] = ch;
        color[x][y] = clr;
    }
    void Print(int x, int y, const char *p, char clr = 1);
BEGIN_WHEN_DEF
    always()
    {
        counter++;
    }

    when(counter * GetPollingInterval() >= 30)
    {
        counter = 0;
        starsCounter++;
        if (starsCounter % 5 == 0)
        {
            for (int x = 0; x < screenWidth; x++)
                for (int y = 0; y < screenHeight; y++)
                {
                    if (screen[x][y] == ' ' || screen[x][y] == '.')
                    {
                        int rnd = rand();
                        if (rnd < RAND_MAX / 100)
                        {
                            screen[x][y] = '.';
                            color[x][y] = rnd % 8;
                        }
                        else
                        {
                            screen[x][y] = ' ';
                            color[x][y] = 7;
                        }
                    }
                }
        }
        BlitToScreen();
    }
END_WHEN_DEF
};

extern std::shared_ptr<ScreenBuffer> screenBuffer;