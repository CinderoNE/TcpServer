#pragma once
#ifndef _EVENT_LOOP_THREAD_POOL_H_
#define _EVENT_LOOP_THREAD_POOL_H_

#include<vector>
#include<memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
{
public:
	EventLoopThreadPool(EventLoop* base_loop,int thread_num);
	~EventLoopThreadPool();

	void Start();
	EventLoop* GetNextLoop();
	
private:
	//主loop,接受连接的loop
	EventLoop* base_loop_;
	int thread_num_;
	std::vector<std::unique_ptr<EventLoopThread>> thread_pool_;
	int next_;
	

};

#endif // !_EVENT_LOOP_THREAD_POOL_H_

