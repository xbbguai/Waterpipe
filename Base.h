#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <chrono>
#include <typeinfo>
#include <future>

namespace Waterpipe
{

#define BEGIN_WHEN_DEF private:virtual void __W_h_e_n_s__() {
#define when(condition) if(condition)
#define always()
#define wire(from, to) {(to) = (from);}
#define END_WHEN_DEF }

#define BEGIN_MESSAGE_DEF private:virtual void __M_e_s_s_a_g_e_s__(Waterpipe::Message __m_s_g__) { {
#define onmsg(idmsg, msg) }{Message &msg = __m_s_g__; if (__m_s_g__.idMsg == idmsg) 
#define begin_async std::future<void> r = std::async(std::launch::async, [&]() -> void {
#define end_async });
#define END_MESSAGE_DEF }}

#define null_for_broadcast static_cast<shared_ptr<Base>>(nullptr)
class Base;

//Structure of the message.
struct Message
{
    std::shared_ptr<Base> receiver {nullptr};       //Receiver object. Be nullptr if broadcasts.
    std::string receiverRTTIName {};                //Receiver class type_info name. If set, the objects of this class will receive the message.
    std::shared_ptr<Base> sender {nullptr};         //Sender object. Be nullptr if not used.
    int idMsg {0};                                  //Message ID.
    int param {0};                                  //Parameter of the message.
    void* data {nullptr};                           //Data attached to this message. nullptr if none.
    std::function<void(const Message msg)> receipt {nullptr};   //If the sender of the message requires any feedback from
                                                                //who processed the message, the sender should pass a
                                                                //std::function to the receiver by using this parameter.
                                                                //The receiver can invoke this std::function after message
                                                                //being processed.
};

class Base : public std::enable_shared_from_this<Base>
{
private:
    virtual void __W_h_e_n_s__() {}
    virtual void __M_e_s_s_a_g_e_s__(Message) {}

    volatile static bool run;
    volatile static bool enableLoop;

    struct WaterPipeObject
    {
        std::shared_ptr<Base> obj;
        std::string classRTTIName;
    };
    //---------------------------------------------------------
    //Members required to build the polling loop.
    //The container of all Base-derived objects.
    static std::mutex mtxLoopObj;
    static std::vector<WaterPipeObject> waterPipeObjects;
    //This is the interval of each polling loop. Similar to a PLC loop.
    static int loopInterval;
    static int loopTimeUsed;
    static int peak;
    //The loop thread.
    static void LoopAll();
    static std::thread loopThread;
    //---------------------------------------------------------

    //---------------------------------------------------------
    //Members required to build the message queue.
    //The message queue.
    static std::mutex mtxQueue;
    static std::queue<Message> messageQueue;
    //The message pump thread.
    static void MessagePump();
    static std::thread pumpThread;
    static std::mutex mtxMsg;
    static std::condition_variable cvMsg;
    //---------------------------------------------------------
public:
    static void EmitMessage(const Message msg);
    static void EmitMessage(std::shared_ptr<Base> receiver, 
                            std::shared_ptr<Base> sender, 
                            const int idMsg, 
                            const int param = 0, 
                            void* data = nullptr,
                            std::function<void(const Message msg)> receipt = nullptr)
    {
        Message msg {receiver, "", sender, idMsg, param, data, receipt};
        EmitMessage(msg);
    }

    void EmitMessage(std::shared_ptr<Base> receiver,
                    const int idMsg,
                    const int param = 0,
                    void* data = nullptr, 
                    std::function<void(const Message msg)> receipt = nullptr)
    {
        Message msg {receiver, "", shared_from_this(), idMsg, param, data, receipt};
        EmitMessage(msg);
    }

    void EmitMessage(std::string RTTIName,
                    const int idMsg,
                    const int param = 0,
                    void* data = nullptr, 
                    std::function<void(const Message msg)> receipt = nullptr)
    {
        Message msg {nullptr, RTTIName, shared_from_this(), idMsg, param, data, receipt};
        EmitMessage(msg);       
    }

    //Polling loop will not start until StartLoop is called.
    //So that, all objects may be prepared before loop starts.
    static void StartLoop()
    {
        enableLoop = true;
    }

    //Stop the pooling loop.
    //Polling loop can be restarted by calling StartLoop
    static void StopLoop()
    {
        enableLoop = false;
        std::lock_guard locker(mtxLoopObj);    //Wait until the loop is stopped.
    }

    //Exit water pipe.
    //The Exit must never be missed when terminating the program.
    //This function releases all shared_ptr that are stored in containers.
    //If not called, destructors may not be called.
    static void Exit()
    {
        run = false;
        loopThread.join();
        pumpThread.join();
        //Empty the container
        waterPipeObjects.clear();
        //So weaired that there is no method to empty a queue.
        while (!messageQueue.empty())
            messageQueue.pop();
    }

    static void SetPollingInterval(int interval)
    {
        loopInterval = interval;
    }

    static int GetPollingInterval()
    {
        return loopInterval;
    }

    static int GetLoopTimeUsed()
    {
        return loopTimeUsed;
    }

    static int GetLoopTimePeak()
    {
        return peak;
    }

    static void ResetLoopTimePeak()
    {
        peak = 0;
    }

    static float GetLoopTimePercentUsage()
    {
        return (float)loopTimeUsed / loopInterval * 100;
    }

    static int GetObjectCount()
    {
        return waterPipeObjects.size();
    }
public:
    Base();
    ~Base();

    //Use this to create an object of a Base-derived class and insert it into waterpipe.
    template<typename T, typename... Args>
    static std::shared_ptr<T> CreateToWaterpipe(Args... args)
    {
        std::shared_ptr<T> obj = std::make_shared<T>(args...);
        InsertIntoWaterpipe(obj);
        return obj;
    }

    //If not using Base::CreateToWaterpipe, you can use InsertIntoWaterpipe
    //to insert a precreated object into waterpipe.
    template<typename T>
    static void InsertIntoWaterpipe(std::shared_ptr<T> obj)
    {
        std::lock_guard lockerLoop(mtxLoopObj);
        WaterPipeObject wpObj {obj, typeid(T).name()};
        Base::waterPipeObjects.push_back(wpObj);
    }

    //Remove an object from waterpipe.
    template<typename T>
    static void RemoveFromWaterpipe(std::shared_ptr<T> obj)
    {
        std::lock_guard lockerLoop(mtxLoopObj);
        int i = -1;
        for (auto it = waterPipeObjects.begin(); it != waterPipeObjects.end(); it++)
        {
            i++;
            if ((*it).obj == obj)
            {
                waterPipeObjects.erase(it);
                break;
            }
        }
    }

    //Remove myself from waterpipe.
    void RemoveFromWaterpipe()
    {
        RemoveFromWaterpipe(shared_from_this());
    }
};

}