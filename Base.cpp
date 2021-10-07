#include "Base.h"
#include <thread>

using namespace Waterpipe;

volatile bool Base::run = true;
volatile bool Base::enableLoop = false;

std::vector<Base::WaterPipeObject> Base::waterPipeObjects;
std::thread Base::loopThread(Base::LoopAll);
std::mutex Base::mtxLoopObj;
int Base::loopInterval = 10; //10 ms
int Base::loopTimeUsed = 0;

int Base::peak = 0;

std::queue<Message> Base::messageQueue;
std::mutex Base::mtxQueue;
std::thread Base::pumpThread(Base::MessagePump);
std::mutex Base::mtxMsg;
std::condition_variable Base::cvMsg;

Base::Base() : std::enable_shared_from_this<Base>()
{
}

Base::~Base()
{
}

//static
//Polling loop thread function.
void Base::LoopAll()
{
    while (run)
    {
        auto timeStart = std::chrono::high_resolution_clock::now();
        if (enableLoop)
        {
            //Copy a loopDrivenObjects before calling each __W_h_e_n__() functions.
            // 1) Will be no dead locks. 
            // 2) Objects will not be destroied because they are now in the copy. 
            //    It will be safe to call __W_h_e_n__ without worrying objects being removed from waterpipe and destroied.
            std::vector<Base::WaterPipeObject> objects;
            std::unique_lock locker(mtxLoopObj);
            objects = waterPipeObjects;
            locker.unlock();
            for (auto &obj : objects)
                obj.obj->__W_h_e_n_s__();
        }
        //Sleep until next loop
        loopTimeUsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timeStart).count();
        if (loopTimeUsed < loopInterval)
            std::this_thread::sleep_for(std::chrono::milliseconds(loopInterval - loopTimeUsed));
        peak = std::max(peak, loopTimeUsed);
    }
}

//static
//Message pump thread function.
void Base::MessagePump()
{
    while (run)
    {
        while (messageQueue.size())
        {
            std::unique_lock queueLock(mtxQueue);
            auto msg = messageQueue.front();
            messageQueue.pop();
            queueLock.unlock();

            if (msg.receiver == nullptr)
            {               
                //The same consideration as polling loop. See comments of LoopAll().
                std::vector<WaterPipeObject> objCopy;
                std::unique_lock loopLock(mtxLoopObj);
                objCopy = waterPipeObjects;
                loopLock.unlock();
                if (msg.receiverRTTIName.empty())
                {
                    //Broadcast to all objects.
                    for (auto &obj : objCopy)
                        obj.obj->__M_e_s_s_a_g_e_s__(msg);
                }
                else
                {
                    //Broadcast to objects with RTTI(Run-time type idenfitier, the run-time class name of an object) name matches.
                    for (int i = 0; i < objCopy.size(); i++)
                    {
                        if (objCopy[i].classRTTIName == msg.receiverRTTIName)
                            objCopy[i].obj->__M_e_s_s_a_g_e_s__(msg);
                    }
                }
            }
            else
                msg.receiver->__M_e_s_s_a_g_e_s__(msg);
        }

        //Make sure I get an empty queue before waiting for the condition variable.
        //The reason is, when message is posted in the same thread of MessagePump,
        //mtxMsg won't be locked and cvMsg.notify_one() seems to have no effect.
        std::unique_lock msgLock(mtxMsg);
        cvMsg.wait(msgLock, [] { return !messageQueue.empty(); });
        msgLock.unlock();
    }
}

void Base::EmitMessage(const Message msg)
{
    if (!run)
        return;

    std::unique_lock queueLock(mtxQueue);
    messageQueue.push(msg);
    queueLock.unlock();

    std::unique_lock msgLock(mtxMsg);
    cvMsg.notify_one();
    msgLock.unlock();
}