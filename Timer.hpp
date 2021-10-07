#pragma once

#include "Base.h"

namespace Waterpipe
{

class Timer : public Base
{
protected:
    volatile int counter {0};
    volatile int countTo {0};
    volatile bool started {false};
    volatile bool triggered {false};
    volatile bool messageSent {false};
    volatile bool autoRestart {false};
    Message msg {};   
public:
    Timer() : Base()
    {
    }

    Timer(int ms, bool autoRestart, bool start = true) : Base()
    {
        SetTimer(ms);
        EnableAutoRestart(autoRestart);
        if (start)
            StartTimer();
    }

    void SetTimer(int ms)
    {
        countTo = ms / Base::GetPollingInterval() + (ms % Base::GetPollingInterval() > 0 ? 1 : 0);
    }

    void StartTimer()
    {
        started = true;
    }

    void PauseTimer()
    {
        started = false;
    }

    void EnableAutoRestart(bool _autoRestart)
    {
        autoRestart = _autoRestart;
    }

    void ResetTimer()
    {
        counter = 0;
        started = false;
        triggered = false;
        ResetMessageSent();
    }

    int ReadTimer()
    {
        return counter / Base::GetPollingInterval();
    }

    bool Read() const
    {
        return triggered;
    }

    void EnableMessage(const int idMsg, const int param,
                       std::shared_ptr<Base> receiver = nullptr,
                       std::string receiverRTTIName = nullptr,
                       std::function<void(const Message msg)> receipt = nullptr)
    {
        msg.idMsg = idMsg;
        msg.param = param;
        msg.sender = shared_from_this();
        msg.receiver = receiver;
        msg.receiverRTTIName = receiverRTTIName;
        msg.receipt = receipt;
    }

    void ResetMessageSent()
    {
        messageSent = false;
    }
BEGIN_WHEN_DEF
    when(started)
    {
        counter++;
    }

    when(counter >= countTo)
    {
        triggered = true;
        if (msg.sender != nullptr && !messageSent)
        {
            EmitMessage(msg);
            messageSent = true;
        }
        if (autoRestart)
        {
            ResetMessageSent();
            counter = 0;
        }
    }
END_WHEN_DEF
};

}