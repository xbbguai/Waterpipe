#include "Base.h"
#include "ScreenBuffer.h"
#include "defines.h"
#include <stdio.h>

using namespace Waterpipe;
using namespace std;

class ScoreBoard : public Base
{
protected:
    int counter {0};
    char buffer[80];
    int hits {0};
    int lost {0};

BEGIN_WHEN_DEF
    always()
    {
        counter++;
    }

    when(counter % 10)
    {
        sprintf(buffer, "HIT: %04d  LOST: %04d      LOOP TIME USED: %dms OF %dms  ", hits, lost, GetLoopTimeUsed(), GetPollingInterval());

        screenBuffer->Print(0, 0, buffer, 7);
    }
END_WHEN_DEF

BEGIN_MESSAGE_DEF
    onmsg(GAME_START, msg)
    {
        hits = 0;
        lost = 0;
    }

    onmsg(HIT, msg)
    {
        hits++;
    }

    onmsg(LOST, msg)
    {
        lost++;
    }
END_MESSAGE_DEF
};