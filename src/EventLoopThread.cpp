#include"EventLoopThread.h"
#include"EventLoop.h"
#include<iostream>
#include<sstream>

EventLoopThread::EventLoopThread():
	th_(),
	tid_(-1),
	thread_name_("Loop Thread : "),
	loop_(nullptr),
	mutex_(),
	cond_()
{
}

EventLoopThread::~EventLoopThread()
{
	loop_->Quit();
	th_.join();
	std::cout << "terminate loop thread : " << tid_ << std::endl;
}

EventLoop* EventLoopThread::StartLoop()
{
	th_ = std::thread(&EventLoopThread::ThreadFun,this);
	{
		//等待EventLoop构建完成
		std::unique_lock<std::mutex> lock(mutex_);
		while (loop_ == nullptr) {
			cond_.wait(lock);
		}
	}
	return loop_;
}

void EventLoopThread::ThreadFun()
{
	EventLoop loop;
	{
		std::lock_guard<std::mutex> guard(mutex_);
		loop_ = &loop;
	}
	cond_.notify_one();
	
	tid_ = std::this_thread::get_id();
	std::stringstream ss;
	ss << tid_;
	thread_name_.append(ss.str());

	std::cout << "start loop in thread : " << tid_ << std::endl;
	loop.Loop();
	

}
