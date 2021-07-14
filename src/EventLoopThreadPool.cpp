#include "EventLoopThreadPool.h"

#include"EventLoopThread.h"
EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop, int thread_num)
    :base_loop_(base_loop),
    thread_num_(thread_num),
    thread_pool_(),
    next_(0)
{
    for (int i = 0; i < thread_num_; ++i) {
        thread_pool_.push_back(std::make_unique<EventLoopThread>());
    }
}


EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::Start()
{
    for (int i = 0; i < thread_num_; ++i) {
        thread_pool_[i]->StartLoop();
    }
}

EventLoop* EventLoopThreadPool::GetNextLoop()
{
    EventLoop* loop = base_loop_;
    if (!thread_pool_.empty()) {
        loop = thread_pool_[next_]->GetLoop();
        //轮询
        if (static_cast<size_t>(++next_) == thread_pool_.size()) {
            next_ = 0;
        }
    }
    return loop;
}
