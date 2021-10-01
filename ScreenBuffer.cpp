#include "ScreenBuffer.h"
#include <iostream>

using namespace Waterpipe;
using namespace std;


void ScreenBuffer::Clear()
{
    for (int x = 0; x < screenWidth; x++)
        for (int y = 0; y < screenHeight; y++)
        {
            screen[x][y] = ' ';
            color[x][y] = 7;
        }
    //Black background
    cout << "\033[40m";
    BlitToScreen();
}

void ScreenBuffer::BlitToScreen()
{
    char currentColor = -1;
    for (int y = 0; y < screenHeight; y++)
    {
        cout << "\033[" << y + 1 << ";" << 1 << "H";
        for (int x = 0; x < screenWidth; x++)
        {
            if (currentColor != color[x][y])
            {
                cout << "\033[" << color[x][y] + 30 << "m";
                currentColor = color[x][y];
            }
            cout << screen[x][y];
        }
    }
}

void ScreenBuffer::Print(int x, int y, const char *p, char clr)
{
    while (*p && x < screenWidth)
    {
        screen[x][y] = *p;
        color[x][y] = clr;
        p++;
        x++;
    }
}
