#pragma once
#ifndef _EVENT_LOOP_THREAD_H_
#define _EVENT_LOOP_THREAD_H_

#include<thread>
#include<mutex>
#include<condition_variable>

class EventLoop;

class EventLoopThread {
public:
	EventLoopThread();
	~EventLoopThread();

	EventLoop* StartLoop();
	EventLoop* GetLoop() { return loop_; }
	void ThreadFun();

private:

	std::thread th_;
	std::thread::id tid_;
	std::string thread_name_;
	EventLoop* loop_;

	std::mutex mutex_;
	std::condition_variable cond_;
};


#endif // !_EVENT_LOOP_THREAD_H_



