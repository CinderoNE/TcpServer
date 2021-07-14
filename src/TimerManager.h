#pragma once
#ifndef _TIMER_MANAGER_H_
#define _TIMER_MANAGER_H_


#include"Timer.h"
#include"Channel.h"

#include<memory>
#include<queue>
#include <functional>
#include<unordered_map>



class EventLoop;
class Channel;

struct TimerComp {
	bool operator()(const std::shared_ptr<Timer>& t1, const std::shared_ptr<Timer>& t2) const {
		return t2->expired_time() < t1->expired_time();
	}
};
class TimerManager {
public:
	using TimerCallback = std::function<void()>;
	using SPTimer = std::shared_ptr<Timer>;
public:
	TimerManager(EventLoop* loop);
	~TimerManager();
	SPTimer AddTimer(Timestamp when, const TimerCallback& cb);
	void AddTimerInLoop(SPTimer new_timer);
	void ResetTimer(const SPTimer& reset_timer);
	void TimerfdRead();
	void HandleExpiredEvent(Timestamp receive_time);
	void ResetExpiredTime();
	bool empty() { return timer_queue_.empty(); }
private:
	EventLoop* loop_;
	int timerfd_;
	Channel timerfd_channel_;
	std::priority_queue<SPTimer, std::deque<SPTimer>, TimerComp> timer_queue_;  //小顶堆
	
};

#endif // !_TIMER_MANAGER_H_

