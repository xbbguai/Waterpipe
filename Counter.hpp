#pragma once
#include "Base.h"

namespace Waterpipe
{

class Counter : public Base
{
protected:
    volatile int count {0};
    volatile bool riseEdge {false};
    volatile bool lastStatus {false};
    volatile bool* listenAt {nullptr};
    volatile int whenCountAt {0};
    volatile bool autoReset {false};
    volatile bool messageSent {false};
    Message msg {};
public:
    Counter(bool *_listenAt = nullptr) : listenAt {_listenAt}, Base()
    {
        Reset();
    }

    void ListenTo(volatile bool to)
    {
        if (riseEdge ? lastStatus == false && to == true : lastStatus == true && to == false)
            count++;
        lastStatus = to;
    }

    void ListenAt(bool *_listenAt)
    {
        listenAt = _listenAt;
    }

    void EnableMessage(const int _whenCountAt, 
                       const int idMsg, 
                       const int param,
                       const bool _autoReset = true, 
                       std::shared_ptr<Base> receiver = nullptr,
                       std::string receiverRTTIName = "",
                       std::function<void(const Message msg)> receipt = nullptr)
    {
        msg.idMsg = idMsg;
        msg.param = param;
        msg.sender = shared_from_this();
        msg.receiver = receiver;
        msg.receiverRTTIName = receiverRTTIName;
        msg.receipt = nullptr;
        whenCountAt = _whenCountAt;
        autoReset = _autoReset;
        ResetMessageSent();
    }

    void ResetMessageSent()
    {
        messageSent = false;
    }

    void CountOnRiseEdge()
    {
        riseEdge = true;
    }
    void CountOnFallEdge()
    {
        riseEdge = false;
    }

    void Reset(int value = 0)
    {
        ResetMessageSent();
        count = value;
        if (listenAt != nullptr)
            lastStatus = *listenAt;
        else
            lastStatus = !riseEdge;
    }

    const int ReadCounter() const
    {
        return count;
    }
BEGIN_WHEN_DEF
    when(listenAt != nullptr)
    {
        ListenTo(*listenAt);
    }

    when(msg.sender != nullptr && count == whenCountAt && !messageSent)
    {
        EmitMessage(msg);
        if (autoReset)
        {
            count = 0;
            messageSent = false;
        }
        else
            messageSent = true;
    }
END_WHEN_DEF
};

}