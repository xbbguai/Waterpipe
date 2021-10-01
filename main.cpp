#include <iostream>
#include "Base.h"
#include "Fort.hpp"
#include "UFO.hpp"
#include "defines.h"
#include "ScoreBoard.hpp"
#include <stdlib.h>

#define Linux 0
#define macOS 1

using namespace Waterpipe;
using namespace std;
shared_ptr<ScreenBuffer> screenBuffer;

int main(int, char**) 
{
    screenBuffer = Base::CreateToWaterpipe<ScreenBuffer>();
    Base::CreateToWaterpipe<Attacker>();
    Base::CreateToWaterpipe<Fort>();
    Base::CreateToWaterpipe<ScoreBoard>();
    
    Base::StartLoop(); 
#if macOS
    system("stty -f /dev/tty raw -echo");
#elif Linux
    system("stty -F /dev/tty raw -echo");
#endif
    while (true)
    {
        char ch = getchar();
        Base::EmitMessage(nullptr, nullptr, KEYBOARD, ch);

        if (ch == 'q' || ch == 'Q')
            break;
    }
    Base::Exit();
#if macOS
    system("stty -f /dev/tty -raw echo");
#elif Linux
    system("stty -F /dev/tty -raw echo");
#endif
    return 0;
}
