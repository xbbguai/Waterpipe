#pragma once

#include "Base.h"

namespace Waterpipe
{

class FallEdge : public Base
{
protected:
    volatile bool savedValue {true};
    volatile bool triggered {false};
    volatile bool* listenTo {nullptr};
    volatile bool messageSent {false};
    volatile bool autoReset {false};
    Message msg {};
public:
    FallEdge(bool* _listenTo = nullptr, bool _autoReset = false) : listenTo {_listenTo}, autoReset {_autoReset}, Base()
    {
        Reset();
    }

    void EnableAutoReset(bool _autoReset)
    {
        autoReset = _autoReset;
    }

    virtual void Reset(bool init = true)
    {
        if (listenTo != nullptr)
            savedValue = *listenTo;
        else
            savedValue = init;
        triggered = false;
        ResetMessageSent();
    }

    virtual void ListenAt(volatile bool at)
    {
        if (savedValue == true && at == false)
            triggered = true;
        if (autoReset && savedValue == false && at == true)
            triggered = false;
        savedValue = at;
    }

    void ListenTo(bool* _listenTo)
    {
        listenTo = _listenTo;
    }

    void EnableMessage(const int idMsg, 
                       const int param,
                       std::shared_ptr<Base> receiver = nullptr,
                       std::string receiverRTTIName = nullptr,
                       std::function<void(const Message msg)> receipt = nullptr)
    {
        msg.idMsg = idMsg;
        msg.param = param;
        msg.sender = shared_from_this();
        msg.receiver = receiver;
        msg.receiverRTTIName = receiverRTTIName;
        msg.receipt = nullptr;
    }

    void ResetMessageSent()
    {
        messageSent = false;
    }

    const bool Read() const
    {
        return triggered;
    }

BEGIN_WHEN_DEF
    when(listenTo != nullptr)
    {
        ListenAt(*listenTo);
    }

    when(msg.sender != nullptr && triggered && !messageSent)
    {
        EmitMessage(msg);
        messageSent = true;
    }

    when(!triggered && messageSent && autoReset)
    {
        messageSent = false;
    }
END_WHEN_DEF
};

class RiseEdge : public FallEdge
{
public:
    RiseEdge(bool* _listenTo = nullptr) : FallEdge(_listenTo)
    {
        Reset();
    }

    virtual void Reset(bool init = false)
    {
        if (listenTo != nullptr)
            savedValue = *listenTo;
        else
            savedValue = init;
        triggered = false;
        ResetMessageSent();
    }

    virtual void ListenAt(volatile bool at)
    {
        if (savedValue == false && at == true)
            triggered = true;
        if (autoReset && savedValue == true && at == false)
            triggered = false;
        savedValue = at;
    }
};

}